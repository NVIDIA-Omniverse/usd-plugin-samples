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

#include <pxr/base/gf/transform.h>
#include <pxr/usd/usdGeom/tokens.h>
#include <pxr/imaging/hd/xformSchema.h>

#include "computedPrimDataSource.h"
#include "localPositionSchema.h"
#include "referencePositionSchema.h"

PXR_NAMESPACE_OPEN_SCOPE

HdOmniGeospatialComputedPrimDataSource::HdOmniGeospatialComputedPrimDataSource(
    HdContainerDataSourceHandle inputDataSource) :
    _inputDataSource(inputDataSource)
{
    _matrixDataSource = 
        HdOmniGeospatialComputedPrimDataSource::_GeospatialMatrixDataSource::New(_inputDataSource);
}

#if PXR_VERSION < 2302
bool HdOmniGeospatialComputedPrimDataSource::Has(const TfToken& name)
{
    return (name == HdXformSchemaTokens->resetXformStack) || 
        (name == HdXformSchemaTokens->matrix);
}
#endif

TfTokenVector HdOmniGeospatialComputedPrimDataSource::GetNames()
{
    // this container data source retrieves the xform tokens
    TfTokenVector result;
    result.push_back(HdXformSchemaTokens->resetXformStack);
    result.push_back(HdXformSchemaTokens->matrix);

    return result;
}

HdDataSourceBaseHandle HdOmniGeospatialComputedPrimDataSource::Get(const TfToken& name)
{
    if (_inputDataSource != nullptr)
    {
        if (name == HdXformSchemaTokens->resetXformStack)
        {
            // we don't modify the underlying time-sampled data
            // for resetXformStack, so return that directly
            HdXformSchema xformSchema = HdXformSchema::GetFromParent(_inputDataSource);
            return xformSchema.IsDefined() ? xformSchema.GetResetXformStack() : nullptr;
        }
        else if (name == HdXformSchemaTokens->matrix)
        {
            // note even if resetXformStack was true we consider
            // the geospatial data to override that
            return _matrixDataSource;
        }
    }

    return nullptr;
}

HdOmniGeospatialComputedPrimDataSource::_GeospatialMatrixDataSource::_GeospatialMatrixDataSource(
    HdContainerDataSourceHandle inputDataSource) : _inputDataSource(inputDataSource)
{
}

VtValue HdOmniGeospatialComputedPrimDataSource::_GeospatialMatrixDataSource::GetValue(Time shutterOffset)
{
    return VtValue(this->GetTypedValue(shutterOffset));
}

GfMatrix4d HdOmniGeospatialComputedPrimDataSource::_GeospatialMatrixDataSource::GetTypedValue(Time shutterOffset)
{
    return this->_ComputeTransformedMatrix(shutterOffset);
}

bool HdOmniGeospatialComputedPrimDataSource::_GeospatialMatrixDataSource::GetContributingSampleTimesForInterval(
    Time startTime,
    Time endTime,
    std::vector<Time>* outSampleTimes)
{
    HdSampledDataSourceHandle sources[] = {
        this->_GetMatrixSource(),
        this->_GetLocalPositionSource()
    };

    return HdGetMergedContributingSampleTimesForInterval(
        TfArraySize(sources),
        sources,
        startTime,
        endTime,
        outSampleTimes);
}

HdMatrixDataSourceHandle HdOmniGeospatialComputedPrimDataSource::_GeospatialMatrixDataSource::_GetMatrixSource() const
{
    return HdXformSchema::GetFromParent(_inputDataSource).GetMatrix();
}

HdVec3dDataSourceHandle HdOmniGeospatialComputedPrimDataSource::_GeospatialMatrixDataSource::_GetLocalPositionSource() const
{
    return HdOmniGeospatialWGS84LocalPositionSchema::GetFromParent(_inputDataSource).GetPosition();
}

HdTokenDataSourceHandle HdOmniGeospatialComputedPrimDataSource::_GeospatialMatrixDataSource::_GetTangentPlaneSource() const
{
    return HdOmniGeospatialWGS84ReferencePositionSchema::GetFromParent(_inputDataSource).GetTangentPlane();
}

HdVec3dDataSourceHandle HdOmniGeospatialComputedPrimDataSource::_GeospatialMatrixDataSource::_GetReferencePositionSource() const
{
    return HdOmniGeospatialWGS84ReferencePositionSchema::GetFromParent(_inputDataSource).GetReferencePosition();
}

HdVec3dDataSourceHandle HdOmniGeospatialComputedPrimDataSource::_GeospatialMatrixDataSource::_GetOrientationSource() const
{
    return HdOmniGeospatialWGS84ReferencePositionSchema::GetFromParent(_inputDataSource).GetOrientation();
}

HdTokenDataSourceHandle HdOmniGeospatialComputedPrimDataSource::_GeospatialMatrixDataSource::_GetStageUpAxisSource() const
{
    return HdOmniGeospatialWGS84ReferencePositionSchema::GetFromParent(_inputDataSource).GetStageUpAxis();
}

HdDoubleDataSourceHandle HdOmniGeospatialComputedPrimDataSource::_GeospatialMatrixDataSource::_GetStageMetersPerUnitSource() const
{
    return HdOmniGeospatialWGS84ReferencePositionSchema::GetFromParent(_inputDataSource).GetStageMetersPerUnit();
}

GfMatrix4d HdOmniGeospatialComputedPrimDataSource::_GeospatialMatrixDataSource::_GetMatrix(const Time shutterOffset) const
{
    HdMatrixDataSourceHandle dataSource = this->_GetMatrixSource();
    if (dataSource != nullptr)
    {
        return dataSource->GetTypedValue(shutterOffset);
    }

    return GfMatrix4d(1.0);
}

GfVec3d HdOmniGeospatialComputedPrimDataSource::_GeospatialMatrixDataSource::_GetLocalPosition(const Time shutterOffset) const
{
    HdVec3dDataSourceHandle dataSource = this->_GetLocalPositionSource();
    if (dataSource != nullptr)
    {
        return dataSource->GetTypedValue(shutterOffset);
    }

    return GfVec3d(1.0);
}

TfToken HdOmniGeospatialComputedPrimDataSource::_GeospatialMatrixDataSource::_GetTangentPlane() const
{
    HdTokenDataSourceHandle dataSource = this->_GetTangentPlaneSource();
    if (dataSource != nullptr)
    {
        return dataSource->GetTypedValue(0.0f);
    }

    return TfToken();
}

GfVec3d HdOmniGeospatialComputedPrimDataSource::_GeospatialMatrixDataSource::_GetReferencePosition() const
{
    HdVec3dDataSourceHandle dataSource = this->_GetReferencePositionSource();
    if (dataSource != nullptr)
    {
        return dataSource->GetTypedValue(0.0f);
    }

    return GfVec3d(1.0);
}

GfVec3d HdOmniGeospatialComputedPrimDataSource::_GeospatialMatrixDataSource::_GetOrientation() const
{
    HdVec3dDataSourceHandle dataSource = this->_GetOrientationSource();
    if (dataSource != nullptr)
    {
        return dataSource->GetTypedValue(0.0f);
    }

    return GfVec3d(1.0);
}

TfToken HdOmniGeospatialComputedPrimDataSource::_GeospatialMatrixDataSource::_GetStageUpAxis() const
{
    HdTokenDataSourceHandle dataSource = this->_GetStageUpAxisSource();
    if (dataSource != nullptr)
    {
        return dataSource->GetTypedValue(0.0f);
    }

    return UsdGeomTokens->y;
}

double HdOmniGeospatialComputedPrimDataSource::_GeospatialMatrixDataSource::_GetStageMetersPerUnit() const
{
    HdDoubleDataSourceHandle dataSource = this->_GetStageMetersPerUnitSource();
    if (dataSource != nullptr)
    {
        return dataSource->GetTypedValue(0.0f);
    }

    return 0.01;
}

GfMatrix4d HdOmniGeospatialComputedPrimDataSource::_GeospatialMatrixDataSource::_ComputeTransformedMatrix(const Time shutterOffset) const
{
    // NOTE: in the case of the geospatially applied prim, we are completely
    // ignoring the fact that resetXformStack may be true at any given time sample
    // that is, geospatial positioning takes priority over local transformation reset

    // to compute the local position, we need to first get the geodetic reference
    TfToken targetFrame = this->_GetTangentPlane();
    GfVec3d tangentPosition = this->_GetReferencePosition();
    GfVec3d orientation = this->_GetOrientation();
    GfVec3d localPosition = this->_GetLocalPosition(shutterOffset);
    double metersPerUnit = this->_GetStageMetersPerUnit();
    TfToken upAxis = this->_GetStageUpAxis();

    // calculate the new geodetic translation
    auto enu = this->_EcefToEnu(this->_GeodeticToEcef(localPosition), tangentPosition);
    GfVec3d translation = this->_EnuToCartesian(enu, upAxis, metersPerUnit, tangentPosition);

    // we only want to replace the translation piece
    // but since the transform may have orientation and scale
    // information, we need to extract that from the existing
    // matrix first
    GfTransform currentTransform(this->_GetMatrix(shutterOffset));
    GfVec3d existingScale = currentTransform.GetScale();
    GfRotation existingRotation = currentTransform.GetRotation();
    GfRotation existingPivotOrientation = currentTransform.GetPivotOrientation();
    GfVec3d existingPivotPosition = currentTransform.GetPivotPosition();

    // now combine the new translation with the existing scale / rotation
    GfTransform newTransform(existingScale, existingPivotOrientation,
        existingRotation, existingPivotPosition, translation);

    return newTransform.GetMatrix();
}

// Geospatial transform functions
// For reference:
// https://onlinelibrary.wiley.com/doi/pdf/10.1002/9780470099728.app3
// https://en.wikipedia.org/wiki/Geographic_coordinate_conversion
// Implementation of Ferrari's solution
GfVec3d HdOmniGeospatialComputedPrimDataSource::_GeospatialMatrixDataSource::_GeodeticToEcef(const GfVec3d & llh) const
{
    double lambda = llh[0] * GeoConstants::radians;
    double phi = llh[1] * GeoConstants::radians;

    double sin_lambda = sin(lambda);
    double N = GeoConstants::semiMajorAxis / sqrt(1 - GeoConstants::eccentricity * sin_lambda * sin_lambda);

    double cos_lambda = cos(lambda);
    double cos_phi = cos(phi);
    double sin_phi = sin(phi);

    return PXR_NS::GfVec3d((llh[2] + N) * cos_lambda * cos_phi, (llh[2] + N) * cos_lambda * sin_phi,
        (llh[2] + (1 - GeoConstants::eccentricity) * N) * sin_lambda);
}

GfVec3d HdOmniGeospatialComputedPrimDataSource::_GeospatialMatrixDataSource::_EcefToEnu(const GfVec3d& ecef, const GfVec3d& llh) const
{
    double lambda = llh[0] * GeoConstants::radians;
    double phi = llh[1] * GeoConstants::radians;

    double sin_lambda = sin(lambda);
    double N = GeoConstants::semiMajorAxis / sqrt(1 - GeoConstants::eccentricity * sin_lambda * sin_lambda);

    double cos_lambda = cos(lambda);
    double cos_phi = cos(phi);
    double sin_phi = sin(phi);

    PXR_NS::GfVec3d pt((llh[2] + N) * cos_lambda * cos_phi,
        (llh[2] + N) * cos_lambda * sin_phi,
        (llh[2] + (1 - GeoConstants::eccentricity) * N) * sin_lambda);

    auto delta = ecef - pt;

    return PXR_NS::GfVec3d(-sin_phi * delta[0] + cos_phi * delta[1],
        -cos_phi * sin_lambda * delta[0] - sin_lambda * sin_phi * delta[1] + cos_lambda * delta[2],
        cos_lambda * cos_phi * delta[0] + cos_lambda * sin_phi * delta[1] + sin_lambda * delta[2]);

}

GfVec3d HdOmniGeospatialComputedPrimDataSource::_GeospatialMatrixDataSource::_EnuToCartesian(
    const GfVec3d& enu,
    const TfToken& upAxis,
    const double& metersPerUnit,
    const GfVec3d& reference) const
{
    auto cartesian = GfVec3d(reference[0] < 0.0 ? -enu[0] : enu[0],
        upAxis == UsdGeomTokens->y ? enu[2] : enu[1],
        upAxis == UsdGeomTokens->z ? enu[2] : enu[1]);

    cartesian /= metersPerUnit;
    return cartesian;
}

PXR_NAMESPACE_CLOSE_SCOPE