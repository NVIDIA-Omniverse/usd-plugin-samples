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
#ifndef HD_OMNI_METRICS_DATA_SOURCE_H_
#define HD_OMNI_METRICS_DATA_SOURCE_H_

#include <pxr/imaging/hd/dataSource.h>
#include <pxr/imaging/hd/dataSourceTypeDefs.h>
#include <pxr/imaging/hd/sceneIndex.h>

#include "api.h"

PXR_NAMESPACE_OPEN_SCOPE

//-----------------------------------------------------------------------------

#define HDOMNIMETRICSDATASOURCE_TOKENS \
    (metricsPreservedXform)

TF_DECLARE_PUBLIC_TOKENS(HdOmniMetricsDataSourceTokens, OMNIMETRICSASSEMBLER_API,
    HDOMNIMETRICSDATASOURCE_TOKENS);

//-----------------------------------------------------------------------------

/// \class HdOmniMetricsDataSource
///
/// A datasource representing a wrapped view of an existing flattened
/// datasource where the xform token is intercepted and a new metric-corrected
/// transform matrix is dynamically computed.
///
class HdOmniMetricsDataSource : public HdContainerDataSource
{
public:

    HD_DECLARE_DATASOURCE(HdOmniMetricsDataSource);

    HdOmniMetricsDataSource(const HdSceneIndexBase& sceneIndex, const SdfPath& primPath,
        HdContainerDataSourceHandle wrappedDataSource);

    void UpdateWrappedDataSource(HdContainerDataSourceHandle wrappedDataSource);

    // data source overrides
    TfTokenVector GetNames() override;
    HdDataSourceBaseHandle Get(const TfToken& name) override;

    // determines if the data source would be dirtied based on the locators given
    bool IsPrimDirtied(const HdDataSourceLocatorSet& locators);

private:

    bool _HasMetricsInformation(HdContainerDataSourceHandle dataSource);
    HdBoolDataSourceHandle _GetInputResetXformStackSource();
    HdDataSourceBaseHandle _ComputeCorrectedXform();

private:

    const HdSceneIndexBase& _sceneIndex;
    SdfPath _primPath;
    HdContainerDataSourceHandle _wrappedDataSource;

    // cached computed datasources
    HdContainerDataSourceAtomicHandle _computedCorrectedXformDataSource;

    class _MetricsCorrectedMatrixDataSource : public HdMatrixDataSource
    {
    public:
        HD_DECLARE_DATASOURCE(_MetricsCorrectedMatrixDataSource);

        _MetricsCorrectedMatrixDataSource(HdContainerDataSourceHandle inputDataSource,
            HdContainerDataSourceHandle parentDataSource,
            bool isMetricsCorrectiveSource);

        // typed sampled data source overrides
        VtValue GetValue(Time shutterOffset) override;
        GfMatrix4d GetTypedValue(Time shutterOffset) override;
        bool GetContributingSampleTimesForInterval(
            Time startTime,
            Time endTime,
            std::vector<Time>* outSampleTimes) override;

    private:

        HdMatrixDataSourceHandle _GetInputMatrixDataSource() const;
        HdMatrixDataSourceHandle _GetParentMatrixDataSource() const;
        HdMatrixDataSourceHandle _GetMetricsPreservedMatrixDataSource() const;
        HdMatrixDataSourceHandle _GetParentMetricsPreservedMatrixDataSource() const;
        GfMatrix4d _ComputeCorrectedMatrix(Time shutterOffset);
        GfMatrix4d _GetMpuCorrective();

        HdContainerDataSourceHandle _inputDataSource;
        HdContainerDataSourceHandle _parentDataSource;
        bool _isMetricsCorrectiveSource;
    };

    HD_DECLARE_DATASOURCE_HANDLES(_MetricsCorrectedMatrixDataSource);
};

HD_DECLARE_DATASOURCE_HANDLES(HdOmniMetricsDataSource);

PXR_NAMESPACE_CLOSE_SCOPE

#endif // HD_OMNI_METRICS_DATA_SOURCE_H_