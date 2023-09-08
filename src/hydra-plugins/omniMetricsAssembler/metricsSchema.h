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
#ifndef OMNI_METRICS_SCHEMA_H_
#define OMNI_METRICS_SCHEMA_H_

#include <pxr/imaging/hd/schema.h>

#include "api.h"

PXR_NAMESPACE_OPEN_SCOPE

//-----------------------------------------------------------------------------

#define HDOMNI_METRICS_SCHEMA_TOKENS  \
    (metrics) \
    (layerMpu) \
    (stageMpu)

TF_DECLARE_PUBLIC_TOKENS(HdOmniMetricsSchemaTokens, OMNIMETRICSASSEMBLER_API,
    HDOMNI_METRICS_SCHEMA_TOKENS);

//-----------------------------------------------------------------------------

class HdOmniMetricsSchema : public HdSchema
{
public:
    HdOmniMetricsSchema(HdContainerDataSourceHandle container);

    OMNIMETRICSASSEMBLER_API
    HdDoubleDataSourceHandle GetLayerMpu();

    OMNIMETRICSASSEMBLER_API
    HdDoubleDataSourceHandle GetStageMpu();

    OMNIMETRICSASSEMBLER_API
    static HdContainerDataSourceHandle
    BuildRetained(
        const HdDoubleDataSourceHandle& layerMpu,
        const HdDoubleDataSourceHandle& stageMpu);

    class Builder
    {
    public:
        OMNIMETRICSASSEMBLER_API
        Builder& SetLayerMpu(const HdDoubleDataSourceHandle& layerMpu);

        OMNIMETRICSASSEMBLER_API
        Builder& SetStageMpu(const HdDoubleDataSourceHandle& stageMpu);

        OMNIMETRICSASSEMBLER_API
        HdContainerDataSourceHandle Build();

    private:
        HdDoubleDataSourceHandle _layerMpu;
        HdDoubleDataSourceHandle _stageMpu;
    };

    OMNIMETRICSASSEMBLER_API
    static HdOmniMetricsSchema GetFromParent(
        const HdContainerDataSourceHandle& fromParentContainer);
    
    OMNIMETRICSASSEMBLER_API
    static const HdDataSourceLocator& GetDefaultLocator();
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // end OMNI_METRICS_SCHEMA_H_