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

#ifndef OMNI_EDF_EDFFILEFORMAT_H_
#define OMNI_EDF_EDFFILEFORMAT_H_

#define NOMINMAX

#include <pxr/pxr.h>
#include <pxr/base/tf/token.h>
#include <pxr/usd/sdf/fileFormat.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/pcp/dynamicFileFormatInterface.h>
#include <pxr/usd/pcp/dynamicFileFormatContext.h>

#include "api.h"
#include "iedfdataprovider.h"

PXR_NAMESPACE_OPEN_SCOPE

/// \class EdfFileFormat
///
/// Represents a generic dynamic file format for external data.
/// Actual acquisition of the external data is done via a set
/// of plug-ins to various back-end external data systems.
///
class EDF_API EdfFileFormat : public SdfFileFormat, public PcpDynamicFileFormatInterface
{
public:
	SdfAbstractDataRefPtr InitData(const FileFormatArguments& args) const override;

	// SdfFileFormat overrides
	bool CanRead(const std::string& filePath) const override;
	bool Read(SdfLayer* layer, const std::string& resolvedPath, bool metadataOnly) const override;
	bool WriteToString(const SdfLayer& layer, std::string* str, const std::string& comment = std::string()) const override;
	bool WriteToStream(const SdfSpecHandle& spec, std::ostream& out, size_t indent) const override;

	// PcpDynamicFileFormatInterface overrides
	void ComposeFieldsForFileFormatArguments(const std::string& assetPath, const PcpDynamicFileFormatContext& context, FileFormatArguments* args, VtValue* contextDependencyData) const override;
	bool CanFieldChangeAffectFileFormatArguments(const TfToken& field, const VtValue& oldValue, const VtValue& newValue, const VtValue& contextDependencyData) const override;

protected:

	SDF_FILE_FORMAT_FACTORY_ACCESS;

	bool _ShouldSkipAnonymousReload() const override;
	bool _ShouldReadAnonymousLayers() const override;

	virtual ~EdfFileFormat();
	EdfFileFormat();
};

TF_DECLARE_PUBLIC_TOKENS(
	EdfFileFormatTokens,
	((Id, "edfFileFormat")) 
	((Version, "1.0")) 
	((Target, "usd")) 
	((Extension, "edf")) 
	((Params, "EdfDataParameters")));

TF_DECLARE_WEAK_AND_REF_PTRS(EdfFileFormat);

PXR_NAMESPACE_CLOSE_SCOPE

#endif