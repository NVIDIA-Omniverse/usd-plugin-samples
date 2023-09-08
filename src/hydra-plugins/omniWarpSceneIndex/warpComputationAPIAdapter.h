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
//

#ifndef OMNI_WARP_SCENE_INDEX_WARP_COMPUTATION_API_ADAPTER_H
#define OMNI_WARP_SCENE_INDEX_WARP_COMPUTATION_API_ADAPTER_H

#include <pxr/usdImaging/usdImaging/apiSchemaAdapter.h>

#include "api.h"

PXR_NAMESPACE_OPEN_SCOPE

class WarpComputationAPIAdapter : public UsdImagingAPISchemaAdapter
{
public:

    using BaseAdapter = UsdImagingAPISchemaAdapter;

    OMNIWARPSCENEINDEX_API
    HdContainerDataSourceHandle GetImagingSubprimData(
            UsdPrim const& prim,
            TfToken const& subprim,
            TfToken const& appliedInstanceName,
            const UsdImagingDataSourceStageGlobals &stageGlobals) override;

#if PXR_VERSION < 2308
    OMNIWARPSCENEINDEX_API
    HdDataSourceLocatorSet InvalidateImagingSubprim(
            UsdPrim const& prim,
            TfToken const& subprim,
            TfToken const& appliedInstanceName,
            TfTokenVector const& properties) override;
#else
    OMNIWARPSCENEINDEX_API
    HdDataSourceLocatorSet InvalidateImagingSubprim(
            UsdPrim const& prim,
            TfToken const& subprim,
            TfToken const& appliedInstanceName,
            TfTokenVector const& properties,
            UsdImagingPropertyInvalidationType invalidationType) override;
#endif
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // OMNI_WARP_SCENE_INDEX_WARP_COMPUTATION_API_ADAPTER_H
