# Copyright (c) 2022 NVIDIA CORPORATION.  All rights reserved.
# NVIDIA CORPORATION and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto.  Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA CORPORATION is strictly prohibited.

###########################################################################
# Example Sim Cloth
#
# Shows a simulation of an FEM cloth model colliding against a static
# rigid body mesh using the wp.sim.ModelBuilder().
#
###########################################################################

import os
import math

import numpy as np

import warp as wp

import warp.sim
import warp.sim.render

from pxr import Usd, UsdGeom, Vt, Sdf

import sys

wp.init()
global_examples = {}


class Example:
    def __init__(self, indices: Vt.IntArray, points: Vt.Vec3fArray):
        self.sim_width = 64
        self.sim_height = 64
        
        self.frame_dt = 1.0 / 60
        self.frame_count = 400

        self.sim_substeps = 32
        self.sim_dt = self.frame_dt / self.sim_substeps
        self.sim_steps = self.frame_count * self.sim_substeps
        self.sim_time = 0.0

        builder = wp.sim.ModelBuilder()

        # sim BCs
        clothEdgeBendingStiffness = 0.01
        clothEdgeDampingStiffness = 0.0
        clothTriAreaStiffness = 1000000.0
        clothTriDampingStiffness = 100.0
        clothTriElasticStiffness = 1000000.0
        
        colliderContactDistance = 1.0
        colliderContactQueryRange = 100.0
        contactDampingStiffness = 10000.0
        contactElasticStiffness = 500000.0
        contactFrictionCoeff = 0.75
        contactFrictionStiffness = 10000.0

        globalScale = 0.01

        # cloth grid
        builder.add_cloth_grid(
            pos=(0.0, 50.0, -25.0),
            rot=wp.quat_from_axis_angle((1.0, 0.0, 0.0), math.pi * 0.5),
            vel=(0.0, 0.0, 0.0),
            dim_x=self.sim_width,
            dim_y=self.sim_height,
            cell_x=1.0,
            cell_y=1.0,
            mass=0.1,
            fix_left=True,
            tri_ke=clothTriElasticStiffness * globalScale,
            tri_ka=clothTriAreaStiffness * globalScale,
            tri_kd=clothTriDampingStiffness * globalScale,
            edge_ke=clothEdgeBendingStiffness * globalScale,
            edge_kd=clothEdgeDampingStiffness * globalScale
        )

        # add collider (must have identity transform until we xforms piped through Hydra plugin)
        mesh = wp.sim.Mesh(points, indices)
        builder.add_shape_mesh(
            body=-1,
            mesh=mesh,
            pos=(0.0, 0.0, 0.0),
            rot=wp.quat_identity(),
            scale=(1.0, 1.0, 1.0),
            ke=1.0e2,
            kd=1.0e2,
            kf=1.0e1,
        )

        # set sim BCs
        self.model = builder.finalize()
        self.model.ground = True
        self.model.allocate_soft_contacts(self.model.particle_count)
        self.model.gravity = (0, -980, 0)
        self.model.soft_contact_ke = contactElasticStiffness * globalScale
        self.model.soft_contact_kf = contactFrictionStiffness * globalScale
        self.model.soft_contact_mu = contactFrictionCoeff
        self.model.soft_contact_kd = contactDampingStiffness * globalScale
        self.model.soft_contact_margin = colliderContactDistance * colliderContactQueryRange
        self.model.particle_radius = colliderContactDistance

        self.integrator = wp.sim.SemiImplicitIntegrator()

        self.state_0 = self.model.state()
        self.state_1 = self.model.state()

    def update(self, sim_time: float):
        
        wp.sim.collide(self.model, self.state_0)

        for s in range(self.sim_substeps):
            self.state_0.clear_forces()

            self.integrator.simulate(self.model, self.state_0, self.state_1, self.sim_dt)

            (self.state_0, self.state_1) = (self.state_1, self.state_0)

def terminate_sim(primPath: Sdf.Path):
    global global_examples
    global_examples[primPath] = None

def initialize_sim_mesh(primPath: Sdf.Path, src_indices: Vt.IntArray, src_points: Vt.Vec3fArray,
    dep_mesh_indices: Vt.IntArray = None, dep_mesh_points: Vt.Vec3fArray = None, sim_params: dict = None):
    global global_examples
    global_examples[primPath] = Example(dep_mesh_indices, dep_mesh_points)

def exec_sim(primPath: Sdf.Path, sim_dt: float, dep_mesh_points: Vt.Vec3fArray = None, sim_params: dict = None):
    # Not respecting sim_dt at all, using internal time
    global global_examples
    global_examples[primPath].update(sim_dt)
    return Vt.Vec3fArray.FromNumpy(global_examples[primPath].state_0.particle_q.numpy())