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

#ifndef OMNI_EDF_EDFPLUGINMANAGER_H_
#define OMNI_EDF_EDFPLUGINMANAGER_H_

#include <string>
#include <unordered_map>

#include <pxr/pxr.h>
#include <pxr/base/tf/singleton.h>
#include <pxr/base/tf/type.h>
#include <pxr/base/plug/plugin.h>

#include "iEdfDataProvider.h"

PXR_NAMESPACE_OPEN_SCOPE

struct _DataProviderInfo
{
public:
	PlugPluginPtr plugin;
	TfType dataProviderType;
};

/// \class EdfPluginmanager
///
/// Singleton object responsible for managing the different data provider
/// plugins registered for use by the EDF file format provider.
///
class EdfPluginManager
{
public:
	static EdfPluginManager& GetInstance()
	{
		return TfSingleton<EdfPluginManager>::GetInstance();
	}

	// prevent copying and assignment
	EdfPluginManager(const EdfPluginManager&) = delete;
	EdfPluginManager& operator=(const EdfPluginManager&) = delete;

	std::unique_ptr<IEdfDataProvider> CreateDataProvider(const std::string& dataProviderId, const EdfDataParameters& parameters);

private:

	EdfPluginManager();
	~EdfPluginManager();

	void _GetDataProviders();

	friend class TfSingleton<EdfPluginManager>;

private:

	bool _pluginsLoaded;
	std::unordered_map<std::string, _DataProviderInfo> _dataProviderPlugins;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif