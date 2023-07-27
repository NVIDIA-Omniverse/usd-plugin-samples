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

    TfTokenVector GetNames() override {
        TfTokenVector result;
        result.reserve(2);

        result.push_back(_tokens->warpComputation);

        if (_api.GetSourceFileAttr()) {
            result.push_back(_tokens->sourceFile);
        }

        return result;
    }

    HdDataSourceBaseHandle Get(const TfToken &name) override {
        if (name == _tokens->sourceFile) {
            if (UsdAttribute attr = _api.GetSourceFileAttr()) {
                return UsdImagingDataSourceAttributeNew(attr, _stageGlobals);
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
    UsdAttribute attr = _api.GetSourceFileAttr();
    std::string value;
    attr.Get(&value, 0.f);
    if (value.length())
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
