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
#ifndef HD_OMNI_GEOSPATIAL_WGS84_REFERENCE_POSITION_DATA_SOURCE_H_
#define HD_OMNI_GEOSPATIAL_WGS84_REFERENCE_POSITION_DATA_SOURCE_H_

#include <pxr/imaging/hd/dataSource.h>
#include <pxr/usdImaging/usdImaging/dataSourceStageGlobals.h>

#include <omniGeospatial/wGS84ReferencePositionAPI.h>

#include "referencePositionSchema.h"

PXR_NAMESPACE_OPEN_SCOPE

class HdOmniGeospatialWGS84ReferencePositionDataSource : public HdContainerDataSource
{
public:
    HD_DECLARE_DATASOURCE(HdOmniGeospatialWGS84ReferencePositionDataSource);

    HdOmniGeospatialWGS84ReferencePositionDataSource(const UsdPrim& prim,
        const UsdImagingDataSourceStageGlobals& stageGlobals);

    TfTokenVector GetNames() override;
    HdDataSourceBaseHandle Get(const TfToken& name) override;

#if PXR_VERSION < 2302
    bool Has(const TfToken& name) override;
#endif

private:
    OmniGeospatialWGS84ReferencePositionAPI _referencePositionApi;
    const UsdImagingDataSourceStageGlobals& _stageGlobals;

    template <typename T>
    class _StageDataSource : public HdTypedSampledDataSource<T>
    {
    public:

        HD_DECLARE_DATASOURCE(_StageDataSource<T>);

        VtValue GetValue(HdSampledDataSource::Time shutterOffset) override
        {
            return VtValue(GetTypedValue(shutterOffset));
        }

        T GetTypedValue(HdSampledDataSource::Time shutterOffset) override
        {
            return _value;
        }

        bool GetContributingSampleTimesForInterval(
            HdSampledDataSource::Time startTime,
            HdSampledDataSource::Time endTime,
            std::vector<HdSampledDataSource::Time>* outSampleTimes) override
        {
            return false;
        }

    private:

        _StageDataSource(const T& value);

        T _value;
    };
};

HD_DECLARE_DATASOURCE_HANDLES(HdOmniGeospatialWGS84ReferencePositionDataSource);

PXR_NAMESPACE_CLOSE_SCOPE

#endif // HD_OMNI_GEOSPATIAL_WGS84_REFERENCE_POSITION_DATA_SOURCE_H_