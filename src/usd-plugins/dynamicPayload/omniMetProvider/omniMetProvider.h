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

#ifndef OMNI_OMNIMETPROVIDER_OMNIMETPROVIDER_H_
#define OMNI_OMNIMETPROVIDER_OMNIMETPROVIDER_H_

#include <string>
#include <vector>
#include <utility>

#include <pxr/pxr.h>
#include <pxr/base/tf/token.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/sdf/schema.h>

#include <iEdfDataProvider.h>

PXR_NAMESPACE_OPEN_SCOPE

TF_DECLARE_PUBLIC_TOKENS(
    OmniMetProviderProviderArgKeys,
    (dataLodLevel)
    (deferredRead)
    (lod1Count)
);

/// \class OmniMetProvider
///
/// Defines a specific EDF back-end data provider for reading information
/// from the Metropolitan Museum of Art REST APIs and converting that
/// into prim and attribute data that can be processed by USD.
///
class OmniMetProvider : public IEdfDataProvider
{
public:

    OmniMetProvider(const EdfDataParameters& parameters);
    virtual ~OmniMetProvider();

    virtual bool Read(std::shared_ptr<IEdfSourceData> sourceData) override;
	virtual bool ReadChildren(const std::string& parentPath, std::shared_ptr<IEdfSourceData> sourceData) override;
    virtual bool IsDataCached() const override;

private:

    int GetDataLodLevel() const;
    size_t GetLod1Count() const;
    bool IsDeferredRead() const;

    void _LoadData(bool includeObjects, size_t objectCount, std::shared_ptr<IEdfSourceData> sourceData);
    std::string _LoadDepartments();
    std::vector<std::string> _LoadObjects(const std::string& departmentId, size_t objectCount);
    std::vector<std::pair<std::string, int>> _ParseDepartments(const std::string& departmentJson, 
        std::shared_ptr<IEdfSourceData> sourceData);
    void _ParseObject(const std::string& objectData, const std::string& parentPath, std::shared_ptr<IEdfSourceData> sourceData);

    // NOTE: these methods are not technically const, since they do change internal state
    // in the edfData object's layer data.  This is ok, because that object is a cache
    // https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es50-dont-cast-away-const
    // the mutuable cache state is allowed to change internally and still keep the semantics
    // of the object not changing from the outside
    void _LoadDepartments(bool includeObjects) const;
    void _LoadObjects(const std::string& departmentId, const std::string& parentPath) const;
    bool _IsDepartmentDataCached() const;
    bool _IsObjectDataCached(const std::string& parentPath) const;
    void _ParseDepartments(const std::string& response) const;
    std::vector<int> _ParseObjectIds(const std::string& response) const;
    void _ParseObject(const std::string& parentPath, const std::string& response) const;

    static size_t _CurlWriteCallback(void* data, size_t size, size_t nmemb, void* userp);
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif