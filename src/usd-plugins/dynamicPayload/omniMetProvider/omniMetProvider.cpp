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

#include <pxr/base/tf/token.h>
#include <pxr/base/vt/value.h>
#include <pxr/base/js/json.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/sdf/schema.h>
#include <pxr/usd/sdf/payload.h>
#include <pxr/usd/sdf/primSpec.h>
#include <pxr/usd/sdf/attributeSpec.h>
#include <pxr/usd/usd/tokens.h>

#include <edfDataProviderFactory.h>

#include "omniMetProvider.h"

#include <iostream>
#include <curl/curl.h>

PXR_NAMESPACE_OPEN_SCOPE

EDF_DEFINE_DATAPROVIDER(OmniMetProvider);

TF_DEFINE_PUBLIC_TOKENS(
    OmniMetProviderProviderArgKeys,
    (dataLodLevel)
    (deferredRead)
    (lod1Count)
);

TF_DEFINE_PRIVATE_TOKENS(
    EdfFieldKeys,
    (EdfDataParameters)
);

TF_DEFINE_PRIVATE_TOKENS(
    OmniMetProviderTypeNames,
    (AmaDepartment)
    (AmaObject)
);

TF_DEFINE_PRIVATE_TOKENS(
    OmniMetProviderFieldKeys,
    (departmentId)
    (displayName)
    (objectID)
    (isHighlight)
    (accessionNumber)
    (accessionYear)
    (isPublicDomain)
    (primaryImage)
    (primaryImageSmall)
    (additionalImages)
    (constituents)
    (department)
    (objectName)
    (title)
    (culture)
    (period)
    (dynasty)
    (reign)
    (portfolio)
    (artistRole)
    (artistPrefix)
    (artistDisplayName)
    (artistDisplayBio)
    (artistSuffix)
    (artistAlphaSort)
    (artistNationality)
    (artistGender)
    (artistWikidata_URL)
    (artistULAN_URL)
    (objectDate)
    (objectBeginDate)
    (objectEndDate)
    (medium)
    (dimensions)
    (measurements)
    (creditLine)
    (geographyType)
    (city)
    (state)
    (county)
    (country)
    (region)
    (subregion)
    (locale)
    (locus)
    (excavation)
    (river)
    (classification)
    (rightsAndReproduction)
    (linkResource)
    (metadataDate)
    (repository)
    (objectURL)
    (objectWikidataURL)
    (isTimelineWork)
    (galleryNumber)
);

enum struct DataLodLevel
{
    Level0 = 0,
    Level1 = 1,
    Level2 = 2
};

// urls used to retrieve the data
static const std::string DEPARTMENT_URL = "https://collectionapi.metmuseum.org/public/collection/v1/departments";
static const std::string OBJECTS_IN_DEPARTMENT_URL = "https://collectionapi.metmuseum.org/public/collection/v1/objects?departmentIds=";
static const std::string OBJECT_URL = "https://collectionapi.metmuseum.org/public/collection/v1/objects/";
static const SdfPath DATA_ROOT_PATH("/Data");

OmniMetProvider::OmniMetProvider(const EdfDataParameters& parameters) : IEdfDataProvider(parameters)
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

OmniMetProvider::~OmniMetProvider()
{
    curl_global_cleanup();
}

bool OmniMetProvider::Read(std::shared_ptr<IEdfSourceData> sourceData)
{
    // this gives the provider a chance to load all data it needs to on first layer read
    // if we are parameterized for a deferred read, we do nothing and read on demand
    // at first ask, if it's not a deferred read, we load all appropriate content from the
    // back-end here
    if(!this->IsDeferredRead())
    {
        // it's not a deferred read, so determine how much data we want to really load
        int lodLevel = this->GetDataLodLevel();
        if (lodLevel == static_cast<int>(DataLodLevel::Level0))
        {
            // load the departments
            this->_LoadData(false, 0, sourceData);
        }
        else if (lodLevel == static_cast<int>(DataLodLevel::Level1))
        {
            // load the departments and their children
            // but cap the number of children at the specified level
            this->_LoadData(true, this->GetLod1Count(), sourceData);
        }
        else
        {
            // max lod level, load everything
            this->_LoadData(true, 0, sourceData);
        }
    }

    return true;
}

void OmniMetProvider::_LoadData(bool includeObjects, size_t objectCount, std::shared_ptr<IEdfSourceData> sourceData)
{
    // load the department data
    std::string departmentData = this->_LoadDepartments();
    std::vector<std::pair<std::string, int>> departments = this->_ParseDepartments(departmentData, sourceData);

    // do we want to load objects as well?
    if (includeObjects)
    {
        for (auto it = departments.begin(); it != departments.end(); it++)
        {
            std::vector<std::string> objectData = this->_LoadObjects(TfStringify(it->second), objectCount);
            for (auto itt = objectData.begin(); itt != objectData.end(); itt++)
            {
                this->_ParseObject(*itt, it->first, sourceData);
            }
        }
    }
}

std::string OmniMetProvider::_LoadDepartments()
{
    std::string departments;
    CURL* departmentCurl = curl_easy_init();
    if (departmentCurl != nullptr)
    {
        CURLcode resultCode;
        curl_easy_setopt(departmentCurl, CURLOPT_URL, DEPARTMENT_URL.c_str());
        curl_easy_setopt(departmentCurl, CURLOPT_HTTPGET, 1L);
        curl_easy_setopt(departmentCurl, CURLOPT_WRITEFUNCTION, OmniMetProvider::_CurlWriteCallback);

        // allocate a string that we can append the result onto
        std::string* result = new std::string();
        curl_easy_setopt(departmentCurl, CURLOPT_WRITEDATA, reinterpret_cast<void*>(result));

        resultCode = curl_easy_perform(departmentCurl);
        if (resultCode == CURLE_OK)
        {
            departments = *result;
        }
        else
        {
            TF_CODING_ERROR("Unable to load departments from '%s'!", DEPARTMENT_URL.c_str());
        }

        // done with the callback data
        delete result;

        // done with the request handle
        curl_easy_cleanup(departmentCurl);
    }

    return departments;
}

std::vector<int> OmniMetProvider::_ParseObjectIds(const std::string& response) const
{
    std::vector<int> objectIds;
    PXR_NS::JsValue jsValue = PXR_NS::JsParseString(response, nullptr);
    if (!jsValue.IsNull())
    {
        PXR_NS::JsObject rootObject = jsValue.GetJsObject();
        PXR_NS::JsObject::iterator it = rootObject.find("objectIDs");
        if (it != rootObject.end())
        {
            PXR_NS::JsArray jsonObjectIdArray = it->second.GetJsArray();
            for (auto objectIdIt = jsonObjectIdArray.begin(); objectIdIt != jsonObjectIdArray.end(); objectIdIt++)
            {
                objectIds.push_back((*objectIdIt).GetInt());
            }
        }
        else
        {
            TF_CODING_ERROR("Unable to find 'objectIDs' array in returned data '%s'!", response.c_str());
        }
    }
    else
    {
        TF_CODING_ERROR("Data returned '%s' was not JSON or was empty!", response.c_str());
    }

    return objectIds;
}

std::vector<std::string> OmniMetProvider::_LoadObjects(const std::string& departmentId, size_t objectCount)
{
    // NOTE: this should be updated to make these requests in parallel in the case
    // where we aren't doing deferred reads
    // ideally we wouldn't want to initialize a new curl handle here, but since this
    // call can be made in the parallel prim indexing, we can't share the easy handle
    // across threads, so we take the overhead hit here
    std::vector<std::string> objects;
    CURL* objectCurl = curl_easy_init();
    std::string url = OBJECTS_IN_DEPARTMENT_URL + departmentId;
    std::string* result = new std::string();
    CURLcode resultCode;

    *result = "";
    curl_easy_setopt(objectCurl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(objectCurl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(objectCurl, CURLOPT_WRITEFUNCTION, OmniMetProvider::_CurlWriteCallback);
    curl_easy_setopt(objectCurl, CURLOPT_WRITEDATA, reinterpret_cast<void*>(result));
    resultCode = curl_easy_perform(objectCurl);
    if (resultCode == CURLE_OK)
    {
        // process result
        std::vector<int> objectIds = this->_ParseObjectIds(*result);

        // objectCount = 0 means load all objects
        // objectCount > 0 means load max that many objects
        size_t counter = 0;
        for (auto objectIdIterator = objectIds.begin(); objectIdIterator != objectIds.end() && (objectCount == 0 || counter < objectCount); objectIdIterator++)
        {
            // reset the URL and result buffer
            // NOTE: this should be updated to make these requests in parallel
            url = OBJECT_URL + TfStringify(*objectIdIterator);
            *result = "";
            curl_easy_setopt(objectCurl, CURLOPT_URL, url.c_str());
            resultCode = curl_easy_perform(objectCurl);
            if (resultCode == CURLE_OK)
            {
                objects.push_back(*result);
            }
            counter++;
        }
    }

    // done with the callback data
    delete result;

    // done with the request handle
    curl_easy_cleanup(objectCurl);

    return objects;
}

std::vector<std::pair<std::string, int>> OmniMetProvider::_ParseDepartments(const std::string& departmentJson, 
    std::shared_ptr<IEdfSourceData> sourceData)
{
    std::vector<std::pair<std::string, int>> parsedDepartments;
    JsValue jsValue = JsParseString(departmentJson, nullptr);
    if (!jsValue.IsNull())
    {
        JsObject rootObject = jsValue.GetJsObject();
        JsObject::iterator it = rootObject.find("departments");
        if (it != rootObject.end())
        {
            JsArray departments = it->second.GetJsArray();
            std::string parent = DATA_ROOT_PATH.GetAsString();
            for (auto departmentIt = departments.begin(); departmentIt != departments.end(); departmentIt++)
            {
                // for each department, create a prim to represent it
                JsObject department = (*departmentIt).GetJsObject();
                int departmentId = department[OmniMetProviderFieldKeys->departmentId.GetString()].GetInt();
                std::string displayName = department[OmniMetProviderFieldKeys->displayName.GetString()].GetString();

                // create the prim
                std::string primName = TfMakeValidIdentifier(displayName);
                sourceData->CreatePrim(DATA_ROOT_PATH, primName, SdfSpecifier::SdfSpecifierDef,
                    OmniMetProviderTypeNames->AmaDepartment);

                // create the attributes for the prim
                SdfPath parentPrim = SdfPath(parent + "/" + primName);
                sourceData->CreateAttribute(parentPrim, OmniMetProviderFieldKeys->departmentId.GetString(),
                    SdfValueTypeNames->Int, SdfVariability::SdfVariabilityUniform, VtValue(departmentId));
                sourceData->CreateAttribute(parentPrim, OmniMetProviderFieldKeys->displayName.GetString(),
                    SdfValueTypeNames->String, SdfVariability::SdfVariabilityUniform, VtValue(displayName));

                parsedDepartments.push_back(std::make_pair(parentPrim.GetAsString(), departmentId));
            }
        }
        else
        {
            TF_CODING_ERROR("Unable to find 'departments' array in returned data '%s'!", departmentJson.c_str());
        }
    }
    else
    {
        TF_CODING_ERROR("Data returned '%s' was not JSON or was empty!", departmentJson.c_str());
    }

    return parsedDepartments;
}

void OmniMetProvider::_ParseObject(const std::string& objectData, const std::string& parentPath,
    std::shared_ptr<IEdfSourceData> sourceData)
{
    // from the parent path given and the data contained in the JSON
    // object retrieved from the server, we can create the full prim
    JsValue jsValue = JsParseString(objectData, nullptr);
    if (!jsValue.IsNull())
    {
        JsObject rootObject = jsValue.GetJsObject();

        // the root object contains all of our properties that we now need
        // to create a prim spec for the object and a set of property
        // specs for it
        // NOTE: this code uses the "default value" of a property spec
        // to represent the authored value coming from the external system
        // We don't need to do sub-composition over the data coming
        // from the external system, so we ever only have a value or not
        // so if HasDefaultValue is true on the property spec, it means
        // there was an authored value that came from the remote system
        // One optimization we could do in the layer above (EdfData) is 
        // to add schema acquisition and checking in the loop.  This would allow us 
        // to create the property spec or not depending on if the value that came in 
        // is different from the true fallback declared in the schema 
        // (but we'd have to change the ask for the property to check whether
        // the schema has the property rather than if the property spec exists)
        std::string objectName = rootObject[OmniMetProviderFieldKeys->objectName.GetString()].GetString();
        std::string primName = TfMakeValidIdentifier(objectName) +
            TfStringify(rootObject[OmniMetProviderFieldKeys->objectID.GetString()].GetInt());

        // create the prim
        SdfPath newPrimParentPath(parentPath);
        sourceData->CreatePrim(newPrimParentPath, primName, SdfSpecifier::SdfSpecifierDef,
            OmniMetProviderTypeNames->AmaObject);

        // set the fact that this prim has an API schema attached to it
        // usdGenSchema doesn't generate a public token for the actual
        // API schema class name, so we hard code that here
        SdfPath parentPrim = SdfPath(parentPath + "/" + primName);
        TfTokenVector apiSchemas;
        apiSchemas.push_back(TfToken("OmniMetArtistAPI"));
        VtValue apiSchemasValue(apiSchemas);
        sourceData->SetField(parentPrim, UsdTokens->apiSchemas, apiSchemasValue);

        // create the attributes for the prim
        sourceData->CreateAttribute(parentPrim, OmniMetProviderFieldKeys->objectID.GetString(),
            SdfValueTypeNames->Int, SdfVariability::SdfVariabilityUniform,
            VtValue(rootObject[OmniMetProviderFieldKeys->objectID.GetString()].GetInt()));
        sourceData->CreateAttribute(parentPrim, OmniMetProviderFieldKeys->isHighlight.GetString(),
            SdfValueTypeNames->Bool, SdfVariability::SdfVariabilityUniform,
            VtValue(rootObject[OmniMetProviderFieldKeys->isHighlight.GetString()].GetBool()));
        sourceData->CreateAttribute(parentPrim, OmniMetProviderFieldKeys->accessionNumber.GetString(),
            SdfValueTypeNames->String, SdfVariability::SdfVariabilityUniform,
            VtValue(rootObject[OmniMetProviderFieldKeys->accessionNumber.GetString()].GetString()));
        sourceData->CreateAttribute(parentPrim, OmniMetProviderFieldKeys->accessionYear.GetString(),
            SdfValueTypeNames->String, SdfVariability::SdfVariabilityUniform,
            VtValue(rootObject[OmniMetProviderFieldKeys->accessionYear.GetString()].GetString()));
        sourceData->CreateAttribute(parentPrim, OmniMetProviderFieldKeys->isPublicDomain.GetString(),
            SdfValueTypeNames->Bool, SdfVariability::SdfVariabilityUniform,
            VtValue(rootObject[OmniMetProviderFieldKeys->isPublicDomain.GetString()].GetBool()));
        sourceData->CreateAttribute(parentPrim, OmniMetProviderFieldKeys->primaryImage.GetString(),
            SdfValueTypeNames->String, SdfVariability::SdfVariabilityUniform,
            VtValue(rootObject[OmniMetProviderFieldKeys->primaryImage.GetString()].GetString()));
        sourceData->CreateAttribute(parentPrim, OmniMetProviderFieldKeys->primaryImageSmall.GetString(),
            SdfValueTypeNames->String, SdfVariability::SdfVariabilityUniform,
            VtValue(rootObject[OmniMetProviderFieldKeys->primaryImageSmall.GetString()].GetString()));

        // TODO: additional images, constituents

        sourceData->CreateAttribute(parentPrim, OmniMetProviderFieldKeys->department.GetString(),
            SdfValueTypeNames->String, SdfVariability::SdfVariabilityUniform,
            VtValue(rootObject[OmniMetProviderFieldKeys->department.GetString()].GetString()));
        sourceData->CreateAttribute(parentPrim, OmniMetProviderFieldKeys->title.GetString(),
            SdfValueTypeNames->String, SdfVariability::SdfVariabilityUniform,
            VtValue(rootObject[OmniMetProviderFieldKeys->title.GetString()].GetString()));
        sourceData->CreateAttribute(parentPrim, OmniMetProviderFieldKeys->culture.GetString(),
            SdfValueTypeNames->String, SdfVariability::SdfVariabilityUniform,
            VtValue(rootObject[OmniMetProviderFieldKeys->culture.GetString()].GetString()));
        sourceData->CreateAttribute(parentPrim, OmniMetProviderFieldKeys->period.GetString(),
            SdfValueTypeNames->String, SdfVariability::SdfVariabilityUniform,
            VtValue(rootObject[OmniMetProviderFieldKeys->period.GetString()].GetString()));
        sourceData->CreateAttribute(parentPrim, OmniMetProviderFieldKeys->dynasty.GetString(),
            SdfValueTypeNames->String, SdfVariability::SdfVariabilityUniform,
            VtValue(rootObject[OmniMetProviderFieldKeys->dynasty.GetString()].GetString()));
        sourceData->CreateAttribute(parentPrim, OmniMetProviderFieldKeys->reign.GetString(),
            SdfValueTypeNames->String, SdfVariability::SdfVariabilityUniform,
            VtValue(rootObject[OmniMetProviderFieldKeys->reign.GetString()].GetString()));
        sourceData->CreateAttribute(parentPrim, OmniMetProviderFieldKeys->portfolio.GetString(),
            SdfValueTypeNames->String, SdfVariability::SdfVariabilityUniform,
            VtValue(rootObject[OmniMetProviderFieldKeys->portfolio.GetString()].GetString()));

        // artist information complying with sample API schema
        std::string namespaceFieldPrefix = "omni:met:artist:";
        JsObject::const_iterator i = rootObject.find(OmniMetProviderFieldKeys->artistRole.GetString());
        if (i != rootObject.end())
        {
            sourceData->CreateAttribute(parentPrim, namespaceFieldPrefix + OmniMetProviderFieldKeys->artistRole.GetString(),
                SdfValueTypeNames->String, SdfVariability::SdfVariabilityUniform,
                VtValue(rootObject[OmniMetProviderFieldKeys->artistRole.GetString()].GetString()));
        }
        i = rootObject.find(OmniMetProviderFieldKeys->artistPrefix.GetString());
        if (i != rootObject.end())
        {
            sourceData->CreateAttribute(parentPrim, namespaceFieldPrefix + OmniMetProviderFieldKeys->artistPrefix.GetString(),
                SdfValueTypeNames->String, SdfVariability::SdfVariabilityUniform,
                VtValue(rootObject[OmniMetProviderFieldKeys->artistPrefix.GetString()].GetString()));
        }
        i = rootObject.find(OmniMetProviderFieldKeys->artistDisplayName.GetString());
        if (i != rootObject.end())
        {
            sourceData->CreateAttribute(parentPrim, namespaceFieldPrefix + OmniMetProviderFieldKeys->artistDisplayName.GetString(),
                SdfValueTypeNames->String, SdfVariability::SdfVariabilityUniform,
                VtValue(rootObject[OmniMetProviderFieldKeys->artistDisplayName.GetString()].GetString()));
        }
        i = rootObject.find(OmniMetProviderFieldKeys->artistDisplayBio.GetString());
        if (i != rootObject.end())
        {
            sourceData->CreateAttribute(parentPrim, namespaceFieldPrefix + OmniMetProviderFieldKeys->artistDisplayBio.GetString(),
                SdfValueTypeNames->String, SdfVariability::SdfVariabilityUniform,
                VtValue(rootObject[OmniMetProviderFieldKeys->artistDisplayBio.GetString()].GetString()));
        }
        i = rootObject.find(OmniMetProviderFieldKeys->artistSuffix.GetString());
        if (i != rootObject.end())
        {
            sourceData->CreateAttribute(parentPrim, namespaceFieldPrefix + OmniMetProviderFieldKeys->artistSuffix.GetString(),
                SdfValueTypeNames->String, SdfVariability::SdfVariabilityUniform,
                VtValue(rootObject[OmniMetProviderFieldKeys->artistSuffix.GetString()].GetString()));
        }
        i = rootObject.find(OmniMetProviderFieldKeys->artistAlphaSort.GetString());
        if (i != rootObject.end())
        {
            sourceData->CreateAttribute(parentPrim, namespaceFieldPrefix + OmniMetProviderFieldKeys->artistAlphaSort.GetString(),
                SdfValueTypeNames->String, SdfVariability::SdfVariabilityUniform,
                VtValue(rootObject[OmniMetProviderFieldKeys->artistAlphaSort.GetString()].GetString()));
        }
        i = rootObject.find(OmniMetProviderFieldKeys->artistNationality.GetString());
        if (i != rootObject.end())
        {
            sourceData->CreateAttribute(parentPrim, namespaceFieldPrefix + OmniMetProviderFieldKeys->artistNationality.GetString(),
                SdfValueTypeNames->String, SdfVariability::SdfVariabilityUniform,
                VtValue(rootObject[OmniMetProviderFieldKeys->artistNationality.GetString()].GetString()));
        }
        i = rootObject.find(OmniMetProviderFieldKeys->artistGender.GetString());
        if (i != rootObject.end())
        {
            sourceData->CreateAttribute(parentPrim, namespaceFieldPrefix + OmniMetProviderFieldKeys->artistGender.GetString(),
                SdfValueTypeNames->String, SdfVariability::SdfVariabilityUniform,
                VtValue(rootObject[OmniMetProviderFieldKeys->artistGender.GetString()].GetString()));
        }
        i = rootObject.find(OmniMetProviderFieldKeys->artistWikidata_URL.GetString());
        if (i != rootObject.end())
        {
            sourceData->CreateAttribute(parentPrim, namespaceFieldPrefix + OmniMetProviderFieldKeys->artistWikidata_URL.GetString(),
                SdfValueTypeNames->String, SdfVariability::SdfVariabilityUniform,
                VtValue(rootObject[OmniMetProviderFieldKeys->artistWikidata_URL.GetString()].GetString()));
        }
        i = rootObject.find(OmniMetProviderFieldKeys->artistULAN_URL.GetString());
        if (i != rootObject.end())
        {
            sourceData->CreateAttribute(parentPrim, namespaceFieldPrefix + OmniMetProviderFieldKeys->artistULAN_URL.GetString(),
                SdfValueTypeNames->String, SdfVariability::SdfVariabilityUniform,
                VtValue(rootObject[OmniMetProviderFieldKeys->artistULAN_URL.GetString()].GetString()));
        }

        // TODO: measurements, creditLine, etc.
    }
    else
    {
        TF_CODING_ERROR("Data returned '%s' was not JSON or was empty!", objectData.c_str());
    }
}

bool OmniMetProvider::ReadChildren(const std::string& parentPath, std::shared_ptr<IEdfSourceData> sourceData)
{
    // if the parent path is the root, we need to load the departments
    // but only if we are in a deferred read scenario
    if (this->IsDeferredRead())
    {
        SdfPath parentPrimPath = SdfPath(parentPath);
        int lodLevel = this->GetDataLodLevel();
        if (parentPrimPath == DATA_ROOT_PATH)
        {
            // load the department data
            std::cout << "Loading department data..." << std::endl;
            std::string departmentData = this->_LoadDepartments();
            std::vector<std::pair<std::string, int>> departments = this->_ParseDepartments(departmentData, 
                sourceData);
        }
        else
        {
            VtValue typeNameValue;
            if(sourceData->HasField(SdfPath(parentPath), SdfFieldKeys->TypeName, &typeNameValue))
            {
                if (typeNameValue.UncheckedGet<TfToken>() == OmniMetProviderTypeNames->AmaDepartment &&
                    this->GetDataLodLevel() != static_cast<int>(DataLodLevel::Level0))
                {
                    // it's a department, we need to load the objects
                    // associated with the department
                    std::string departmentIdPath = parentPath + "." + OmniMetProviderFieldKeys->departmentId.GetString();
                    VtValue departmentId;
                    if (sourceData->HasAttribute(SdfPath(departmentIdPath), &departmentId))
                    {
                        size_t objectCount = 0;
                        if (lodLevel == static_cast<int>(DataLodLevel::Level1))
                        {
                            objectCount = this->GetLod1Count();
                        }

                        // load the object data
                        std::cout << "Loading object data for " + parentPath + "..." << std::endl;
                        std::vector<std::string> objectData = this->_LoadObjects(TfStringify(departmentId.UncheckedGet<int>()), objectCount);
                        for (auto it = objectData.begin(); it != objectData.end(); it++)
                        {
                            this->_ParseObject(*it, parentPath, sourceData);
                        }
                    }
                }
            }
        }

        return true;
    }

    return false;
}

bool OmniMetProvider::IsDataCached() const
{
    return !this->IsDeferredRead();
}

int OmniMetProvider::GetDataLodLevel() const
{
    int dataLodLevel = 0;
    EdfDataParameters parameters = this->GetParameters();
    std::unordered_map<std::string, std::string>::const_iterator it = parameters.providerArgs.find(OmniMetProviderProviderArgKeys->dataLodLevel);
    if (it != parameters.providerArgs.end())
    {
        dataLodLevel = TfUnstringify<int>(it->second);
        if (dataLodLevel < 0)
        {
            dataLodLevel = 0;
        }
    }

    return dataLodLevel;
}

size_t OmniMetProvider::GetLod1Count() const
{
    // although the incoming string from the parameter set
    // might be interpretable as a negative integer
    // it doesn't really make practical sense, so if
    // it is interpreted as negative, we clamp to 0
    // and return an unsigned version to the caller
    size_t lod1Count = 0;
    EdfDataParameters parameters = this->GetParameters();
    std::unordered_map<std::string, std::string>::const_iterator it = parameters.providerArgs.find(OmniMetProviderProviderArgKeys->lod1Count);
    if (it != parameters.providerArgs.end())
    {
        lod1Count = TfUnstringify<int>(it->second);
        if (lod1Count < 0)
        {
            lod1Count = 0;
        }
    }

    return static_cast<size_t>(lod1Count);
}

bool OmniMetProvider::IsDeferredRead() const
{
    bool deferredRead = false;
    EdfDataParameters parameters = this->GetParameters();
    std::unordered_map<std::string, std::string>::const_iterator it = parameters.providerArgs.find(OmniMetProviderProviderArgKeys->deferredRead);
    if (it != parameters.providerArgs.end())
    {
        deferredRead = TfUnstringify<bool>(it->second);
    }

    return deferredRead;
}

size_t OmniMetProvider::_CurlWriteCallback(void* data, size_t size, size_t nmemb, void* userp)
{
    std::string* result = reinterpret_cast<std::string*>(userp);
    result->append(reinterpret_cast<const char* const>(data), nmemb);

    return nmemb;
}

PXR_NAMESPACE_CLOSE_SCOPE
