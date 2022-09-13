// Copyright (c) 2022, NVIDIA CORPORATION.  All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto.  Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.

#include <pxr/pxr.h>
#include <pxr/base/tf/pyModule.h>
#include <pxr/base/tf/registryManager.h>
#include <pxr/base/tf/scriptModuleLoader.h>
#include <pxr/base/tf/token.h>

PXR_NAMESPACE_USING_DIRECTIVE

TF_WRAP_MODULE
{
    TF_WRAP(OmniExampleOmniMeshLod);
    TF_WRAP(OmniExampleOmniExternalDataSourceAPI);
    TF_WRAP(OmniExampleOmniTemperatureDataAPI);
    TF_WRAP(OmniExampleTokens);
}

TF_REGISTRY_FUNCTION(TfScriptModuleLoader) {
    // List of direct dependencies for this library.
    const std::vector<TfToken> reqs = {
        TfToken("sdf"),
        TfToken("tf"),
        TfToken("usd"),
        TfToken("vt")
    };
    TfScriptModuleLoader::GetInstance().
        RegisterLibrary(TfToken("omniExampleSchema"), TfToken("OmniExampleSchema"), reqs);
}