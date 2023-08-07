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
#include <pxr/imaging/hd/retainedDataSource.h>
#include <pxr/imaging/hd/overlayContainerDataSource.h>
#include <pxr/imaging/hd/dependenciesSchema.h>

#include "geospatialSceneIndex.h"
#include "referencePositionSchema.h"
#include "localPositionSchema.h"
#include "geospatialDataSource.h"

PXR_NAMESPACE_OPEN_SCOPE

TF_DEFINE_PRIVATE_TOKENS(
    _tokens,
    (positionToXform)
);

OmniGeospatialSceneIndexRefPtr OmniGeospatialSceneIndex::New(
    const HdSceneIndexBaseRefPtr& inputSceneIndex,
    const HdContainerDataSourceHandle& inputArgs)
{
    return TfCreateRefPtr(new OmniGeospatialSceneIndex(inputSceneIndex, inputArgs));
}

OmniGeospatialSceneIndex::OmniGeospatialSceneIndex(const HdSceneIndexBaseRefPtr& inputSceneIndex,
    const HdContainerDataSourceHandle& inputArgs) :
    HdSingleInputFilteringSceneIndexBase(inputSceneIndex)
{
}

OmniGeospatialSceneIndex::~OmniGeospatialSceneIndex() = default;

HdSceneIndexPrim OmniGeospatialSceneIndex::GetPrim(const SdfPath &primPath) const
{
    // lookup the prim to see if we have wrapped it yet
    auto iterBoolPair = this->_IsPrimWrapped(primPath);
    if (iterBoolPair.second)
    {
        // we have it wrapped already, so return the wrapped prim
        return iterBoolPair.first->second;
    }

    // we haven't wrapped it yet, but we only need to wrap it
    // if it is Xformable - geospatial transforms have the potential
    // to affect anything that has a transform, so even if it is
    // never affected (e.g. resetXform is true or it is not the child
    // of a geospatially applied prim) we wrap it here for simplicity
    // sake at the cost of an extra HdSceneIndexPrim (as in some cases
    // it will even retain its original data source)
    // note that unlike the flattening scene index we wrap lazily
    // instead of walking the tree at construction time - this is because
    // there is a low chance of geospatial information being attached
    // to a prim and in cases where the scene isn't goesptially grounded
    // but the scene index is still applied we don't want to walk the
    // whole scene
    HdSceneIndexPrim sceneIndexPrim = this->_GetInputSceneIndex()->GetPrim(primPath);
    HdXformSchema xformSchema = HdXformSchema::GetFromParent(sceneIndexPrim.dataSource);
    if (xformSchema.IsDefined() && !xformSchema.GetResetXformStack())
    {
        return this->_WrapPrim(primPath, sceneIndexPrim);
    }

    // otherwise we don't need to wrap it and can return it directly
    return sceneIndexPrim;
}

SdfPathVector OmniGeospatialSceneIndex::GetChildPrimPaths(const SdfPath& primPath) const
{
    // no change in topology occurs as part of this scene index
    // so we can ask the input scene to get the child prim paths directly
    return this->_GetInputSceneIndex()->GetChildPrimPaths(primPath);
}

SdfPathTable<HdSceneIndexPrim>::_IterBoolPair OmniGeospatialSceneIndex::_IsPrimWrapped(const SdfPath& primPath) const
{
    bool result = false;
    const auto it = _wrappedPrims.find(primPath);
    if (it != _wrappedPrims.end())
    {
        // because SdfPathTable inserts all parents
        // when a path gets inserted, there may be an empty
        // entry in our cache if a child path was visited first
        // to verify we have to check the prim type and data source
        if (it->second.primType != TfToken() || it->second.dataSource != nullptr)
        {
            // not an auto-insertion of the parent
            result = true;
        }
    }

    return std::make_pair(it, result);
}

HdSceneIndexPrim& OmniGeospatialSceneIndex::_WrapPrim(const SdfPath& primPath, const HdSceneIndexPrim& hdPrim) const
{
    // PRECONDITION: The table must not yet contain a wrapped prim, check via _IsPrimWrapped first!
    // wrapping a scene index prim involves creating our geospatial data source to wrap the original 
    // scene index prim's data source - this will allow us to intercept the xform token to return 
    // a compute geospatial transform and still provide access to the original xform via the wrapped data source
    HdContainerDataSourceHandle wrappedDataSource = HdOmniGeospatialDataSource::New(*this, primPath, hdPrim.dataSource);
    const auto it = _wrappedPrims.find(primPath);
    if (it != _wrappedPrims.end())
    {
        // in this case, the entry is there, but it was auto-created
        // by SdfPathTable, meaning it should have empty entries
        TF_VERIFY(it->second.primType == TfToken());
        TF_VERIFY(it->second.dataSource == nullptr);
        it->second.primType = hdPrim.primType;
        it->second.dataSource = std::move(wrappedDataSource);

        return it->second;
    }
    else
    {
        auto iterBoolPair = _wrappedPrims.insert(
            {
                primPath, 
                HdSceneIndexPrim {
                    hdPrim.primType,
                    std::move(wrappedDataSource)
                }
            }
        );

        return iterBoolPair.first->second;
    }
}

void OmniGeospatialSceneIndex::_PrimsAdded(const HdSceneIndexBase& sender,
    const HdSceneIndexObserver::AddedPrimEntries& entries)
{
    HdSceneIndexObserver::DirtiedPrimEntries dirtyEntries;
    for(const HdSceneIndexObserver::AddedPrimEntry& entry : entries)
    {
        HdSceneIndexPrim sceneIndexPrim = this->_GetInputSceneIndex()->GetPrim(entry.primPath);

        // cache the prim if necessary
        HdXformSchema xformSchema = HdXformSchema::GetFromParent(sceneIndexPrim.dataSource);
        if (xformSchema.IsDefined() && !xformSchema.GetResetXformStack())
        {
            auto iterBoolPair = this->_IsPrimWrapped(entry.primPath);
            if (iterBoolPair.second)
            {
                /// we already wrapped this prim, so we need to update it
                HdSceneIndexPrim& wrappedPrim = iterBoolPair.first->second;
                wrappedPrim.primType = entry.primType;
                if (wrappedPrim.dataSource != nullptr)
                {
                    HdOmniGeospatialDataSource::Cast(wrappedPrim.dataSource)->UpdateWrappedDataSource(sceneIndexPrim.dataSource);
                }

                // if we updated it, we have to now see if we need
                // to dirty any cached values alreday in the hierarchy
                static HdDataSourceLocatorSet locators = {
                    HdXformSchema::GetDefaultLocator()
                };

                this->_DirtyHierarchy(entry.primPath, locators, &dirtyEntries);
            }
            else
            {
                // we don't yet have this prim wrapped - do so now
                this->_WrapPrim(entry.primPath, sceneIndexPrim);
            }
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

void OmniGeospatialSceneIndex::_PrimsRemoved(const HdSceneIndexBase& sender,
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

void OmniGeospatialSceneIndex::_PrimsDirtied(const HdSceneIndexBase& sender,
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

void OmniGeospatialSceneIndex::_DirtyHierarchy(const SdfPath& primPath, const HdDataSourceLocatorSet& locators, 
    HdSceneIndexObserver::DirtiedPrimEntries* dirtyEntries)
{
    // find subtree range retrieves a start end pair of children
    // in the subtree of the given prim path
    auto startEndRangeIterator = _wrappedPrims.FindSubtreeRange(primPath);
    for (auto it = startEndRangeIterator.first; it != startEndRangeIterator.second;)
    {
        // if we have a valid wrapper for the prim, we need to check
        // whether it needs to be dirtied - this involves checking the
        // data sources to see if they have cached data and if so
        // this indicates it needs to be updated
        if (it->second.dataSource != nullptr)
        {
            HdOmniGeospatialDataSourceHandle geospatialDataSource =
                HdOmniGeospatialDataSource::Cast(it->second.dataSource);
            if (geospatialDataSource != nullptr && geospatialDataSource->IsPrimDirtied(locators))
            {
                if (it->first != primPath)
                {
                    dirtyEntries->emplace_back(it->first, locators);
                }

                it++;
            }
            else
            {
                it++;
            }
        }
        else
        {
            it++;
        }
    }
}

PXR_NAMESPACE_CLOSE_SCOPE