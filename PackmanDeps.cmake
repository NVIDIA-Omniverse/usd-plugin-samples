# SPDX-FileCopyrightText: Copyright (c) 2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
#

# tell nvopenusdbuildtools where to find python
set(PXR_OPENUSD_PYTHON_DIR "${CMAKE_CURRENT_LIST_DIR}/_build/usd-deps/python")

# add the path to OpenUSD to the prefix path
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}/_build/usd-deps/usd")

# add the path to find nvopenusdbuildtools
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/_build/host-deps/nvopenusdbuildtools/cmake")
