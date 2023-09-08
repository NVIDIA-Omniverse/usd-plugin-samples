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

#include "metricsSchema.h"

PXR_NAMESPACE_OPEN_SCOPE

TF_DEFINE_PUBLIC_TOKENS(HdOmniMetricsSchemaTokens, HDOMNI_METRICS_SCHEMA_TOKENS);

HdOmniMetricsSchema::HdOmniMetricsSchema(HdContainerDataSourceHandle container) :
    HdSchema(container)
{
}

HdDoubleDataSourceHandle HdOmniMetricsSchema::GetLayerMpu()
{
    return _GetTypedDataSource<HdDoubleDataSource>(HdOmniMetricsSchemaTokens->layerMpu);
}

HdDoubleDataSourceHandle HdOmniMetricsSchema::GetStageMpu()
{
    return _GetTypedDataSource<HdDoubleDataSource>(HdOmniMetricsSchemaTokens->stageMpu);
}

HdContainerDataSourceHandle HdOmniMetricsSchema::BuildRetained(
    const HdDoubleDataSourceHandle& layerMpu,
    const HdDoubleDataSourceHandle& stageMpu)
{
    TfToken names[2];
    HdDataSourceBaseHandle values[2];

    size_t count = 0;
    if(layerMpu != nullptr)
    {
        names[count] = HdOmniMetricsSchemaTokens->layerMpu;
        values[count++] = layerMpu;
    }

    if (stageMpu != nullptr)
    {
        names[count] = HdOmniMetricsSchemaTokens->stageMpu;
        values[count++] = stageMpu;
    }

    return HdRetainedContainerDataSource::New(count, names, values);
}

HdOmniMetricsSchema HdOmniMetricsSchema::GetFromParent(const HdContainerDataSourceHandle& fromParentContainer)
{
    return HdOmniMetricsSchema(fromParentContainer ? 
        HdContainerDataSource::Cast(fromParentContainer->Get(HdOmniMetricsSchemaTokens->metrics))
        : nullptr);
}

const HdDataSourceLocator& HdOmniMetricsSchema::GetDefaultLocator()
{
    static const HdDataSourceLocator locator(HdOmniMetricsSchemaTokens->metrics);

    return locator;
}

HdOmniMetricsSchema::Builder& HdOmniMetricsSchema::Builder::SetLayerMpu(const HdDoubleDataSourceHandle& layerMpu)
{
    _layerMpu = layerMpu;
    return *this;
}

HdOmniMetricsSchema::Builder& HdOmniMetricsSchema::Builder::SetStageMpu(const HdDoubleDataSourceHandle& stageMpu)
{
    _stageMpu = stageMpu;
    return *this;
}

HdContainerDataSourceHandle HdOmniMetricsSchema::Builder::Build()
{
    return HdOmniMetricsSchema::BuildRetained(
        _layerMpu,
        _stageMpu
    );
}

PXR_NAMESPACE_CLOSE_SCOPE