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
import math
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
        self.frame_dt = 1.0 / 200
        self.frame_count = 400

        self.mesh = None

        self.sim_substeps = 64
        self.sim_dt = self.frame_dt / self.sim_substeps
        self.sim_steps = self.frame_count * self.sim_substeps
        self.sim_time = 0.0

        self.radius = 0.1

        self.builder = wp.sim.ModelBuilder()
        self.builder.default_particle_radius = self.radius

    def update(self):
        self.model.particle_grid.build(self.state_0.particle_q, self.radius * 2.0)

        # Do we need to call collide once we have a mesh shape ????
        #w p.sim.collide(self.model, self.state_0)

        for s in range(self.sim_substeps):
            self.state_0.clear_forces()

            self.integrator.simulate(self.model, self.state_0, self.state_1, self.sim_dt)

            # swap states
            (self.state_0, self.state_1) = (self.state_1, self.state_0)

def terminate_sim(primPath: Sdf.Path):
    global global_examples
    global_examples[primPath] = None


def initialize_sim_particles(primPath: Sdf.Path,
    orig_positions: Vt.Vec3fArray, mesh_indices: Vt.IntArray = None, mesh_points: Vt.Vec3fArray = None, sim_params: dict = None):
    global global_examples
    global_examples[primPath] = Example2()

    '''
    This does not work, add_shape_mesh() throws exception. Reason unknown

    global_examples[primPath].mesh = wp.Mesh(
        points=wp.array(mesh_points, dtype=wp.vec3),
        indices=wp.array(mesh_indices, dtype=int),
    )

    global_examples[primPath].builder.add_shape_mesh(
        body=-1,
        mesh=global_examples[primPath].mesh,
        pos=(0.0, 0.0, 0.0),
        scale=(1.0, 1.0, 1.0),
        ke=1.0e2,
        kd=1.0e2,
        kf=1.0e1,
        density=1e3,
    )
    '''

    for pt in orig_positions:
        global_examples[primPath].builder.add_particle(pt, (5.0, 0.0, 0.0), 0.1)

    global_examples[primPath].model = global_examples[primPath].builder.finalize()

    global_examples[primPath].model.particle_kf = 25.0
    global_examples[primPath].model.soft_contact_kd = 100.0
    global_examples[primPath].model.soft_contact_kf *= 2.0
    global_examples[primPath].state_0 = global_examples[primPath].model.state()
    global_examples[primPath].state_1 = global_examples[primPath].model.state()
    global_examples[primPath].integrator = wp.sim.SemiImplicitIntegrator()


def exec_sim(primPath: Sdf.Path, sim_dt: float, dep_vertices: Vt.Vec3fArray = None, sim_params: dict = None):
    # Not respecting sim_dt at all, using internal time
    global global_examples
    global_examples[primPath].update()
    return Vt.Vec3fArray.FromNumpy(global_examples[primPath].state_0.particle_q.numpy())

   


