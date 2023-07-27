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
#ifndef OMNI_GEOSPATIAL_WGS84_REFERENCE_POSITION_API_ADAPTER_H_
#define OMNI_GEOSPATIAL_WGS84_REFERENCE_POSITION_API_ADAPTER_H_

#include <pxr/pxr.h>
#include <pxr/usdImaging/usdImaging/apiSchemaAdapter.h>

#include "api.h"

PXR_NAMESPACE_OPEN_SCOPE

class OmniGeospatialWGS84ReferencePositionAPIAdapter : public UsdImagingAPISchemaAdapter
{
public:

    using BaseAdapter = UsdImagingAPISchemaAdapter;

#if PXR_VERSION >= 2302
    OMNIGEOSCENEINDEX_API
    HdContainerDataSourceHandle GetImagingSubprimData(
        const UsdPrim& prim,
        const TfToken& subprim,
        const TfToken& appliedInstanceName,
        const UsdImagingDataSourceStageGlobals& stageGlobals
    ) override;
#else
    OMNIGEOSCENEINDEX_API
    HdContainerDataSourceHandle GetImagingSubprimData(
        const TfToken& subprim,
        const UsdPrim& prim,
        const TfToken& appliedInstanceName,
        const UsdImagingDataSourceStageGlobals& stageGlobals
    ) override;
#endif

#if PXR_VERSION >= 2302
    OMNIGEOSCENEINDEX_API
    HdDataSourceLocatorSet InvalidateImagingSubprim(
        const UsdPrim& prim,
        const TfToken& subprim,
        const TfToken& appliedInstanceName,
        const TfTokenVector& properties
    ) override;
#else
    OMNIGEOSCENEINDEX_API
    HdDataSourceLocatorSet InvalidateImagingSubprim(
        const TfToken& subprim,
        const TfToken& appliedInstanceName,
        const TfTokenVector& properties
    ) override;
#endif
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // OMNI_GEOSPATIAL_WGS84_REFERENCE_POSITION_API_ADAPTER_H_