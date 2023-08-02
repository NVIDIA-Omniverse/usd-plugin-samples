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

#ifndef OMNI_WARP_SCENE_INDEX_WARP_COMPUTATION_SCHEMA_H
#define OMNI_WARP_SCENE_INDEX_WARP_COMPUTATION_SCHEMA_H

#include <pxr/imaging/hd/schema.h>

#include "api.h"

PXR_NAMESPACE_OPEN_SCOPE

//-----------------------------------------------------------------------------

#define OMNIWARPCOMPUTATION_SCHEMA_TOKENS \
    (warpComputation) \
    (sourceFile) \
    (dependentPrims) \
    (simulationParams) \

TF_DECLARE_PUBLIC_TOKENS(OmniWarpComputationSchemaTokens, OMNIWARPSCENEINDEX_API,
    OMNIWARPCOMPUTATION_SCHEMA_TOKENS);

//-----------------------------------------------------------------------------

class OmniWarpComputationSchema : public HdSchema
{
public:
    OmniWarpComputationSchema(HdContainerDataSourceHandle container)
    : HdSchema(container) {}

    //ACCESSORS

    OMNIWARPSCENEINDEX_API
    HdStringDataSourceHandle GetSourceFile();

    OMNIWARPSCENEINDEX_API
    HdPathArrayDataSourceHandle GetDependentPrims();

    OMNIWARPSCENEINDEX_API
    HdSampledDataSourceHandle GetSimulationParams();

    // RETRIEVING AND CONSTRUCTING

    /// Builds a container data source which includes the provided child data
    /// sources. Parameters with nullptr values are excluded. This is a
    /// low-level interface. For cases in which it's desired to define
    /// the container with a sparse set of child fields, the Builder class
    /// is often more convenient and readable.
    OMNIWARPSCENEINDEX_API
    static HdContainerDataSourceHandle
    BuildRetained(
        const HdStringDataSourceHandle &sourceFile,
        const HdPathArrayDataSourceHandle &dependentPrims,
        const HdSampledDataSourceHandle &simulationParams
    );

    /// \class OmniWarpComputationSchema::Builder
    /// 
    /// Utility class for setting sparse sets of child data source fields to be
    /// filled as arguments into BuildRetained. Because all setter methods
    /// return a reference to the instance, this can be used in the "builder
    /// pattern" form.
    class Builder
    {
    public:
        OMNIWARPSCENEINDEX_API
        Builder &SetSourceFile(
            const HdStringDataSourceHandle &sourceFile);

		OMNIWARPSCENEINDEX_API
        Builder &SetDependentPrims(
            const HdPathArrayDataSourceHandle &dependentPrims);

        Builder &SetSimulationParams(
            const HdSampledDataSourceHandle &simulationParams);

        /// Returns a container data source containing the members set thus far.
        OMNIWARPSCENEINDEX_API
        HdContainerDataSourceHandle Build();

    private:
        HdStringDataSourceHandle _sourceFile;
        HdPathArrayDataSourceHandle _dependentPrims;
        HdSampledDataSourceHandle _simulationParams;
    };

    /// Retrieves a container data source with the schema's default name token
    /// "warpComputation" from the parent container and constructs a
    /// OmniWarpComputationSchema instance.
    /// Because the requested container data source may not exist, the result
    /// should be checked with IsDefined() or a bool comparison before use.
    OMNIWARPSCENEINDEX_API
    static OmniWarpComputationSchema GetFromParent(
        const HdContainerDataSourceHandle &fromParentContainer);

    /// Returns a token where the container representing this schema is found in
    /// a container by default.
    OMNIWARPSCENEINDEX_API
    static const TfToken &GetSchemaToken();

    /// Returns an HdDataSourceLocator (relative to the prim-level data source)
    /// where the container representing this schema is found by default.
    OMNIWARPSCENEINDEX_API
    static const HdDataSourceLocator &GetDefaultLocator();

    /// Returns an HdDataSourceLocator (relative to the prim-level data source)
    /// where the source file can be found.
    /// This is often useful for checking intersection against the
    /// HdDataSourceLocatorSet sent with HdDataSourceObserver::PrimsDirtied.
    OMNIWARPSCENEINDEX_API
    static const HdDataSourceLocator &GetSourceFileLocator();

    /// Returns an HdDataSourceLocator (relative to the prim-level data source)
    /// where the dependent prims.
    /// This is often useful for checking intersection against the
    /// HdDataSourceLocatorSet sent with HdDataSourceObserver::PrimsDirtied.
    OMNIWARPSCENEINDEX_API
    static const HdDataSourceLocator &GetDependentPrimsLocator();


    /// Returns an HdDataSourceLocator (relative to the prim-level data source)
    /// where the simulation params can be found.
    /// This is often useful for checking intersection against the
    /// HdDataSourceLocatorSet sent with HdDataSourceObserver::PrimsDirtied.
    OMNIWARPSCENEINDEX_API
    static const HdDataSourceLocator &GetSimulationParamsLocator();
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif