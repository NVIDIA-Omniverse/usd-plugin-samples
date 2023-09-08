// Copyright 2023 NVIDIA CORPORATION
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <pxr/imaging/hd/xformSchema.h>

#include "geospatialDataSource.h"
#include "computedPrimDataSource.h"
#include "computedDependentDataSource.h"
#include "localPositionSchema.h"

PXR_NAMESPACE_OPEN_SCOPE

TF_DEFINE_PUBLIC_TOKENS(HdOmniGeospatialDataSourceTokens,
    HDOMNIGEOSPATIALDATASOURCE_TOKENS);

HdOmniGeospatialDataSource::HdOmniGeospatialDataSource(const HdSceneIndexBase& index, const SdfPath& primPath,
    HdContainerDataSourceHandle wrappedDataSource) :
    _sceneIndex(index),
    _primPath(primPath),
    _wrappedDataSource(wrappedDataSource)
{
}

void HdOmniGeospatialDataSource::UpdateWrappedDataSource(
    HdContainerDataSourceHandle wrappedDataSource)
{
    _wrappedDataSource = wrappedDataSource;
}

#if PXR_VERSION < 2302
bool HdOmniGeospatialDataSource::Has(const TfToken& name)
{
    if (name == HdOmniGeospatialDataSourceTokens->geospatialPreservedXform)
    {
        return true;
    }

    return (_wrappedDataSource != nullptr) ? _wrappedDataSource->Has(name) : false;
}
#endif

TfTokenVector HdOmniGeospatialDataSource::GetNames()
{
    // since we only wrapped Xformables, this should
    // also return HdXformSchemaTokens->xform
    TfTokenVector result = (_wrappedDataSource == nullptr) ? TfTokenVector() : _wrappedDataSource->GetNames();
    result.push_back(HdOmniGeospatialDataSourceTokens->geospatialPreservedXform);

    return result;
}

HdDataSourceBaseHandle HdOmniGeospatialDataSource::Get(const TfToken& name)
{
    if (name == HdXformSchemaTokens->xform)
    {
        // this is an intercept of the flattened transform matrix
        // we need to dynamically compute a geospatial one
        return this->_ComputeGeospatialXform();
    }
    else if (name == HdOmniGeospatialDataSourceTokens->geospatialPreservedXform)
    {
        // this would be the original flattened matrix of the wrapped data source
        if (_wrappedDataSource != nullptr)
        {
            return _wrappedDataSource->Get(HdXformSchemaTokens->xform);
        }
    }

    // all other token values should be defer to the wrapped data source (if any)
    if (_wrappedDataSource != nullptr)
    {
        return _wrappedDataSource->Get(name);
    }

    return nullptr;
}

bool HdOmniGeospatialDataSource::IsPrimDirtied(const HdDataSourceLocatorSet& locators)
{
    static const HdContainerDataSourceHandle containerNull(nullptr);
    if (locators.Intersects(HdXformSchema::GetDefaultLocator()))
    {
        if (HdContainerDataSource::AtomicLoad(_computedGeospatialPrimDataSource) != nullptr ||
            HdContainerDataSource::AtomicLoad(_computedGeospatialDependentDataSource) != nullptr)
        {
            HdContainerDataSource::AtomicStore(_computedGeospatialPrimDataSource, containerNull);
            HdContainerDataSource::AtomicStore(_computedGeospatialDependentDataSource, containerNull);
            
            return true;
        }
    }

    return false;
}

HdDataSourceBaseHandle HdOmniGeospatialDataSource::_ComputeGeospatialXform()
{
    // since matrices are time sampled, we actually don't compute anything
    // here, we just setup the right HdMatrixDataSources to be able to 
    // compute a final value at a specific time sample when asked
    // to do that, we have two cases:
    // 1. The wrapped prim in question has a local geodetic position applied
    //    In this case, all of the information we need to compute the position
    //    is stored inside of the wrapped prim itself (i.e. the geodetic root 
    //    tangentFrame and geodtic position from the applied API schema)
    // 2. The wrapped prim in question does not have a local geodetic position
    //    applied, but it's parent in the stage hierarchy does, which means
    //    that we need the wrapped prim plus it's parent prim to be able to
    //    compute the new correct transform
    //
    // Case 1 is easy - we can detect whether we have the information or not
    // and create the right data source to return.
    //
    // Case 2 is a bit more difficult to do performantly - at the moment
    // we will walk the parent prim hierarchy to the root to determine
    // this information, but likely you would want to cache this locally
    // on the wrapped prim.  We can certainly do that, but then we have to
    // be concerned about invalidating it at the right time.  We'll leave this
    // as a TODO for the future.
    //
    if (this->_HasGeospatialInformation(_wrappedDataSource))
    {
        // this is case 1, and we can create a data source specifically
        // catered to do that computation
        HdContainerDataSourceHandle computedGeospatialPrimDataSource =
            HdContainerDataSource::AtomicLoad(_computedGeospatialPrimDataSource);

        if (computedGeospatialPrimDataSource != nullptr)
        {
            // we have a previously cached value so can return that directly
            return computedGeospatialPrimDataSource;
        }

        // otherwise we have to compute a new one
        // since the container responsible for the xform token
        // needs to take into account both resetXform and matrix
        // and since both of those can be time-sampled, we have to make
        // sure we can respond appropriately to any query
        // so we will need a complete view of the wrapped data source
        // to perform the computation
        computedGeospatialPrimDataSource = HdOmniGeospatialComputedPrimDataSource::New(_wrappedDataSource);
        HdContainerDataSource::AtomicStore(_computedGeospatialPrimDataSource, computedGeospatialPrimDataSource);

        return computedGeospatialPrimDataSource;
    }
    else
    {
        // this is case 2, in order to perform this transformation appropriately
        // we have to walk the parent hierarchy to find the parent with a local position
        // geospatial API attached to it - if none exists we can return the wrapped
        // data source directly, but if one does exist we need a new data source capable
        // of handling the dynamic compute at any time sample
        HdContainerDataSourceHandle computedGeospatialDependentDataSource =
            HdContainerDataSource::AtomicLoad(_computedGeospatialDependentDataSource);

        if (computedGeospatialDependentDataSource != nullptr)
        {
            // we have a previously cached value and can return that directly
            return computedGeospatialDependentDataSource;
        }

        // otherwise we have to compute a new one
        // so we need to follow the prim hierarchy up until we reach
        // a geospatially applied one (if any)
        if (_primPath != SdfPath::AbsoluteRootPath())
        {
            HdContainerDataSourceHandle geospatialDataSource = nullptr;
            for (SdfPath p = _primPath.GetParentPath(); p != SdfPath::AbsoluteRootPath(); p = p.GetParentPath())
            {
                HdSceneIndexPrim prim = _sceneIndex.GetPrim(p);
                if (this->_HasGeospatialInformation(prim.dataSource))
                {
                    // found it!
                    geospatialDataSource = prim.dataSource;
                }
            }

            // if we didn't find a geospatially applied parent, we don't need to do anything
            if (geospatialDataSource == nullptr)
            {
                if (_wrappedDataSource != nullptr)
                {
                    HdContainerDataSourceHandle dataSource = HdContainerDataSource::Cast(_wrappedDataSource->Get(HdXformSchemaTokens->xform));
                    if (dataSource != nullptr)
                    {
                        HdContainerDataSource::AtomicStore(_computedGeospatialDependentDataSource, dataSource);
                        return _computedGeospatialDependentDataSource;
                    }

                    return nullptr;
                }

                return nullptr;
            }

            // otherwise we need a new datasource that can perform the compute between
            // the immediate parent and the prim in question
            SdfPath parentPath = _primPath.GetParentPath();
            HdSceneIndexPrim parentSceneIndexPrim = _sceneIndex.GetPrim(parentPath);
            computedGeospatialDependentDataSource = HdOmniGeospatialComputedDependentDataSource::New(_wrappedDataSource,
                parentSceneIndexPrim.dataSource);

            HdContainerDataSource::AtomicStore(_computedGeospatialDependentDataSource, computedGeospatialDependentDataSource);

            return computedGeospatialDependentDataSource;
        }
        else
        {
            // it's the root path, and we don't have to do anything here
            // NOTE: this makes the assumption that root never has geospatial information applied
            if (_wrappedDataSource != nullptr)
            {
                return _wrappedDataSource->Get(HdXformSchemaTokens->xform);
            }
        }
    }

    return nullptr;
}

bool HdOmniGeospatialDataSource::_HasGeospatialInformation(HdContainerDataSourceHandle handle)
{
    HdOmniGeospatialWGS84LocalPositionSchema localPositionSchema = HdOmniGeospatialWGS84LocalPositionSchema::GetFromParent(handle);
    return localPositionSchema.IsDefined();
}

PXR_NAMESPACE_CLOSE_SCOPE