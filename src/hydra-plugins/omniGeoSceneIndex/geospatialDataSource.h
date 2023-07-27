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
#ifndef HD_OMNI_GEOSPATIAL_DATA_SOURCE_H_
#define HD_OMNI_GEOSPATIAL_DATA_SOURCE_H_

#include <pxr/imaging/hd/dataSource.h>
#include <pxr/imaging/hd/dataSourceTypeDefs.h>
#include <pxr/imaging/hd/sceneIndex.h>

#include "api.h"

PXR_NAMESPACE_OPEN_SCOPE

//-----------------------------------------------------------------------------

#define HDOMNIGEOSPATIALDATASOURCE_TOKENS \
    (geospatialPreservedXform)

TF_DECLARE_PUBLIC_TOKENS(HdOmniGeospatialDataSourceTokens, OMNIGEOSCENEINDEX_API,
    HDOMNIGEOSPATIALDATASOURCE_TOKENS);

//-----------------------------------------------------------------------------

/// \class HdOmniGeospatialDataSource
///
/// A datasource representing a wrapped view of an existing flattened
/// data source where the xform token is intercepted and a new geospatial
/// matrix dynamically calculated. 
///
class HdOmniGeospatialDataSource : public HdContainerDataSource
{
public:

    HD_DECLARE_DATASOURCE(HdOmniGeospatialDataSource);

    HdOmniGeospatialDataSource(const HdSceneIndexBase& sceneIndex, const SdfPath& primPath,
        HdContainerDataSourceHandle wrappedDataSource);

    void UpdateWrappedDataSource(HdContainerDataSourceHandle wrappedDataSource);

    // data source overrides
    TfTokenVector GetNames() override;
    HdDataSourceBaseHandle Get(const TfToken& name) override;

#if PXR_VERSION < 2302
    bool Has(const TfToken& name) override;
#endif

    // determines if the data source would be dirtied based on the locators given
    bool IsPrimDirtied(const HdDataSourceLocatorSet& locators);

private:

    bool _HasGeospatialInformation(HdContainerDataSourceHandle dataSource);
    HdDataSourceBaseHandle _ComputeGeospatialXform();

private:

    const HdSceneIndexBase& _sceneIndex;
    SdfPath _primPath;
    HdContainerDataSourceHandle _wrappedDataSource;

    // cached computed datasources
    HdContainerDataSourceAtomicHandle _computedGeospatialPrimDataSource;
    HdContainerDataSourceAtomicHandle _computedGeospatialDependentDataSource;
};

HD_DECLARE_DATASOURCE_HANDLES(HdOmniGeospatialDataSource);

PXR_NAMESPACE_CLOSE_SCOPE

#endif // HD_OMNI_GEOSPATIAL_DATA_SOURCE_H_