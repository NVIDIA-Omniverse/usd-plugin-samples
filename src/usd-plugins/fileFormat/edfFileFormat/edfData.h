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

#ifndef OMNI_EDF_EDFDATA_H_
#define OMNI_EDF_EDFDATA_H_

#include <string>
#include <set>

#include <pxr/pxr.h>
#include <pxr/base/tf/declarePtrs.h>
#include <pxr/usd/sdf/abstractData.h>
#include <pxr/usd/sdf/fileFormat.h>

#include <tbb/concurrent_hash_map.h>

#include "iEdfDataProvider.h"

PXR_NAMESPACE_OPEN_SCOPE

TF_DECLARE_PUBLIC_TOKENS(
	EdfDataParametersTokens,
	(dataProviderId)
	(providerArgs)
);

TF_DECLARE_WEAK_AND_REF_PTRS(EdfData);

/// \class EdfSourceData
///
/// Serves as a wrapper around EdfData for data providers to populate
/// information into.
///
class EdfSourceData : public IEdfSourceData
{
public:

	EdfSourceData(EdfData* data);
	virtual ~EdfSourceData();

	virtual void CreatePrim(const SdfPath& parentPath, const std::string& name, const SdfSpecifier& specifier,
		const TfToken& typeName) override;
	virtual void CreateAttribute(const SdfPath& parentPrimPath, const std::string& name, const SdfValueTypeName& typeName,
		const SdfVariability& variability, const VtValue& value) override;
	virtual void SetField(const SdfPath& primPath, const TfToken& fieldName, const VtValue& value) override;
    virtual bool HasField(const SdfPath& primPath, const TfToken& fieldName, VtValue* value) override;
	virtual bool HasAttribute(const SdfPath& attributePath, VtValue* defaultValue) override;

private:

	EdfData* _data;
};

/// \class EdfData
/// 
/// This class is used to hold the data required to open
/// a layer from files of "edf" format.  This data is initialized
/// by metadata unique to the prim the payload is attached to
/// and turned into file format args to create the appropriate
/// layer identifier for USD. 
///
class EdfData : public SdfAbstractData
{
public:

	static EdfDataRefPtr CreateFromParameters(const EdfDataParameters& parameters);

	// SdfAbstractData overrides
	void CreateSpec(const SdfPath& path, SdfSpecType specType) override;
	void Erase(const SdfPath& path, const TfToken& fieldName) override;
	void EraseSpec(const SdfPath& path) override;
	VtValue Get(const SdfPath& path, const TfToken& fieldName) const override;
	SdfSpecType GetSpecType(const SdfPath& path) const override;
	bool Has(const SdfPath& path, const TfToken& fieldName, SdfAbstractDataValue* value) const override;
	bool Has(const SdfPath& path, const TfToken& fieldName, VtValue* value = nullptr) const override;
	bool HasSpec(const SdfPath& path) const override;
	bool IsEmpty() const override;
	std::vector<TfToken> List(const SdfPath& path) const override;
	void MoveSpec(const SdfPath& oldPath, const SdfPath& newPath) override;
	void Set(const SdfPath& path, const TfToken& fieldName, const VtValue& value) override;
	void Set(const SdfPath& path, const TfToken& fieldName, const SdfAbstractDataConstValue& value) override;
	bool StreamsData() const override;
	bool IsDetached() const override;
	std::set<double> ListAllTimeSamples() const override;
	std::set<double> ListTimeSamplesForPath(const SdfPath& path) const override;
	bool GetBracketingTimeSamples(double time, double* tLower, double* tUpper) const override;
	size_t GetNumTimeSamplesForPath(const SdfPath& path) const override;
	bool GetBracketingTimeSamplesForPath(const SdfPath& path, double time, double* tLower, double* tUpper) const override;
	bool QueryTimeSample(const SdfPath& path, double time, VtValue* optionalValue = nullptr) const override;
	bool QueryTimeSample(const SdfPath& path, double time, SdfAbstractDataValue* optionalValue) const override;
	void SetTimeSample(const SdfPath& path, double time, const VtValue& value) override;
	void EraseTimeSample(const SdfPath& path, double time) override;

	virtual bool Read();

protected:

	// SdfAbstractDataOverrides
	void _VisitSpecs(SdfAbstractDataSpecVisitor* visitor) const override;

private:

	friend class EdfSourceData;

	// can only be constructed via CreateFromParameters
	EdfData(std::unique_ptr<IEdfDataProvider> dataProvider);

	// helper methods for retrieving spec properties
	// modeled after  SdfData
	bool _GetSpecTypeAndFieldValue(const SdfPath& path, 
	    const TfToken& fieldName, SdfSpecType* specType, VtValue* value) const;
	bool _GetFieldValue(const SdfPath& path, 
    	const TfToken& fieldName, VtValue* value) const;

	// helper methods for setting properties for the root
	// we don't have functionality for the public Set API
	// but we need to do it internally - if we ever added
	// support for the set API (i.e. the backend provider
	// supported writes), we could call this internally
	void _SetFieldValue(const SdfPath& path, const TfToken& fieldName, const VtValue& value);
    void _SetFieldValue(const SdfPath& path, const TfToken& fieldName, const VtValue& value) const;
    void _CreateSpec(const SdfPath& path, const SdfSpecType& specType);

	// instance methods for callbacks on context
	void _CreatePrim(const SdfPath& parentPath, const std::string& name,
		const SdfSpecifier& specifier, const TfToken& typeName);
	void _CreateAttribute(const SdfPath& primPath, const std::string& name,
		const SdfValueTypeName& typeName, const SdfVariability& variability, const VtValue& value);

private:

	// holds a pointer to the specific data provider to use
	// to query back-end data
	std::unique_ptr<IEdfDataProvider> _dataProvider;

	// holds a shared pointer to the source data object
	// used to callback on to create prims / attributes
	std::shared_ptr<IEdfSourceData> _sourceData;

    // mimic the storage structure of SdfData, just put it
    // in a concurrent_hash_map rather than a TfHashMap
    // the downside here is if we lock one field value for a write
    // the whole prim gets locked, but for our purposes
    // here that should be ok - the advantage we get is 
	// that on deferred reads we should be able to multithread
	// the back-end object acquisition during prim indexing
    typedef std::pair<TfToken, VtValue> _FieldValuePair;
    struct _SpecData {
        _SpecData() : specType(SdfSpecTypeUnknown) {}

        SdfSpecType specType;
        std::vector<_FieldValuePair> fields;
    };

    // Hash structure consistent with what TBB expects
    // but forwarded to what's already in USD
    struct SdfPathHash {
        static size_t hash(const SdfPath& path)
        {
            return path.GetHash();
        }

        static bool equal(const SdfPath& path1, const SdfPath& path2)
        {
            return path1 == path2;
        }
    };

    typedef tbb::concurrent_hash_map<SdfPath, _SpecData, SdfPathHash> SpecData;
    mutable SpecData _specData;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif