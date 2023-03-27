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

#include <pxr/base/plug/plugin.h>
#include <pxr/base/plug/registry.h>
#include <pxr/base/tf/token.h>
#include <pxr/usd/sdf/schema.h>

#include "edfData.h"
#include "edfDataProviderFactory.h"
#include "edfPluginManager.h"

#include <iostream>

PXR_NAMESPACE_OPEN_SCOPE

static const SdfPath ROOT_PATH("/");
static const SdfPath DATA_ROOT_PATH("/Data");

TF_DEFINE_PUBLIC_TOKENS(
	EdfDataParametersTokens,

	// plugin metadata to specify an id for a specific data provider
	(dataProviderId)

	// plugin metadata describing the arguments for the provider to use
	// to load the layer
	(providerArgs)
);

EdfDataParameters EdfDataParameters::FromFileFormatArgs(const SdfFileFormat::FileFormatArguments& args)
{
	EdfDataParameters parameters;
	parameters.dataProviderId = *(TfMapLookupPtr(args, EdfDataParametersTokens->dataProviderId));
	
	// unpack the file format argument representation of the provider arguments
	std::string prefix = EdfDataParametersTokens->providerArgs.GetString() + ":";
	size_t prefixLength = prefix.length();
	for (SdfFileFormat::FileFormatArguments::const_iterator it = args.begin(); it != args.end(); it++)
	{
		size_t index = it->first.find(prefix);
		if (index == 0)
		{
			// this is an unpacked prefixed provider argument
			parameters.providerArgs[it->first.substr(prefixLength)] = it->second;
		}
	}

	return parameters;
}

EdfSourceData::EdfSourceData(EdfData* data)
{
	this->_data = data;
}

EdfSourceData::~EdfSourceData()
{
	this->_data = nullptr;
}

void EdfSourceData::CreatePrim(const SdfPath& parentPath, const std::string& name, const SdfSpecifier& specifier,
	const TfToken& typeName)
{
	if (this->_data != nullptr)
	{
		this->_data->_CreatePrim(parentPath, name, specifier, typeName);
	}
}

void EdfSourceData::CreateAttribute(const SdfPath& parentPrimPath, const std::string& name, const SdfValueTypeName& typeName,
	const SdfVariability& variability, const VtValue& value)
{
	if (this->_data != nullptr)
	{
		this->_data->_CreateAttribute(parentPrimPath, name, typeName, variability, value);
	}
}

void EdfSourceData::SetField(const SdfPath& primPath, const TfToken& fieldName, const VtValue& value)
{
	if (this->_data != nullptr)
	{
		this->_data->_SetFieldValue(primPath, fieldName, value);
	}
}

bool EdfSourceData::HasField(const SdfPath& primPath, const TfToken& fieldName, VtValue* value)
{
    if (this ->_data != nullptr)
    {
        return this->_data->Has(primPath, fieldName, value);
    }

    return false;
}

bool EdfSourceData::HasAttribute(const SdfPath& attributePath, VtValue* defaultValue)
{
	if (this->_data != nullptr)
	{
		return this->_data->Has(attributePath, SdfFieldKeys->Default, defaultValue);
	}

	return false;
}

EdfData::EdfData(std::unique_ptr<IEdfDataProvider> dataProvider)
{
	this->_dataProvider = std::move(dataProvider);
	this->_sourceData = std::make_shared<EdfSourceData>(this);
}

EdfDataRefPtr EdfData::CreateFromParameters(const EdfDataParameters& parameters)
{
	std::unique_ptr<IEdfDataProvider> dataProvider = EdfPluginManager::GetInstance().CreateDataProvider(parameters.dataProviderId, parameters);
	if (dataProvider == nullptr)
	{
		// there was no provider responsible for this data or it didn't load properly,
		// so the best we can do is provide an empty EdfData object with no backing provider
		// this will load nothing except an empty default Root prim
		return TfCreateRefPtr(new EdfData(nullptr));
	}

	return TfCreateRefPtr(new EdfData(std::move(dataProvider)));
}

void EdfData::CreateSpec(const SdfPath& path, SdfSpecType specType)
{
	// not supported in this PoC
	// the data provider can create new prim / property specs
	// via the callbacks, but the external public API cannot

	// but if it were, here's how it would be
	// done concurrently
	/*
	this->_CreateSpec(path, specType);
	*/
}

void EdfData::Erase(const SdfPath& path, const TfToken& fieldName)
{
	// not supported in this PoC

	// but if it were, here's how it would be
	// done concurrently
	/*
	SpecData::accessor accessor;
    if (_specData.find(accessor, path))
    {
        _SpecData& spec = accessor->second;
        size_t fieldSize = spec.fields.size();
        for (size_t i = 0; i < fieldSize; i++)
        {
            if (spec.fields[i].first == fieldName)
            {
                spec.fields.erase(spec.fields.begin() + i);
                accessor.release();

                return;
            }
        }
    }

    accessor.release();
	*/
}

void EdfData::EraseSpec(const SdfPath& path)
{
	// not supported in this PoC

	// but it it were, here's how we'd do it
	// with the concurrent hash
	/*
	SpecData::const_accessor accessor;
    if (_specData.find(accessor, path))
    {
        _specData.erase(accessor);
    }

    accessor.release();
	*/
}

VtValue EdfData::Get(const SdfPath& path, const TfToken& fieldName) const
{
	VtValue val;
	this->Has(path, fieldName, &val);
	return val;
}

SdfSpecType EdfData::GetSpecType(const SdfPath& path) const
{
	// in all cases we either have the spec data available
	// because we created e.g. the root on Read
	// or because the data provider created
	// prims / properties when it performed its Read
	// or we don't know
	SpecData::const_accessor accessor;
	if (_specData.find(accessor, path))
    {
		return accessor->second.specType;
	}

	accessor.release();

	return SdfSpecType::SdfSpecTypeUnknown;
}

bool EdfData::Has(const SdfPath& path, const TfToken& fieldName, SdfAbstractDataValue* value) const
{
    if (value != nullptr)
	{
		VtValue val;
		if (this->Has(path, fieldName, &val))
		{
			return value->StoreValue(val);
		}
	}
	else
	{
		VtValue val;
		return this->Has(path, fieldName, &val);
	}

	return false;
}

bool EdfData::Has(const SdfPath& path, const TfToken& fieldName, VtValue* value) const
{
	// in general, we can just get the value for whatever is being asked for
	// from the hash (and know whether it was there or not)
	// children are a special case, because those we want to ask the back-end
	// provider to load - one tricky bit is understanding when we want the data provider
	// to load and when we want to use the cached value
	// as a general rule, if SdfChildrenKeys isn't present in the list of fields
	// for the prim, but we have the prim, we need to ask the data provider to load
	// for simplicity sake, this is a one-time load -> the data provider will use
	// the callbacks to insert the children prims / attributes
	// if we asked the data provider to load the children, and after that the field
	// still isn't present, then we insert the field with an empty list since
	// the provider never created any children (maybe the back-end query returned nothing)
    std::cout << path.GetAsString() << " " << fieldName.GetString() << std::endl;
	bool hasValue = this->_GetFieldValue(path, fieldName, value);
	if (!hasValue && fieldName == SdfChildrenKeys->PrimChildren &&
		this->_dataProvider != nullptr)
	{
		// give the data provider an opportunity to load their children
		this->_dataProvider->ReadChildren(path.GetAsString(), this->_sourceData);

		// after the read call, we check again to see if it's present
		hasValue = this->_GetFieldValue(path, fieldName, value);
		if (!hasValue)
		{
			// if it still doesn't exist, we assume that there were no children
			// and we cache that fact now
			TfTokenVector primChildren;
			VtValue primChildrenValue(primChildren);
			this->_SetFieldValue(path, SdfChildrenKeys->PrimChildren, primChildrenValue);

			if(value != nullptr)
			{
				*value = primChildrenValue;
			}
		
			hasValue = true;
		}
	}

	return hasValue;
}

bool EdfData::HasSpec(const SdfPath& path) const
{
	return this->GetSpecType(path) != SdfSpecType::SdfSpecTypeUnknown;
}

bool EdfData::IsEmpty() const
{
	return false;
}

std::vector<TfToken> EdfData::List(const SdfPath& path) const
{
	TfTokenVector names;
    SpecData::const_accessor accessor;
    if (_specData.find(accessor, path))
    {
        size_t numFields = accessor->second.fields.size();
        names.resize(numFields);
        for (size_t i = 0; i < numFields; i++)
        {
            names[i] = accessor->second.fields[i].first;
        }
    }

    accessor.release();

    return names;
}

void EdfData::MoveSpec(const SdfPath& oldPath, const SdfPath& newPath)
{
	// not supported in this PoC

	// but it it were, here's how we'd do it
	// with the concurrent hash
	/*
	SpecData::accessor accessor;
    if (_specData.find(accessor, path))
    {
        SpecData::accessor writeAccessor;
        _specData.insert(writeAccessor, newPath);
        writeAccessor->second = accessor->second;
        writeAccessor.release();
        _specData.erase(accessor);
    }

    accessor.release();
	*/
}

void EdfData::Set(const SdfPath& path, const TfToken& fieldName, const VtValue& value)
{
	// not supported in this PoC

	// but it it were, here's how we'd do it
	// with the concurrent hash
	/*
	this->_SetFieldValue(path, fieldName, value);
	*/
}

void EdfData::Set(const SdfPath& path, const TfToken& fieldName, const SdfAbstractDataConstValue& value)
{
	// not supported in this PoC

	// but it it were, here's how we'd do it
	// with the concurrent hash
	/*
	VtValue wrappedValue;
	value.GetValue(&wrappedValue);
	this->_SetFieldValue(path, fieldName, wrappedValue);
	*/
}

bool EdfData::StreamsData() const
{
	// by default, we assume the backing provider will stream data
	// but it will tell us whether it has cached that data or not later
	return true;
}

bool EdfData::IsDetached() const
{
	if (this->_dataProvider != nullptr)
	{
		return this->_dataProvider->IsDataCached();
	}
	else
	{
		return SdfAbstractData::IsDetached();
	}
}

std::set<double> EdfData::ListAllTimeSamples() const
{
	// not supported in this POC
	return std::set<double>();
}

std::set<double> EdfData::ListTimeSamplesForPath(const SdfPath& path) const
{
	// not supported in this POC
	return std::set<double>();
}

bool EdfData::GetBracketingTimeSamples(double time, double* tLower, double* tUpper) const
{
	// not supported in this POC
	return false;
}

size_t EdfData::GetNumTimeSamplesForPath(const SdfPath& path) const
{
	// not supported in this POC
	return 0;
}

bool EdfData::GetBracketingTimeSamplesForPath(const SdfPath& path, double time, double* tLower, double* tUpper) const
{
	// not supported in this POC
	return false;
}

bool EdfData::QueryTimeSample(const SdfPath& path, double time, VtValue* optionalValue) const
{
	// not supported in this POC
	return false;
}

bool EdfData::QueryTimeSample(const SdfPath& path, double time, SdfAbstractDataValue* optionalValue) const
{
	// not supported in this POC
	return false;
}

void EdfData::SetTimeSample(const SdfPath& path, double time, const VtValue& value)
{
	// not supported in this POC
}

void EdfData::EraseTimeSample(const SdfPath& path, double time)
{
	// not supported in this POC
}

void EdfData::_VisitSpecs(SdfAbstractDataSpecVisitor* visitor) const
{
	std::cout << "VISIT: " << std::endl;
	// TODO:
}

bool EdfData::Read()
{
	// TODO: need to look at what happens on layer reload
	// is a new data object created or should this one be cleared out?

	// on first read, create the specs for the absolute root path and
	// for the /Data path where the provider will root their data
	SpecData::accessor accessor;
	_specData.insert(accessor, SdfPath::AbsoluteRootPath());
    accessor->second.specType = SdfSpecType::SdfSpecTypePseudoRoot;
	accessor.release();

	// insert known field names for the root path
	// this includes at minimum:
	// SdfFieldKeys->DefaultPrim
	// SdfChildrenKeys->PrimChildren
	TfTokenVector rootChildren({DATA_ROOT_PATH.GetNameToken()});
	VtValue primChildrenValue(rootChildren);
	VtValue defaultPrimValue(DATA_ROOT_PATH.GetNameToken());
	this->_SetFieldValue(ROOT_PATH, SdfChildrenKeys->PrimChildren, primChildrenValue);
	this->_SetFieldValue(ROOT_PATH, SdfFieldKeys->DefaultPrim, defaultPrimValue);

	// insert the data root path
	_specData.insert(accessor, DATA_ROOT_PATH);
	accessor->second.specType = SdfSpecType::SdfSpecTypePrim;
	accessor.release();

	// insert known field names for the data root path
	// this includes at minimum:
	// SdfFieldKeys->Specifier
	// SdfFieldKeys->TypeName
	// SdfFieldKeys->PrimChildren
	// SdfFieldKeys->PropertyChildren
    // prim children is loaded on demand during both deferred
    // and non-deferred reads, so we don't set it here
	TfTokenVector dataRootPropertyChildren;
	VtValue specifierValue(SdfSpecifier::SdfSpecifierDef);
    VtValue typeNameValue;
	VtValue dataRootPropertyChildrenValue(dataRootPropertyChildren);
	this->_SetFieldValue(DATA_ROOT_PATH, SdfFieldKeys->Specifier, specifierValue);
	this->_SetFieldValue(DATA_ROOT_PATH, SdfFieldKeys->TypeName, typeNameValue);
	this->_SetFieldValue(DATA_ROOT_PATH, SdfChildrenKeys->PropertyChildren, dataRootPropertyChildrenValue);

	// if we have a valid provider, ask it to read it's data based on what parameters
	// it was initialized with, otherwise just return true because we only have an empty
	// root in the default implementation
	bool readResult = true;
	if (this->_dataProvider != nullptr)
	{
		readResult = this->_dataProvider->Read(this->_sourceData);
	}

	return readResult;
}

void EdfData::_CreatePrim(const SdfPath& parentPath, const std::string& name, 
	const SdfSpecifier& specifier, const TfToken& typeName)
{
	SdfPath primPath = SdfPath(parentPath.GetAsString() + "/" + name);
	this->_CreateSpec(primPath, SdfSpecType::SdfSpecTypePrim);
	this->_SetFieldValue(primPath, SdfFieldKeys->TypeName, VtValue(typeName));
	this->_SetFieldValue(primPath, SdfFieldKeys->Specifier, VtValue(specifier));

    // add this prim to the PrimChildren property of parentPath
    VtValue existingPrimChildrenValue;
    if (this->_GetFieldValue(parentPath, SdfChildrenKeys->PrimChildren, &existingPrimChildrenValue))
    {
        // there are already children present, so append to the list
        TfTokenVector existingChildren = existingPrimChildrenValue.UncheckedGet<TfTokenVector>();
        existingChildren.push_back(TfToken(name));

        // set the value back
        this->_SetFieldValue(parentPath, SdfChildrenKeys->PrimChildren, VtValue(existingChildren));
    }
    else
    {
        // no children present yet
        TfTokenVector children;
        children.push_back(TfToken(name));
        this->_SetFieldValue(parentPath, SdfChildrenKeys->PrimChildren, VtValue(children));
    }
}

void EdfData::_CreateAttribute(const SdfPath& primPath, const std::string& name,
	const SdfValueTypeName& typeName, const SdfVariability& variability, const VtValue& value)
{
	// creating an attribute means setting the attribute path
	// which is a combination of the prim path and the attribute name
	// the type name field key of the attribute
	// the variability field key of the attribute
	// and a default field key holding its value
	SdfPath attributePath = SdfPath(primPath.GetAsString() + "." + name);
	this->_CreateSpec(attributePath, SdfSpecType::SdfSpecTypeAttribute);
	this->_SetFieldValue(attributePath, SdfFieldKeys->TypeName, VtValue(typeName));
	this->_SetFieldValue(attributePath, SdfFieldKeys->Variability, VtValue(variability));
	this->_SetFieldValue(attributePath, SdfFieldKeys->Default, value);

    // add this attribute to PropertyChildren of primPath
    VtValue existingPropertyChildrenValue;
    if (this->_GetFieldValue(primPath, SdfChildrenKeys->PropertyChildren, &existingPropertyChildrenValue))
    {
        // there are already children present, so append to the list
        TfTokenVector existingChildren = existingPropertyChildrenValue.UncheckedGet<TfTokenVector>();
        existingChildren.push_back(TfToken(name));

        // set the value back
        this->_SetFieldValue(primPath, SdfChildrenKeys->PropertyChildren, VtValue(existingChildren));
    }
    else
    {
        // no children present yet
        TfTokenVector children;
        children.push_back(TfToken(name));
        this->_SetFieldValue(primPath, SdfChildrenKeys->PropertyChildren, VtValue(children));
    }
}

void EdfData::_CreateSpec(const SdfPath& path, const SdfSpecType& specType)
{
	SpecData::accessor accessor;
    if (_specData.find(accessor, path))
    {
        accessor->second.specType = specType;
    }
    else
    {
        _specData.insert(accessor, path);
        accessor->second.specType = specType;
    }

    accessor.release();
}

bool EdfData::_GetSpecTypeAndFieldValue(const SdfPath& path, 
    const TfToken& fieldName, SdfSpecType* specType, VtValue* value) const
{
    // specType and value can be nullptrs here - this just means
    // we want to know if we have the field at all for a possible
    // subsequent call in the future
    if (specType != nullptr)
    {
        *specType = SdfSpecTypeUnknown;
    }

    SpecData::const_accessor accessor;
    if (_specData.find(accessor, path))
    {
        const _SpecData &spec = accessor->second;
        if (specType != nullptr)
        {
            *specType = spec.specType;
        }
        for (auto const& f: spec.fields)
        {
            if (f.first == fieldName)
            {
                // copy so that we don't give
                // back a direct pointer to a released
                // accessor
                if (value != nullptr)
                {
                    *value = value;
                }

                accessor.release();

                return true;
            }
        }
    }

    accessor.release();

    return false;
}

bool EdfData::_GetFieldValue(const SdfPath& path, 
    const TfToken& fieldName, VtValue* value) const
{
    // value can be a nullptr here - this just means
    // we want to know if we have the field at all for a
    // possible subsequent call in the future
    SpecData::const_accessor accessor;
    if (_specData.find(accessor, path))
    {
        const _SpecData &spec = accessor->second;
        for (auto const& f: spec.fields)
        {
            if (f.first == fieldName)
            {
                // copy so that we don't give
                // back a direct pointer to a released
                // accessor
                if (value != nullptr)
                {
                    *value = f.second;
                }

                accessor.release();

                return true;
            }
        }
    }

    accessor.release();

    return false;
}

void EdfData::_SetFieldValue(const SdfPath& path, const TfToken& fieldName, const VtValue& value)
{
	// TODO: if we ever wanted to add support for querying whether
	// the backend data provider could support writes, we should
	// query that here and ask them to write to their backing data store
	SpecData::accessor accessor;
    if (_specData.find(accessor, path))
    {
        _SpecData& spec = accessor->second;
        for (auto &f: spec.fields)
        {
            if (f.first == fieldName)
            {
                f.second = value;
                accessor.release();

                return;
            }
        }

        // if we get here, we didn't have the field yet so create it
        spec.fields.emplace_back(std::piecewise_construct,
                                std::forward_as_tuple(fieldName),
                                std::forward_as_tuple());

        spec.fields.back().second = value;
        accessor.release();

        return;
    }

    accessor.release();
}

void EdfData::_SetFieldValue(const SdfPath& path, const TfToken& fieldName, const VtValue& value) const
{
    // TODO: if we ever wanted to add support for querying whether
    // the backend data provider could support writes, we should
    // query that here and ask them to write to their backing data store
    SpecData::accessor accessor;
    if (_specData.find(accessor, path))
    {
        _SpecData& spec = accessor->second;
        for (auto& f : spec.fields)
        {
            if (f.first == fieldName)
            {
                f.second = value;
                accessor.release();

                return;
            }
        }

        // if we get here, we didn't have the field yet so create it
        spec.fields.emplace_back(std::piecewise_construct,
            std::forward_as_tuple(fieldName),
            std::forward_as_tuple());

        spec.fields.back().second = value;
        accessor.release();

        return;
    }

    accessor.release();
}

PXR_NAMESPACE_CLOSE_SCOPE
