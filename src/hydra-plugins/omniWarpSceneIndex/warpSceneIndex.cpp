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

#include <string>

#include <pxr/base/tf/pyInvoke.h>
#include <pxr/base/tf/errorMark.h>
#include <pxr/base/tf/pyExceptionState.h>
#include <pxr/base/tf/pyInterpreter.h>
#include <pxr/imaging/hd/primvarSchema.h>
#include <pxr/imaging/hd/primvarsSchema.h>
#include <pxr/imaging/hd/retainedDataSource.h>
#include <pxr/imaging/hd/tokens.h>
#include <pxr/imaging/hd/meshSchema.h>

#include "warpSceneIndex.h"
#include "tokens.h"

#ifdef PXR_PYTHON_SUPPORT_ENABLED
#include <pxr/base/tf/pyInterpreter.h>
#endif // PXR_PYTHON_SUPPORT_ENABLED

PXR_NAMESPACE_OPEN_SCOPE

UsdImagingStageSceneIndexRefPtr FindUsdImagingSceneIndex(std::vector<HdSceneIndexBaseRefPtr>&);

OmniWarpSceneIndexRefPtr
OmniWarpSceneIndex::New(
    const HdSceneIndexBaseRefPtr &inputSceneIndex)
{
    return TfCreateRefPtr(
        new OmniWarpSceneIndex(
            inputSceneIndex));
}

OmniWarpSceneIndex::OmniWarpSceneIndex(
    const HdSceneIndexBaseRefPtr &inputSceneIndex)
  : HdSingleInputFilteringSceneIndexBase(inputSceneIndex)
{
}

/// A convenience data source implementing the primvar schema from
/// a triple of primvar value, interpolation and role. The latter two
/// are given as tokens. The value can be given either as data source
/// or as thunk returning a data source which is evaluated on each
/// Get.
class _PrimvarDataSource final : public HdContainerDataSource
{
public:
    HD_DECLARE_DATASOURCE(_PrimvarDataSource);

    TfTokenVector GetNames() override {
        return {HdPrimvarSchemaTokens->primvarValue,
                HdPrimvarSchemaTokens->interpolation,
                HdPrimvarSchemaTokens->role};
    }

    HdDataSourceBaseHandle Get(const TfToken &name) override {
        if (name == HdPrimvarSchemaTokens->primvarValue) {
            return _primvarValueSrc;
        }
        if (name == HdPrimvarSchemaTokens->interpolation) {
            return
                HdPrimvarSchema::BuildInterpolationDataSource(
                    _interpolation);
        }
        if (name == HdPrimvarSchemaTokens->role) {
            return
                HdPrimvarSchema::BuildRoleDataSource(
                    _role);
        }

        return nullptr;
    }

private:
    _PrimvarDataSource(
        const HdDataSourceBaseHandle &primvarValueSrc,
        const TfToken &interpolation,
        const TfToken &role)
      : _primvarValueSrc(primvarValueSrc)
      , _interpolation(interpolation)
      , _role(role)
    {
    }

    HdDataSourceBaseHandle _primvarValueSrc;
    TfToken _interpolation;
    TfToken _role;
};

class _PointsDataSource : public HdVec3fArrayDataSource
{
public:
    HD_DECLARE_DATASOURCE(_PointsDataSource);

    VtValue GetValue(const Time shutterOffset) override {
        return VtValue(GetTypedValue(shutterOffset));
    }

    VtVec3fArray GetTypedValue(const Time shutterOffset) override
    {
        if (HdPrimvarSchema primvar = _schema.GetPrimvar(HdTokens->points))
        {
            if (HdSampledDataSourceHandle valueDataSource = primvar.GetPrimvarValue())
            {
                auto pointsOrigVt = valueDataSource->GetValue(0.f);
                if (pointsOrigVt.IsHolding<VtArray<GfVec3f>>())
                {
                    VtVec3fArray pointsOrigArray = pointsOrigVt.UncheckedGet<VtArray<GfVec3f>>();
                    return _pythonModule->ExecSim(pointsOrigArray);
                }
            }
        }
        return VtVec3fArray();
    }

    bool GetContributingSampleTimesForInterval(
                            const Time startTime,
                            const Time endTime,
                            std::vector<Time> * const outSampleTimes) override
    {
        return true;
    }

private:

    _PointsDataSource(HdPrimvarsSchema &primVarSchema, OmniWarpPythonModuleSharedPtr pythonModule)
        : _schema(primVarSchema),
          _pythonModule(pythonModule)
    {
    }
    HdPrimvarsSchema& _schema;
    OmniWarpPythonModuleSharedPtr _pythonModule;
};

class _MeshPrimVarsOverrideDataSource : public HdContainerDataSource
{
public:
    HD_DECLARE_DATASOURCE(_MeshPrimVarsOverrideDataSource);

    TfTokenVector GetNames() override
    {
        if (!_primDataSource) {
            return {};
        }
        return _primDataSource->GetNames();
    }

    HdDataSourceBaseHandle Get(const TfToken &name) override
    {
        HdDataSourceBaseHandle result = _primDataSource->Get(name);

        if (name == HdTokens->points)
        {
            return _PrimvarDataSource::New(
                _PointsDataSource::New(_schema, _pythonModule),
                HdPrimvarSchemaTokens->vertex,
                HdPrimvarSchemaTokens->point);
        }
        return result;
    }

private:
    _MeshPrimVarsOverrideDataSource(const HdContainerDataSourceHandle &primDataSource,
        HdPrimvarsSchema &primVarSchema, OmniWarpPythonModuleSharedPtr pythonModule)
      :  _schema(primVarSchema),
        _pythonModule(pythonModule),
        _primDataSource(primDataSource)
    {
    }

    HdPrimvarsSchema _schema;
    OmniWarpPythonModuleSharedPtr _pythonModule;
    HdContainerDataSourceHandle const _primDataSource;
};

class _WarpMeshDataSource : public HdContainerDataSource
{
public:
    HD_DECLARE_DATASOURCE(_WarpMeshDataSource);

    TfTokenVector GetNames() override
    {
        if (!_inputDs) {
            return {};
        }
        // We append our token for the WarpMesh python file token
        // We do our init for indices here. Only on reload?
        return _inputDs->GetNames();
    }

    HdDataSourceBaseHandle Get(const TfToken &name) override
    {
        auto result = _inputDs->Get(name);

        if (name == HdPrimvarsSchemaTokens->primvars)
        {
            auto primVarSchema = HdPrimvarsSchema::GetFromParent(_inputDs);
            if (auto primVarContainer = HdContainerDataSource::Cast(result))
            {
                return _MeshPrimVarsOverrideDataSource::New(primVarContainer, primVarSchema, _pythonModule);
            }
        }
        return result;
    }

private:
    _WarpMeshDataSource(OmniWarpSceneIndexConstRefPtr inputSi,
        const SdfPath& primPath,
        const HdContainerDataSourceHandle &primDataSource)
      : _inputSi(inputSi),
        _primPath(primPath),
        _inputDs(primDataSource),
        _pythonModule(nullptr)
    {
        _pythonModule = _inputSi->GetWarpPythonModule(_primPath);
        if (!_pythonModule)
        {
            HdMeshSchema meshSchema = HdMeshSchema::GetFromParent(_inputDs);
            HdMeshTopologySchema meshTopologySchema = meshSchema.GetTopology();
            OmniWarpComputationSchema warpCompSchema = OmniWarpComputationSchema::GetFromParent(_inputDs);
            _pythonModule = _inputSi->CreateWarpPythonModule(_primPath, warpCompSchema, meshTopologySchema);
        }
    }

    HdContainerDataSourceHandle _inputDs;
    OmniWarpSceneIndexConstRefPtr _inputSi;
    OmniWarpPythonModuleSharedPtr _pythonModule;

    const SdfPath& _primPath;
};

HdSceneIndexPrim
OmniWarpSceneIndex::GetPrim(const SdfPath& primPath) const
{
    HdSceneIndexPrim prim = _GetInputSceneIndex()->GetPrim(primPath);

    if (prim.primType == HdPrimTypeTokens->mesh && prim.dataSource)
    {
        if (OmniWarpComputationSchema warpCompSchema = OmniWarpComputationSchema::GetFromParent(prim.dataSource))
        {
            prim.dataSource = _WarpMeshDataSource::New(
                OmniWarpSceneIndexConstRefPtr(this), primPath, prim.dataSource);
        }
    }

    return prim;
}

SdfPathVector
OmniWarpSceneIndex::GetChildPrimPaths(const SdfPath &primPath) const
{
    return _GetInputSceneIndex()->GetChildPrimPaths(primPath);
}

void OmniWarpSceneIndex::_PrimsAdded(
    const HdSceneIndexBase &sender,
    const HdSceneIndexObserver::AddedPrimEntries &entries)
{
    if (!_IsObserved()) {
        return;
    }

    for (const HdSceneIndexObserver::AddedPrimEntry& entry : entries)
    {
        if (entry.primType == HdPrimTypeTokens->mesh)
        {
            if (auto filteringIdx = dynamic_cast<HdFilteringSceneIndexBase const*>(&sender))
            {
               auto sceneIdx = FindUsdImagingSceneIndex(filteringIdx->GetInputScenes());
               _pdCache[entry.primPath] = sceneIdx;
            }
        }
    }

    _SendPrimsAdded(entries);
    return;
}

void
OmniWarpSceneIndex::_PrimsRemoved(
    const HdSceneIndexBase &sender,
    const HdSceneIndexObserver::RemovedPrimEntries &entries)
{
    if (!_IsObserved()) {
        return;
    }

    _WarpPythonModuleMap::iterator it = _pythonModuleMap.begin();
    while (it != _pythonModuleMap.end())
    {
        bool bErased = false;
        for (const HdSceneIndexObserver::RemovedPrimEntry& entry : entries)
        {
            if (it->first.HasPrefix(entry.primPath))
            {
                //+++ Support a CloseSim method
                bErased = true;
                it = _pythonModuleMap.erase(it);
                break;
            }
        }
        if (!bErased)
        {
            it++;
        }
    }

    _SendPrimsRemoved(entries);
}

void
OmniWarpSceneIndex::_PrimsDirtied(
    const HdSceneIndexBase &sender,
    const HdSceneIndexObserver::DirtiedPrimEntries &entries)
{
    if (!_IsObserved()) {
        return;
    }

    // +++ Not sure this is the right locator for points data
    static const HdDataSourceLocatorSet pointDeformLocators
    {
        HdPrimvarsSchema::GetDefaultLocator().Append(
            HdPrimvarSchemaTokens->point),
        OmniWarpComputationSchema::GetDefaultLocator().Append(
            OmniWarpComputationSchema::GetSourceFileLocator())
    };

    // If mesh original points or python module path changes
    // remove our _pythonModule for this prim and allow
    // it to be re-created

    //+++ Multithreaded access to _pythonModuleMap
    _WarpPythonModuleMap::iterator it = _pythonModuleMap.begin();
    while (it != _pythonModuleMap.end())
    {
        bool bErased = false;
        for (const HdSceneIndexObserver::DirtiedPrimEntry &entry : entries)
        {
            if (it->first.HasPrefix(entry.primPath))
            {
                if (pointDeformLocators.Intersects(entry.dirtyLocators))
                {
                    bErased = true;
                    it = _pythonModuleMap.erase(it);
                    break;
                }
            }
        }
        if (!bErased)
        {
            it++;
        }
    }

    _SendPrimsDirtied(entries);
}

OmniWarpPythonModuleSharedPtr
OmniWarpSceneIndex::GetWarpPythonModule(const SdfPath &primPath) const
{
    //+++ Multithreaded access to _pythonModuleMap
    auto pythonModule = _pythonModuleMap.find(primPath);
    if (pythonModule == _pythonModuleMap.end())
    {
        return OmniWarpPythonModuleSharedPtr(nullptr);
    }
    return pythonModule->second;
}

OmniWarpPythonModuleSharedPtr
OmniWarpSceneIndex::CreateWarpPythonModule(const SdfPath &primPath,
    OmniWarpComputationSchema& warpSchema,
    HdMeshTopologySchema& topologySchema) const
{
    //+++ Multithreaded access to _pythonModuleMap
    std::string moduleName = warpSchema.GetSourceFile()->GetTypedValue(0);
    _PrimDelegateHashMap::const_iterator it = _pdCache.find(primPath);

    OmniWarpPythonModuleSharedPtr pythonModule =
        std::make_shared<OmniWarpPythonModule>(primPath, moduleName, it->second);
    pythonModule->InitSim(topologySchema);
    _pythonModuleMap[primPath] = pythonModule;
    return _pythonModuleMap.find(primPath)->second;
}

static UsdImagingStageSceneIndexRefPtr FindUsdImagingSceneIndex(std::vector<HdSceneIndexBaseRefPtr>& inputScenes)
{
    UsdImagingStageSceneIndexRefPtr retVal;

    for (size_t i = 0; i < inputScenes.size(); i++)
    {
        HdSceneIndexBaseRefPtr const &sceneIdx = inputScenes[i];
        if (UsdImagingStageSceneIndexRefPtr const imagingSI = TfDynamic_cast<UsdImagingStageSceneIndexRefPtr>(sceneIdx))
        {
            retVal = imagingSI;
            break;
        }
        if (HdFilteringSceneIndexBaseRefPtr const filteringSi = TfDynamic_cast<HdFilteringSceneIndexBaseRefPtr>(sceneIdx))
        {
            retVal = FindUsdImagingSceneIndex(filteringSi->GetInputScenes());
            if (retVal)
            {
                break;
            }
        }
    }
    return retVal;
}

PXR_NAMESPACE_CLOSE_SCOPE