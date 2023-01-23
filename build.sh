# Copyright 2023 NVIDIA CORPORATION
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -e
CWD="$( cd "$( dirname "$0" )" && pwd )"

# default config is release
CLEAN=false
GENERATE=false
STAGE=false
CONFIGURE=false

while [ $# -gt 0 ]
do
    if [[ "$1" == "clean" ]]
    then
        CLEAN=true
    fi
    if [[ "$1" == "--generate" ]]
    then
        GENERATE=true
    fi
    if [[ "$1" == "--stage" ]]
    then
        STAGE=true
    fi
    if [[ "$1" == "--configure" ]]
    then
        CONFIGURE=true
    fi
    shift
done

# do we need to clean?
if [[ "$CLEAN" == "true" ]]
then
    rm -rf $CWD/_install
fi

# do we need to generate?
if [[ "$GENERATE" == "true" ]]
then
    # pull down NVIDIA USD libraries
    # NOTE: If you have your own local build, you can comment out these steps
    $CWD/tools/packman/packman pull deps/usd-deps.packman.xml -p linux-$(arch) -t config=debug
    $CWD/tools/packman/packman pull deps/usd-deps.packman.xml -p linux-$(arch) -t config=release

    # generate the schema code and plug-in information
    # NOTE: this will pull the NVIDIA repo_usd package to do this work
    $CWD/tools/packman/python.sh bootstrap.py usd "$@"
fi

# NOTE: this is where you would integrate your build step

# do we need to stage?
if [[ "$STAGE" == "true" ]]
then
    cp -r $CWD/src/kit-extension/exts/omni.example.schema $CWD/_install/linux-$(arch)/omni.example.schema/
    mv -f $CWD/_install/linux-$(arch)/omniExampleSchema $CWD/_install/linux-$(arch)/omni.example.schema/OmniExampleSchema
    mv -f $CWD/_install/linux-$(arch)/omniExampleCodelessSchema $CWD/_install/linux-$(arch)/omni.example.schema/OmniExampleCodelessSchema

    # reparent the lib/python directory so we don't have to use [[lib.python.OmniExampleSchema]] in the extension.toml file
    mv -f $CWD/_install/linux-$(arch)/omni.example.schema/omniExampleSchema/lib/python/*.* $CWD/_install/linux-$(arch)/omni.example.schema/OmniExampleSchema
    rm -d $CWD/_install/linux-$(arch)/omni.example.schema/OmniExampleSchema/lib/python

    # under premake, a shared library will get generated without any content, we can remove this
    rm -rf $CWD/_install/linux-$(arch)/omni.example.schema/OmniExampleCodelessSchema/lib
fi

# do we need to configure?
# When using premake, the plugInfo.json files
# do not get their tokens replaced as premake
# does not have this functionality built in like cmake
if [[ "$CONFIGURE" == "true" ]]
then
    $CWD/tools/packman/python.sh boostrap.py usd --configure-plugInfo
fi