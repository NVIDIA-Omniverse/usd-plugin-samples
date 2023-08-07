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

#include <pxr/base/tf/stringUtils.h>
#include <pxr/imaging/hd/retainedDataSource.h>
#include <pxr/usdImaging/usdImaging/dataSourceAttribute.h>

#include "warpComputationAPIAdapter.h"
#include "warpComputationAPI.h"

PXR_NAMESPACE_OPEN_SCOPE

TF_DEFINE_PRIVATE_TOKENS(
    _tokens,
    (warpComputation)
    (sourceFile)
    (dependentPrims)
    (simulationParams)
);

TF_REGISTRY_FUNCTION(TfType)
{
    typedef WarpComputationAPIAdapter Adapter;
    TfType t = TfType::Define<Adapter, TfType::Bases<Adapter::BaseAdapter> >();
    t.SetFactory< UsdImagingAPISchemaAdapterFactory<Adapter> >();
}

// ----------------------------------------------------------------------------

namespace
{

class SimulationParamsDataSource : public HdSampledDataSource
{
public:
    HD_DECLARE_DATASOURCE(SimulationParamsDataSource);

    SimulationParamsDataSource(
        const VtDictionary &dict)
    : _customData(dict)
    {
    }

    VtValue
    GetValue(Time shutterOffset)
    {
        return VtValue(_customData);
    }

    bool
    GetContributingSampleTimesForInterval(
        Time startTime,
        Time endTime,
        std::vector<Time> * outSampleTimes)
    {
        return false;
    }
    VtDictionary _customData;
};


class DependentPrimsDataSource : public HdPathArrayDataSource
{
public:
    HD_DECLARE_DATASOURCE(DependentPrimsDataSource);

    DependentPrimsDataSource(
        const UsdRelationship &rel)
    : _usdRel(rel)
    {
    }

    VtValue
    GetValue(
            HdSampledDataSource::Time shutterOffset)
    {
        return VtValue(GetTypedValue(shutterOffset));
    }

    VtArray<SdfPath>
    GetTypedValue(
            HdSampledDataSource::Time shutterOffset)
    {
        SdfPathVector paths;
        _usdRel.GetForwardedTargets(&paths);
        VtArray<SdfPath> vtPaths(paths.begin(), paths.end());
        return vtPaths;
    }

    bool
    GetContributingSampleTimesForInterval(
            HdSampledDataSource::Time startTime,
            HdSampledDataSource::Time endTime,
            std::vector<HdSampledDataSource::Time> *outSampleTimes)
    {
        return false;
    }

private:
    UsdRelationship _usdRel;
};

HD_DECLARE_DATASOURCE_HANDLES(DependentPrimsDataSource);

class _WarpComputationDataSource : public HdContainerDataSource
{
public:
    HD_DECLARE_DATASOURCE(_WarpComputationDataSource);

    _WarpComputationDataSource(
        const UsdPrim &prim,
        const UsdImagingDataSourceStageGlobals &stageGlobals)
    : _api(prim)
    , _stageGlobals(stageGlobals)
    {
    }

    TfTokenVector GetNames() override
    {
        TfTokenVector result;
        result.reserve(4);

        result.push_back(_tokens->warpComputation);

        if (UsdAttribute attr = _api.GetSourceFileAttr()) {
            result.push_back(_tokens->sourceFile);

            VtDictionary customData = attr.GetCustomData();
            VtDictionary::iterator iter = customData.begin();
            if (iter != customData.end())
            {
                result.push_back(_tokens->simulationParams);
            }
        }

        if (_api.GetDependentPrimsRel()) {
            result.push_back(_tokens->dependentPrims);
        }

        return result;
    }

    HdDataSourceBaseHandle Get(const TfToken &name) override {
        if (name == _tokens->sourceFile)
        {
            if (UsdAttribute attr = _api.GetSourceFileAttr())
            {
                return UsdImagingDataSourceAttributeNew(attr, _stageGlobals);
            }
        }
        else if (name == _tokens->dependentPrims)
        {
            if (UsdRelationship rel = _api.GetDependentPrimsRel())
            {
                return DependentPrimsDataSource::New(rel);
            }
        }
        else if (name == _tokens->simulationParams)
        {
            if (UsdAttribute attr = _api.GetSourceFileAttr())
            {
                VtDictionary customData = attr.GetCustomData();
                VtDictionary::iterator iter = customData.begin();
                if (iter != customData.end())
                {
                    return SimulationParamsDataSource::New(customData);
                }
            }

        }
        return nullptr;
    }

private:

    OmniWarpSceneIndexWarpComputationAPI _api;
    const UsdImagingDataSourceStageGlobals &_stageGlobals;
};

HD_DECLARE_DATASOURCE_HANDLES(_WarpComputationDataSource);

} // anonymous namespace

// ----------------------------------------------------------------------------

HdContainerDataSourceHandle
WarpComputationAPIAdapter::GetImagingSubprimData(
    UsdPrim const& prim,
    TfToken const& subprim,
    TfToken const& appliedInstanceName,
    const UsdImagingDataSourceStageGlobals &stageGlobals)
{
    OmniWarpSceneIndexWarpComputationAPI _api(prim);
    std::string pythonModuleName;
    UsdAttribute attr = _api.GetSourceFileAttr();
    attr.Get(&pythonModuleName, 0.f);

    if (pythonModuleName.length())
    {
        return HdRetainedContainerDataSource::New(
            _tokens->warpComputation,
            _WarpComputationDataSource::New(
                prim, stageGlobals));
    }
    return nullptr;
}

#if PXR_VERSION < 2308
HdDataSourceLocatorSet
WarpComputationAPIAdapter::InvalidateImagingSubprim(
    UsdPrim const& prim,
    TfToken const& subprim,
    TfToken const& appliedInstanceName,
    TfTokenVector const& properties)
#else
HdDataSourceLocatorSet
WarpComputationAPIAdapter::InvalidateImagingSubprim(
    UsdPrim const& prim,
    TfToken const& subprim,
    TfToken const& appliedInstanceName,
    TfTokenVector const& properties,
    const UsdImagingPropertyInvalidationType invalidationType)
#endif
{
#if 0
    if (!subprim.IsEmpty() || appliedInstanceName.IsEmpty()) {
        return HdDataSourceLocatorSet();
    }

    std::string prefix = TfStringPrintf(
        "collections:%s:", appliedInstanceName.data());

    for (const TfToken &propertyName : properties) {
        if (TfStringStartsWith(propertyName.GetString(), prefix)) {
            return HdDataSourceLocator(
                _tokens->usdCollections, appliedInstanceName);
        }
    }
#endif
    return HdDataSourceLocatorSet();
}

PXR_NAMESPACE_CLOSE_SCOPE
