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

#include "metricsDataSource.h"
#include "metricsSchema.h"

PXR_NAMESPACE_OPEN_SCOPE

TF_DEFINE_PUBLIC_TOKENS(HdOmniMetricsDataSourceTokens,
    HDOMNIMETRICSDATASOURCE_TOKENS);

HdOmniMetricsDataSource::HdOmniMetricsDataSource(const HdSceneIndexBase& index, const SdfPath& primPath,
    HdContainerDataSourceHandle wrappedDataSource) :
    _sceneIndex(index),
    _primPath(primPath),
    _wrappedDataSource(wrappedDataSource)
{
}

void HdOmniMetricsDataSource::UpdateWrappedDataSource(
    HdContainerDataSourceHandle wrappedDataSource)
{
    _wrappedDataSource = wrappedDataSource;
}

TfTokenVector HdOmniMetricsDataSource::GetNames()
{
    // this will return everything supported by the wrapped
    // data source - in some cases (xform) we will
    // intercept, but most will be pass through
    // we keep access to the underlying wrapped xform
    // via a new token added by this datasource (this is required
    // for computations involving a parent that has already
    // been corrected)
    TfTokenVector result = (_wrappedDataSource == nullptr) ? TfTokenVector() : _wrappedDataSource->GetNames();
    result.push_back(HdOmniMetricsDataSourceTokens->metricsPreservedXform);

    return result;
}

HdDataSourceBaseHandle HdOmniMetricsDataSource::Get(const TfToken& name)
{
    if (name == HdXformSchemaTokens->xform)
    {
        // this is an intercept of the flattened transform matrix
        // we need to (potentially) compute a metrics-corrected
        // flattened transform matrix
        return this->_ComputeCorrectedXform();
    }
    else if (name == HdOmniMetricsDataSourceTokens->metricsPreservedXform)
    {
        // this would be the original flattened matrix of the wrapped data source
        if (_wrappedDataSource != nullptr)
        {
            return _wrappedDataSource->Get(HdXformSchemaTokens->xform);
        }
    }

    // all other token values should be defer to the wrapped data source (if any)
    if (_wrappedDataSource != nullptr)
    {
        return _wrappedDataSource->Get(name);
    }

    return nullptr;
}

bool HdOmniMetricsDataSource::IsPrimDirtied(const HdDataSourceLocatorSet& locators)
{
    static const HdContainerDataSourceHandle containerNull(nullptr);
    if (locators.Intersects(HdXformSchema::GetDefaultLocator()))
    {
        if (HdContainerDataSource::AtomicLoad(_computedCorrectedXformDataSource) != nullptr)
        {
            HdContainerDataSource::AtomicStore(_computedCorrectedXformDataSource, containerNull);
            
            return true;
        }
    }

    return false;
}

HdDataSourceBaseHandle HdOmniMetricsDataSource::_ComputeCorrectedXform()
{
    // there are two cases to consider on the underlying wrapped data source:
    // 1. The wrapped data source has metrics information.
    //    This means that the adapter determined there was a metrics
    //    divergence in the layers for the stage and the strongest
    //    opinionated layer for the xformOpOrder attribute.  In this case
    //    it means that we have to correct the divergence directly by
    //    computing a new flattened local transform for the hydra prim
    // 2. The wrapped data source does not have metrics information.
    //    This means that either the underlying prim has no Xformable data
    //    at all or that there was no metrics divergence detected.
    //    However, it could be the child of a divergent prim, and since
    //    all xforms have been flattened by the flattening scene index
    //    prior to us wrapping the data, we need to compute a new flattened
    //    matrix that takes into account the changes on the parent.
    //
    // the tricky thing is the dirtying associated with the cached data -
    // computing whether a prim with divergence changed directly is easy
    // but that change also invalidates the children (recusrively)
    
    // if we have already cached the value, and the cache is valid
    // return the computed cached value rather than recompute it
    HdContainerDataSourceHandle computedCorrectedXformDataSource =
        HdContainerDataSource::AtomicLoad(_computedCorrectedXformDataSource);
    if (computedCorrectedXformDataSource != nullptr)
    {
        return computedCorrectedXformDataSource;
    }
    
    if (this->_HasMetricsInformation(_wrappedDataSource))
    {
        // in this case, we need the parent's flattened transform to recover
        // the original local transform of the prim, once we have the original
        // local transform we can apply the corrective as the last xformOp
        // then reflatten by multiplying the parent transform again
        SdfPath parentPath = _primPath.GetParentPath();
        HdSceneIndexPrim parentPrim = _sceneIndex.GetPrim(parentPath);

        computedCorrectedXformDataSource = HdXformSchema::Builder()
            .SetMatrix(HdOmniMetricsDataSource::_MetricsCorrectedMatrixDataSource::New(
                _wrappedDataSource, parentPrim.dataSource, true))
            .SetResetXformStack(this->_GetInputResetXformStackSource())
            .Build();
    }
    else
    {
        HdContainerDataSourceHandle metricsDataSource = nullptr;
        if (_primPath == SdfPath::AbsoluteRootPath())
        {
            // just directly get whatever the absolute root path has
            computedCorrectedXformDataSource = HdContainerDataSource::Cast(_wrappedDataSource->Get(HdXformSchemaTokens->xform));
        }
        else
        {
            for(SdfPath p = _primPath.GetParentPath(); p != SdfPath::AbsoluteRootPath(); p = p.GetParentPath())
            {
                HdSceneIndexPrim prim = _sceneIndex.GetPrim(p);
                if (this->_HasMetricsInformation(prim.dataSource))
                {
                    // a parent along the chain did have a metrics
                    // corrected xform, so we will need to recompute
                    metricsDataSource = prim.dataSource;
                    break;
                }
            }

            if (metricsDataSource != nullptr)
            {
                // compute a new flattened xform from the parent
                SdfPath parentPath = _primPath.GetParentPath();
                HdSceneIndexPrim parentPrim = _sceneIndex.GetPrim(parentPath);
                
                computedCorrectedXformDataSource = HdXformSchema::Builder()
                    .SetMatrix(HdOmniMetricsDataSource::_MetricsCorrectedMatrixDataSource::New(
                        _wrappedDataSource, parentPrim.dataSource, false))
                    .SetResetXformStack(this->_GetInputResetXformStackSource())
                    .Build();
            }
            else
            {
                // no parent in the chain had a metrics corrected xform
                // so the result is really just the original flattened matrix
                computedCorrectedXformDataSource = HdContainerDataSource::Cast(_wrappedDataSource->Get(HdXformSchemaTokens->xform));
            }
        }
    }

    // cache the data source we intend to use
    HdContainerDataSource::AtomicStore(_computedCorrectedXformDataSource, computedCorrectedXformDataSource);

    return computedCorrectedXformDataSource;
}

bool HdOmniMetricsDataSource::_HasMetricsInformation(HdContainerDataSourceHandle handle)
{
    HdOmniMetricsSchema metricsSchema = HdOmniMetricsSchema::GetFromParent(handle);
    return metricsSchema.IsDefined();
}

HdBoolDataSourceHandle HdOmniMetricsDataSource::_GetInputResetXformStackSource()
{
    if (_wrappedDataSource != nullptr)
    {
        return HdBoolDataSource::Cast(
            _wrappedDataSource->Get(HdXformSchemaTokens->resetXformStack)
        );
    }

    return nullptr;
}


HdOmniMetricsDataSource::_MetricsCorrectedMatrixDataSource::_MetricsCorrectedMatrixDataSource(
    HdContainerDataSourceHandle inputDataSource,
    HdContainerDataSourceHandle parentDataSource,
    bool isMetricsCorrectiveSource) : 
    _inputDataSource(inputDataSource),
    _parentDataSource(parentDataSource),
    _isMetricsCorrectiveSource(isMetricsCorrectiveSource)
{
}

VtValue HdOmniMetricsDataSource::_MetricsCorrectedMatrixDataSource::GetValue(Time shutterOffset)
{
    return VtValue(this->GetTypedValue(shutterOffset));
}

GfMatrix4d HdOmniMetricsDataSource::_MetricsCorrectedMatrixDataSource::GetTypedValue(Time shutterOffset)
{
    return this->_ComputeCorrectedMatrix(shutterOffset);
}

bool HdOmniMetricsDataSource::_MetricsCorrectedMatrixDataSource::GetContributingSampleTimesForInterval(
    Time startTime,
    Time endTime,
    std::vector<Time>* outSampleTimes)
{
    HdSampledDataSourceHandle sources[] = {
        this->_GetInputMatrixDataSource(),
        this->_GetParentMatrixDataSource()
    };

    return HdGetMergedContributingSampleTimesForInterval(
        TfArraySize(sources),
        sources,
        startTime,
        endTime,
        outSampleTimes
    );
}

HdMatrixDataSourceHandle HdOmniMetricsDataSource::_MetricsCorrectedMatrixDataSource::_GetParentMetricsPreservedMatrixDataSource() const
{
    HdOmniMetricsDataSourceHandle metricsDataSource = HdOmniMetricsDataSource::Cast(_parentDataSource);
    
    if (metricsDataSource != nullptr)
    {
        HdContainerDataSourceHandle xformDataSource =
            HdContainerDataSource::Cast(
                metricsDataSource->Get(HdOmniMetricsDataSourceTokens->metricsPreservedXform));

        if (xformDataSource == nullptr)
        {
            return this->_GetParentMatrixDataSource();
        }

        HdMatrixDataSourceHandle matrixDataSource = HdMatrixDataSource::Cast(
            xformDataSource->Get(HdXformSchemaTokens->matrix));

        if (matrixDataSource == nullptr)
        {
            TF_WARN("Xform schema not defined on preserved container data source!");
        }

        return (matrixDataSource != nullptr) ? matrixDataSource : this->_GetParentMatrixDataSource();
    }

    // if it didn't have metrics information attached
    // just get the original matrix
    return this->_GetParentMatrixDataSource();
}

HdMatrixDataSourceHandle HdOmniMetricsDataSource::_MetricsCorrectedMatrixDataSource::_GetParentMatrixDataSource() const
{
    HdXformSchema xformSchema = HdXformSchema::GetFromParent(_parentDataSource);
    if (xformSchema.IsDefined())
    {
        return xformSchema.GetMatrix();
    }

    return nullptr;
}

HdMatrixDataSourceHandle HdOmniMetricsDataSource::_MetricsCorrectedMatrixDataSource::_GetInputMatrixDataSource() const
{
    return HdXformSchema::GetFromParent(_inputDataSource).GetMatrix();
}

GfMatrix4d HdOmniMetricsDataSource::_MetricsCorrectedMatrixDataSource::_ComputeCorrectedMatrix(Time shutterOffset)
{
    // since we are dealing with flattened transformations, we have to recover
    // the local transform of the input data source in question
    // we can do this by knowing the prim's flattened transform
    // and the original transform of its parent (the _parentDataSource)
    // Let FT be the flattened transform, P be the transform of the parent,
    // and LT be the child's local transform.  The flattened transform would
    // then have been computed as FT = (P)(LT), thus to recover LT we divide
    // out by P, which results in LT = (FT) / (P) = FT * (P)^-1
    // so we need the inverse of the original parent transform
    HdMatrixDataSourceHandle parentPreservedMatrixDataSource = this->_GetParentMetricsPreservedMatrixDataSource();
    HdMatrixDataSourceHandle parentMatrixDataSource = this->_GetParentMatrixDataSource();
    HdMatrixDataSourceHandle inputMatrixDataSource = this->_GetInputMatrixDataSource();
    GfMatrix4d parentMatrix = (parentPreservedMatrixDataSource != nullptr) ? 
        parentPreservedMatrixDataSource->GetTypedValue(shutterOffset) :
        GfMatrix4d(1.0);
    GfMatrix4d currentFlattenedTransform = inputMatrixDataSource->GetTypedValue(shutterOffset);
    GfMatrix4d inverseParentMatrix = parentMatrix.GetInverse();
    GfMatrix4d originalLocalTransform = currentFlattenedTransform * inverseParentMatrix;

    // do we need to apply a corrective?
    if (_isMetricsCorrectiveSource)
    {
        // this is a computation requiring a new metrics corrected local
        // transform computed from the original data rather than the
        // flattened transform already there
        GfMatrix4d mpuCorrective = this->_GetMpuCorrective();
        GfMatrix4d correctedTransform = originalLocalTransform * mpuCorrective;

        // now apply the parent transform to get the new flattened child transform
        GfMatrix4d parentMatrix = (parentMatrixDataSource != nullptr) ?
            parentMatrixDataSource->GetTypedValue(shutterOffset) :
            GfMatrix4d(1.0);
        return parentMatrix * correctedTransform;
    }
    else
    {
        // no local corrective necessary, just reconcatenate with the new parent
        // transform to form the final new flattened child
        GfMatrix4d parentUpdatedMatrix = (parentMatrixDataSource != nullptr) ?
            parentMatrixDataSource->GetTypedValue(shutterOffset) :
            GfMatrix4d(1.0);

        return parentUpdatedMatrix * originalLocalTransform;
    }
}

GfMatrix4d HdOmniMetricsDataSource::_MetricsCorrectedMatrixDataSource::_GetMpuCorrective()
{
    // retrieve the layer and stage MPU values from the wrapped prim
    HdOmniMetricsSchema metricsSchema = HdOmniMetricsSchema::GetFromParent(_inputDataSource);
    if (!metricsSchema.IsDefined())
    {
        TF_WARN("MPU divergency was detected but data source has no metrics information!");
        return GfMatrix4d(1.0);
    }

    double mpuCorrectiveValue = metricsSchema.GetLayerMpu()->GetTypedValue(0.0) /
        metricsSchema.GetStageMpu()->GetTypedValue(0.0);

    GfMatrix4d uniformScaleTransform(1.0);
    uniformScaleTransform.SetScale(mpuCorrectiveValue);

    return uniformScaleTransform;
}

PXR_NAMESPACE_CLOSE_SCOPE