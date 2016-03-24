// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2015 Intel Corporation. All Rights Reserved.

//////////////////////////////////////////////////////////
// This set of tests is valid only for the SR300 camera //
//////////////////////////////////////////////////////////

#define CATCH_CONFIG_MAIN
#include "catch/catch.hpp"

#include "unit-tests-common.h"

#include <climits>
#include <sstream>
#include <algorithm>

TEST_CASE( "SR300 metadata enumerates correctly", "[live] [sr300]" )
{
    // Require at least one device to be plugged in
    safe_context ctx;
    const int device_count = rs_get_device_count(ctx, require_no_error());
    REQUIRE(device_count > 0);

    // For each device
    for(int i=0; i<device_count; ++i)
    {
        rs_device * dev = rs_get_device(ctx, 0, require_no_error());
        REQUIRE(dev != nullptr);

        SECTION( "device name is Intel RealSense SR300" )
        {
            const char * name = rs_get_device_name(dev, require_no_error());
            REQUIRE(name == std::string("Intel RealSense SR300"));
        }
    }
}

TEST_CASE( "SR300 devices support all required options", "[live] [sr300]" )
{
    // Require at least one device to be plugged in
    safe_context ctx;
    const int device_count = rs_get_device_count(ctx, require_no_error());
    REQUIRE(device_count > 0);

    // For each device
    for(int i=0; i<device_count; ++i)
    {
        rs_device * dev = rs_get_device(ctx, 0, require_no_error());
        REQUIRE(dev != nullptr);

        SECTION( "device supports standard picture options and SR300 extension options, and nothing else" )
        {
            const int supported_options[] = {
                RS_OPTION_COLOR_BACKLIGHT_COMPENSATION,
                RS_OPTION_COLOR_BRIGHTNESS,
                RS_OPTION_COLOR_CONTRAST,
                RS_OPTION_COLOR_EXPOSURE,
                RS_OPTION_COLOR_GAIN,
                RS_OPTION_COLOR_GAMMA,
                RS_OPTION_COLOR_HUE,
                RS_OPTION_COLOR_SATURATION,
                RS_OPTION_COLOR_SHARPNESS,
                RS_OPTION_COLOR_WHITE_BALANCE,
                RS_OPTION_COLOR_ENABLE_AUTO_EXPOSURE,
                RS_OPTION_COLOR_ENABLE_AUTO_WHITE_BALANCE,
                RS_OPTION_F200_LASER_POWER,
                RS_OPTION_F200_ACCURACY,
                RS_OPTION_F200_MOTION_RANGE,
                RS_OPTION_F200_FILTER_OPTION,
                RS_OPTION_F200_CONFIDENCE_THRESHOLD,                
                RS_OPTION_SR300_AUTO_RANGE_ENABLE_MOTION_VERSUS_RANGE,
                RS_OPTION_SR300_AUTO_RANGE_ENABLE_LASER,
                RS_OPTION_SR300_AUTO_RANGE_MIN_MOTION_VERSUS_RANGE,
                RS_OPTION_SR300_AUTO_RANGE_MAX_MOTION_VERSUS_RANGE,
                RS_OPTION_SR300_AUTO_RANGE_START_MOTION_VERSUS_RANGE,
                RS_OPTION_SR300_AUTO_RANGE_MIN_LASER,
                RS_OPTION_SR300_AUTO_RANGE_MAX_LASER,
                RS_OPTION_SR300_AUTO_RANGE_START_LASER,
                RS_OPTION_SR300_AUTO_RANGE_UPPER_THRESHOLD,
                RS_OPTION_SR300_AUTO_RANGE_LOWER_THRESHOLD
            };

            for(int i=0; i<RS_OPTION_COUNT; ++i)
            {
                if(std::find(std::begin(supported_options), std::end(supported_options), i) != std::end(supported_options))
                {
                    REQUIRE(rs_device_supports_option(dev, (rs_option)i, require_no_error()) == 1);
                }
                else
                {
                    REQUIRE(rs_device_supports_option(dev, (rs_option)i, require_no_error()) == 0);
                }
            }
        }
    }
}

///////////////////////////////////
// Calibration information tests //
///////////////////////////////////

TEST_CASE( "SR300 device extrinsics are within expected parameters", "[live] [sr300]" )
{
    // Require at least one device to be plugged in
    safe_context ctx;
    const int device_count = rs_get_device_count(ctx, require_no_error());
    REQUIRE(device_count > 0);

    // For each device
    for(int i=0; i<device_count; ++i)
    {
        rs_device * dev = rs_get_device(ctx, 0, require_no_error());
        REQUIRE(dev != nullptr);

        SECTION( "no extrinsic transformation between DEPTH and INFRARED" )
        {
            rs_extrinsics extrin;
            rs_get_device_extrinsics(dev, RS_STREAM_DEPTH, RS_STREAM_INFRARED, &extrin, require_no_error());

            require_identity_matrix(extrin.rotation);
            require_zero_vector(extrin.translation);
        }

        // TODO: Expected depth/color baseline

        SECTION( "depth scale is 0.000125 (by default)" )
        {
            REQUIRE( rs_get_device_depth_scale(dev, require_no_error()) == Approx(0.000125f) );
        }
    }
}

/////////////////////
// Streaming tests //
/////////////////////

inline void test_sr300_streaming(std::initializer_list<stream_mode> modes)
{
    safe_context ctx;
    REQUIRE(rs_get_device_count(ctx, require_no_error()) == 1);

    rs_device * dev = rs_get_device(ctx, 0, require_no_error());
    REQUIRE(dev != nullptr);
    REQUIRE(rs_get_device_name(dev, require_no_error()) == std::string("Intel RealSense SR300"));

    test_streaming(dev, modes);
}

///////////////////////////
// Depth streaming tests //
///////////////////////////

TEST_CASE( "SR300 streams 640x480 depth", "[live] [sr300] [one-camera]" )
{
    test_sr300_streaming({{RS_STREAM_DEPTH, 640, 480, RS_FORMAT_Z16, 60}});
}

TEST_CASE( "SR300 streams 640x240 depth", "[live] [sr300] [one-camera]" )
{
    test_sr300_streaming({{RS_STREAM_DEPTH, 640, 240, RS_FORMAT_Z16, 60}});
}

TEST_CASE( "SR300 streams 640x480 depth (30 fps)", "[live] [sr300] [one-camera]" )
{
    test_sr300_streaming({{RS_STREAM_DEPTH, 640, 480, RS_FORMAT_Z16, 30}});
}

TEST_CASE( "SR300 streams 640x240 depth (30 fps)", "[live] [sr300] [one-camera]" )
{
    test_sr300_streaming({{RS_STREAM_DEPTH, 640, 240, RS_FORMAT_Z16, 30}});
}

TEST_CASE( "SR300 streams 640x240 depth (110 fps)", "[live] [sr300] [one-camera]" )
{
    test_sr300_streaming({{RS_STREAM_DEPTH, 640, 240, RS_FORMAT_Z16, 110}});
}

///////////////////////////
// Color streaming tests //
///////////////////////////

TEST_CASE( "SR300 streams 1080p color", "[live] [sr300] [one-camera]" )
{
    test_sr300_streaming({{RS_STREAM_COLOR, 1920, 1080, RS_FORMAT_YUYV, 30}});
}

TEST_CASE( "SR300 streams 720p color", "[live] [sr300] [one-camera]" )
{
    test_sr300_streaming({{RS_STREAM_COLOR, 1280, 720, RS_FORMAT_YUYV, 30}});
}

TEST_CASE( "SR300 streams VGA color", "[live] [sr300] [one-camera]" )
{
    test_sr300_streaming({{RS_STREAM_COLOR, 640, 480, RS_FORMAT_YUYV, 30}});
}

TEST_CASE( "SR300 streams 720p color (60 fps)", "[live] [sr300] [one-camera]" )
{
    test_sr300_streaming({{RS_STREAM_COLOR, 1280, 720, RS_FORMAT_YUYV, 60}});
}

TEST_CASE( "SR300 streams VGA color (60 fps)", "[live] [sr300] [one-camera]" )
{
    test_sr300_streaming({{RS_STREAM_COLOR, 640, 480, RS_FORMAT_YUYV, 60}});
}

TEST_CASE( "SR300 streams VGA depth and HD color", "[live] [sr300] [one-camera]" )
{
    test_sr300_streaming({{RS_STREAM_DEPTH, 640, 480, RS_FORMAT_Z16, 60},
                          {RS_STREAM_COLOR, 1920, 1080, RS_FORMAT_YUYV, 30}});
}

TEST_CASE( "SR300 streams HVGA depth and HD color", "[live] [sr300] [one-camera]" )
{
    test_sr300_streaming({{RS_STREAM_DEPTH, 640, 240, RS_FORMAT_Z16, 60},
                          {RS_STREAM_COLOR, 1920, 1080, RS_FORMAT_YUYV, 30}});
}

TEST_CASE( "SR300 streams VGA depth and VGA color", "[live] [sr300] [one-camera]" )
{
    test_sr300_streaming({{RS_STREAM_DEPTH, 640, 480, RS_FORMAT_Z16, 60},
                          {RS_STREAM_COLOR, 640, 480, RS_FORMAT_YUYV, 30}});
}

TEST_CASE( "SR300 streams HVGA depth and VGA color", "[live] [sr300] [one-camera]" )
{
    test_sr300_streaming({{RS_STREAM_DEPTH, 640, 240, RS_FORMAT_Z16, 60},
                          {RS_STREAM_COLOR, 640, 480, RS_FORMAT_YUYV, 30}});
}

TEST_CASE( "SR300 streams VGA depth and VGA color (60 fps)", "[live] [sr300] [one-camera]" )
{
    test_sr300_streaming({{RS_STREAM_DEPTH, 640, 480, RS_FORMAT_Z16, 60},
                          {RS_STREAM_COLOR, 640, 480, RS_FORMAT_YUYV, 60}});
}

TEST_CASE( "SR300 streams HVGA depth and VGA color (60 fps)", "[live] [sr300] [one-camera]" )
{
    test_sr300_streaming({{RS_STREAM_DEPTH, 640, 240, RS_FORMAT_Z16, 60},
                          {RS_STREAM_COLOR, 640, 480, RS_FORMAT_YUYV, 60}});
}

//////////////////////////////
// Infrared streaming tests //
//////////////////////////////

TEST_CASE( "SR300 streams 640x480 infrared (30 fps)", "[live] [sr300] [one-camera]" )
{
    test_sr300_streaming({{RS_STREAM_INFRARED, 640, 480, RS_FORMAT_Y16, 30}});
}

TEST_CASE( "SR300 streams 640x480 infrared (60 fps)", "[live] [sr300] [one-camera]" )
{
    test_sr300_streaming({{RS_STREAM_INFRARED, 640, 480, RS_FORMAT_Y16, 60}});
}

TEST_CASE( "SR300 streams 640x480 infrared (120 fps)", "[live] [sr300] [one-camera]" )
{
    test_sr300_streaming({{RS_STREAM_INFRARED, 640, 480, RS_FORMAT_Y16, 120}});
}

TEST_CASE( "SR300 streams 640x480 infrared (200 fps)", "[live] [sr300] [one-camera]" )
{
    test_sr300_streaming({{RS_STREAM_INFRARED, 640, 480, RS_FORMAT_Y16, 200}});
}

TEST_CASE( "SR300 streams 640x480 depth and infrared", "[live] [sr300] [one-camera]" )
{
    test_sr300_streaming({{RS_STREAM_DEPTH, 640, 480, RS_FORMAT_Z16, 60},
                          {RS_STREAM_INFRARED, 640, 480, RS_FORMAT_Y16, 60}});
}

TEST_CASE( "SR300 streams 640x240 depth and infrared", "[live] [sr300] [one-camera]" )
{
    test_sr300_streaming({{RS_STREAM_DEPTH, 640, 240, RS_FORMAT_Z16, 60},
                          {RS_STREAM_INFRARED, 640, 240, RS_FORMAT_Y16, 60}});
}

TEST_CASE( "SR300 streams 640x240 depth and infrared (110 fps)", "[live] [sr300] [one-camera]" )
{
    test_sr300_streaming({{RS_STREAM_DEPTH, 640, 240, RS_FORMAT_Z16, 110},
                          {RS_STREAM_INFRARED, 640, 240, RS_FORMAT_Y16, 110}});
}

TEST_CASE( "SR300 streams 640x480 depth, infrared, and color", "[live] [sr300] [one-camera]" )
{
    test_sr300_streaming({{RS_STREAM_DEPTH, 640, 480, RS_FORMAT_Z16, 60},
                          {RS_STREAM_INFRARED, 640, 480, RS_FORMAT_Y16, 60},
                          {RS_STREAM_COLOR, 640, 480, RS_FORMAT_RGB8, 60}});
}

TEST_CASE( "SR300 streams 640x240 depth and infrared (110 fps), and 1080P color (30 fps)", "[live] [sr300] [one-camera]" )
{
    test_sr300_streaming({{RS_STREAM_DEPTH, 640, 240, RS_FORMAT_Z16, 110},
                          {RS_STREAM_INFRARED, 640, 240, RS_FORMAT_Y16, 110},
                          {RS_STREAM_COLOR, 1920, 1080, RS_FORMAT_RGB8, 30}});
}

TEST_CASE( "SR300 streams 640x480 infrared (200 fps), and VGA color (60 fps)", "[live] [sr300] [one-camera]" )
{
    test_sr300_streaming({{RS_STREAM_INFRARED, 640, 480, RS_FORMAT_Y16, 200},
                          {RS_STREAM_COLOR, 640, 480, RS_FORMAT_RGB8, 60}});
}

TEST_CASE( "SR300 streams 640x480 infrared (200 fps), and 1080P color (30 fps)", "[live] [sr300] [one-camera]" )
{
    test_sr300_streaming({{RS_STREAM_INFRARED, 640, 480, RS_FORMAT_Y16, 200},
                          {RS_STREAM_COLOR, 1920, 1080, RS_FORMAT_RGB8, 30}});
}

/////////////
// Options //
/////////////

enum { BEFORE_START_DEVICE = 1, AFTER_START_DEVICE = 2 };
inline void test_sr300_option(rs_option option, std::initializer_list<int> values, int when)
{
    safe_context ctx;
    REQUIRE(rs_get_device_count(ctx, require_no_error()) == 1);

    rs_device * dev = rs_get_device(ctx, 0, require_no_error());
    REQUIRE(dev != nullptr);
    REQUIRE(rs_get_device_name(dev, require_no_error()) == std::string("Intel RealSense SR300"));

    if(when & BEFORE_START_DEVICE)
    {
        test_option(dev, option, values, {});
    }

    if(when & AFTER_START_DEVICE)
    {
        rs_enable_stream_preset(dev, RS_STREAM_DEPTH, RS_PRESET_BEST_QUALITY, require_no_error());
        rs_start_device(dev, require_no_error());

        // Currently, setting/getting options immediately after streaming frequently raises hardware errors
        // todo - Internally block or retry failed calls within the first few seconds after streaming
        std::this_thread::sleep_for(std::chrono::seconds(1));
        test_option(dev, option, values, {});
    }
}

TEST_CASE( "SR300 supports RS_OPTION_F200_LASER_POWER", "[live] [sr300]" )
{
    test_sr300_option(RS_OPTION_F200_LASER_POWER, {0, 1, 2, 4, 8, 15}, AFTER_START_DEVICE);
}

TEST_CASE( "SR300 supports RS_OPTION_F200_ACCURACY", "[live] [sr300]" )
{
    test_sr300_option(RS_OPTION_F200_ACCURACY, {0, 1, 2, 3}, AFTER_START_DEVICE);
}

TEST_CASE( "SR300 supports RS_OPTION_F200_MOTION_RANGE", "[live] [sr300]" )
{
    test_sr300_option(RS_OPTION_F200_MOTION_RANGE, {0, 1, 8, 25, 50, 100}, AFTER_START_DEVICE);
}

TEST_CASE( "SR300 supports RS_OPTION_F200_FILTER_OPTION", "[live] [sr300]" )
{
    test_sr300_option(RS_OPTION_F200_FILTER_OPTION, {0, 1, 2, 3, 4, 5, 6, 7}, AFTER_START_DEVICE);
}

TEST_CASE( "SR300 supports RS_OPTION_F200_CONFIDENCE_THRESHOLD", "[live] [sr300]" )
{
    test_sr300_option(RS_OPTION_F200_LASER_POWER, {0, 1, 2, 4, 8, 15}, AFTER_START_DEVICE);
}

//////////////////////////////////////////
// Stop, reconfigure, and restart tests //
//////////////////////////////////////////

TEST_CASE( "a single SR300 can stream a variety of reasonable streaming mode combinations", "[live] [sr300] [one-camera]" )
{
    safe_context ctx;

    SECTION( "exactly one device is connected" )
    {
        int device_count = rs_get_device_count(ctx, require_no_error());
        REQUIRE(device_count == 1);
    }

    rs_device * dev = rs_get_device(ctx, 0, require_no_error());
    REQUIRE(dev != nullptr);

    SECTION( "device name is Intel RealSense SR300" )
    {
        const char * name = rs_get_device_name(dev, require_no_error());
        REQUIRE(name == std::string("Intel RealSense SR300"));
    }

    SECTION( "streaming is possible in some reasonable configurations" )
    {
        test_streaming(dev, {
            {RS_STREAM_DEPTH, 640, 480, RS_FORMAT_Z16, 60}
        });

        test_streaming(dev, {
            {RS_STREAM_DEPTH, 640, 480, RS_FORMAT_Z16, 60},
            {RS_STREAM_COLOR, 640, 480, RS_FORMAT_RGB8, 60}
        });

        test_streaming(dev, {
            {RS_STREAM_DEPTH, 640, 480, RS_FORMAT_Z16, 60},
            {RS_STREAM_INFRARED, 640, 480, RS_FORMAT_Y16, 60}
        });

        test_streaming(dev, {
            {RS_STREAM_DEPTH, 640, 480, RS_FORMAT_Z16, 60},
            {RS_STREAM_COLOR, 640, 480, RS_FORMAT_RGB8, 60},
            {RS_STREAM_INFRARED, 640, 480, RS_FORMAT_Y16, 60}
        });
    }
}
