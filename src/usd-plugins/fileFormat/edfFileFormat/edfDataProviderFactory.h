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

#ifndef OMNI_EDF_EDFDATAPROVIDERFACTORY_H_
#define OMNI_EDF_EDFDATAPROVIDERFACTORY_H_

#include <pxr/pxr.h>
#include <pxr/base/tf/registryManager.h>
#include <pxr/base/tf/type.h>

#include "api.h"
#include "iEdfDataProvider.h"

PXR_NAMESPACE_OPEN_SCOPE

#ifdef doxygen
#define EDF_DEFINE_DATAPROVIDER(ProviderClass, BaseClass1, ...)
#else
#define EDF_DEFINE_DATAPROVIDER(...)      \
TF_REGISTRY_FUNCTION(TfType) {            \
	EdfDefineDataProvider<__VA_ARGS__>(); \
}
#endif

class EdfDataProviderFactoryBase : public TfType::FactoryBase
{
public:
	EDF_API virtual ~EdfDataProviderFactoryBase();
	EDF_API virtual IEdfDataProvider* New(const EdfDataParameters& parameters) const = 0;
};

template <class T>
class EdfDataProviderFactory : public EdfDataProviderFactoryBase
{
public:
	virtual IEdfDataProvider* New(const EdfDataParameters& parameters) const override
	{
		return new T(parameters);
	}
};

template <class DataProvider, class ...Bases>
void EdfDefineDataProvider()
{
	TfType::Define<DataProvider, TfType::Bases<Bases...>>().template SetFactory<EdfDataProviderFactory<DataProvider> >();
}

PXR_NAMESPACE_CLOSE_SCOPE

#endif