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

#include "metricsDoubleDataSource.h"

PXR_NAMESPACE_OPEN_SCOPE

HdOmniMetricsDoubleDataSource::HdOmniMetricsDoubleDataSource(double value) : _value(value)
{
}

VtValue HdOmniMetricsDoubleDataSource::GetValue(Time shutterOffset)
{
    return VtValue(this->GetTypedValue(shutterOffset));
}

double HdOmniMetricsDoubleDataSource::GetTypedValue(Time shutterOffset)
{
    return _value;
}

bool HdOmniMetricsDoubleDataSource::GetContributingSampleTimesForInterval(
    Time startTime,
    Time endTime,
    std::vector<Time>* outSampleTimes)
{
    return false;
}

PXR_NAMESPACE_CLOSE_SCOPE