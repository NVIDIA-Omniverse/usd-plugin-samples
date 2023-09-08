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
#ifndef HD_OMNI_GEOSPATIAL_WGS84_LOCAL_POSITION_DATA_SOURCE_H_
#define HD_OMNI_GEOSPATIAL_WGS84_LOCAL_POSITION_DATA_SOURCE_H_

#include <pxr/imaging/hd/dataSource.h>
#include <pxr/usdImaging/usdImaging/dataSourceStageGlobals.h>

#include <omniGeospatial/wGS84LocalPositionAPI.h>

#include "localPositionSchema.h"

PXR_NAMESPACE_OPEN_SCOPE

class HdOmniGeospatialWGS84LocalPositionDataSource : public HdContainerDataSource
{
public:
    HD_DECLARE_DATASOURCE(HdOmniGeospatialWGS84LocalPositionDataSource);

    HdOmniGeospatialWGS84LocalPositionDataSource(const UsdPrim& prim, 
        const UsdImagingDataSourceStageGlobals& stageGlobals);

    TfTokenVector GetNames() override;
    HdDataSourceBaseHandle Get(const TfToken& name) override;

#if PXR_VERSION < 2302
    bool Has(const TfToken& name) override;
#endif

private:
    OmniGeospatialWGS84LocalPositionAPI _localPositionApi;
    const UsdImagingDataSourceStageGlobals& _stageGlobals;
};

HD_DECLARE_DATASOURCE_HANDLES(HdOmniGeospatialWGS84LocalPositionDataSource);

PXR_NAMESPACE_CLOSE_SCOPE

#endif // HD_OMNI_GEOSPATIAL_WGS84_LOCAL_POSITION_DATA_SOURCE_H_