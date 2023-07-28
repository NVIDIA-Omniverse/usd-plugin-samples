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

#include <pxr/base/work/utils.h>
#include <pxr/imaging/hd/xformSchema.h>

#include "metricsSceneIndex.h"
#include "metricsDataSource.h"

PXR_NAMESPACE_OPEN_SCOPE

OmniMetricsSceneIndexRefPtr OmniMetricsSceneIndex::New(
    const HdSceneIndexBaseRefPtr& inputSceneIndex,
    const HdContainerDataSourceHandle& inputArgs)
{
    return TfCreateRefPtr(new OmniMetricsSceneIndex(inputSceneIndex, inputArgs));
}

OmniMetricsSceneIndex::OmniMetricsSceneIndex(const HdSceneIndexBaseRefPtr& inputSceneIndex,
    const HdContainerDataSourceHandle& inputArgs) :
    HdSingleInputFilteringSceneIndexBase(inputSceneIndex)
{
    _WrapPrimsRecursively(SdfPath::AbsoluteRootPath());
}

OmniMetricsSceneIndex::~OmniMetricsSceneIndex() = default;

HdSceneIndexPrim OmniMetricsSceneIndex::GetPrim(const SdfPath &primPath) const
{
    // if we have the prim wrapped, return the wrapped one
    const auto it = _wrappedPrims.find(primPath);
    if (it != _wrappedPrims.end())
    {
        return it->second;
    }

    // there shouldn't be a scenario where the prim isn't wrapped
    // but in case there is, we return whatever the base scene index
    // gives back
    return this->_GetInputSceneIndex()->GetPrim(primPath);
}

SdfPathVector OmniMetricsSceneIndex::GetChildPrimPaths(const SdfPath& primPath) const
{
    // no change in topology occurs as part of this scene index
    // so we can ask the input scene to get the child prim paths directly
    return this->_GetInputSceneIndex()->GetChildPrimPaths(primPath);
}

void OmniMetricsSceneIndex::_PrimsAdded(const HdSceneIndexBase& sender,
    const HdSceneIndexObserver::AddedPrimEntries& entries)
{
    HdSceneIndexObserver::DirtiedPrimEntries dirtyEntries;
    for(const HdSceneIndexObserver::AddedPrimEntry& entry : entries)
    {
        HdSceneIndexPrim sceneIndexPrim = this->_GetInputSceneIndex()->GetPrim(entry.primPath);
        HdContainerDataSourceHandle dataSource = sceneIndexPrim.dataSource;

        // attempt to insert a wrapped version for this prim
        auto it = _wrappedPrims.insert(
            {
                entry.primPath,
                HdSceneIndexPrim()
            });

        // get a reference to the inserted prim
        // this will be the existing one if insertion failed
        HdSceneIndexPrim &prim = it.first->second;
        prim.primType = entry.primType;

        // if the wrapper does exist, we have to update the data source
        if (prim.dataSource != nullptr)
        {
            HdOmniMetricsDataSource::Cast(prim.dataSource)->UpdateWrappedDataSource(dataSource);
        }
        else
        {
            // new insertion, so it wasn't wrapped previously
            // wrap the data source here
            prim.dataSource = HdOmniMetricsDataSource::New(*this, entry.primPath, dataSource);
        }

        // if this was a new insertion in the middle of the hieararchy
        // we need to invalidate descendent flattened attributes
        if (!it.second)
        {
            // Open Issue: we don't handle this here, because it's just a PoC
            // looking at spheres, but in general, we would need to build a set
            // containing the locators we are interested in (at minimum this would
            // be the transform of the prim itself, HdXformSchemaTokens->xform)
            // and make sure the entire prim hierarchy is dirtied if the data source
            // associated is dirtied based on that locator
            // since this likely requires a plug-in system to solve metrics assembly
            // generically, we defer this to a more general solution
        }
    }

    // forward on the notification
    this->_SendPrimsAdded(entries);

    // also, if we had to dirty entries because of an insertion in the middle
    // of the stage hierarchy, send those along too
    if (!dirtyEntries.empty())
    {
        this->_SendPrimsDirtied(dirtyEntries);
    }
}

void OmniMetricsSceneIndex::_PrimsRemoved(const HdSceneIndexBase& sender,
    const HdSceneIndexObserver::RemovedPrimEntries& entries)
{
    for (const HdSceneIndexObserver::RemovedPrimEntry& entry : entries)
    {
        if (entry.primPath.IsAbsoluteRootPath())
        {
            // removing the whole scene
            _wrappedPrims.ClearInParallel();
            TfReset(_wrappedPrims);
        }
        else
        {
            auto startEndRangeIterator = _wrappedPrims.FindSubtreeRange(entry.primPath);
            for (auto it = startEndRangeIterator.first; it != startEndRangeIterator.second; it++)
            {
                WorkSwapDestroyAsync(it->second.dataSource);
            }

            if(startEndRangeIterator.first != startEndRangeIterator.second)
            {
                _wrappedPrims.erase(startEndRangeIterator.first);
            }
        }
    }

    _SendPrimsRemoved(entries);
}

void OmniMetricsSceneIndex::_PrimsDirtied(const HdSceneIndexBase& sender,
    const HdSceneIndexObserver::DirtiedPrimEntries& entries)
{
    HdSceneIndexObserver::DirtiedPrimEntries dirtyEntries;
    for (const HdSceneIndexObserver::DirtiedPrimEntry& entry : entries)
    {
        HdDataSourceLocatorSet locators;
        if (entry.dirtyLocators.Intersects(HdXformSchema::GetDefaultLocator()))
        {
            locators.insert(HdXformSchema::GetDefaultLocator());
        }

        // Open Issue: what about the radius locator?  we would need that, but
        // it depends on where our scene index resides - it may already have
        // been converted by the ImplicitSceneIndex into a mesh (and it's hard
        // to know where exactly our scene index will be inserted)
        // we don't solve it here because a general metrics assembler wouldn't
        // be considering spheres only, so we defer that to a more general solution

        if (!locators.IsEmpty())
        {
            this->_DirtyHierarchy(entry.primPath, locators, &dirtyEntries);
        }
    }

    _SendPrimsDirtied(entries);
    if (!dirtyEntries.empty())
    {
        _SendPrimsDirtied(dirtyEntries);
    }
}

void OmniMetricsSceneIndex::_DirtyHierarchy(const SdfPath& primPath, const HdDataSourceLocatorSet& locators, 
    HdSceneIndexObserver::DirtiedPrimEntries* dirtyEntries)
{
    // find subtree range retrieves a start end pair of children
    // in the subtree of the given prim path
    auto startEndRangeIterator = _wrappedPrims.FindSubtreeRange(primPath);
    for (auto it = startEndRangeIterator.first; it != startEndRangeIterator.second;)
    {
        HdOmniMetricsDataSourceHandle dataSource = HdOmniMetricsDataSource::Cast(it->second.dataSource);
        if (dataSource != nullptr)
        {
            if (dataSource->IsPrimDirtied(locators))
            {
                if (it->first != primPath)
                {
                    dirtyEntries->emplace_back(it->first, locators);
                }
                it++;
            }
            else
            {
                it = it.GetNextSubtree();
            }
        }
        else
        {
            it = it++;
        }
    }
}

void OmniMetricsSceneIndex::_WrapPrimsRecursively(const SdfPath& primPath)
{
    HdSceneIndexPrim prim = this->_GetInputSceneIndex()->GetPrim(primPath);
    HdOmniMetricsDataSourceHandle wrappedDataSource = HdOmniMetricsDataSource::New(*this, primPath, prim.dataSource);

    _wrappedPrims.insert(
        {
            primPath,
            HdSceneIndexPrim
            {
                prim.primType,
                std::move(wrappedDataSource)
            }
        }
    );

    for (const SdfPath& childPath : this->_GetInputSceneIndex()->GetChildPrimPaths(primPath))
    {
        this->_WrapPrimsRecursively(childPath);
    }
}

PXR_NAMESPACE_CLOSE_SCOPE