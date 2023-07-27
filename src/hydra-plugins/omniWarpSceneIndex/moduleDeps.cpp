#include "pxr/pxr.h"
#include "pxr/base/tf/registryManager.h"
#include "pxr/base/tf/scriptModuleLoader.h"
#include "pxr/base/tf/token.h"

#include <vector>

PXR_NAMESPACE_OPEN_SCOPE

TF_REGISTRY_FUNCTION(TfScriptModuleLoader) {
    const std::vector<TfToken> reqs = {
        
    };

    TfScriptModuleLoader::GetInstance().
        RegisterLibrary(TfToken("omniWarpSceneIndex"), TfToken("OmniWarpSceneIndex"), reqs);
}

PXR_NAMESPACE_CLOSE_SCOPE