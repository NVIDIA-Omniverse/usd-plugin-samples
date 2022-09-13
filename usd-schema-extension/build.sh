#!/bin/bash -e

# Copyright (c) 2022, NVIDIA CORPORATION.  All rights reserved.
#
# NVIDIA CORPORATION and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto.  Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA CORPORATION is strictly prohibited.

set -e

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

./repo.sh usdgenschema

if [[ -v TEAMCITY_VERSION ]]; then
    echo "##teamcity[blockClosed name='Generate schema']"
fi

# Step 2: Build C++ / Python libraries as needed
# Note: we wouldn't perform this step if the schema is codeless
if [[ -v TEAMCITY_VERSION ]]; then
    echo "##teamcity[blockOpened name='Build libraries']"
fi

./repo.sh build --$BUILD_OPTION

if [[ -v TEAMCITY_VERSION ]]; then
    echo "##teamcity[blockClosed name='Build libraries']"
fi

# Step 3: Test schema
if [[ "$SKIPTEST" == "false" ]]
then
    if [[ -v TEAMCITY_VERSION ]]; then
        echo "##teamcity[blockOpened name='Run tests']"
    fi

    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/_install/omniExampleSchema/linux-$(arch)_$CONFIG/lib:$(pwd)/_repo/deps/repo_usdgenschema/_build/deps/usd_py37_$CONFIG/lib

    ./repo.sh test --config $CONFIG

    if [[ -v TEAMCITY_VERSION ]]; then
        echo "##teamcity[blockClosed name='Run tests']"
    fi
fi

# Step 4: Gather up license dependencies
if [[ -v TEAMCITY_VERSION ]]; then
    echo "##teamcity[blockOpened name='Gather licenses']"
fi

./repo.sh licensing gather -d src/omniExampleSchema -p _repo/deps/repo_usdgenschema/deps/usd-deps.packman.xml --platform linux-$(arch) --config $CONFIG
./repo.sh licensing gather -d src/omniExampleCodelessSchema -p _repo/deps/repo_usdgenschema/deps/usd-deps.packman.xml --platform linux-$(arch) --config $CONFIG

# the license file should be generated in a local _build directory under where the PACKAGE-INFO.yaml file is
# need to copy it to the _install location
# NOTE - as of now, repo_licensing doesn't seem to gather up the USD license dependencies
# if this is ever fixed, those should also be copied to _install
# NOTE - this is a little wasteful because for every configuration we will actually create the codeless schema package 
# many times, but it's always the same package since there's no code so the configuration doesn't matter
# we do both debug and release of this thing so that packman configuration doesn't need special cases to only pull one version
mkdir -p _install/omniExampleSchema/linux-$(arch)_$CONFIG/PACKAGE-LICENSES
mkdir -p _install/omniExampleCodelessSchema/PACKAGE-LICENSES
cp src/omniExampleSchema/_build/PACKAGE-LICENSES/omni-example-schema-LICENSE.txt _install/omniExampleSchema/linux-$(arch)_$CONFIG/PACKAGE-LICENSES/omni-example-schema-LICENSE.txt
cp src/omniExampleCodelessSchema/_build/PACKAGE-LICENSES/omni-example-codeless-schema-LICENSE.txt _install/omniExampleCodelessSchema/PACKAGE-LICENSES/omni-example-codeless-schema-LICENSE.txt

if [[ -v TEAMCITY_VERSION ]]; then
    echo "##teamcity[blockClosed name='Gather licenses']"
fi

# Step 5: Create the repo packages
if [[ "$PACKAGE" == "true" ]]
then
    if [[ -v TEAMCITY_VERSION ]]; then
        echo "##teamcity[blockOpened name='Create packages']"
    fi

    ./repo.sh package --mode omni-example-schema --platform-target linux-$(arch) --root . --config $CONFIG
    ./repo.sh package --mode omni-example-codeless-schema --platform-target linux-$(arch) --root . --config $CONFIG

    if [[ -v TEAMCITY_VERSION ]]; then
        echo "##teamcity[blockClosed name='Create packages']"
    fi
fi
