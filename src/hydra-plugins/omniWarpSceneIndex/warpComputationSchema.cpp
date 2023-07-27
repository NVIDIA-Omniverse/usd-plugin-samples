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
//

#include <pxr/base/trace/trace.h>
#include <pxr/imaging/hd/retainedDataSource.h>

#include "warpComputationSchema.h"

PXR_NAMESPACE_OPEN_SCOPE

TF_DEFINE_PUBLIC_TOKENS(OmniWarpComputationSchemaTokens,
    OMNIWARPCOMPUTATION_SCHEMA_TOKENS);

HdStringDataSourceHandle
OmniWarpComputationSchema::GetSourceFile()
{
    return _GetTypedDataSource<HdStringDataSource>(
        OmniWarpComputationSchemaTokens->sourceFile);
}

/*static*/
HdContainerDataSourceHandle
OmniWarpComputationSchema::BuildRetained(
        const HdStringDataSourceHandle &sourceFile
)
{
    TfToken names[1];
    HdDataSourceBaseHandle values[1];

    size_t count = 0;
    if (sourceFile) {
        names[count] = OmniWarpComputationSchemaTokens->sourceFile;
        values[count++] = sourceFile;
    }

    return HdRetainedContainerDataSource::New(count, names, values);
}

/*static*/
OmniWarpComputationSchema
OmniWarpComputationSchema::GetFromParent(
        const HdContainerDataSourceHandle &fromParentContainer)
{
    return OmniWarpComputationSchema(
        fromParentContainer
        ? HdContainerDataSource::Cast(fromParentContainer->Get(
                OmniWarpComputationSchemaTokens->warpComputation))
        : nullptr);
}

/*static*/
const TfToken &
OmniWarpComputationSchema::GetSchemaToken()
{
    return OmniWarpComputationSchemaTokens->warpComputation;
} 
/*static*/
const HdDataSourceLocator &
OmniWarpComputationSchema::GetDefaultLocator()
{
    static const HdDataSourceLocator locator(
        OmniWarpComputationSchemaTokens->warpComputation
    );
    return locator;
} 
/*static*/
const HdDataSourceLocator &
OmniWarpComputationSchema::GetSourceFileLocator()
{
    static const HdDataSourceLocator locator(
        OmniWarpComputationSchemaTokens->warpComputation,
        OmniWarpComputationSchemaTokens->sourceFile
    );
    return locator;
}

OmniWarpComputationSchema::Builder &
OmniWarpComputationSchema::Builder::SetSourceFile(
    const HdStringDataSourceHandle &sourceFile)
{
    _sourceFile = sourceFile;
    return *this;
}

HdContainerDataSourceHandle
OmniWarpComputationSchema::Builder::Build()
{
    return OmniWarpComputationSchema::BuildRetained(
        _sourceFile
    );
}


PXR_NAMESPACE_CLOSE_SCOPE