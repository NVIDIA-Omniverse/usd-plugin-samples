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

#include <pxr/usdImaging/usdImaging/dataSourceAttribute.h>

#include "localPositionDataSource.h"

PXR_NAMESPACE_OPEN_SCOPE

HdOmniGeospatialWGS84LocalPositionDataSource::HdOmniGeospatialWGS84LocalPositionDataSource(
    const UsdPrim& prim,
    const UsdImagingDataSourceStageGlobals& stageGlobals) :
    _stageGlobals(stageGlobals)
{
    _localPositionApi = OmniGeospatialWGS84LocalPositionAPI(prim);
}

#if PXR_VERSION < 2302
bool HdOmniGeospatialWGS84LocalPositionDataSource::Has(const TfToken& name)
{
    return (name == HdOmniGeospatialWGS84LocalPositionSchemaTokens->position);
}
#endif

TfTokenVector HdOmniGeospatialWGS84LocalPositionDataSource::GetNames()
{
    // return the hydra attribute names this data source is responsible for
    TfTokenVector names;
    names.push_back(HdOmniGeospatialWGS84LocalPositionSchemaTokens->position);
    
    return names;
}

HdDataSourceBaseHandle HdOmniGeospatialWGS84LocalPositionDataSource::Get(const TfToken& name)
{
    // retrieves the data source values for the attributes this data source
    // supports
    if (name == HdOmniGeospatialWGS84LocalPositionSchemaTokens->position)
    {
        return UsdImagingDataSourceAttribute<GfVec3d>::New(
            _localPositionApi.GetPositionAttr(), _stageGlobals);
    }

    // this is a name we don't support
    return nullptr;
}

PXR_NAMESPACE_CLOSE_SCOPE