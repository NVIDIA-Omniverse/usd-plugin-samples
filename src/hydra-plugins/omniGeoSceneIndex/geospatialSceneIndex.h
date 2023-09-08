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
#ifndef OMNI_GEO_SCENE_INDEX_H_
#define OMNI_GEO_SCENE_INDEX_H_

#include <pxr/pxr.h>
#include <pxr/usd/sdf/pathTable.h>
#include <pxr/imaging/hd/filteringSceneIndex.h>

#include "api.h"

PXR_NAMESPACE_OPEN_SCOPE

TF_DECLARE_REF_PTRS(OmniGeospatialSceneIndex);

///
/// \class OmniGeospatialSceneIndex
/// 
/// A scene index responsible for observing an input flattened scene
/// index and producing a comparable scene in which geospatial transforms
/// have been applied to prims with geospatial state attached to them
/// and for updating the transform of their children as needed.
///
/// Note that with Render Delegate 2.0 and the ability to pull data
/// from a non-flattened scene, this implementation will have to be
/// revisited to work with the unflattened xform representation of
/// the hydra prims.
///
class OmniGeospatialSceneIndex : public HdSingleInputFilteringSceneIndexBase
{
public:
    OMNIGEOSCENEINDEX_API
    static OmniGeospatialSceneIndexRefPtr New(const HdSceneIndexBaseRefPtr& inputSceneIndex,
        const HdContainerDataSourceHandle& inputArgs = nullptr);

    OMNIGEOSCENEINDEX_API
    ~OmniGeospatialSceneIndex() override;

    OMNIGEOSCENEINDEX_API
    HdSceneIndexPrim GetPrim(const SdfPath& primPath) const override;

    OMNIGEOSCENEINDEX_API
    SdfPathVector GetChildPrimPaths(const SdfPath& primPath) const override;

protected:

    OmniGeospatialSceneIndex(const HdSceneIndexBaseRefPtr& inputSceneIndex,
        const HdContainerDataSourceHandle& inputArgs);

    // these three are provided by HdSingleInputFilteringSceneIndexBase
    // and must be overridden by inheritors
    virtual void _PrimsAdded(const HdSceneIndexBase& sender,
        const HdSceneIndexObserver::AddedPrimEntries& entries) override;

    virtual void _PrimsRemoved(const HdSceneIndexBase& sender,
        const HdSceneIndexObserver::RemovedPrimEntries& entries) override;

    virtual void _PrimsDirtied(const HdSceneIndexBase& sender,
        const HdSceneIndexObserver::DirtiedPrimEntries& entries) override;

private:

    SdfPathTable<HdSceneIndexPrim>::_IterBoolPair _IsPrimWrapped(const SdfPath& primPath) const;
    HdSceneIndexPrim& _WrapPrim(const SdfPath& primPath, const HdSceneIndexPrim& hdPrim) const;
    void _DirtyHierarchy(const SdfPath& primPath, const HdDataSourceLocatorSet& locators, HdSceneIndexObserver::DirtiedPrimEntries* dirtyEntries);

    /*HdContainerDataSourceHandle _ComputeDataSource(
        const SdfPath& primPath,
        const HdContainerDataSourceHandle& primDataSource) const;

    void _ComputeChildDataSources(const SdfPath& parentPath,
        const HdContainerDataSourceHandle& parentDataSource) const;

    HdContainerDataSourceHandle _ComputeMatrixDependenciesDataSource(
        const SdfPath& primPath) const;*/

private:

    // marked as mutable because it is an internal cache
    // that is written to on-demand from the GetPrim method
    // which is a const method by interface definition in HdSceneIndexBase
    mutable SdfPathTable<HdSceneIndexPrim> _wrappedPrims;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif