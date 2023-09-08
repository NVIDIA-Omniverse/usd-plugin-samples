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

#include <pxr/imaging/hd/retainedDataSource.h>

#include <omniGeospatial/wGS84LocalPositionAPI.h>
#include <omniGeospatial/wGS84ReferencePositionAPI.h>

#include "localPositionAPIAdapter.h"
#include "localPositionDataSource.h"
#include "localPositionSchema.h"
#include "referencePositionDataSource.h"
#include "referencePositionSchema.h"

PXR_NAMESPACE_OPEN_SCOPE

TF_REGISTRY_FUNCTION(TfType)
{
    typedef OmniGeospatialWGS84LocalPositionAPIAdapter Adapter;
    TfType t = TfType::Define<Adapter, TfType::Bases<Adapter::BaseAdapter> >();
    t.SetFactory<UsdImagingAPISchemaAdapterFactory<Adapter> >();
}

#if PXR_VERSION >= 2302
HdContainerDataSourceHandle OmniGeospatialWGS84LocalPositionAPIAdapter::GetImagingSubprimData(
    const UsdPrim& prim,
    const TfToken& subprim,
    const TfToken& appliedInstanceName,
    const UsdImagingDataSourceStageGlobals& stageGlobals)
#else
HdContainerDataSourceHandle OmniGeospatialWGS84LocalPositionAPIAdapter::GetImagingSubprimData(
    const TfToken& subprim,
    const UsdPrim& prim,
    const TfToken& appliedInstanceName,
    const UsdImagingDataSourceStageGlobals& stageGlobals)
#endif
{
    // at the point we are invoked here, the stage scene index has already determined
    // that the API schema applies to the prim, so we can safely create our
    // data source
    if (!subprim.IsEmpty() || !appliedInstanceName.IsEmpty())
    {
        // there shouldn't be a subprim or an applied instance name
        // if there is, we don't really know what to do with it
        // so we return null to indicate there is no data source
        // for this prim setup
        return nullptr;
    }

    // to make it a bit easier, we will traverse the parent structure here to find a geodetic root
    // rather than traversing it in the scene index - this is because we have all of the information
    // we need at the point where this prim is getting processed
    HdDataSourceBaseHandle referencePositionDataSource = nullptr;
    for (UsdPrim parentPrim = prim; !parentPrim.IsPseudoRoot(); parentPrim = parentPrim.GetParent())
    {
        if (parentPrim.HasAPI<OmniGeospatialWGS84ReferencePositionAPI>())
        {
            // bake the geodetic root information into this local prim
            referencePositionDataSource = HdOmniGeospatialWGS84ReferencePositionDataSource::New(parentPrim, stageGlobals);
            break;
        }
    }

    // only process local position if we found a geodetic root - if we didn't
    // it means that this is an unrooted local position so we keep whatever
    // transform information the prim would have had otherwise
    if (referencePositionDataSource != nullptr)
    {
        return HdRetainedContainerDataSource::New(
            HdOmniGeospatialWGS84LocalPositionSchemaTokens->localPositionApi,
            HdOmniGeospatialWGS84LocalPositionDataSource::New(prim, stageGlobals),
            HdOmniGeospatialWGS84ReferencePositionSchemaTokens->referencePositionApi,
            referencePositionDataSource
        );
    }

    return nullptr;
}

#if PXR_VERSION >= 2302
HdDataSourceLocatorSet OmniGeospatialWGS84LocalPositionAPIAdapter::InvalidateImagingSubprim(
    const UsdPrim& prim,
    const TfToken& subprim,
    const TfToken& appliedInstanceName,
    const TfTokenVector& properties)
#else
HdDataSourceLocatorSet OmniGeospatialWGS84LocalPositionAPIAdapter::InvalidateImagingSubprim(
    const TfToken& subprim,
    const TfToken& appliedInstanceName,
    const TfTokenVector& properties)
#endif
{
    if (!subprim.IsEmpty() || !appliedInstanceName.IsEmpty())
    {
        return HdDataSourceLocatorSet();
    }

    TfToken geospatialPrefix("omni:geospatial:wgs84:local");
    for (const TfToken& propertyName : properties)
    {
        if (TfStringStartsWith(propertyName, geospatialPrefix))
        {
            return HdOmniGeospatialWGS84LocalPositionSchema::GetDefaultLocator();
        }
    }

    return HdDataSourceLocatorSet();
}

PXR_NAMESPACE_CLOSE_SCOPE