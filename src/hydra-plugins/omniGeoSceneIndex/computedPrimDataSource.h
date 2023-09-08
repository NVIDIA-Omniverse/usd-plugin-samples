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
#ifndef HD_OMNI_GEOSPATIAL_COMPUTED_PRIM_DATA_SOURCE_H_
#define HD_OMNI_GEOSPATIAL_COMPUTED_PRIM_DATA_SOURCE_H_

#include <pxr/imaging/hd/dataSource.h>
#include <pxr/imaging/hd/dataSourceTypeDefs.h>

PXR_NAMESPACE_OPEN_SCOPE

/// \class HdOmniGeospatialComputedPrimDataSource
///
/// A datasource representing a container data source mimicing
/// that of a container data source for xform data, but returning
/// computed values based on geospatial data applied to the prim.
///
class HdOmniGeospatialComputedPrimDataSource : public HdContainerDataSource
{
public:

    HD_DECLARE_DATASOURCE(HdOmniGeospatialComputedPrimDataSource);

    HdOmniGeospatialComputedPrimDataSource(HdContainerDataSourceHandle inputDataSource);

    // data source overrides
    TfTokenVector GetNames() override;
    HdDataSourceBaseHandle Get(const TfToken& name) override;

#if PXR_VERSION < 2302
    bool Has(const TfToken& name) override;
#endif

private:

    HdDataSourceBaseHandle _ComputeGeospatialXform();
    GfVec3d _GeodeticToEcef(const GfVec3d& llh) const;
    GfVec3d _EcefToEnu(const GfVec3d& ecef, const GfVec3d& llh) const;
    GfVec3d _EnuToCartesian(const GfVec3d& enu, const TfToken& upAxis, const double& metersPerUnit, const GfVec3d& reference) const;

private:

    HdContainerDataSourceHandle _inputDataSource;
    HdMatrixDataSourceHandle _matrixDataSource;

    class _GeospatialMatrixDataSource : public HdMatrixDataSource
    {
    public:

        HD_DECLARE_DATASOURCE(_GeospatialMatrixDataSource);

        VtValue GetValue(Time shutterOffset) override;
        GfMatrix4d GetTypedValue(Time shutterOffset) override;
        bool GetContributingSampleTimesForInterval(
            Time startTime,
            Time endTime,
            std::vector<Time>* outSampleTimes) override;    

    private:

        _GeospatialMatrixDataSource(HdContainerDataSourceHandle inputDataSource);

        HdMatrixDataSourceHandle _GetMatrixSource() const;
        HdVec3dDataSourceHandle _GetLocalPositionSource() const;
        HdTokenDataSourceHandle _GetTangentPlaneSource() const;
        HdVec3dDataSourceHandle _GetReferencePositionSource() const;
        HdVec3dDataSourceHandle _GetOrientationSource() const;
        HdTokenDataSourceHandle _GetStageUpAxisSource() const;
        HdDoubleDataSourceHandle _GetStageMetersPerUnitSource() const;
        GfMatrix4d _GetMatrix(const Time shutterOffset) const;
        GfVec3d _GetLocalPosition(const Time shutterOffset) const;
        TfToken _GetTangentPlane() const;
        GfVec3d _GetReferencePosition() const;
        GfVec3d _GetOrientation() const;
        TfToken _GetStageUpAxis() const;
        double _GetStageMetersPerUnit() const;

        // geospatial transform methods
        GfMatrix4d _ComputeTransformedMatrix(const Time shutterOffset) const;
        GfVec3d _GeodeticToEcef(const GfVec3d& llh) const;
        GfVec3d _EcefToEnu(const GfVec3d& ecef, const GfVec3d& llh) const;
        GfVec3d _EnuToCartesian(const GfVec3d& enu, const TfToken& upAxis, const double& metersPerUnit, const GfVec3d& reference) const;

        struct GeoConstants
        {
            static constexpr double semiMajorAxis = 6378137.0;
            static constexpr double semiMinorAxis = 6356752.3142;
            static constexpr double flattening = 1.0 / 298.257223563;
            static constexpr double eccentricity = flattening * (2 - flattening);
            static constexpr double radians = M_PI / 180.0;
            static constexpr double degrees = 180.0 / M_PI;
        };

        HdContainerDataSourceHandle _inputDataSource;
    };

    HD_DECLARE_DATASOURCE_HANDLES(_GeospatialMatrixDataSource);
};

HD_DECLARE_DATASOURCE_HANDLES(HdOmniGeospatialComputedPrimDataSource);

PXR_NAMESPACE_CLOSE_SCOPE

#endif // HD_OMNI_GEOSPATIAL_COMPUTED_PRIM_DATA_SOURCE_H_