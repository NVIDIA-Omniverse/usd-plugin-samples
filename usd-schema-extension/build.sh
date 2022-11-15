#!/bin/bash -e

# Copyright (c) 2022, NVIDIA CORPORATION.  All rights reserved.
#
# NVIDIA CORPORATION and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto.  Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA CORPORATION is strictly prohibited.

set -e

CWD="$( cd "$( dirname "$0" )" && pwd )"

# default config is release
CONFIG=release
PACKAGE=false
SKIPTEST=false

while [ $# -gt 0 ]
do
    if [[ "$1" == "debug" ]]
    then
        CONFIG=debug
    fi
    if [[ "$1" == "--package" ]]
    then
        PACKAGE=true
    fi
    if [[ "$1" == "--skip-test" ]]
    then
        SKIPTEST=true
    fi
    shift
done

echo Building $CONFIG configuration

if [[ "$CONFIG" == "debug" ]]
then
    BUILD_OPTION=debug-only
fi

if [[ "$CONFIG" == "release" ]]
then
    BUILD_OPTION=release-only
fi

# Step 1: Run usdgenschema
if [[ -v TEAMCITY_VERSION ]]; then
    echo "##teamcity[blockOpened name='Generate schema']"
fi

$CWD/repo.sh usdgenschema

if [[ -v TEAMCITY_VERSION ]]; then
    echo "##teamcity[blockClosed name='Generate schema']"
fi

# Step 2: Build C++ / Python libraries as needed
# Note: we wouldn't perform this step if the schema is codeless
if [[ -v TEAMCITY_VERSION ]]; then
    echo "##teamcity[blockOpened name='Build libraries']"
fi

$CWD/repo.sh build --$BUILD_OPTION

if [[ -v TEAMCITY_VERSION ]]; then
    echo "##teamcity[blockClosed name='Build libraries']"
fi

# Step 3: Test schema
if [[ "$SKIPTEST" == "false" ]]
then
    if [[ -v TEAMCITY_VERSION ]]; then
        echo "##teamcity[blockOpened name='Run tests']"
    fi

    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/_install/omniExampleSchema/linux-$(arch)_$CONFIG/lib:$(pwd)/_build/deps/usd_$CONFIG/lib

    $CWD/repo.sh test --config $CONFIG

    if [[ -v TEAMCITY_VERSION ]]; then
        echo "##teamcity[blockClosed name='Run tests']"
    fi
fi

# Step 4: Create the repo packages
if [[ "$PACKAGE" == "true" ]]
then
    if [[ -v TEAMCITY_VERSION ]]; then
        echo "##teamcity[blockOpened name='Create packages']"
    fi

    $CWD/repo.sh package --mode omni-example-schema --platform-target linux-$(arch) --root . --config $CONFIG
    $CWD/repo.sh package --mode omni-example-codeless-schema --platform-target linux-$(arch) --root . --config $CONFIG

    if [[ -v TEAMCITY_VERSION ]]; then
        echo "##teamcity[blockClosed name='Create packages']"
    fi
fi
