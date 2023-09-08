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

#include <omniGeospatial/wGS84ReferencePositionAPI.h>

#include "referencePositionAPIAdapter.h"
#include "referencePositionDataSource.h"
#include "referencePositionSchema.h"

PXR_NAMESPACE_OPEN_SCOPE

TF_REGISTRY_FUNCTION(TfType)
{
    typedef OmniGeospatialWGS84ReferencePositionAPIAdapter Adapter;
    TfType t = TfType::Define<Adapter, TfType::Bases<Adapter::BaseAdapter> >();
    t.SetFactory<UsdImagingAPISchemaAdapterFactory<Adapter> >();
}

#if PXR_VERSION >= 2302
HdContainerDataSourceHandle OmniGeospatialWGS84ReferencePositionAPIAdapter::GetImagingSubprimData(
    const UsdPrim& prim,
    const TfToken& subprim,
    const TfToken& appliedInstanceName,
    const UsdImagingDataSourceStageGlobals& stageGlobals)
#else
HdContainerDataSourceHandle OmniGeospatialWGS84ReferencePositionAPIAdapter::GetImagingSubprimData(
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

    return HdRetainedContainerDataSource::New(
        HdOmniGeospatialWGS84ReferencePositionSchemaTokens->referencePositionApi,
        HdOmniGeospatialWGS84ReferencePositionDataSource::New(prim, stageGlobals)
    );
}

#if PXR_VERSION >= 2302
HdDataSourceLocatorSet OmniGeospatialWGS84ReferencePositionAPIAdapter::InvalidateImagingSubprim(
    const UsdPrim& prim,
    const TfToken& subprim,
    const TfToken& appliedInstanceName,
    const TfTokenVector& properties)
#else
HdDataSourceLocatorSet OmniGeospatialWGS84ReferencePositionAPIAdapter::InvalidateImagingSubprim(
    const TfToken& subprim,
    const TfToken& appliedInstanceName,
    const TfTokenVector& properties)
#endif
{
    if (!subprim.IsEmpty() || !appliedInstanceName.IsEmpty())
    {
        return HdDataSourceLocatorSet();
    }

    TfToken geospatialPrefix("omni:geospatial:wgs84:reference");
    for (const TfToken& propertyName : properties)
    {
        if (TfStringStartsWith(propertyName, geospatialPrefix))
        {
            return HdOmniGeospatialWGS84ReferencePositionSchema::GetDefaultLocator();
        }
    }


    return HdDataSourceLocatorSet();
}

PXR_NAMESPACE_CLOSE_SCOPE