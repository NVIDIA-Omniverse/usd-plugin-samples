# Copyright (c) 2022, NVIDIA CORPORATION.  All rights reserved.
#
# NVIDIA CORPORATION and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto.  Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA CORPORATION is strictly prohibited.

import os

from pxr import Plug

# the logic below assumes that the C++ / Python schema libs were built with the standard structure
# this structure is slightly modified when creating the kit extension to make it more
# intuitive for the user to import the Python schema library via kit (the original schema structure
# can be used independently of the kit-extension as normal - adding to PYTHONPATH and PATH / LD_LIBRARY_PATH
# appropriately)
# the kit-extension structure looks like the following:
# extension-root
#   PACKAGE-INFO.yaml
#   PACKAGE-LICENSES
#   lib
#     omniExampleSchema.dll
#   config
#     extension.toml
#   OmniExampleSchema
#     __init__.py
#     _omniExampleSchema.pyd
#   omni
#     example
#        schema
#           __init__.py
#   usd
#     OmniExampleSchema
#       resources
#         plugInfo.json
#         generatedSchema.usda
#         OmniExampleSchema
#           schema.usda
plugin_root = os.path.join(os.path.dirname(__file__), "..", "..", "..", "usd", "OmniExampleSchema", "resources")
Plug.Registry().RegisterPlugins(plugin_root)