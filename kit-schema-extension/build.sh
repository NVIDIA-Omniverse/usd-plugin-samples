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

mkdir -p $CWD/_install/linux-$(arch)_$CONFIG/omni.example.schema
mkdir -p $CWD/_install/linux-$(arch)_$CONFIG/omni.example.schema/lib
mkdir -p $CWD/_install/linux-$(arch)_$CONFIG/omni.example.schema/OmniExampleSchema
cp $CWD/src/omni.example.schema/PACKAGE-INFO.yaml $CWD/_install/linux-$(arch)_$CONFIG/omni.example.schema/PACKAGE-INFO.yaml
cp $CWD/../usd-schema-extension/_install/omniExampleSchema/linux-$(arch)_$CONFIG/lib/libomniExampleSchema.so $CWD/_install/linux-$(arch)_$CONFIG/omni.example.schema/lib/libomniExampleSchema.so
cp $CWD/../usd-schema-extension/_install/omniExampleSchema/linux-$(arch)_$CONFIG/lib/python/OmniExampleSchema/__init__.py $CWD/_install/linux-$(arch)_$CONFIG/omni.example.schema/OmniExampleSchema/__init__.py
cp $CWD/../usd-schema-extension/_install/omniExampleSchema/linux-$(arch)_$CONFIG/lib/python/OmniExampleSchema/_omniExampleSchema.so $CWD/_install/linux-$(arch)_$CONFIG/omni.example.schema/OmniExampleSchema/_omniExampleSchema.so
cp -r $CWD/../usd-schema-extension/_install/omniExampleSchema/linux-$(arch)_$CONFIG/usd $CWD/_install/linux-$(arch)_$CONFIG/omni.example.schema/usd
cp -r $CWD/src/omni.example.schema/config $CWD/_install/linux-$(arch)_$CONFIG/omni.example.schema/config
cp -r $CWD/src/omni.example.schema/omni $CWD/_install/linux-$(arch)_$CONFIG/omni.example.schema/omni

mkdir -p $CWD/_install/linux-$(arch)_$CONFIG/omni.example.codeless.schema
cp $CWD/src/omni.example.codeless.schema/PACKAGE-INFO.yaml $CWD/_install/linux-$(arch)_$CONFIG/omni.example.codeless.schema/PACKAGE-INFO.yaml
cp -r $CWD/src/omni.example.codeless.schema/config $CWD/_install/linux-$(arch)_$CONFIG/omni.example.codeless.schema/config
cp -r $CWD/src/omni.example.codeless.schema/omni $CWD/_install/linux-$(arch)_$CONFIG/omni.example.codeless.schema/omni
cp -r $CWD/../usd-schema-extension/_install/omniExampleCodelessSchema/usd $CWD/_install/linux-$(arch)_$CONFIG/omni.example.codeless.schema/usd

if [[ -v TEAMCITY_VERSION ]]; then
    echo "##teamcity[blockClosed name='Copying schema extension with kit extension']"
fi

# Step 3: Package the extensions if requested
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