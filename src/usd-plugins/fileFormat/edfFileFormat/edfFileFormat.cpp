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

#include "edfFileFormat.h"
#include "edfData.h"

PXR_NAMESPACE_OPEN_SCOPE

EdfFileFormat::EdfFileFormat() : SdfFileFormat(
										EdfFileFormatTokens->Id,
										EdfFileFormatTokens->Version,
										EdfFileFormatTokens->Target,
										EdfFileFormatTokens->Extension)
{
}

EdfFileFormat::~EdfFileFormat()
{
}

bool EdfFileFormat::CanRead(const std::string& filePath) const
{
	return true;
}

bool EdfFileFormat::Read(SdfLayer* layer, const std::string& resolvedPath, bool metadataOnly) const
{
	// these macros emit methods defined in the Pixar namespace
	// but not properly scoped, so we have to use the namespace
	// locally here - note this isn't strictly true since we had to open
	// the namespace scope anyway because the macros won't allow non-Pixar namespaces
	// to be used because of some auto-generated content
	PXR_NAMESPACE_USING_DIRECTIVE
	if (!TF_VERIFY(layer))
	{
		return false;
	}

	// construct the SdfAbstractData object from the file format args
	// and set that as the layer data - note this is a different object
	// from that constructed in the InitData method - this may or may
	// not be an issue, something to be investigated in more detail when
	// working through the backend - either way we associate it with the layer
	// so we always have a mapping from the dynamic layer and the specific
	// set of parameters that created it
	const FileFormatArguments& args = layer->GetFileFormatArguments();
	SdfAbstractDataRefPtr layerData = this->InitData(args);

	// inform the data provider that it's time to read the content
	// this is a good time for it to cache data that it needs to generate
	// the prim / property specs when asked for them via the data apis
	EdfData& edfData = dynamic_cast<EdfData&>(*layerData);
	bool readSuccess = edfData.Read();
	if (readSuccess)
	{
		this->_SetLayerData(layer, layerData);

		// for now, this is dynamic content read one way from a source external system
		// therefore we mark that the layer is read-only
		// later we will remove this restriction and explore what it means to edit
		// data that is sourced from external data formats
		layer->SetPermissionToSave(false);
		layer->SetPermissionToEdit(false);
	}

	return readSuccess;
}

bool EdfFileFormat::WriteToString(const SdfLayer& layer, std::string* str, const std::string& comment) const
{
	// TODO:
	return false;
}

bool EdfFileFormat::WriteToStream(const SdfSpecHandle& spec, std::ostream& out, size_t indent) const
{
	// TODO:
	return false;
}

SdfAbstractDataRefPtr EdfFileFormat::InitData(const FileFormatArguments& args) const
{
	// create the data parameters object to capture what data was used to create the layer
	EdfDataParameters parameters = EdfDataParameters::FromFileFormatArgs(args);
	return EdfData::CreateFromParameters(parameters);
}

bool EdfFileFormat::_ShouldSkipAnonymousReload() const
{
	return false;
}

bool EdfFileFormat::_ShouldReadAnonymousLayers() const
{
	return true;
}

void EdfFileFormat::ComposeFieldsForFileFormatArguments(const std::string& assetPath, const PcpDynamicFileFormatContext& context, FileFormatArguments* args, VtValue* contextDependencyData) const
{
	VtValue val;
	if (context.ComposeValue(EdfFileFormatTokens->Params, &val) && val.IsHolding<VtDictionary>())
	{
		// the composition engine has composed the metadata values of the prim appropriately
		// for the currently composed stage, we read these metadata values that were composed
		// and make them part of the file format arguments to load the dependent layer
		VtDictionary dict = val.UncheckedGet<VtDictionary>();
		const VtValue* dictVal = TfMapLookupPtr(dict, EdfDataParametersTokens->dataProviderId);
		if (dictVal != nullptr)
		{
			(*args)[EdfDataParametersTokens->dataProviderId] = dictVal->UncheckedGet<std::string>();
		}

		// unfortunately, FileFormatArguments is a typedef for a map<string, string>
		// which means we have to unpack the provider arguments dictionary
		// to keep the unpacking simple, we assume for now that the providerArgs 
		// is itself a dictionary containing only string paris and values
		// we can remove this restriction later for simple types (using TfStringify)
		// but would need some work (recursively) for embedded lists and dictionary values
		dictVal = TfMapLookupPtr(dict, EdfDataParametersTokens->providerArgs);
		if (dictVal != nullptr)
		{
			std::string prefix = EdfDataParametersTokens->providerArgs.GetString();
			VtDictionary providerArgs = dictVal->UncheckedGet<VtDictionary>();
			for (VtDictionary::iterator it = providerArgs.begin(); it != providerArgs.end(); it++)
			{
				(*args)[prefix + ":" + it->first] = it->second.UncheckedGet<std::string>();
			}
		}
	}
}

bool EdfFileFormat::CanFieldChangeAffectFileFormatArguments(const TfToken& field, const VtValue& oldValue, const VtValue& newValue, const VtValue& contextDependencyData) const
{
	const VtDictionary& oldDictionaryValue = oldValue.IsHolding<VtDictionary>() ? 
		oldValue.UncheckedGet<VtDictionary>() : VtGetEmptyDictionary();
	const VtDictionary& newDictionaryValue = newValue.IsHolding<VtDictionary>() ?
		newValue.UncheckedGet<VtDictionary>() : VtGetEmptyDictionary();

	// nothing to do if both metadata values are empty
	if (oldDictionaryValue.empty() && newDictionaryValue.empty())
	{
		return false;
	}

	// our layer is new if:
	// 1. there is a new provider
	// 2. there is a change to the value of the provider specific data
	const VtValue* oldProviderId = 
		TfMapLookupPtr(oldDictionaryValue, EdfDataParametersTokens->dataProviderId);
	const VtValue* newProviderId = 
		TfMapLookupPtr(newDictionaryValue, EdfDataParametersTokens->dataProviderId);
	if (oldProviderId != nullptr && newProviderId != nullptr)
	{
		if (oldProviderId->UncheckedGet<std::string>() != newProviderId->UncheckedGet<std::string>())
		{
			// different providers!
			return true;
		}
		else
		{
			// same provider, but the specific provider metadata may have changed
			const VtValue* oldProviderDictionaryValue = 
				TfMapLookupPtr(oldDictionaryValue, EdfDataParametersTokens->providerArgs);
			const VtValue* newProviderDictionaryValue =
				TfMapLookupPtr(newDictionaryValue, EdfDataParametersTokens->providerArgs);
			const VtDictionary& oldProviderDictionary = oldProviderDictionaryValue->IsHolding<VtDictionary>() ?
				oldProviderDictionaryValue->UncheckedGet<VtDictionary>() : VtGetEmptyDictionary();
			const VtDictionary& newProviderDictionary = newProviderDictionaryValue->IsHolding<VtDictionary>() ?
				newProviderDictionaryValue->UncheckedGet<VtDictionary>() : VtGetEmptyDictionary();
			
			return oldProviderDictionary != newProviderDictionary;
		}
	}
	else
	{
		// one of them (or both) are nullptrs
		if (oldProviderId == nullptr && newProviderId == nullptr)
		{
			// no change to provider, don't need to check parameters
			return false;
		}

		// otherwise one changed
		return true;
	}
}

// these macros emit methods defined in the Pixar namespace
// but not properly scoped, so we have to use the namespace
// locally here
TF_DEFINE_PUBLIC_TOKENS(
	EdfFileFormatTokens,
	((Id, "edfFileFormat"))
	((Version, "1.0"))
	((Target, "usd"))
	((Extension, "edf"))
	((Params, "EdfDataParameters"))
);

TF_REGISTRY_FUNCTION(TfType)
{
	SDF_DEFINE_FILE_FORMAT(EdfFileFormat, SdfFileFormat);
}

PXR_NAMESPACE_CLOSE_SCOPE