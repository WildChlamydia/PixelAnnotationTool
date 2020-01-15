/*
    Copyright (c) 2017 Kuprashevich Maksim, Lanit-Tercom

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#ifndef TENSORFLOW_DETECTOR_H
#define TENSORFLOW_DETECTOR_H

#include "tensorflow_cnn_inferencer.hpp"

///
/// \brief The TensorflowDetector class
/// Specialization of TensorflowCNNInferencer for Detection purposes.
///
/// Expects:
///     Detection model from Tensorflow Models Object Detection API.
///     Expects 4 outputs: bounding boxes, scores, classes and num of detections
///     (optional, not needed but it exists in OD API).
///     Sequence should be such.
///
///     To run model you need to setup input data type in right flag.
///     For now usually model from OD API works with UINT8. So you need to setup it:
///     detector.setInputDtype(TensorflowDetector::DT_UINT8);
///     because default is float.
///     UPD: For now constructor is doing it.
///

class TensorflowDetector : public TensorflowCNNInferencer
{
public:
    TensorflowDetector() {
        setInputDtype(INPUT_TYPE::DT_UINT8);
    }

    TensorflowDetector(TensorflowDetector&& that);
    virtual ~TensorflowDetector() = default;

    ///
    /// \brief getOutputBoundingBoxes
    /// \param tresh Treshold for scores
    /// \return Normalized [0.0, 1.0] boxes.
    ///
    std::vector<std::vector<cv::Rect2f>> getOutputBoundingBoxes(float tresh = 0.5);

    ///
    /// \brief getOutputScores Probabilities to be the assigned class. Range is [0.0, 1.0]
    /// \return probabilities
    ///
    std::vector<std::vector<float>> getOutputScores();

    std::vector<std::vector<int>> getOutputClasses(float tresh = 0.5);

#ifdef TFDEBUG
    ///
    /// \brief getFramesWithBoundingBoxes Debug visualization method
    /// \param tresh
    /// \return Image with drawn bounding boxes
    ///
    std::vector<cv::Mat> getFramesWithBoundingBoxes(float tresh = 0.5);
#endif

    virtual std::string inference(const std::vector<cv::Mat> &imgs);

protected:
    virtual void clearSession();
    void parseOutput();

#ifdef TFDEBUG
    std::vector<cv::Mat> _bb_frames;
    std::vector<cv::Mat> _frames;
#endif

    std::vector<std::vector<cv::Rect2f>> _boxes;
    std::vector<std::vector<float>> _scores;
    std::vector<std::vector<int>> _classes;



    ///////////////////////////////////////////////
    /// Some magic constants
    /// Most probable will never change
    /// and exists just for readability
    static constexpr short DIM_INDX_BBOXES = 0;
    static constexpr short DIM_INDX_SCORES = 1;
    static constexpr short DIM_INDX_CLASSES = 2;

    static constexpr short DIM_SIZE_BBOXES = 3;
    static constexpr short DIM_SIZE_SCORES = 2;
    static constexpr short DIM_SIZE_CLASSES = 2;

    static constexpr short BBOX_TENSOR_INDEX_Y = 0;
    static constexpr short BBOX_TENSOR_INDEX_X = 1;
    static constexpr short BBOX_TENSOR_INDEX_H = 2;
    static constexpr short BBOX_TENSOR_INDEX_W = 3;

    static constexpr short DIM_INDX_N_OF_EXAMPLES = 0;
    static constexpr short DIM_INDX_N_OF_BBOXES = 1;
};

#endif // TENSORFLOW_DETECTOR_H
