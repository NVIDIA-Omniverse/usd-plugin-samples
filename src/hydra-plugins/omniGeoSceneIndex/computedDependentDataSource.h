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
#ifndef HD_OMNI_GEOSPATIAL_COMPUTED_DEPENDENT_DATA_SOURCE_H_
#define HD_OMNI_GEOSPATIAL_COMPUTED_DEPENDENT_DATA_SOURCE_H_

#include <pxr/imaging/hd/dataSource.h>
#include <pxr/imaging/hd/dataSourceTypeDefs.h>

PXR_NAMESPACE_OPEN_SCOPE

/// \class HdOmniGeospatialComputedDependentDataSource
///
/// A datasource representing a container data source mimicing
/// that of a container data source for xform data, but returning
/// computed values based on geospatial data applied to the parent
/// (or some parent in the hierarchy) of this prim.
///
class HdOmniGeospatialComputedDependentDataSource : public HdContainerDataSource
{
public:

    HD_DECLARE_DATASOURCE(HdOmniGeospatialComputedDependentDataSource);

    HdOmniGeospatialComputedDependentDataSource(HdContainerDataSourceHandle inputDataSource,
        HdContainerDataSourceHandle parentDataSource);

    // data source overrides
    TfTokenVector GetNames() override;
    HdDataSourceBaseHandle Get(const TfToken& name) override;

#if PXR_VERSION < 2302
    bool Has(const TfToken& name) override;
#endif

private:

    HdDataSourceBaseHandle _ComputeGeospatiallyAffectedXform();

private:

    HdContainerDataSourceHandle _inputDataSource;
    HdContainerDataSourceHandle _parentDataSource;
    HdMatrixDataSourceHandle _matrixDataSource;

    class _GeospatiallyAffectedMatrixDataSource : public HdMatrixDataSource
    {
    public:

        HD_DECLARE_DATASOURCE(_GeospatiallyAffectedMatrixDataSource);

        VtValue GetValue(Time shutterOffset) override;
        GfMatrix4d GetTypedValue(Time shutterOffset) override;
        bool GetContributingSampleTimesForInterval(
            Time startTime,
            Time endTime,
            std::vector<Time>* outSampleTimes) override;    

    private:

        _GeospatiallyAffectedMatrixDataSource(HdContainerDataSourceHandle inputDataSource,
            HdContainerDataSourceHandle parentDataSource);

        HdMatrixDataSourceHandle _GetMatrixSource() const;
        HdBoolDataSourceHandle _GetResetXformStackSource() const;
        HdMatrixDataSourceHandle _GetParentMatrixSource() const;
        HdMatrixDataSourceHandle _GetParentOriginalMatrixSource() const;
        GfMatrix4d _GetMatrix(const Time shutterOffset) const;
        bool _GetResetXformStack(const Time shutterOffset) const;
        GfMatrix4d _GetParentMatrix(const Time shutterOffset) const;
        GfMatrix4d _GetParentOriginalMatrix(const Time shutterOffset) const;

        // geospatial transform methods
        GfMatrix4d _ComputeTransformedMatrix(const Time shutterOffset) const;

        HdContainerDataSourceHandle _inputDataSource;
        HdContainerDataSourceHandle _parentDataSource;
    };

    HD_DECLARE_DATASOURCE_HANDLES(_GeospatiallyAffectedMatrixDataSource);
};

HD_DECLARE_DATASOURCE_HANDLES(HdOmniGeospatialComputedDependentDataSource);

PXR_NAMESPACE_CLOSE_SCOPE

#endif // HD_OMNI_GEOSPATIAL_COMPUTED_DEPENDENT_DATA_SOURCE_H_