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

#include <pxr/base/tf/pyInvoke.h>
#include <pxr/base/tf/errorMark.h>
#include <pxr/base/tf/pyExceptionState.h>
#include <pxr/base/tf/pyInterpreter.h>
#include <pxr/imaging/hd/tokens.h>

#include "warpPythonModule.h"
#include "tokens.h"

PXR_NAMESPACE_OPEN_SCOPE

OmniWarpPythonModule::OmniWarpPythonModule(const SdfPath &primPath,
    const std::string& moduleName, UsdImagingStageSceneIndexConstRefPtr usdImagingSi)
    : _primPath(primPath),
      _moduleName(moduleName),
      _usdImagingSi(usdImagingSi)
{
}

OmniWarpPythonModule::~OmniWarpPythonModule()
{
    TfPyLock pyLock;
    boost::python::object result;
    bool ok = TfPyInvokeAndReturn(_moduleName.c_str(), "terminate_sim", &result, _primPath);
}

void OmniWarpPythonModule::InitSim(HdMeshTopologySchema& topologySchema)
{
    HdIntArrayDataSourceHandle faceIndicesDs = topologySchema.GetFaceVertexIndices();

    TfPyLock pyLock;
    //+++ doesn't handle changing (timesampled > 1) faceIndicesDs
    boost::python::object result;
    bool ok = TfPyInvokeAndReturn(_moduleName.c_str(), "initialize_sim", &result, _primPath, faceIndicesDs->GetTypedValue(0.f));
}

VtVec3fArray OmniWarpPythonModule::ExecSim(VtVec3fArray origPoints)
{
    TfPyLock pyLock;
    boost::python::object result;

    float dt = 0.f;
    if (_usdImagingSi)
    {
        dt = _usdImagingSi->GetTime().GetValue();
    }

    if (TfPyInvokeAndReturn(_moduleName.c_str(), "exec_sim", &result, _primPath, origPoints, dt))
    {
        boost::python::extract<VtVec3fArray> theResults(result);
        if (theResults.check())
        {
            return theResults();
        }
    }

    return origPoints;
}

#if 0
bool isWarpEnabled(const SdfPath& /*primPath*/)
{
    bool warpEnabled = false;
    {
        TfPyLock pyLock;

        boost::python::object result;
        bool ok = TfPyInvokeAndReturn("pxr.WarpSceneIndexPlugin.warpSceneIndex", "is_enabled", &result);
        if (ok)
        {
            boost::python::extract<bool> bResult(result);
            if (bResult.check())
            {
                warpEnabled = bResult();
            }
        }
    }
    return warpEnabled;
}
#endif

PXR_NAMESPACE_CLOSE_SCOPE