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

import contextlib
import io
import packmanapi
import os
import sys

REPO_ROOT = os.path.dirname(os.path.realpath(__file__))
REPO_DEPS_FILE = os.path.join(REPO_ROOT, "deps", "repo-deps.packman.xml")

if __name__ == "__main__":
    # pull all repo dependencies first
    # and add them to the python path
    with contextlib.redirect_stdout(io.StringIO()):
        deps = packmanapi.pull(REPO_DEPS_FILE)

    for dep_path in deps.values():
        if dep_path not in sys.path:
            sys.path.append(dep_path)

    sys.path.append(REPO_ROOT)

    import omni.repo.usd
    omni.repo.usd.bootstrap(REPO_ROOT)