#ifndef STATE_H
#define STATE_H

/******************************************************************
 *                                                                 *
 *  State                                                          *
 *                                                                 *
 ******************************************************************/

#include "viso_stereo.h"

namespace VisoGui
{
    namespace Core
    {
        namespace Models
        {
            class State
            {
            public:
                class VisualOdometry
                {
                public:
                    Viso::VisualOdometryStereo::parameters params;
                    int frame;
                    double x;
                    double y;
                    double z;
                    std::vector<std::pair<int, int>> xzGraph;
                    std::vector<std::pair<int, int>> yfGraph;
                    double num_matches;
                    double num_inliers;
                    std::string img_file_name;

                public:
                    VisualOdometry()
                    {
                        this->Clear();
                    };
                    void Clear()
                    {
                        params = Viso::VisualOdometryStereo::parameters();
                        frame = 0;
                        x = 0.0;
                        y = 0.0;
                        z = 0.0;
                        xzGraph.clear();
                        yfGraph.clear();
                        num_matches = 0.0;
                        num_inliers = 0.0;
                        img_file_name.clear();
                    };
                };
                VisualOdometry VO;

                State() : VO() {};
                void Clear()
                {
                    this->VO.Clear();
                }
            };
        }
    }
}

#endif // STATE_H
