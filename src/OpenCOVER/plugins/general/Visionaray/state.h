/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

#pragma once

#ifndef VSNRAY_PLUGIN_STATE_H
#define VSNRAY_PLUGIN_STATE_H 1

namespace visionaray
{

    //-------------------------------------------------------------------------------------------------
    // Control scene update behavior
    //

    enum device_type
    {
        CPU,
        GPU
    };
    enum data_variance
    {
        Static,
        AnimationFrames,
        Dynamic
    };
    enum algorithm
    {
        Simple,
        Whitted,
        Pathtracing
    };

    //-------------------------------------------------------------------------------------------------
    // State that affects rendering of a frame
    //

    struct render_state
    {
        algorithm algo = Simple;
        unsigned min_bounces = 1;
        unsigned max_bounces = 10;
        unsigned num_bounces = 4;
        device_type device = CPU;
        data_variance data_var = AnimationFrames;
        unsigned num_threads = 0;

        // non-persistent state for control flow
        int animation_frame = 0;
        bool rebuild = true;
    };

    //-------------------------------------------------------------------------------------------------
    // State that controls debug features
    //

    struct debug_state
    {
        bool debug_mode = true;
        bool suppress_rendering = false;     // Suppress ray tracing and resort to OpenGL
        bool show_bvh = false;
        bool show_bvh_costs = false;
        bool show_geometric_normals = false;
        bool show_shading_normals = false;
        bool show_tex_coords = false;
    };

} // namespace visionaray

#endif // VSNRAY_PLUGIN_STATE_H
