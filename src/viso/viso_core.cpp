/*
Copyright 2012. All rights reserved.
Institute of Measurement and Control Systems
Karlsruhe Institute of Technology, Germany

This file is part of libviso2.
Authors: Andreas Geiger

libviso2 is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or any later version.

libviso2 is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
libviso2; if not, write to the Free Software Foundation, Inc., 51 Franklin
Street, Fifth Floor, Boston, MA 02110-1301, USA
*/

/*
  Documented C++ sample code of stereo visual odometry (modify to your needs)
  To run this demonstration, download the Karlsruhe dataset sequence
  '2010_03_09_drive_0019' from: www.cvlibs.net!
  Usage: ./viso2 path/to/sequence/2010_03_09_drive_0019
*/

#include <filesystem>
#include <png.hpp>

#include "settings.h"
#include "core_utils.h"
#include "viso_core.h"

using PairOfInts = std::pair<int32_t, int32_t>;
using PairOfStrings = std::pair<std::string, std::string>;
using PairOfUint8Arrays = std::pair<std::unique_ptr<uint8_t[]>, std::unique_ptr<uint8_t[]>>;
using VectorOfStringPairs = std::vector<std::pair<std::string, std::string>>;
using VoDims = std::unique_ptr<int32_t[]>;
using VoParams = Viso::VisualOdometryStereo::parameters;
using namespace std;
using namespace VisoGui::Core::Models;
using namespace VisoGui::Core::Utils;

namespace Viso
{
  bool Core::StartViso(State &state, const string &lpCmdLine)
  {
    // Read processing params from command line
    ProcessingParams processingParams;
    if (!GetProcessingParams(lpCmdLine, processingParams))
    {
      return false;
    }

    // Create unique pointers for image data buffers and allocate memory
    PairOfUint8Arrays imageData = CreateImageDataArrays(processingParams);

    // Init visual odometry, calibration params and
    // current pose (this matrix transforms a point from the current
    // frame's camera coordinates to the first frame's camera coordinates)
    VoParams voParams = SetupVOParams();
    auto voDims = GetVODims(processingParams);
    VisualOdometryStereo viso(voParams);
    Matrix pose = Matrix::eye(4);

    // Init state
    state.Clear();
    SetStateVOParams(state, voParams);

    // Loop through all frames
    for (size_t i = 0; i < processingParams.imageFiles.size(); i++)
    {
      bool success = false;

      // Try to read and process current frame pair (l/r) data
      auto framePair = processingParams.imageFiles[i];
      success = GetImageData(processingParams, framePair, imageData) &&
                ComputeVO(viso, imageData, voDims);

      double num_matches = 0.0;
      double num_inliers = 0.0;
      if (success)
      {
        // on success, update current pose
        pose = pose * Matrix::inv(viso.getMotion());

        num_matches = viso.getNumberOfMatches();
        num_inliers = viso.getNumberOfInliers();
      }
      else
      {
        CoreUtils::PrintError("Error: VO failed processing current frame!");
      }

      PrintVOStats(success, i, pose, num_matches, num_inliers);
      SetStateVOStats(state, success, i, pose, num_matches, num_inliers, framePair.first);
    }

    // output
    CoreUtils::PrintInfo("Viso processing complete! Exiting ...");
    SetStateVOCompleted(state);

    // exit
    return true;
  }

  // Visual odometry methods
  VoParams Core::SetupVOParams()
  {
    // set most important visual odometry parameters
    // for a full parameter list, look at: viso_stereo.h
    VisualOdometryStereo::parameters params;

    // calibration parameters for sequence 2010_03_09_drive_0019
    params.calib.f = 645.24;  // focal length in pixels
    params.calib.cu = 635.96; // principal point (u-coordinate) in pixels
    params.calib.cv = 194.13; // principal point (v-coordinate) in pixels
    params.base = 0.5707;     // baseline in meters

    return params;
  }

  VoDims Core::GetVODims(const ProcessingParams &processingParams)
  {
    // Create unique pointer for VO dimensions and allocate memory
    unique_ptr<int32_t[]> voDims = make_unique<int32_t[]>(3);

    int width = processingParams.imageDimension.first;
    int height = processingParams.imageDimension.second;

    voDims[0] = width;
    voDims[1] = height;
    voDims[2] = width;

    return voDims;
  }

  bool Core::ComputeVO(VisualOdometryStereo &viso, const PairOfUint8Arrays &imageData, const VoDims &voDims)
  {
    bool success;

    try
    {
      // compute visual odometry
      success = viso.process(imageData.first.get(), imageData.second.get(), voDims.get());
    }
    catch (...)
    {
      success = false;
    }

    return success;
  }

  // Processing parameters methods
  bool Core::GetProcessingParams(const string &lpCmdLine, ProcessingParams &processingParams)
  {
    if (!ParseCommandLine(lpCmdLine, processingParams))
    {
      return false;
    }

    // Search all image files
    if (!GetImageFiles(processingParams, processingParams.imageFiles))
    {
      return false;
    }

    // Get image dimension
    if (!GetImageDimension(processingParams.imageFiles, processingParams.imageDimension))
    {
      return false;
    }

    return true;
  }

  bool Core::ParseCommandLine(const string &lpCmdLine, ProcessingParams &processingParams)
  {
    CoreUtils::PrintInfo("LibViso2 GUI app v1.0.0\n");

    bool showUsage = false;
    auto parseResult = CoreUtils::ParseCommandLine(lpCmdLine, processingParams);
    if (parseResult == CommandLineParseStatus::MultiplePathFound)
    {
      CoreUtils::PrintError("Error: Multiple image paths provided.\n");
      showUsage = true;
    }
    else if (parseResult == CommandLineParseStatus::NoPathFound)
    {
      CoreUtils::PrintError("Error: No images path provided.\n");
      showUsage = true;
    }
    else if (parseResult == CommandLineParseStatus::ShowHelp)
    {
      CoreUtils::PrintError("Help information\n");
      showUsage = true;
    }

    if (showUsage)
    {
      CoreUtils::PrintInfo("Usage: <images_path> [/p1=<images_prefix_left>] [/p2=<images_prefix_right>]");
      CoreUtils::PrintInfo("  <images_path>            : Path to the folder containing image files (mandatory)");
      CoreUtils::PrintInfo("  /p1=<images_prefix_left> : Prefix for left images (optional)");
      CoreUtils::PrintInfo("  /p2=<images_prefix_right>: Prefix for right images (optional)");
      return false;
    }
    else
    {
      // Print recap of passed parameters
      CoreUtils::PrintInfo("Parameters:");
      CoreUtils::PrintInfo("  images_path\t\t: " + processingParams.imagesPath);
      CoreUtils::PrintInfo("  images_prefix_left\t: " + processingParams.imagesPrefixLeft);
      CoreUtils::PrintInfo("  images_prefix_right\t: " + processingParams.imagesPrefixRight);
      CoreUtils::PrintInfo("  images_files_extension: " + Settings::Core::imagesExtension + "\n\n");

      return true;
    }
  }

  // Image processing methods
  bool Core::GetImageFiles(const ProcessingParams &processingParams, VectorOfStringPairs &imageFiles)
  {
    auto getResult = CoreUtils::GetImageFiles(processingParams, imageFiles);

    if (getResult == GetImageFilesStatus::InvalidPath)
    {
      CoreUtils::PrintError("Invalid images_path provided.");
      return false;
    }
    else if (getResult == GetImageFilesStatus::NoImageFound)
    {
      CoreUtils::PrintError("No image found in provided images_path.");
      return false;
    }
    else if (getResult == GetImageFilesStatus::ImagesNumberMismatch)
    {
      CoreUtils::PrintError("Mismatch in the number of left and right images, or not all L images have a matching R image.");
      return false;
    }

    CoreUtils::PrintInfo("Found " + to_string(imageFiles.size()) + " image pair(s)\n");

    return true;
  }

  bool Core::GetImageDimension(const VectorOfStringPairs &imageFiles, PairOfInts &imageDimension)
  {
    if (!CoreUtils::GetImageDimension(imageFiles, imageDimension))
    {
      CoreUtils::PrintError("Error: Couldn't read input images!");
      return false;
    }

    return true;
  }

  PairOfUint8Arrays Core::CreateImageDataArrays(const ProcessingParams &processingParams)
  {
    // Create unique pointers for image data buffers and allocate memory
    int width = processingParams.imageDimension.first;
    int height = processingParams.imageDimension.first;

    unique_ptr<uint8_t[]> leftImage = make_unique<uint8_t[]>(width * height);
    unique_ptr<uint8_t[]> rightImage = make_unique<uint8_t[]>(width * height);

    return std::make_pair(std::move(leftImage), std::move(rightImage));
  }

  bool Core::GetImageData(const ProcessingParams &processingParams, const PairOfStrings &imagePair, PairOfUint8Arrays &imageData)
  {
    if (!CoreUtils::GetImageData(processingParams, imagePair, imageData))
    {
      CoreUtils::PrintError("Error: Couldn't read input images!");
      return false;
    }

    return true;
  }

  // State update methods
  void Core::SetStateVOParams(State &state, const VisualOdometryStereo::parameters &voParams)
  {
    // Set VO Calibration parameters
    state.VO.params = voParams;
  }

  void Core::SetStateVOStats(State &state, bool success, int frame, Matrix &pose,
                             double num_matches, double num_inliers, const string &img_file_name)
  {
    // Set current processed frame
    state.VO.frame = frame;

    if (success)
    {
      // Extract coordinates from pose
      Viso::FLOAT x = 0.f, y = 0.f, z = 0.f;

      pose.getData(&x, 0, 3, 0, 3);
      pose.getData(&y, 1, 3, 1, 3);
      pose.getData(&z, 2, 3, 2, 3);

      // Set current coords
      state.VO.x = x;
      state.VO.y = y;
      state.VO.z = z;

      // Store coords in graphs
      state.VO.xzGraph.push_back({x, z});
      state.VO.yfGraph.push_back({y, frame});

      // Set current matches and inliers
      state.VO.num_matches = num_matches;
      state.VO.num_inliers = num_inliers;
    }

    // Set current image file name
    state.VO.img_file_name = img_file_name;
  }

  void Core::SetStateVOCompleted(State &state)
  {
    // Set vo processing completed by setting processed file to empty string
    state.VO.img_file_name = "";
  }

  // Console print methods
  void Core::PrintVOStats(bool success, int frame, const Matrix &pose, double num_matches, double num_inliers)
  {
    // output some statistics
    if (Settings::Core::isConsoleEnabled)
    {
      cout << "Processing: Frame: " << frame;

      if (success)
      {
        cout << ", Matches: " << num_matches;
        cout << ", Inliers: " << 100.0 * num_inliers / num_matches << " %" << ", Current pose: " << endl;
        cout << pose << endl
             << endl;
      }
      else
      {
        cout << " ... failed!" << endl;
      }
    }
  }
};
