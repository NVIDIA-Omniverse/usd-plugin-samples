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

wp.init()

sim_params_global = {
    'wave_amplitude': 1.5,
    'wave_directionality': 0.0,
    'wind_speed': 10.0,
    'water_depth': 50.0,
    'scale': 1.0,
    'direction': 0.0,
    }

#warp function definitions

# fractional part of a (w.r.t. floor(a))
@wp.func
def frac(a: float):
    return a - wp.floor(a)

# square of a
@wp.func
def sqr(a: float):
    return a * a


@wp.func
def alpha_beta_spectrum(omega: float, 
                        peak_omega: float,
                        alpha: float,
                        beta: float,
                        gravity: float): 
    return ( (alpha * gravity * gravity / wp.pow(omega, 5.0)) * wp.exp(- beta * wp.pow(peak_omega/omega, 4.0)) )


@wp.func
def jonswap_peak_sharpening(omega: float, 
                            peak_omega: float,
                            gamma: float): 
    sigma = float(0.07)
    if omega > peak_omega: 
        sigma = float(0.09)
    return wp.pow(gamma, wp.exp(- 0.5 * sqr( (omega - peak_omega) / (sigma * peak_omega)) ))


@wp.func
def jonswap_spectrum(omega: float, 
                     gravity: float,
                     wind_speed: float,
                     fetch_km: float,
                     gamma: float): 
    #https://wikiwaves.org/Ocean-Wave_Spectra#JONSWAP_Spectrum
    fetch = 1000.0 * fetch_km
    alpha = 0.076 * wp.pow(wind_speed * wind_speed / (gravity * fetch), 0.22)
    peak_omega = 22.0 * wp.pow(wp.abs(gravity * gravity / (wind_speed * fetch)), 1.0/3.0)
    return (jonswap_peak_sharpening(omega, peak_omega, gamma) * alpha_beta_spectrum(omega, peak_omega, alpha, 1.25, gravity))


@wp.func
def TMA_spectrum(omega: float, 
                 gravity: float,
                 wind_speed: float,
                 fetch_km: float,
                 gamma: float,
                 waterdepth: float): 
    #https://dl.acm.org/doi/10.1145/2791261.2791267
    omegaH = omega * wp.sqrt(waterdepth/gravity)
    omegaH = wp.max(0.0, wp.min(2.2, omegaH))    
    phi = 0.5 * omegaH * omegaH
    if omegaH > 1.0:
        phi = 1.0 - 0.5 * sqr(2.0 - omegaH)
    return phi * jonswap_spectrum(omega, gravity, wind_speed, fetch_km, gamma); 
    


#warp kernel definitions
@wp.kernel
def update_profile(profile: wp.array(dtype=wp.vec3),
                   profile_res: int,
                   profile_data_num: int,
                   lambdaMin: float,
                   lambdaMax: float,
                   profile_extend: float,
                   time: float,
                   windspeed: float,
                   waterdepth: float
                   ):

    x = wp.tid()
    randself = wp.rand_init(7)
    # sampling parameters
    omega0 = wp.sqrt(2.0 * 3.14159 * 9.80665 / lambdaMin) 
    omega1 = wp.sqrt(2.0 * 3.14159 * 9.80665 / lambdaMax) 
    omega_delta = wp.abs(omega1 - omega0) /  float(profile_data_num)
    # we blend three displacements for seamless spatial profile tiling
    space_pos_1 = profile_extend * float(x) / float(profile_res)
    space_pos_2 = space_pos_1 + profile_extend
    space_pos_3 = space_pos_1 - profile_extend
    p1 = wp.vec2(0.0,0.0)
    p2 = wp.vec2(0.0,0.0)
    p3 = wp.vec2(0.0,0.0)
    for i in range(0, profile_data_num):
        omega = wp.abs(omega0 + (omega1 - omega0) * float(i) / float(profile_data_num))   # linear sampling of omega
        k = omega * omega / 9.80665	
        phase = -time * omega + wp.randf(randself) * 2.0 * 3.14159
        amplitude = float(10000.0) * wp.sqrt(wp.abs(2.0 * omega_delta * TMA_spectrum(omega, 9.80665,  windspeed,  100.0, 3.3, waterdepth)))
        p1 = wp.vec2( p1[0] + amplitude * wp.sin(phase + space_pos_1 * k), p1[1] - amplitude * wp.cos(phase + space_pos_1 * k) )
        p2 = wp.vec2( p2[0] + amplitude * wp.sin(phase + space_pos_2 * k), p2[1] - amplitude * wp.cos(phase + space_pos_2 * k) )
        p3 = wp.vec2( p3[0] + amplitude * wp.sin(phase + space_pos_3 * k), p3[1] - amplitude * wp.cos(phase + space_pos_3 * k) )
    # cubic blending coefficients
    s = float(float(x) / float(profile_res))
    c1 = float(2.0 * s * s * s - 3.0 * s * s + 1.0)
    c2 = float(-2.0 * s * s * s + 3.0 * s * s)
    disp_out = wp.vec3( (p1[0] + c1 * p2[0] + c2 * p3[0]) / float(profile_data_num), (p1[1] + c1 * p2[1] + c2 * p3[1]) / float(profile_data_num), 0. )
    wp.store(profile, x, disp_out)



@wp.kernel
def update_points(out_points: wp.array(dtype=wp.vec3),
                 in_points: wp.array(dtype=wp.vec3),
                 profile: wp.array(dtype=wp.vec3),
                 profile_res: int,
                 profile_extent: float,
                 amplitude: float,
                 directionality: float,
                 direction: float,
                 antiAlias: int,
                 camPosX: float,
                 camPosY: float,
                 camPosZ: float):

    tid = wp.tid()
    p_crd = in_points[tid]

    p_crd = wp.vec3(p_crd[0], p_crd[2], p_crd[1])

    randself = wp.rand_init(7)
    disp_x = float(0.)
    disp_y = float(0.)
    disp_z = float(0.)
    w_sum = float(0.)
    direction_count = (int)(128)
    for d in range(0, direction_count):
        r = float(d) * 2. * 3.14159265359 / float(direction_count) + 0.02
        dir_x = wp.cos(r)
        dir_y = wp.sin(r)
        # directional amplitude
        t = wp.abs( direction - r ) 
        if (t > 3.14159265359):
            t = 2.0 * 3.14159265359 - t 
        t = pow(t, 1.2) 
        dirAmp = (2.0 * t * t * t - 3.0 * t * t + 1.0) * 1.0  + (- 2.0 * t * t * t + 3.0 * t * t) * (1.0 - directionality) 
        dirAmp = dirAmp / (1.0 + 10.0 * directionality)
        rand_phase = wp.randf(randself)
        x_crd = (p_crd[0] * dir_x + p_crd[2] * dir_y) / profile_extent + rand_phase
        pos_0 = int(wp.floor(x_crd * float(profile_res))) % profile_res
        if x_crd < 0.:
            pos_0 = pos_0 + profile_res - 1 
        pos_1 = int(pos_0 + 1) % profile_res
        p_disp_0 = profile[pos_0] 
        p_disp_1 = profile[pos_1]
        w = frac( x_crd * float(profile_res) )
        prof_height_x = dirAmp * float((1. - w) * p_disp_0[0] + w * p_disp_1[0])
        prof_height_y = dirAmp * float((1. - w) * p_disp_0[1] + w * p_disp_1[1])
        disp_x = disp_x + dir_x * prof_height_x
        disp_y = disp_y + prof_height_y
        disp_z = disp_z + dir_y * prof_height_x
        w_sum = w_sum + 1.
    # simple anti-aliasing: reduce amplitude with increasing distance to viewpoint 
    if (antiAlias > 0):
        v1 = wp.normalize(  wp.vec3( p_crd[0] - camPosX, max( 100.0, wp.abs(p_crd[1] - camPosY)), p_crd[2] - camPosZ) )
        amplitude *= wp.sqrt( wp.abs(v1[1]) ) 
    # write output vertex position
    outP = wp.vec3(p_crd[0] + amplitude * disp_x / w_sum, p_crd[1] + amplitude * disp_y / w_sum, p_crd[2] + amplitude * disp_z / w_sum)
    wp.store(out_points, tid, wp.vec3(outP[0], outP[2], outP[1]))



class Example:
    def __init__(self, indices: Vt.IntArray, points: Vt.Vec3fArray):

        # profile buffer intializations
        print('[Ocean deformer] Initializing profile buffer.')

        self.profile_extent = 410.0  #physical size of profile, should be around half the resolution
        self.profile_res = int(8192)
        self.profile_wavenum = int(1000)
        self.profile_CUDA = wp.zeros(self.profile_res, dtype=wp.vec3, device="cuda:0")

        self.points_in = wp.array(points, dtype=wp.vec3, device="cuda:0")
        self.points_out = wp.array(points, dtype=wp.vec3, device="cuda:0")

        print(self.points_in)
        print(self.points_out)
        

    def update(self, sim_time: float):
        
        global sim_params_global

        # params
        wave_amplitude = sim_params_global["wave_amplitude"]
        wave_directionality = sim_params_global["wave_directionality"]
        wind_speed = sim_params_global["wind_speed"]
        water_depth = sim_params_global["water_depth"]
        scale = sim_params_global["scale"]
        direction = sim_params_global["direction"]

        # Parameters 
        time = float(sim_time)
        amplitude = max(0.0001, min(1000.0, float(wave_amplitude)))
        minWavelength = 0.1
        maxWavelength = 250.0
        direction = float(direction) % 6.28318530718
        directionality = max(0.0, min(1.0, 0.02 * float(wave_directionality)))
        windspeed = max(0.0, min(30.0, float(wind_speed)))
        waterdepth = max(1.0, min(1000.0, float(water_depth)))
        scale = min(10000.0, max(0.001, float(scale)))
        antiAlias = int(0)

        campos = [0.0, 0.0, 0.0]

        # create 1D profile buffer for this timestep using wave paramters stored in internal self CUDA memory
        wp.launch(
            kernel=update_profile,
            dim=self.profile_res, 
            inputs=[self.profile_CUDA, int(self.profile_res), int(self.profile_wavenum), float(minWavelength), float(maxWavelength), float(self.profile_extent), float(time), float(windspeed), float(waterdepth)], 
            outputs=[],
            device="cuda:0")
                                
        # update point positions using the profile buffer created above
        wp.launch(
            kernel=update_points,
            dim=len(self.points_out), 
            inputs=[self.points_out, self.points_in, self.profile_CUDA, int(self.profile_res), float(self.profile_extent*scale), float(amplitude), float(directionality), float(direction), int(antiAlias), float(campos[0]), float(campos[1]), float(campos[2]) ], 
            outputs=[],
            device="cuda:0")



global_examples = {}

def terminate_sim(primPath: Sdf.Path):

    global global_examples
    global_examples[primPath] = None

def initialize_sim_mesh(primPath: Sdf.Path, src_indices: Vt.IntArray, src_points: Vt.Vec3fArray,
    dep_mesh_indices: Vt.IntArray = None, dep_mesh_points: Vt.Vec3fArray = None, sim_params: dict = None):
    global global_examples
    global sim_params_global
    if sim_params:
        sim_params_global = sim_params

    global_examples[primPath] = Example(src_indices, src_points)

def exec_sim(primPath: Sdf.Path, sim_dt: float, dep_mesh_points: Vt.Vec3fArray = None, sim_params: dict = None):
    global global_examples
    global sim_params_global
    if sim_params:
        sim_params_global = sim_params

    # Sim expects 60 samples per second (or hydra time of 1.0)
    global_examples[primPath].update(sim_dt / 60.0)
    return Vt.Vec3fArray.FromNumpy(global_examples[primPath].points_out.numpy())
