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
#ifndef HD_OMNI_GEOSPATIAL_WGS84_REFERENCE_POSITION_SCHEMA_H_
#define HD_OMNI_GEOSPATIAL_WGS84_REFERENCE_POSITION_SCHEMA_H_

#include <pxr/imaging/hd/schema.h>
#include <pxr/imaging/hd/dataSourceLocator.h>

#include "api.h"

PXR_NAMESPACE_OPEN_SCOPE

//-----------------------------------------------------------------------------

#define HDOMNIGEOSPATIALWGS84REFERENCEPOSITION_SCHEMA_TOKENS \
    (referencePositionApi) \
    (tangentPlane) \
    (referencePosition) \
    (orientation) \
    (stageUpAxis) \
    (stageMetersPerUnit) \

TF_DECLARE_PUBLIC_TOKENS(HdOmniGeospatialWGS84ReferencePositionSchemaTokens, OMNIGEOSCENEINDEX_API,
    HDOMNIGEOSPATIALWGS84REFERENCEPOSITION_SCHEMA_TOKENS);

//-----------------------------------------------------------------------------

class HdOmniGeospatialWGS84ReferencePositionSchema : public HdSchema
{
public:
    HdOmniGeospatialWGS84ReferencePositionSchema(HdContainerDataSourceHandle container)
        : HdSchema(container) { }

    OMNIGEOSCENEINDEX_API
    HdTokenDataSourceHandle GetTangentPlane();

    OMNIGEOSCENEINDEX_API
    HdVec3dDataSourceHandle GetReferencePosition();
    
    OMNIGEOSCENEINDEX_API
    HdVec3dDataSourceHandle GetOrientation();

    OMNIGEOSCENEINDEX_API
    HdTokenDataSourceHandle GetStageUpAxis();

    OMNIGEOSCENEINDEX_API
    HdDoubleDataSourceHandle GetStageMetersPerUnit();

    OMNIGEOSCENEINDEX_API
    static HdOmniGeospatialWGS84ReferencePositionSchema GetFromParent(
        const HdContainerDataSourceHandle& fromParentContainer);

    OMNIGEOSCENEINDEX_API
    static const HdDataSourceLocator& GetDefaultLocator();

    OMNIGEOSCENEINDEX_API
    static HdContainerDataSourceHandle BuildRetained(
        const HdTokenDataSourceHandle& tangentPlane,
        const HdVec3dDataSourceHandle& referencePosition,
        const HdVec3dDataSourceHandle& orientation,
        const HdTokenDataSourceHandle& stageUpAxis,
        const HdDoubleDataSourceHandle& stageMetersPerUnit
    );
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // HD_OMNI_GEOSPATIAL_WGS84_REFERENCE_POSITION_SCHEMA_H_