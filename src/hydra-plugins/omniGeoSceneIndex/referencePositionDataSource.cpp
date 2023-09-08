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

#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usdGeom/tokens.h>
#include <pxr/usdImaging/usdImaging/dataSourceAttribute.h>

#include "referencePositionDataSource.h"

PXR_NAMESPACE_OPEN_SCOPE

HdOmniGeospatialWGS84ReferencePositionDataSource::HdOmniGeospatialWGS84ReferencePositionDataSource(
    const UsdPrim& prim,
    const UsdImagingDataSourceStageGlobals& stageGlobals) :
    _stageGlobals(stageGlobals)
{
    _referencePositionApi = OmniGeospatialWGS84ReferencePositionAPI(prim);
}

#if PXR_VERSION < 2302
bool HdOmniGeospatialWGS84ReferencePositionDataSource::Has(const TfToken& name)
{
    return (name == HdOmniGeospatialWGS84ReferencePositionSchemaTokens->tangentPlane) ||
        (name == HdOmniGeospatialWGS84ReferencePositionSchemaTokens->referencePosition) ||
        (name == HdOmniGeospatialWGS84ReferencePositionSchemaTokens->orientation) ||
        (name == HdOmniGeospatialWGS84ReferencePositionSchemaTokens->stageUpAxis) ||
        (name == HdOmniGeospatialWGS84ReferencePositionSchemaTokens->stageMetersPerUnit);
}
#endif

TfTokenVector HdOmniGeospatialWGS84ReferencePositionDataSource::GetNames()
{
    // return the hydra attribute names this data source is responsible for
    TfTokenVector names;
    names.push_back(HdOmniGeospatialWGS84ReferencePositionSchemaTokens->tangentPlane);
    names.push_back(HdOmniGeospatialWGS84ReferencePositionSchemaTokens->referencePosition);
    names.push_back(HdOmniGeospatialWGS84ReferencePositionSchemaTokens->orientation);
    names.push_back(HdOmniGeospatialWGS84ReferencePositionSchemaTokens->stageUpAxis);
    names.push_back(HdOmniGeospatialWGS84ReferencePositionSchemaTokens->stageMetersPerUnit);

    return names;
}

HdDataSourceBaseHandle HdOmniGeospatialWGS84ReferencePositionDataSource::Get(const TfToken& name)
{
    // retrieves the data source values for the attributes this data source
    // supports
    if (name == HdOmniGeospatialWGS84ReferencePositionSchemaTokens->tangentPlane)
    {
        return UsdImagingDataSourceAttribute<TfToken>::New(
            _referencePositionApi.GetTangentPlaneAttr(), _stageGlobals);
    }
    else if (name == HdOmniGeospatialWGS84ReferencePositionSchemaTokens->referencePosition)
    {
        return UsdImagingDataSourceAttribute<GfVec3d>::New(
            _referencePositionApi.GetReferencePositionAttr(), _stageGlobals);
    }
    else if (name == HdOmniGeospatialWGS84ReferencePositionSchemaTokens->orientation)
    {
        return UsdImagingDataSourceAttribute<GfVec3d>::New(
            _referencePositionApi.GetOrientationAttr(), _stageGlobals);
    }
    else if (name == HdOmniGeospatialWGS84ReferencePositionSchemaTokens->stageUpAxis)
    {
        TfToken upAxis = UsdGeomTokens->y;
        UsdStageWeakPtr stage = _referencePositionApi.GetPrim().GetStage();
        if (stage != nullptr)
        {
            upAxis = UsdGeomGetStageUpAxis(stage);
        }

        return _StageDataSource<TfToken>::New(upAxis);
    }
    else if (name == HdOmniGeospatialWGS84ReferencePositionSchemaTokens->stageMetersPerUnit)
    {
        double mpu = 0.01;
        UsdStageWeakPtr stage = _referencePositionApi.GetPrim().GetStage();
        if (stage != nullptr)
        {
            mpu = UsdGeomGetStageMetersPerUnit(stage);
        }

        return _StageDataSource<double>::New(mpu);
    }

    // this is a name we don't support
    return nullptr;
}

template <typename T>
HdOmniGeospatialWGS84ReferencePositionDataSource::_StageDataSource<T>::_StageDataSource(const T& value) : _value(value)
{
}

PXR_NAMESPACE_CLOSE_SCOPE