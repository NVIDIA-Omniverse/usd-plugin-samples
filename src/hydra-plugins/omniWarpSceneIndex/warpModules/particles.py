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

import os
import warp as wp
import warp.sim
import warp.sim.render
import numpy as np
from pxr import Vt, Sdf

wp.init()

global_examples = {}

# need radius of spehere

class Example2:
    def __init__(self):
        self.frame_dt = 1.0 / 60
        self.frame_count = 400

        self.sim_substeps = 64
        self.sim_dt = self.frame_dt / self.sim_substeps
        self.sim_steps = self.frame_count * self.sim_substeps
        self.sim_time = 0.0

        self.radius = 0.1

        self.builder = wp.sim.ModelBuilder()
        self.builder.default_particle_radius = self.radius

    def update(self):
        self.model.particle_grid.build(self.state_0.particle_q, self.radius * 2.0)

        for s in range(self.sim_substeps):
            self.state_0.clear_forces()

            self.integrator.simulate(self.model, self.state_0, self.state_1, self.sim_dt)

            # swap states
            (self.state_0, self.state_1) = (self.state_1, self.state_0)

def terminate_sim(primPath: Sdf.Path):
    global global_examples
    global_examples[primPath] = None

def initialize_sim_particles(primPath: Sdf.Path,
    src_positions: Vt.Vec3fArray, dep_mesh_indices: Vt.IntArray = None, dep_mesh_points: Vt.Vec3fArray = None, sim_params: dict = None):
    global global_examples
    global_examples[primPath] = Example2()

    for pt in src_positions:
        global_examples[primPath].builder.add_particle(pt, (5.0, 0.0, 0.0), 0.1)
    global_examples[primPath].model = global_examples[primPath].builder.finalize()
    global_examples[primPath].model.particle_kf = 25.0
    global_examples[primPath].model.soft_contact_kd = 100.0
    global_examples[primPath].model.soft_contact_kf *= 2.0
    global_examples[primPath].state_0 = global_examples[primPath].model.state()
    global_examples[primPath].state_1 = global_examples[primPath].model.state()
    global_examples[primPath].integrator = wp.sim.SemiImplicitIntegrator()

def exec_sim(primPath: Sdf.Path, sim_dt: float, dep_mesh_points: Vt.Vec3fArray = None, sim_params: dict = None):
    # Not respecting sim_dt at all, using internal time
    global global_examples
    global_examples[primPath].update()
    return Vt.Vec3fArray.FromNumpy(global_examples[primPath].state_0.particle_q.numpy())
   


