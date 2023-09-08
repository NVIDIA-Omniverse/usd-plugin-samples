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

#include <pxr/imaging/hd/xformSchema.h>

#include "geospatialDataSource.h"
#include "computedDependentDataSource.h"

PXR_NAMESPACE_OPEN_SCOPE

HdOmniGeospatialComputedDependentDataSource::HdOmniGeospatialComputedDependentDataSource(
    HdContainerDataSourceHandle inputDataSource, 
    HdContainerDataSourceHandle parentDataSource) :
    _inputDataSource(inputDataSource),
    _parentDataSource(parentDataSource)
{
    _matrixDataSource = 
        HdOmniGeospatialComputedDependentDataSource::_GeospatiallyAffectedMatrixDataSource::New(
            _inputDataSource, parentDataSource);
}

#if PXR_VERSION < 2302
bool HdOmniGeospatialComputedDependentDataSource::Has(const TfToken& name)
{
    return (name == HdXformSchemaTokens->resetXformStack) || 
        (name == HdXformSchemaTokens->matrix);
}
#endif

TfTokenVector HdOmniGeospatialComputedDependentDataSource::GetNames()
{
    // this container data source retrieves the xform tokens
    TfTokenVector result;
    result.push_back(HdXformSchemaTokens->resetXformStack);
    result.push_back(HdXformSchemaTokens->matrix);

    return result;
}

HdDataSourceBaseHandle HdOmniGeospatialComputedDependentDataSource::Get(const TfToken& name)
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
            return _matrixDataSource;
        }
    }

    return nullptr;
}

HdOmniGeospatialComputedDependentDataSource::_GeospatiallyAffectedMatrixDataSource::_GeospatiallyAffectedMatrixDataSource(
    HdContainerDataSourceHandle inputDataSource,
    HdContainerDataSourceHandle parentDataSource) : 
    _inputDataSource(inputDataSource),
    _parentDataSource(parentDataSource)
{
}

VtValue HdOmniGeospatialComputedDependentDataSource::_GeospatiallyAffectedMatrixDataSource::GetValue(Time shutterOffset)
{
    return VtValue(this->GetTypedValue(shutterOffset));
}

GfMatrix4d HdOmniGeospatialComputedDependentDataSource::_GeospatiallyAffectedMatrixDataSource::GetTypedValue(Time shutterOffset)
{
    return this->_ComputeTransformedMatrix(shutterOffset);
}

bool HdOmniGeospatialComputedDependentDataSource::_GeospatiallyAffectedMatrixDataSource::GetContributingSampleTimesForInterval(
    Time startTime,
    Time endTime,
    std::vector<Time>* outSampleTimes)
{
    HdSampledDataSourceHandle sources[] = {
        this->_GetMatrixSource(),
        this->_GetParentMatrixSource()
    };

    return HdGetMergedContributingSampleTimesForInterval(
        TfArraySize(sources),
        sources,
        startTime,
        endTime,
        outSampleTimes);
}

HdMatrixDataSourceHandle HdOmniGeospatialComputedDependentDataSource::
    _GeospatiallyAffectedMatrixDataSource::_GetMatrixSource() const
{
    return HdXformSchema::GetFromParent(_inputDataSource).GetMatrix();
}

HdBoolDataSourceHandle HdOmniGeospatialComputedDependentDataSource::
_GeospatiallyAffectedMatrixDataSource::_GetResetXformStackSource() const
{
    return HdXformSchema::GetFromParent(_inputDataSource).GetResetXformStack();
}

HdMatrixDataSourceHandle HdOmniGeospatialComputedDependentDataSource::
    _GeospatiallyAffectedMatrixDataSource::_GetParentMatrixSource() const
{
    return HdXformSchema::GetFromParent(_parentDataSource).GetMatrix();
}

HdMatrixDataSourceHandle HdOmniGeospatialComputedDependentDataSource::
    _GeospatiallyAffectedMatrixDataSource::_GetParentOriginalMatrixSource() const
{
    // the parent data source here should be a geospatial data source
    // but in the even it is not, this method will simply return the same
    // matrix as that of _GetParentMatrixSource
    HdOmniGeospatialDataSourceHandle geospatialDataSource = 
        HdOmniGeospatialDataSource::Cast(_parentDataSource);
    
    if (geospatialDataSource != nullptr)
    {
        HdContainerDataSourceHandle xformDataSource =
            HdContainerDataSource::Cast(
                geospatialDataSource->Get(HdOmniGeospatialDataSourceTokens->geospatialPreservedXform));

        if (xformDataSource == nullptr)
        {
            TF_WARN("Parent data source could not retrieve preserved xform!");
            return this->_GetParentMatrixSource();
        }

        HdMatrixDataSourceHandle matrixDataSource = HdMatrixDataSource::Cast(
            xformDataSource->Get(HdXformSchemaTokens->matrix));

        if (matrixDataSource == nullptr)
        {
            TF_WARN("Xform schema not defined on preserved container data source!");
        }

        return (matrixDataSource != nullptr) ? matrixDataSource : this->_GetParentMatrixSource();
    }
    else
    {
        TF_WARN("Parent data source has no geospatial data source!");
    }

    return this->_GetParentMatrixSource();
}

GfMatrix4d HdOmniGeospatialComputedDependentDataSource::
    _GeospatiallyAffectedMatrixDataSource::_GetMatrix(const Time shutterOffset) const
{
    HdMatrixDataSourceHandle dataSource = this->_GetMatrixSource();
    if (dataSource != nullptr)
    {
        return dataSource->GetTypedValue(shutterOffset);
    }

    return GfMatrix4d(1.0);
}

bool HdOmniGeospatialComputedDependentDataSource::
    _GeospatiallyAffectedMatrixDataSource::_GetResetXformStack(const Time shutterOffset) const
{
    HdBoolDataSourceHandle dataSource = this->_GetResetXformStackSource();
    if (dataSource != nullptr)
    {
        return dataSource->GetTypedValue(shutterOffset);
    }

    return false;
}

GfMatrix4d HdOmniGeospatialComputedDependentDataSource::
    _GeospatiallyAffectedMatrixDataSource::_GetParentMatrix(const Time shutterOffset) const
{
    HdMatrixDataSourceHandle dataSource = this->_GetParentMatrixSource();
    if (dataSource != nullptr)
    {
        return dataSource->GetTypedValue(shutterOffset);
    }

    return GfMatrix4d(1.0);
}

GfMatrix4d HdOmniGeospatialComputedDependentDataSource::
_GeospatiallyAffectedMatrixDataSource::_GetParentOriginalMatrix(const Time shutterOffset) const
{
    HdMatrixDataSourceHandle dataSource = this->_GetParentOriginalMatrixSource();
    if (dataSource != nullptr)
    {
        return dataSource->GetTypedValue(shutterOffset);
    }

    return GfMatrix4d(1.0);
}

GfMatrix4d HdOmniGeospatialComputedDependentDataSource::
    _GeospatiallyAffectedMatrixDataSource::_ComputeTransformedMatrix(const Time shutterOffset) const
{
    // this prim did not have geospatial information applied to it,
    // but it is the child of one that did, so we compute the updated
    // value based on the recomputed value of the parent
    // however, we actually only want to do this if this prim does
    // not have a resetXformStack applied
    bool resetXformStack = this->_GetResetXformStack(shutterOffset);
    if (!resetXformStack)
    {
        // to compute the affected matrix, we first need to acquire the parent information
        GfMatrix4d flattenedParentTransform = this->_GetParentMatrix(shutterOffset);
        GfMatrix4d originalParentTransform = this->_GetParentOriginalMatrix(shutterOffset);

        // since we are dealing with flattened transformations, we have to recover
        // the local transform of the prim data source in question
        // we can do this by knowing the prim's flattened transform
        // and the original transform of its parent (the _dependsOnDataSource)
        // Let FT be the flattened transform, P be the transform of the parent,
        // and LT be the child's local transform.  The flattened transform would
        // then have been computed as FT = (P)(LT), thus to recover LT we divide
        // out by P, which results in LT = (FT) / (P) = FT * (P)^-1
        // so we need the inverse of the original parent transform
        GfMatrix4d inverseParentTransform = originalParentTransform.GetInverse();
        GfMatrix4d originalChildTransform = this->_GetMatrix(shutterOffset);
        GfMatrix4d childLocalTransform = originalChildTransform * inverseParentTransform;

        // once we have the local transform, we can re-apply the new
        // flattened parent transform - this is the new geospatially affected transform
        // of the child
        return flattenedParentTransform * childLocalTransform;
    }

    // if resetXformStack was true, the original flattened transform of
    // of the input data source is valid here and we don't recompute
    return this->_GetMatrix(shutterOffset);
}

PXR_NAMESPACE_CLOSE_SCOPE