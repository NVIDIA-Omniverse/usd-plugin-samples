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
#ifndef HD_OMNI_METRICS_DOUBLE_DATA_SOURCE_H_
#define HD_OMNI_METRICS_DOUBLE_DATA_SOURCE_H_

#include <pxr/imaging/hd/dataSource.h>
#include <pxr/imaging/hd/dataSourceTypeDefs.h>

PXR_NAMESPACE_OPEN_SCOPE

///
/// \class HdOmniMetricsDoubleDataSource
///
/// Concrete implementation for a simple data source that
/// holds a uniform double value.
///
class HdOmniMetricsDoubleDataSource : public HdDoubleDataSource
{
public:
    HD_DECLARE_DATASOURCE(HdOmniMetricsDoubleDataSource);

    VtValue GetValue(Time shutterOffset) override;
    double GetTypedValue(Time shutterOffset) override;
    bool GetContributingSampleTimesForInterval(
        Time startTime,
        Time endTime,
        std::vector<Time>* outSampleTimes) override;

private:

    HdOmniMetricsDoubleDataSource(double value);

    double _value;
};

HD_DECLARE_DATASOURCE_HANDLES(HdOmniMetricsDoubleDataSource);

PXR_NAMESPACE_CLOSE_SCOPE

#endif // HD_OMNI_METRICS_DOUBLE_DATA_SOURCE_H_