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

#include <pxr/base/tf/instantiateSingleton.h>
#include <pxr/base/plug/registry.h>
#include <pxr/base/js/value.h>
#include <pxr/base/js/utils.h>

#include "edfPluginManager.h"
#include "edfDataProviderFactory.h"

PXR_NAMESPACE_OPEN_SCOPE

TF_INSTANTIATE_SINGLETON(EdfPluginManager);

TF_DEFINE_PRIVATE_TOKENS(
	EdfDataProviderPlugInTokens,

	// metadata describing a unique id for the data provider plugin
	(dataProviderId)
);

EdfPluginManager::EdfPluginManager()
{
	this->_pluginsLoaded = false;
}

EdfPluginManager::~EdfPluginManager()
{
}

std::unique_ptr<IEdfDataProvider> EdfPluginManager::CreateDataProvider(const std::string& dataProviderId, const EdfDataParameters& parameters)
{
	// load the plugins if not already loaded
	this->_GetDataProviders();

	// attempt to find the plugin responsible for the data provider id
	const std::unordered_map<std::string, _DataProviderInfo>::iterator it = this->_dataProviderPlugins.find(dataProviderId);
	if (it == this->_dataProviderPlugins.end())
	{
		TF_CODING_ERROR("Failed to find plugin for %s", dataProviderId.c_str());

		return nullptr;
	}

	// load the corresponding plugin if not already loaded
	if (!it->second.plugin->Load())
	{
		TF_CODING_ERROR("Failed to load plugin %s for %s", it->second.plugin->GetName().c_str(), it->second.dataProviderType.GetTypeName().c_str());
		
		return nullptr;
	}

	std::unique_ptr<IEdfDataProvider> dataProvider;
	EdfDataProviderFactoryBase* factory = it->second.dataProviderType.GetFactory<EdfDataProviderFactoryBase>();
	if (factory != nullptr)
	{
		dataProvider.reset(factory->New(parameters));
	}

	if (dataProvider == nullptr)
	{
		TF_CODING_ERROR("Failed to create data provider %s from plugin %s", it->second.dataProviderType.GetTypeName().c_str(), it->second.plugin->GetName().c_str());
	}

	return dataProvider;
}

void EdfPluginManager::_GetDataProviders()
{
	// this uses the standard Pixar plug-in mechansim to load and discover
	// plug-ins of a certain type
	if (!this->_pluginsLoaded)
	{
		std::set<TfType> dataProviderTypes;
		PlugRegistry::GetAllDerivedTypes(TfType::Find<IEdfDataProvider>(), &dataProviderTypes);
		for (const TfType dataProviderType : dataProviderTypes)
		{
			// get the plugin for the specified type from the plugin registry
			const PlugPluginPtr plugin = PlugRegistry::GetInstance().GetPluginForType(dataProviderType);
			if (plugin == nullptr)
			{
				TF_CODING_ERROR("Failed to find plugin for %s", dataProviderType.GetTypeName().c_str());
				continue;
			}

			std::string dataProviderId;
			const JsOptionalValue dataProviderIdVal = JsFindValue(plugin->GetMetadataForType(dataProviderType), EdfDataProviderPlugInTokens->dataProviderId.GetString());
			if (!dataProviderIdVal.has_value() || !dataProviderIdVal->Is<std::string>())
			{
				TF_CODING_ERROR("'%s' metadata for '%s' must be specified!", EdfDataProviderPlugInTokens->dataProviderId.GetText(), dataProviderType.GetTypeName().c_str());
				continue;
			}

			dataProviderId = dataProviderIdVal->GetString();

			// store the map between the data provider id and the plugin
			_DataProviderInfo providerInfo;
			providerInfo.plugin = plugin;
			providerInfo.dataProviderType = dataProviderType;
			this->_dataProviderPlugins[dataProviderId] = providerInfo;
		}

		this->_pluginsLoaded = true;
	}
}

PXR_NAMESPACE_CLOSE_SCOPE