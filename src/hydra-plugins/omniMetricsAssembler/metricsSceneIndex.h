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
#ifndef OMNI_METRICS_SCENE_INDEX_H_
#define OMNI_METRICS_SCENE_INDEX_H_

#include <pxr/pxr.h>
#include <pxr/usd/sdf/pathTable.h>
#include <pxr/imaging/hd/filteringSceneIndex.h>

#include "api.h"

PXR_NAMESPACE_OPEN_SCOPE

TF_DECLARE_REF_PTRS(OmniMetricsSceneIndex);

///
/// \class OmniMetricsSceneIndex
/// 
/// A scene index responsible for observing an input flattened scene
/// index and producing a comparable scene in which metrics correctives
/// have been added to the appropriate places in the scene hiearchy
/// to correct for metrics divergences.
///
/// Note that with Render Delegate 2.0 and the ability to pull data
/// from a non-flattened scene, this implementation will have to be
/// revisited to work with the unflattened xform representation of
/// the hydra prims.
///
class OmniMetricsSceneIndex : public HdSingleInputFilteringSceneIndexBase
{
public:
    OMNIMETRICSASSEMBLER_API
    static OmniMetricsSceneIndexRefPtr New(const HdSceneIndexBaseRefPtr& inputSceneIndex,
        const HdContainerDataSourceHandle& inputArgs = nullptr);

    OMNIMETRICSASSEMBLER_API
    ~OmniMetricsSceneIndex() override;

    OMNIMETRICSASSEMBLER_API
    HdSceneIndexPrim GetPrim(const SdfPath& primPath) const override;

    OMNIMETRICSASSEMBLER_API
    SdfPathVector GetChildPrimPaths(const SdfPath& primPath) const override;

protected:

    OmniMetricsSceneIndex(const HdSceneIndexBaseRefPtr& inputSceneIndex,
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

    void _DirtyHierarchy(const SdfPath& primPath, const HdDataSourceLocatorSet& locators, HdSceneIndexObserver::DirtiedPrimEntries* dirtyEntries);
    void _WrapPrimsRecursively(const SdfPath& primPath);

private:

    // wraps all prims in the scene with a metrics data source
    SdfPathTable<HdSceneIndexPrim> _wrappedPrims;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif