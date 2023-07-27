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
#include "pxr/imaging/hd/instancerTopologySchema.h"

#include "warpSceneIndex.h"
#include "tokens.h"

#ifdef PXR_PYTHON_SUPPORT_ENABLED
#include <pxr/base/tf/pyInterpreter.h>
#endif // PXR_PYTHON_SUPPORT_ENABLED

PXR_NAMESPACE_OPEN_SCOPE

UsdImagingStageSceneIndexRefPtr FindUsdImagingSceneIndex(const std::vector<HdSceneIndexBaseRefPtr>&);

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
        return _pythonModule->ExecSim();
    }

    bool GetContributingSampleTimesForInterval(
                            const Time startTime,
                            const Time endTime,
                            std::vector<Time> * const outSampleTimes) override
    {
        return false;
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

class _InstancePositionsDataSource : public HdVec3fArrayDataSource
{
public:
    HD_DECLARE_DATASOURCE(_InstancePositionsDataSource);

    VtValue GetValue(const Time shutterOffset) override {
        return VtValue(GetTypedValue(shutterOffset));
    }

    VtVec3fArray GetTypedValue(const Time shutterOffset) override
    {
        return _pythonModule->ExecSim();
    }

    bool GetContributingSampleTimesForInterval(
                            const Time startTime,
                            const Time endTime,
                            std::vector<Time> * const outSampleTimes) override
    {
        return false;
    }

private:

    _InstancePositionsDataSource(HdPrimvarsSchema &primVarSchema, OmniWarpPythonModuleSharedPtr pythonModule)
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
        if (!_inputDs) {
            return {};
        }
        return _inputDs->GetNames();
    }

    HdDataSourceBaseHandle Get(const TfToken &name) override
    {
        if (name == HdTokens->points)
        {
            return _PrimvarDataSource::New(
                _PointsDataSource::New(_schema, _pythonModule),
                HdPrimvarSchemaTokens->vertex,
                HdPrimvarSchemaTokens->point);
        }
        HdDataSourceBaseHandle result = _inputDs->Get(name);
        return result;
    }

private:
    _MeshPrimVarsOverrideDataSource(const HdContainerDataSourceHandle &primDataSource,
        HdPrimvarsSchema &primVarSchema, OmniWarpPythonModuleSharedPtr pythonModule)
      :  _schema(primVarSchema),
        _pythonModule(pythonModule),
        _inputDs(primDataSource)
    {
    }

    HdPrimvarsSchema _schema;
    OmniWarpPythonModuleSharedPtr _pythonModule;
    HdContainerDataSourceHandle const _inputDs;
};

class _InstancerPrimVarsOverrideDataSource : public HdContainerDataSource
{
public:
    HD_DECLARE_DATASOURCE(_InstancerPrimVarsOverrideDataSource);

    TfTokenVector GetNames() override
    {
        if (!_inputDs) {
            return {};
        }
        return _inputDs->GetNames();
    }

    HdDataSourceBaseHandle Get(const TfToken &name) override
    {
        if (name == HdInstancerTokens->translate)
        {
            return _PrimvarDataSource::New(
                _InstancePositionsDataSource::New(_schema, _pythonModule),
                 HdPrimvarSchemaTokens->instance,
                 HdPrimvarRoleTokens->vector);
        }
        HdDataSourceBaseHandle result = _inputDs->Get(name);
        return result;
    }

private:
    _InstancerPrimVarsOverrideDataSource(const HdContainerDataSourceHandle &primDataSource,
        HdPrimvarsSchema &primVarSchema, OmniWarpPythonModuleSharedPtr pythonModule)
      :  _schema(primVarSchema),
        _pythonModule(pythonModule),
        _inputDs(primDataSource)
    {
    }

    HdPrimvarsSchema _schema;
    OmniWarpPythonModuleSharedPtr _pythonModule;
    HdContainerDataSourceHandle const _inputDs;
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
    _WarpMeshDataSource(const SdfPath& primPath,
        const HdContainerDataSourceHandle &primDataSource,
        OmniWarpPythonModuleSharedPtr pythonModule)
      : _primPath(primPath),
        _inputDs(primDataSource),
        _pythonModule(pythonModule)
    {
    }

    HdContainerDataSourceHandle _inputDs;
    OmniWarpPythonModuleSharedPtr _pythonModule;

    const SdfPath& _primPath;
};

class _WarpInstancerDataSource : public HdContainerDataSource
{
public:
    HD_DECLARE_DATASOURCE(_WarpInstancerDataSource);

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
                return _InstancerPrimVarsOverrideDataSource::New(primVarContainer, primVarSchema, _pythonModule);
            }
        }
        return result;
    }

private:
    _WarpInstancerDataSource(const SdfPath& primPath,
        const HdContainerDataSourceHandle &primDataSource,
        OmniWarpPythonModuleSharedPtr pythonModule)
      : _primPath(primPath),
        _inputDs(primDataSource),
        _pythonModule(pythonModule)
    {
    }

    HdContainerDataSourceHandle _inputDs;
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
                primPath, prim.dataSource, GetWarpPythonModule(primPath));
        }
    }
    else if (prim.primType == HdPrimTypeTokens->instancer && prim.dataSource)
    {
        HdInstancerTopologySchema topologySchema = HdInstancerTopologySchema::GetFromParent(prim.dataSource);
        if (HdPathArrayDataSourceHandle const ds = topologySchema.GetPrototypes())
        {
            auto protoTypes = ds->GetTypedValue(0.0f);
            for (size_t i = 0; i < protoTypes.size(); ++i)
            {
                auto protoPrim = _GetInputSceneIndex()->GetPrim(protoTypes[i]);
                OmniWarpComputationSchema warpCompSchema = OmniWarpComputationSchema::GetFromParent(protoPrim.dataSource);
                if (warpCompSchema)
                {
                    prim.dataSource = _WarpInstancerDataSource::New(
                        primPath, prim.dataSource, GetWarpPythonModule(primPath));
                }
            }
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
            auto prim = _GetInputSceneIndex()->GetPrim(entry.primPath);
            HdMeshSchema meshSchema = HdMeshSchema::GetFromParent(prim.dataSource);
            HdPrimvarsSchema primVarsSchema = HdPrimvarsSchema::GetFromParent(prim.dataSource);
            OmniWarpComputationSchema warpCompSchema = OmniWarpComputationSchema::GetFromParent(prim.dataSource);
            if (meshSchema && warpCompSchema && primVarsSchema)
            {
                assert(GetWarpPythonModule(entry.primPath) == nullptr);
                HdMeshTopologySchema meshTopologySchema = meshSchema.GetTopology();

                UsdImagingStageSceneIndexRefPtr usdImagingSi;
                if (auto filteringIdx = dynamic_cast<HdFilteringSceneIndexBase const*>(&sender))
                {
                    // SceneIndexPlugins do not have access to the current stage/frame time.
                    // Only the UsdImagingStageSceneIndex has this. We store this for each Mesh,
                    // nullptr is a valid value. If valid, warp simulation can use the exact
                    // stage time. If null, the warp has to emulate frame time
                    usdImagingSi = FindUsdImagingSceneIndex(filteringIdx->GetInputScenes());
                }
                HdPrimvarSchema origPoints = primVarsSchema.GetPrimvar(HdTokens->points);
                CreateWarpPythonModule(entry.primPath, warpCompSchema, meshTopologySchema, origPoints, usdImagingSi);
            }
        }
        else if (entry.primType == HdPrimTypeTokens->instancer)
        {
            auto prim = _GetInputSceneIndex()->GetPrim(entry.primPath);

            HdPrimvarsSchema primVarSchema = HdPrimvarsSchema::GetFromParent(prim.dataSource);
            HdInstancerTopologySchema topologySchema = HdInstancerTopologySchema::GetFromParent(prim.dataSource);
            HdPathArrayDataSourceHandle const ds = topologySchema.GetPrototypes();
            if (primVarSchema && ds)
            {
                auto protoTypes = ds->GetTypedValue(0.0f);
                for (size_t i = 0; i < protoTypes.size(); ++i)
                {
                    auto protoPrim = _GetInputSceneIndex()->GetPrim(protoTypes[i]);
                    if (protoPrim.primType == TfToken())
                    {
                        continue;
                    }
                    OmniWarpComputationSchema warpCompSchema = OmniWarpComputationSchema::GetFromParent(protoPrim.dataSource);
                    if (warpCompSchema)
                    {
                        assert(GetWarpPythonModule(entry.primPath) == nullptr);
                        UsdImagingStageSceneIndexRefPtr usdImagingSi;
                        if (auto filteringIdx = dynamic_cast<HdFilteringSceneIndexBase const*>(&sender))
                        {
                            // SceneIndexPlugins do not have access to the current stage/frame time.
                            // Only the UsdImagingStageSceneIndex has this. We store this for each Mesh,
                            // nullptr is a valid value. If valid, warp simulation can use the exact
                            // stage time. If null, the warp has to emulate frame time
                            usdImagingSi = FindUsdImagingSceneIndex(filteringIdx->GetInputScenes());
                        }
                        HdPrimvarSchema positionsPos = primVarSchema.GetPrimvar(HdInstancerTokens->translate);
                        CreateWarpPythonModule(entry.primPath, warpCompSchema, positionsPos, usdImagingSi);
                        break;
                    }
                }
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
    HdMeshTopologySchema& topologySchema,
    HdPrimvarSchema& primVarSchema,
    UsdImagingStageSceneIndexRefPtr usdImagingSi)
{
    //+++ Multithreaded access to _pythonModuleMap
    std::string moduleName = warpSchema.GetSourceFile()->GetTypedValue(0);

    HdIntArrayDataSourceHandle faceIndicesDs = topologySchema.GetFaceVertexIndices();
    VtIntArray indices = faceIndicesDs->GetTypedValue(0.f);

    HdSampledDataSourceHandle valueDataSource = primVarSchema.GetPrimvarValue();
    auto pointsVt = valueDataSource->GetValue(0.f);
    VtVec3fArray pointsArray =  pointsVt.UncheckedGet<VtArray<GfVec3f>>();

    // Force terminate of old module
    _pythonModuleMap[primPath] = nullptr;

    OmniWarpPythonModuleSharedPtr pythonModule =
        std::make_shared<OmniWarpPythonModule>(primPath, moduleName, usdImagingSi);
    pythonModule->InitMesh(indices, pointsArray);
    _pythonModuleMap[primPath] = pythonModule;
    return _pythonModuleMap.find(primPath)->second;
}

OmniWarpPythonModuleSharedPtr
OmniWarpSceneIndex::CreateWarpPythonModule(const SdfPath &primPath,
    OmniWarpComputationSchema& warpSchema,
    HdPrimvarSchema& primVarSchema,
    UsdImagingStageSceneIndexRefPtr usdImagingSi)
{
    //+++ Multithreaded access to _pythonModuleMap
    std::string moduleName = warpSchema.GetSourceFile()->GetTypedValue(0);

    // Force terminate of old module
    _pythonModuleMap[primPath] = nullptr;

    HdSampledDataSourceHandle valueDataSource = primVarSchema.GetPrimvarValue();
    auto positionsVt = valueDataSource->GetValue(0.f);
    VtVec3fArray positionsArray =  positionsVt.UncheckedGet<VtArray<GfVec3f>>();

    OmniWarpPythonModuleSharedPtr pythonModule =
        std::make_shared<OmniWarpPythonModule>(primPath, moduleName, usdImagingSi);
    pythonModule->InitParticles(positionsArray);
    _pythonModuleMap[primPath] = pythonModule;
    return _pythonModuleMap.find(primPath)->second;
}

static UsdImagingStageSceneIndexRefPtr FindUsdImagingSceneIndex(const std::vector<HdSceneIndexBaseRefPtr>& inputScenes)
{
    TfRefPtr<UsdImagingStageSceneIndex> retVal;

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