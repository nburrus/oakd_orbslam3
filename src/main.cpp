/**
 * @file main.cpp
 * @author Duncan Hamill (duncanrhamill@googlemail.com)
 * @brief OAK-D/ORB_SLAM3 Experiments main file.
 * 
 * @version 0.1
 * @date 2021-01-13
 * 
 * @copyright Copyright (c) Duncan Hamill 2021
 */

/* -------------------------------------------------------------------------
 * INCLUDES
 * ------------------------------------------------------------------------- */

#include <iostream>

#include <opencv2/opencv.hpp>
#include <opencv2/ximgproc/disparity_filter.hpp>

#include "System.h"

#include "depthai/depthai.hpp"
#include "util.h"

#include "string_template.hpp"

/* -------------------------------------------------------------------------
 * CONSTANTS
 * ------------------------------------------------------------------------- */

// WLS parameters, taken from the OpenCV WLS filter docs recommended values.
#define WLS_LAMBDA (8000)
#define WLS_SIGMA (1.0)

/* -------------------------------------------------------------------------
 * STRUCTS
 * ------------------------------------------------------------------------- */

// A simple pose structure containing position vector and rotation matrix.
typedef struct _Pose {
    cv::Mat position;
    cv::Mat rotation;
} Pose;

/* -------------------------------------------------------------------------
 * MAIN
 * ------------------------------------------------------------------------- */

int main(int argc, char *argv[]) {

    std::cout << "OAK-D/ORB_SLAM3 Experiment" << std::endl;

    std::unordered_map<std::string, std::string> config_vars;

    // Create the pipeline that we're going to build. Pipelines are depthai's
    // way of chaining up different series or parallel process, sort of like
    // gstreamer. 
    //
    // Our pipeline is going to extract the left and right rectified images
    // from the cameras so we can pass these into the SLAM system, as well as
    // disparity maps for building a point cloud.
    dai::Pipeline pipeline;

    // We need to create all the nodes in our pipeline, which are:
    //  - the left and right monochrome (greyscale) stereo cameras of the OAK-D
    //  - a stereo depth node, which generates disparity maps and rectified
    //    images. The disparity map will be used in constructing the global 
    //    point cloud, and the rectified images for SLAM tracking.
    //  - output nodes, which allow us to get the rectified image data and
    //    disparity map to use outside the pipeline.
    auto mono_left = pipeline.create<dai::node::MonoCamera>();
    auto mono_right = pipeline.create<dai::node::MonoCamera>();
    auto stereo = pipeline.create<dai::node::StereoDepth>();
    auto xout_left = pipeline.create<dai::node::XLinkOut>();
    auto xout_right = pipeline.create<dai::node::XLinkOut>();
    // auto xout_disp = pipeline.create<dai::node::XLinkOut>();

    // And we set the names of each output node, so we can access them later as
    // output queues
    xout_left->setStreamName("rectified_left");
    xout_right->setStreamName("rectified_right");
    // xout_disp->setStreamName("disparity");

    // Now we set which cameras are actually connected to the left and right
    // nodes, and set their resolution and framerate
    mono_left->setBoardSocket(dai::CameraBoardSocket::LEFT);
    mono_left->setResolution(dai::MonoCameraProperties::SensorResolution::THE_480_P);
    mono_left->setFps(20.0);
    mono_right->setBoardSocket(dai::CameraBoardSocket::RIGHT);
    mono_right->setResolution(dai::MonoCameraProperties::SensorResolution::THE_480_P);
    
    const int stereo_width = 640;
    const int stereo_height = 480;

    config_vars["width"] = std::to_string(stereo_width);
    config_vars["height"] = std::to_string(stereo_height);

    config_vars["fps"] = "20";
    mono_right->setFps(20.0);    

    // Now we set the stereo node to output rectified images and disp maps. We
    // also set the rectify frames to not be mirrored, and to use black to fill
    // the edges of the rectified images. We need non-flipped images as we're
    // going to use them later down the line as input to the SLAM,
    // unfortunately this means our output disparity map will be flipped, so
    // we'll have to correct that later. We don't output depth as this would
    // disable the disparity map output.
    // 
    // We also enable extended disparity depth, which increases the maximum 
    // disparity and therefore provides a shorter minimum depth.
    // stereo->setOutputDepth(false);
    // stereo->setRectifyEdgeFillColor(0);
    // stereo->setExtendedDisparity(true);

    // We now link the cameras up to the stereo node
    mono_left->out.link(stereo->left);
    mono_right->out.link(stereo->right);

    // mono_left->out.link(xout_left->input);
    // mono_right->out.link(xout_right->input);

    // And the stereo rectified and disp outputs to the output nodes
    stereo->rectifiedLeft.link(xout_left->input);
    stereo->rectifiedRight.link(xout_right->input);
    // stereo->disparity.link(xout_disp->input);

    // Now we can connect to the OAK-D device and start our pipeline
    dai::Device device(pipeline);
    device.startPipeline();

    auto calib = device.readCalibration();
    auto leftIntrinsics = calib.getCameraIntrinsics(dai::CameraBoardSocket::LEFT, stereo_width, stereo_height);
    auto rightIntrinsics = calib.getCameraIntrinsics(dai::CameraBoardSocket::RIGHT, stereo_width, stereo_height);
    auto leftDistortion = calib.getDistortionCoefficients(dai::CameraBoardSocket::LEFT);
    auto rightDistortion = calib.getDistortionCoefficients(dai::CameraBoardSocket::RIGHT);
    auto distModel = calib.getDistortionModel(dai::CameraBoardSocket::LEFT);
    auto baselineMeters = 1e-2 * calib.getBaselineDistance(dai::CameraBoardSocket::LEFT, dai::CameraBoardSocket::RIGHT);

    config_vars["cam1_fx"] = std::to_string(rightIntrinsics[0][0]);
    config_vars["cam1_fy"] = std::to_string(rightIntrinsics[1][1]);
    config_vars["cam1_cx"] = std::to_string(rightIntrinsics[0][2]);
    config_vars["cam1_cy"] = std::to_string(rightIntrinsics[1][2]);

    // config_vars["cam1_k1"] = std::to_string(leftDistortion[0]);
    // config_vars["cam1_k2"] = std::to_string(leftDistortion[1]);
    // config_vars["cam1_p1"] = std::to_string(leftDistortion[2]);
    // config_vars["cam1_p2"] = std::to_string(leftDistortion[3]);
    // config_vars["cam1_k3"] = std::to_string(leftDistortion[4]);

    config_vars["cam1_k1"] = std::to_string(0.0);
    config_vars["cam1_k2"] = std::to_string(0.0);
    config_vars["cam1_p1"] = std::to_string(0.0);
    config_vars["cam1_p2"] = std::to_string(0.0);
    config_vars["cam1_k3"] = std::to_string(0.0);

    config_vars["cam2_fx"] = std::to_string(rightIntrinsics[0][0]);
    config_vars["cam2_fy"] = std::to_string(rightIntrinsics[1][1]);
    config_vars["cam2_cx"] = std::to_string(rightIntrinsics[0][2]);
    config_vars["cam2_cy"] = std::to_string(rightIntrinsics[1][2]);

    // config_vars["cam2_k1"] = std::to_string(rightDistortion[0]);
    // config_vars["cam2_k2"] = std::to_string(rightDistortion[1]);
    // config_vars["cam2_p1"] = std::to_string(rightDistortion[2]);
    // config_vars["cam2_p2"] = std::to_string(rightDistortion[3]);
    // config_vars["cam2_k3"] = std::to_string(rightDistortion[4]);

    config_vars["cam2_k1"] = std::to_string(0.0);
    config_vars["cam2_k2"] = std::to_string(0.0);
    config_vars["cam2_p1"] = std::to_string(0.0);
    config_vars["cam2_p2"] = std::to_string(0.0);
    config_vars["cam2_k3"] = std::to_string(0.0);

    auto leftFromRight = calib.getCameraExtrinsics(dai::CameraBoardSocket::RIGHT, dai::CameraBoardSocket::LEFT);

    // config_vars["Stereo.T_c1_c2"] = cv::format("[\n    %f, %f, %f, %f,\n    %f, %f, %f, %f, \n    %f, %f, %f, %f, \n    %f, %f, %f, %f ]\n",
    //     leftFromRight[0][0], leftFromRight[0][1], leftFromRight[0][2], leftFromRight[0][3]*1e-2,
    //     leftFromRight[1][0], leftFromRight[1][1], leftFromRight[1][2], leftFromRight[1][3]*1e-2,
    //     leftFromRight[2][0], leftFromRight[2][1], leftFromRight[2][2], leftFromRight[2][3]*1e-2,
    //     leftFromRight[3][0], leftFromRight[3][1], leftFromRight[3][2], leftFromRight[3][3]);

    config_vars["Stereo.T_c1_c2"] = cv::format("[\n    %f, %f, %f, %f,\n    %f, %f, %f, %f, \n    %f, %f, %f, %f, \n    %f, %f, %f, %f ]\n",
        1.0, 0.0, 0.0, baselineMeters,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0);
    
    config_vars["baseline_meters_times_fx"] = std::to_string(baselineMeters * rightIntrinsics[0][0]);
    config_vars["baseline_meters"] = std::to_string(baselineMeters);

    // [ 0.999983, 0.00445005, 0.00385861, 0.0636739954352379,
    //         -0.00443664, 0.999984, -0.00347621, -0.000252007856033742,
    //         -0.00387402, 0.00345903, 0.999986, -8.87895439518616e-05,
    //         0.0,0.0,0.0,1.0]

    tpl::render_template_file ("oak_d_orbslam_settings.yaml.tpl", "oak_d_orbslam_settings.yaml", config_vars);

    // Finally to actually see the outputs we need to get their output queues
    // We use a max buffer size of 8 frames and set it into non-blocking mode.
    auto rectif_left_queue = device.getOutputQueue("rectified_left", 8, false);
    auto rectif_right_queue = device.getOutputQueue("rectified_right", 8, false);
    // auto disp_queue = device.getOutputQueue("disparity", 8, false);
    
    // Create the WLS (weighted least squares) filter, which we use to improve
    // the quality of our disparity map. Also set the lambda and sigma values
    // auto wls_filter = cv::ximgproc::createDisparityWLSFilterGeneric(false);
    // wls_filter->setLambda(WLS_LAMBDA);
    // wls_filter->setSigmaColor(WLS_SIGMA);

    // To use OpenCV's reprojectImageTo3D we need a Q matrix, which is obtained
    // from stereoRectify. This means we'll have to extract some data from the
    // device itself, which is why this is done here. 
    // TODO: actually calculate these
    // cv::Mat R1, R2, P1, P2, Q;

    // Create the SLAM system. First argument is path to the ORB_SLAM3 vocab
    // file. The second is the path to the settings file for this particular
    // camera setup. The values in this file were taken from what's printed out
    // of `depthai_demo.py`.
    //
    // While the OAK-D does have an IMU we can't use it right now, but support
    // is coming soon! For now just use stereo mode for SLAM, which isn't as
    // accurate as IMU_STEREO.
    //
    // The last input tells the system to display it's UI.
    ORB_SLAM3::System SLAM(
        "ORB_SLAM3/Vocabulary/ORBvoc.txt",
        "oak_d_orbslam_settings.yaml",
        ORB_SLAM3::System::STEREO, 
        true
    );

    // Formatter, for printing out matrices in a reasonable way.
    cv::Ptr<cv::Formatter> fmt = cv::Formatter::get(cv::Formatter::FMT_DEFAULT);
    fmt->set64fPrecision(3);
    fmt->set32fPrecision(3);

    // We also want somewhere to store our pose data
    Pose pose;

    // The time of each frame is required for SLAM, so we take an epoch time
    // (i.e. our start time) now
    auto slam_epoch = std::chrono::steady_clock::now();

    // Now for the main loop
    while (1) {
        // Read the output frames from the OAK-D. These are blocking calls, so
        // they will wait until there's data available.
        auto rectif_left_frame = rectif_left_queue->get<dai::ImgFrame>();
        auto rectif_right_frame = rectif_left_queue->get<dai::ImgFrame>();
        // auto disp_map_frame = disp_queue->get<dai::ImgFrame>();

        // Convert the frames into opencv images
        auto rectif_left = imgframe_to_mat(rectif_left_frame);
        auto rectif_right = imgframe_to_mat(rectif_right_frame);
        // auto disp_map = imgframe_to_mat(disp_map_frame);

        rectif_left(cv::Rect(0,0,640,20)) = 0;
        rectif_left(cv::Rect(600,0,40,480)) = 0;

        rectif_right(cv::Rect(0,0,640,20)) = 0;
        rectif_right(cv::Rect(600,0,40,480)) = 0;

        // Get the time between the epoch and now, allowing us to get a
        // timestamp (in seconds) to pass into the slam system.
        auto elapsed_time = std::chrono::steady_clock::now() - slam_epoch;
        double frame_timestamp_s = elapsed_time.count() / 1000000000.0;

        std::cout << std::setprecision(4) << frame_timestamp_s << ": ";

        // Pass the images into the SLAM system. This produces a matrix with
        // the pose information of the camera.
        Sophus::SE3f camFromWorld = SLAM.TrackStereo(
            rectif_left,
            rectif_right,
            frame_timestamp_s
        );

        bool couldTrack = SLAM.GetTrackingState() == ORB_SLAM3::Tracking::eTrackingState::OK;

        // The output pose may be empty if the system was unable to track the
        // movement, so only get position and rotation if pose isn't empty. We
        // also put this info an a localisation fix available flag for later
        // use. 
        bool loc_fix_available = couldTrack;
        if (loc_fix_available) {
            // Print the updated position, but transpose it so that instead of
            // a column vector we have a row vector, which is easier to read.
            auto t = camFromWorld.inverse().translation();
            std::cout << "position: " << t[0] << ", " << t[1] << ", " << t[2] << std::endl;
        }
        else {
            // If we didn't get a pose update log it.
            std::cout << "no pose update" << std::endl;
        }

        // The raw disparity map is flipped, since we flipped the rectified
        // images, so we must flip it as well.
        // cv::flip(disp_map, disp_map, 1);

        // Filter the disparity map
        // cv::Mat filtered_disp_map;
        // wls_filter->filter(disp_map, rectif_right, filtered_disp_map);

        // Apply a colormap to the filtered disparity map, but don't normalise
        // it. Normalising the map will mean that the color doesn't correspond
        // directly with disparity.
        // cv::Mat colour_disp;
        // cv::applyColorMap(filtered_disp_map, colour_disp, cv::COLORMAP_JET);
        // cv::imshow("disparity", colour_disp);

        // See if q pressed, if so quit
        if (cv::waitKey(1) == 'q') {
            break;
        }
    }

    // Stop all SLAM threads
    SLAM.Shutdown();

    return EXIT_SUCCESS;
}