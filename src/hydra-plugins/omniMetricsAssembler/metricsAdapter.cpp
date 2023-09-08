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

#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/xformable.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/imaging/hd/retainedDataSource.h>
#include <pxr/imaging/hd/overlayContainerDataSource.h>
#include <pxr/imaging/hd/xformSchema.h>

#include "metricsAdapter.h"
#include "metricsDoubleDataSource.h"
#include "metricsSchema.h"

PXR_NAMESPACE_OPEN_SCOPE

TF_REGISTRY_FUNCTION(TfType)
{
    typedef OmniMetricsAssemblerAdapter Adapter;
    TfType t = TfType::Define<Adapter, TfType::Bases<Adapter::BaseAdapter> >();
    t.SetFactory<UsdImagingAPISchemaAdapterFactory<Adapter> >();
}

OmniMetricsAssemblerAdapter::~OmniMetricsAssemblerAdapter()
{
}

HdContainerDataSourceHandle OmniMetricsAssemblerAdapter::GetImagingSubprimData(
    const UsdPrim& prim,
    const TfToken& subprim,
    const TfToken& appliedInstanceName,
    const UsdImagingDataSourceStageGlobals& stageGlobals)
{
    if (prim.IsA<UsdGeomSphere>())
    {
        double stageMpu = 0.0;
        UsdStageRefPtr stage = prim.GetStage();
        if (!this->_GetMpuFromLayer(stage->GetRootLayer(), stageMpu))
        {
            // no explicitly authored MPU, so assume the documented
            // default value of centimeters
            // Open Issue: interesting case is when it isn't defined on
            // another layer - should we assume documented default of cm
            // or assume this means we should use the stage MPU?
            stageMpu = 0.01;
        }

        // this PoC only looks at Spheres as a simplification of a much more general problem
        // in this case, an MPU divergence is defined as layer.MPU != stage.MPU for the layer
        // containing the site of the strongest opinion of the `radius` property of the sphere
        UsdGeomSphere sphere = UsdGeomSphere(prim);
        UsdAttribute radiusAttr = sphere.GetRadiusAttr();

        // GetPropertyStack will give us the property specs for the attribute
        // in strongest to weakest order
        SdfPropertySpecHandleVector propertySpecs = radiusAttr.GetPropertyStack(UsdTimeCode::Default());
        if (propertySpecs.size() != 0)
        {
            // only need to process if there are any property specs for the attribute
            // and we only want the strongest
            // Open Issue: may need to take into account whether the property is blocked
            // which would indicate that it has no authored value
            SdfPropertySpecHandle strongestSpec = propertySpecs[0];
            SdfLayerHandle targetLayer = strongestSpec->GetLayer();
            double layerMpu = 0.0;
            if (!this->_GetMpuFromLayer(targetLayer, layerMpu))
            {
                // no explicitly authored layerMpu, so assume
                // it's in the same MPU as the stage
                return nullptr;
            }

            // are the layer MPU and stage MPU different?  if so, we have a metrics divergence
            if (layerMpu != stageMpu)
            {
                // there is a divergence, we record this information
                // in a hydra data source and send that data source back
                HdDataSourceBaseHandle metricsDataSource = HdOmniMetricsSchema::Builder()
                    .SetLayerMpu(HdOmniMetricsDoubleDataSource::New(layerMpu))
                    .SetStageMpu(HdOmniMetricsDoubleDataSource::New(stageMpu))
                    .Build();

                return HdRetainedContainerDataSource::New(
                    HdOmniMetricsSchemaTokens->metrics,
                    metricsDataSource);
            }
        }
        else
        {
            // in this case, there are no authored values for the property spec
            // this one is semantically tricky, because we rely on a (potential)
            // fallback value from the schema - but we have no layer target on which
            // this is technically assigned.  As such, we assume tha the fallback
            // value is defined on the root layer itself.
            TF_STATUS("No property specs in the property stack for the radius attribute!");
        }
    }

    return nullptr;
}

HdDataSourceLocatorSet OmniMetricsAssemblerAdapter::InvalidateImagingSubprim(
    const UsdPrim& prim,
    const TfToken& subprim,
    const TfToken& appliedInstanceName,
    const TfTokenVector& properties)
{
    if (prim.IsA<UsdGeomSphere>())
    {
        // invalidate the prim by invalidating its xform
        static const HdDataSourceLocatorSet locators {
            HdXformSchema::GetDefaultLocator()
        };

        return locators;
    }

    return HdDataSourceLocatorSet();
}

bool OmniMetricsAssemblerAdapter::_GetMpuFromLayer(const SdfLayerHandle& layer, double& mpu)
{
    SdfDataRefPtr metadata = layer->GetMetadata();
    VtValue mpuValue;
    if (metadata->Has(SdfPath::AbsoluteRootPath(), UsdGeomTokens->metersPerUnit, &mpuValue))
    {
        mpu = mpuValue.Get<double>();
    }
    else
    {
        TF_WARN("Unable to retrieve MPU metadata from layer!");
        return false;
    }

    return true;
}

PXR_NAMESPACE_CLOSE_SCOPE