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
#ifndef OMNI_METRICS_SCENE_INDEX_PLUGIN_H_
#define OMNI_METRICS_SCENE_INDEX_PLUGIN_H_

#include <pxr/pxr.h>
#include <pxr/imaging/hd/sceneIndexPlugin.h>

#include "api.h"

PXR_NAMESPACE_OPEN_SCOPE

///
/// \class OmniMetricsSceneIndexPlugin
///
/// Defines the Hydra 2.0 scene index plugin that creates
/// the OmniMetricsSceneIndex.
///
class OmniMetricsSceneIndexPlugin : public HdSceneIndexPlugin
{
public:
    OmniMetricsSceneIndexPlugin();

protected:
    HdSceneIndexBaseRefPtr _AppendSceneIndex(const HdSceneIndexBaseRefPtr& inputScene,
        const HdContainerDataSourceHandle& inputArgs) override;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // OMNI_METRICS_SCENE_INDEX_PLUGIN_H_