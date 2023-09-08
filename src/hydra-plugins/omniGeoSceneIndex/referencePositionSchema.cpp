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
#include <pxr/imaging/hd/retainedDataSource.h>

#include "referencePositionSchema.h"

PXR_NAMESPACE_OPEN_SCOPE

TF_DEFINE_PUBLIC_TOKENS(HdOmniGeospatialWGS84ReferencePositionSchemaTokens,
    HDOMNIGEOSPATIALWGS84REFERENCEPOSITION_SCHEMA_TOKENS);

HdTokenDataSourceHandle HdOmniGeospatialWGS84ReferencePositionSchema::GetTangentPlane()
{
    return _GetTypedDataSource<HdTokenDataSource>(
        HdOmniGeospatialWGS84ReferencePositionSchemaTokens->tangentPlane);
}

HdVec3dDataSourceHandle HdOmniGeospatialWGS84ReferencePositionSchema::GetReferencePosition()
{
    return _GetTypedDataSource<HdVec3dDataSource>(
        HdOmniGeospatialWGS84ReferencePositionSchemaTokens->referencePosition);
}
    
HdVec3dDataSourceHandle HdOmniGeospatialWGS84ReferencePositionSchema::GetOrientation()
{
    return _GetTypedDataSource<HdVec3dDataSource>(
        HdOmniGeospatialWGS84ReferencePositionSchemaTokens->orientation);
}

HdTokenDataSourceHandle HdOmniGeospatialWGS84ReferencePositionSchema::GetStageUpAxis()
{
    return _GetTypedDataSource<HdTokenDataSource>(
        HdOmniGeospatialWGS84ReferencePositionSchemaTokens->stageUpAxis);
}

HdDoubleDataSourceHandle HdOmniGeospatialWGS84ReferencePositionSchema::GetStageMetersPerUnit()
{
    return _GetTypedDataSource<HdDoubleDataSource>(
        HdOmniGeospatialWGS84ReferencePositionSchemaTokens->stageMetersPerUnit);
}

HdOmniGeospatialWGS84ReferencePositionSchema HdOmniGeospatialWGS84ReferencePositionSchema::GetFromParent(
    const HdContainerDataSourceHandle& fromParentContainer)
{
    if (fromParentContainer == nullptr)
    {
        return HdOmniGeospatialWGS84ReferencePositionSchema(nullptr);
    }

    return HdOmniGeospatialWGS84ReferencePositionSchema(
        HdContainerDataSource::Cast(fromParentContainer->Get(
            HdOmniGeospatialWGS84ReferencePositionSchemaTokens->referencePositionApi))
    );
}

const HdDataSourceLocator& HdOmniGeospatialWGS84ReferencePositionSchema::GetDefaultLocator()
{
    static const HdDataSourceLocator locator(
        HdOmniGeospatialWGS84ReferencePositionSchemaTokens->referencePositionApi
    );

    return locator;
}

HdContainerDataSourceHandle HdOmniGeospatialWGS84ReferencePositionSchema::BuildRetained(
    const HdTokenDataSourceHandle& tangentPlane,
    const HdVec3dDataSourceHandle& referencePosition,
    const HdVec3dDataSourceHandle& orientation,
    const HdTokenDataSourceHandle& stageUpAxis,
    const HdDoubleDataSourceHandle& stageMetersPerUnit)
{
    TfToken names[5];
    HdDataSourceBaseHandle values[5];
    size_t count = 0;
    if (tangentPlane != nullptr)
    {
        names[count] = HdOmniGeospatialWGS84ReferencePositionSchemaTokens->tangentPlane;
        values[count] = tangentPlane;
        count++;
    }

    if (referencePosition != nullptr)
    {
        names[count] = HdOmniGeospatialWGS84ReferencePositionSchemaTokens->referencePosition;
        values[count] = referencePosition;
        count++;
    }

    if (orientation != nullptr)
    {
        names[count] = HdOmniGeospatialWGS84ReferencePositionSchemaTokens->orientation;
        values[count] = orientation;
        count++;
    }

    if (stageUpAxis != nullptr)
    {
        names[count] = HdOmniGeospatialWGS84ReferencePositionSchemaTokens->stageUpAxis;
        values[count] = stageUpAxis;
        count++;
    }

    if (stageMetersPerUnit != nullptr)
    {
        names[count] = HdOmniGeospatialWGS84ReferencePositionSchemaTokens->stageMetersPerUnit;
        values[count] = stageMetersPerUnit;
        count++;
    }

    return HdRetainedContainerDataSource::New(count, names, values);
}

PXR_NAMESPACE_CLOSE_SCOPE