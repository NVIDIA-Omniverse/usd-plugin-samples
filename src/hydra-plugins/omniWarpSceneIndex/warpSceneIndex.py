# Copyright 2023 NVIDIA CORPORATION
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import warp as wp
import numpy as np
from pxr import Vt, Sdf

@wp.kernel
def deform(positions: wp.array(dtype=wp.vec3), t: float):
    tid = wp.tid()

    x = positions[tid]

    offset = -wp.sin(x[0]) * 0.02
    scale = wp.sin(t)

    x = x + wp.vec3(0.0, offset * scale, 0.0)

    positions[tid] = x

class Example:
    def __init__(self, indices: Vt.IntArray, points: Vt.Vec3fArray):
        self.mesh = wp.Mesh(
            points=wp.array(points, dtype=wp.vec3),
            indices=wp.array(indices, dtype=int),
        )

    def update(self, sim_time: float):
        wp.launch(kernel=deform, dim=len(self.mesh.points), inputs=[self.mesh.points, sim_time])

        # refit the mesh BVH to account for the deformation
        self.mesh.refit()

wp.init()
global_examples = {}

def terminate_sim(primPath: Sdf.Path):
    global global_examples
    global_examples[primPath] = None

def initialize_sim_mesh(primPath: Sdf.Path, indices: Vt.IntArray, orig_points: Vt.Vec3fArray):
    global global_examples
    global_examples[primPath] = Example(indices, orig_points)

def exec_sim(primPath: Sdf.Path, sim_dt: float):
    global global_examples

    # Sim expects 60 samples per second (or hydra time of 1.0)
    global_examples[primPath].update(sim_dt / 60.0)
    return Vt.Vec3fArray.FromNumpy(global_examples[primPath].mesh.points.numpy())

def is_enabled():
    return True
