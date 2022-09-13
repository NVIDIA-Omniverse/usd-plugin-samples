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

# Step 1: TODO: Need to test the kit extensions here
# (not sure if we have to do it pre-copy or post-copy yet)


# Step 2: Aggregate everything under the _install directory
if [[ -v TEAMCITY_VERSION ]]; then
    echo "##teamcity[blockOpened name='Copying schema extension with kit extension']"
fi

mkdir -p _install/omni.example.schema
mkdir -p _install/omni.example.codeless.schema
cp -r ../usd-schema-extension/_install/omniExampleSchema/linux-$(arch)_$CONFIG _install/omni.example.schema/linux-$(arch)_$CONFIG/omniExampleSchema
cp -r ../usd-schema-extension/_install/omniExampleCodelessSchema _install/omni.example.codeless.schema/omniExampleCodelessSchema
cp -r src/omni.example.schema/config _install/omni.example.schema/linux-$(arch)_$CONFIG/config
cp -r src/omni.example.schema/omni _install/omni.example.schema/linux-$(arch)_$CONFIG/omni
cp -r src/omni.example.codeless.schema/config _install/omni.example.codeless.schema/config
cp -r src/omni.example.codeless.schema/omni _install/omni.example.codeless.schema/omni

if [[ -v TEAMCITY_VERSION ]]; then
    echo "##teamcity[blockClosed name='Copying schema extension with kit extension']"
fi

# Step 3: Run repo licensing to gather up the licenses
if [[ -v TEAMCITY_VERSION ]]; then
    echo "##teamcity[blockOpened name='Gather licenses']"
fi
./repo.sh licensing gather -d src/omni.example.schema -p deps/repo-deps.packman.xml --platform linux-$(arch) --config %CONFIG%
./repo.sh licensing gather -d src/omni.example.codeless.schema -p deps/repo-deps.packman.xml --platform linux-$(arch) --config %CONFIG%
if [[ -v TEAMCITY_VERSION ]]; then
    echo "##teamcity[blockClosed name='Gather licenses']"
fi

:: copy licenses to the root package directory
cp -r src/omni.example.schema/_build/PACKAGE-LICENSES _install/omni.example.schema/windows-x86_64_%CONFIG%/PACKAGE-LICENSES
cp -r src/omni.example.codeless.schema/_build/PACKAGE-LICENSES _install/omni.example.codeless.schema/PACKAGE-LICENSES

# Step 4: Package the extensions if requested
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