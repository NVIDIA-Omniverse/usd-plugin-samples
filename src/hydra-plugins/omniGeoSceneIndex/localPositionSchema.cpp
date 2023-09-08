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

#include "localPositionSchema.h"

PXR_NAMESPACE_OPEN_SCOPE

TF_DEFINE_PUBLIC_TOKENS(HdOmniGeospatialWGS84LocalPositionSchemaTokens,
    HDOMNIGEOSPATIALWGS84LOCALPOSITION_SCHEMA_TOKENS);

HdVec3dDataSourceHandle HdOmniGeospatialWGS84LocalPositionSchema::GetPosition()
{
    return _GetTypedDataSource<HdVec3dDataSource>(
        HdOmniGeospatialWGS84LocalPositionSchemaTokens->position);
}

HdOmniGeospatialWGS84LocalPositionSchema HdOmniGeospatialWGS84LocalPositionSchema::GetFromParent(
    const HdContainerDataSourceHandle& fromParentContainer)
{
    if (fromParentContainer == nullptr)
    {
        return HdOmniGeospatialWGS84LocalPositionSchema(nullptr);
    }

    return HdOmniGeospatialWGS84LocalPositionSchema(
        HdContainerDataSource::Cast(fromParentContainer->Get(
            HdOmniGeospatialWGS84LocalPositionSchemaTokens->localPositionApi))
    );
}

const HdDataSourceLocator& HdOmniGeospatialWGS84LocalPositionSchema::GetDefaultLocator()
{
    static const HdDataSourceLocator locator(
        HdOmniGeospatialWGS84LocalPositionSchemaTokens->localPositionApi
    );

    return locator;
}

HdContainerDataSourceHandle HdOmniGeospatialWGS84LocalPositionSchema::BuildRetained(
    const HdVec3dDataSourceHandle& position)
{
    TfToken names[1];
    HdDataSourceBaseHandle values[1];
    size_t count = 0;
    if (position != nullptr)
    {
        names[count] = HdOmniGeospatialWGS84LocalPositionSchemaTokens->position;
        values[count] = position;
        count++;
    }

    return HdRetainedContainerDataSource::New(count, names, values);
}

PXR_NAMESPACE_CLOSE_SCOPE