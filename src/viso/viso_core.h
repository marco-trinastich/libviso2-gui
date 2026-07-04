#ifndef VISO_CORE_H
#define VISO_CORE_H

#include <optional>
#include <viso_stereo.h>

#include "models.h"
#include "state.h"

namespace Viso
{
    class Core
    {
        using string = std::string;
        using PairOfInts = std::pair<int32_t, int32_t>;
        using PairOfStrings = std::pair<std::string, std::string>;
        using PairOfUint8Arrays = std::pair<std::unique_ptr<uint8_t[]>, std::unique_ptr<uint8_t[]>>;
        using VectorOfStringPairs = std::vector<std::pair<std::string, std::string>>;
        using State = VisoGui::Core::Models::State;
        using VoDims = std::unique_ptr<int32_t[]>;
        using VoParams = VisualOdometryStereo::parameters;
        using ProcessingParams = VisoGui::Core::Models::ProcessingParams;

    public:
        static bool StartViso(State &state, const string &lpCmdLine);

    private:
        static VoParams SetupVOParams();
        static std::unique_ptr<int32_t[]> GetVODims(const ProcessingParams &processingParams);
        static bool ComputeVO(VisualOdometryStereo &viso, const PairOfUint8Arrays &imageData, const VoDims &voDims);
        static bool GetProcessingParams(const string &lpCmdLine, ProcessingParams &processingParams);
        static bool ParseCommandLine(const string &lpCmdLine, ProcessingParams &processingParams);
        static bool GetImageFiles(const ProcessingParams &processingParams, VectorOfStringPairs &imageFiles);
        static bool GetImageDimension(const VectorOfStringPairs &imageFiles, PairOfInts &imageDimension);
        static PairOfUint8Arrays CreateImageDataArrays(const ProcessingParams &processingParams);
        static bool GetImageData(const ProcessingParams &processingParams, const PairOfStrings &imagePair, PairOfUint8Arrays &imageData);

        static void SetStateVOParams(State &state, const VoParams &voParams);
        static void SetStateVOStats(State &state, bool success, int frame, Matrix &pose, double num_matches, double num_inliers,
                                    const string &img_file_name);
        static void SetStateVOCompleted(State &state);

        static void PrintVOStats(bool success, int frame, const Matrix &pose, double num_matches, double num_inliers);
    };
}

#endif // VISO_CORE_H
