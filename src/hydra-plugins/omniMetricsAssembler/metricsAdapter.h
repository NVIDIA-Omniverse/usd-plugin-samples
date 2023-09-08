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
#ifndef OMNI_METRICS_ASSEMBLER_ADAPTER_H_
#define OMNI_METRICS_ASSEMBLER_ADAPTER_H_

#include <pxr/pxr.h>
#include <pxr/usdImaging/usdImaging/apiSchemaAdapter.h>

#include "api.h"

PXR_NAMESPACE_OPEN_SCOPE

///
/// \class OmniMetricsAssemblerAdapter
///
/// Stage scene index adapter which has the opportunity to evaluate each
/// prim in a scene to determine if the prim has divergent metrics present.
///
/// We use a trick here that a null adapter, while deriving from UsdImagingAPISchemaAdapter
/// gets a call for each USD prim traversed in the scene by the stage scene index.  These
/// are known as "keyless adapters" and are supported by the UsdImagingAdapterRegistry.
///
class OmniMetricsAssemblerAdapter : public UsdImagingAPISchemaAdapter
{
public:

    OMNIMETRICSASSEMBLER_API
    ~OmniMetricsAssemblerAdapter() override;

    using BaseAdapter = UsdImagingAPISchemaAdapter;

    OMNIMETRICSASSEMBLER_API
    HdContainerDataSourceHandle GetImagingSubprimData(
        const UsdPrim& prim,
        const TfToken& subprim,
        const TfToken& appliedInstanceName,
        const UsdImagingDataSourceStageGlobals& stageGlobals
    ) override;

    OMNIMETRICSASSEMBLER_API
    HdDataSourceLocatorSet InvalidateImagingSubprim(
        const UsdPrim& prim,
        const TfToken& subprim,
        const TfToken& appliedInstanceName,
        const TfTokenVector& properties
    ) override;

private:

    ///
    /// Retrieves the MPU value from the layer and returns it in mpu.
    ///
    /// Returns true if the MPU value was able to be retrieved from the layer
    /// and false otherwise.
    ///
    bool _GetMpuFromLayer(const SdfLayerHandle& layer, double& mpu);

};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // OMNI_METRICS_ASSEMBLER_ADAPTER_H_