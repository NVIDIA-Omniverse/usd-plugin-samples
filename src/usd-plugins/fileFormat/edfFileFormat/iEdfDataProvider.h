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

#ifndef OMNI_EDF_IEDFDATAPROVIDER_H_
#define OMNI_EDF_IEDFDATAPROVIDER_H_

#include <unordered_map>
#include <functional>
#include <memory>

#include <pxr/pxr.h>
#include <pxr/base/tf/token.h>
#include <pxr/base/vt/value.h>
#include <pxr/usd/sdf/fileFormat.h>
#include <pxr/usd/sdf/specType.h>
#include <pxr/usd/sdf/primSpec.h>

#include "api.h"

PXR_NAMESPACE_OPEN_SCOPE

///
/// \struct EdfDataParameters
/// 
/// Represents a class used to hold the specific metadata
/// parameter values used to construct the dynamic layer. 
///
struct EdfDataParameters
{
public:

	std::string dataProviderId;
	std::unordered_map<std::string, std::string> providerArgs;

	// conversion functions to and from USD structures
	static EdfDataParameters FromFileFormatArgs(const SdfFileFormat::FileFormatArguments& args);
};

///
/// \class IEdfSourceData
///
/// Interface for data providers to create prim / attribute information
/// and to read back attribute values as needed.
///
class IEdfSourceData
{
public:

	EDF_API virtual ~IEdfSourceData();

	/// Creates a new prim from data read from a back-end data source.
	/// \param parentPath The prim path that will be the parent of the newly created prim.
	/// \param name The name of the new prim.  This must be a valid USD identifier.
	/// \param specifier The spec type of the new prim (e.g., def, over, etc.).
	/// \param typeName The name of the type of the prim.
	///
	EDF_API virtual void CreatePrim(const SdfPath& parentPath, const std::string& name, const SdfSpecifier& specifier,
		const TfToken& typeName) = 0;
	
	/// Creates a new attribute on the specified prim.
	/// \param parentPrimPath The prim path of the prim that will contain the attribute.
	/// \param name The name of the attribute.
	/// \param typeName The name of the type of the attribute.
	/// \param variability The variability of the attribute (e.g., uniformm, varying, etc.).
	/// \param value The default value of the new attribute.
	///
	EDF_API virtual void CreateAttribute(const SdfPath& parentPrimPath, const std::string& name, const SdfValueTypeName& typeName,
		const SdfVariability& variability, const VtValue& value) = 0;

	/// Sets the value of a field on a prim at the given path.
	/// If the value exists, the current value will be overwritten.
	/// \param primPath The full path of the prim to set the field value for.
	/// \param fieldName The name of the field to set.
	/// \param value The value to set.
	///
	EDF_API virtual void SetField(const SdfPath& primPath, const TfToken& fieldName, const VtValue& value) = 0;

    /// Determines if the field fieldName exists on the given prim path.
    /// If the field exists, the current value will be returned in value if value is valid.
    /// \param primPath The full path of the prim to look for the field.
    /// \param fieldName The name of the field to look for on the prim.
    /// \param value A pointer to a VtValue object that will be filled with the value of
    ///              the field if it exists.
    /// 
    EDF_API virtual bool HasField(const SdfPath& primPath, const TfToken& fieldName, VtValue* value) = 0;

	/// Determines if the attribute at the given path exists and if so, returns the default value.
	/// \param attributePath The full path of the attribute (i.e., primPath + "." + attributeName).
	/// \param defaultValue A pointer to a VtValue object that will be filled with the default value
	///                     of the attribute if it exists.
	///
	EDF_API virtual bool HasAttribute(const SdfPath& attributePath, VtValue* defaultValue) = 0;
};

///
/// \class IEdfDataProvider
///
/// Interface for acquring data from an external data system based on a set of
/// metadata parameters fed to a dynamic payload.  This object is responsible for
/// acquiring the data from the external system and turning it into USD representations
/// that can be added to a layer.
///
class IEdfDataProvider
{
public:
	EDF_API virtual ~IEdfDataProvider();

	// disallow copies
	IEdfDataProvider(const IEdfDataProvider&) = delete;
	IEdfDataProvider& operator=(const IEdfDataProvider&) = delete;

	/// Asks the data provider to read whatever information they would like to read
	/// from the back-end system when a payload layer is first opened.
	///
	/// \param sourceData The source data interface which the data provider
	///	                  can use to create prims / attributes as needed when
	///                   they read data from their back-end.
	///
	EDF_API virtual bool Read(std::shared_ptr<IEdfSourceData> sourceData) = 0;

	/// Asks the data provider to read whatever would be considered the
	/// children of the provided prim path.  This gives the opportunity
	/// for the data provider to defer reading hierarhical children
	/// from their back-end store all at once when the data is large.
	///
	/// \param primPath The path of the prim to create children for.
	///                 This value is either "/Data", indicating the root
	///					of the hierarchy, or the full path to a prim
	///                 that was created by the data provider previously
	///                 on a Read / ReadChildren call.
	///
	/// \param sourceData The source data interface which the data provider
	///                   can use to create prims / attributes as needed when
	///                   they read data from their back-end.
	///
	EDF_API virtual bool ReadChildren(const std::string& primPath, std::shared_ptr<IEdfSourceData> sourceData) = 0;

	/// Asks the data provider whether all of its data was read on the initial
	/// Read call (i.e. the data has been cached in the source) or not.
	///
	/// \returns True if all data was read on initial Read, false otherwise.
	EDF_API virtual bool IsDataCached() const = 0;

protected:

	EDF_API IEdfDataProvider(const EdfDataParameters& parameters);
	EDF_API const EdfDataParameters& GetParameters() const;

private:

	EdfDataParameters _parameters;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif