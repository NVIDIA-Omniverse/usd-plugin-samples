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

#ifndef OMNI_WARP_SCENE_INDEX_WARP_PYTHON_MODULE_H
#define OMNI_WARP_SCENE_INDEX_WARP_PYTHON_MODULE_H

#include <string>

#include <pxr/pxr.h>
#include <pxr/base/tf/declarePtrs.h>
#include <pxr/base/vt/value.h>
#include <pxr/imaging/hd/meshSchema.h>
#include <pxr/usdImaging/usdImaging/stageSceneIndex.h>

#include "api.h"

PXR_NAMESPACE_OPEN_SCOPE

TF_DECLARE_REF_PTRS(OmniWarpPythonModule);

///
/// \class OmniWarpPythonModule
///
///
///
///
///
class OmniWarpPythonModule
{
public:
    OmniWarpPythonModule(const SdfPath &primPath, const std::string& moduleName,
        UsdImagingStageSceneIndexConstRefPtr usdImagingSi);
    ~OmniWarpPythonModule();

    void InitParticles(VtVec3fArray positions);
    void InitMesh(VtIntArray indices, VtVec3fArray vertices);
    void InitParticlesWithDependentMesh(VtVec3fArray positions,
        VtIntArray indices, VtVec3fArray vertices);

    VtVec3fArray ExecSim();
    VtVec3fArray ExecSim(VtVec3fArray dependentVertices);

private:
        std::string _moduleName;
        SdfPath _primPath;
        UsdImagingStageSceneIndexConstRefPtr _usdImagingSi;
};

using OmniWarpPythonModuleSharedPtr = std::shared_ptr<class OmniWarpPythonModule>;

PXR_NAMESPACE_CLOSE_SCOPE

#endif // OMNI_WARP_SCENE_INDEX_WARP_PYTHON_MODULE_H