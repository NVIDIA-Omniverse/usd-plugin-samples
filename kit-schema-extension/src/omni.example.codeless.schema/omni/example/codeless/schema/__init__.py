# Copyright (c) 2022, NVIDIA CORPORATION.  All rights reserved.
#
# NVIDIA CORPORATION and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto.  Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA CORPORATION is strictly prohibited.

import os

from pxr import Plug

# the logic below assumes that the schema libs were built with the standard structure
# and that the root of the schema package is at the same level as the config directory
# that is, the structure looks like the following:
# extension-root
#   config
#     extension.toml
#   omni
#     example
#        schema
#           __init__.py
#   omniExampleSCodelessSchema
#      PACKAGE-INFO.yaml
#      PACKAGE-LICENSES
#      usd
#         OmniExampleSCodelesschema
#            resources
#               plugInfo.json
#               generatedSchema.usda
#               OmniExampleCodelessSchema
#                  schema.usda
plugin_root = os.path.join(os.path.dirname(__file__), "..", "..", "..", "omniExampleCodelessSchema", "usd", "OmniExampleCodelessSchema", "resources")
Plug.Registry().RegisterPlugins(plugin_root)