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

#ifndef OMNI_WARP_SCENE_INDEX_WARP_SCENE_INDEX_H
#define OMNI_WARP_SCENE_INDEX_WARP_SCENE_INDEX_H

#include <pxr/pxr.h>
#include <pxr/imaging/hd/filteringSceneIndex.h>
#include <pxr/usdImaging/usdImaging/stageSceneIndex.h>

#include "api.h"
#include "warpPythonModule.h"
#include "warpComputationSchema.h"

PXR_NAMESPACE_OPEN_SCOPE

TF_DECLARE_REF_PTRS(OmniWarpSceneIndex);

class _PointsDataSource;
class _WarpMeshDataSource;

///
/// \class OmniWarpSceneIndex
///
///
///
///
///
class OmniWarpSceneIndex :
    public HdSingleInputFilteringSceneIndexBase
{
public:
    OMNIWARPSCENEINDEX_API
    static OmniWarpSceneIndexRefPtr
    New(const HdSceneIndexBaseRefPtr &inputSceneIndex);
    
    OMNIWARPSCENEINDEX_API
    HdSceneIndexPrim GetPrim(const SdfPath &primPath) const override;
    OMNIWARPSCENEINDEX_API
    SdfPathVector GetChildPrimPaths(const SdfPath &primPath) const override;
    
protected:
    OmniWarpSceneIndex(
        const HdSceneIndexBaseRefPtr &inputSceneIndex);

    void _PrimsAdded(
        const HdSceneIndexBase &sender,
        const HdSceneIndexObserver::AddedPrimEntries &entries) override;

    void _PrimsRemoved(
        const HdSceneIndexBase &sender,
        const HdSceneIndexObserver::RemovedPrimEntries &entries) override;

    void _PrimsDirtied(
        const HdSceneIndexBase &sender,
        const HdSceneIndexObserver::DirtiedPrimEntries &entries) override;
private:
    friend _PointsDataSource;
    friend _WarpMeshDataSource;

    OmniWarpPythonModuleSharedPtr GetWarpPythonModule(const SdfPath &primPath) const;

    OmniWarpPythonModuleSharedPtr CreateWarpPythonModule(const SdfPath &primPath,
        OmniWarpComputationSchema& warpSchema,
        HdMeshTopologySchema& topologySchema) const;

    // SceneIndexPlugins do not have access to the current stage/frame time.
    // Only the UsdImagingStageSceneIndex has this. We store this for each Mesh,
    // nullptr is a valid value. If valid, warp simulation can use the exact
    // stage time. If null, the warp has to emulate frame time
    typedef TfHashMap<SdfPath, UsdImagingStageSceneIndexRefPtr, TfHash> _PrimDelegateHashMap;
    _PrimDelegateHashMap _pdCache;

    // For a prim, AddPrim() is called once, while GetPrim() is called many times
    // We don't know until GetPrim() whether prim supports WarpComputationAPI
    // We only want a single WarpPythonModule per prim, so support deferred creation
    // until after GetPrim()

    typedef std::unordered_map<SdfPath, OmniWarpPythonModuleSharedPtr, SdfPath::Hash> _WarpPythonModuleMap;
    mutable _WarpPythonModuleMap _pythonModuleMap;

};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // OMNI_WARP_SCENE_INDEX_WARP_SCENE_INDEX_H