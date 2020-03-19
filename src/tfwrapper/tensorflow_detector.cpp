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

#include "tensorflow_detector.hpp"

TensorflowDetector::TensorflowDetector(TensorflowDetector &&that)
{

#ifdef TFDEBUG
    this->_bb_frames = std::move(that._bb_frames);
    this->_frames = std::move(that._frames);
#endif

    this->_boxes = std::move(that._boxes);
    this->_scores = std::move(that._scores);
    this->_classes = std::move(that._classes);
}

std::vector<std::vector<cv::Rect2f>> TensorflowDetector::getOutputBoundingBoxes(float tresh)
{
    if (!_boxes.size()) {
        parseOutput();
    }

    std::vector<std::vector<cv::Rect2f>> boxes;
    boxes.resize(_boxes.size());

    // display scores
//    for (auto s: _scores[0]) {
//        std::cerr << s << ", ";
//    }
//     std::cerr << "\n\n\n\n";

    for (short i = 0, total = _boxes.size(); i < total; ++ i) {

        // Do not care about float comparison - close to impossible that some scores will be equal.
        auto it = std::lower_bound(_scores[i].begin(), _scores[i].end(), tresh,
                                   [](float first, float second){ return first > second; });

        std::vector<cv::Rect2f> bb(_boxes[i].begin(), _boxes[i].begin() + (it - _scores[i].begin()));
        boxes[i] = bb;
    }

    return boxes;
}

void TensorflowDetector::parseOutput()
{
    using namespace tensorflow;

    if (_output_tensors.size() < _output_node_names.size()) {
        tf_utils::DebugOutput("Error", "Output size mismatch with expected!");
        return;
    }

    const int examples = _output_tensors[DIM_INDX_BBOXES].shape().dim_size(DIM_INDX_N_OF_EXAMPLES);
    _boxes.resize(examples);
    _scores.resize(examples);
    _classes.resize(examples);

    for (short example = 0; example < examples; ++example) {


        // Bounding boxes
        const Tensor& bb_tensor = _output_tensors[DIM_INDX_BBOXES];
        auto dims = bb_tensor.shape();

        _boxes[example].resize(dims.dim_size(DIM_INDX_N_OF_BBOXES));

        const auto& bb_mapped_tensor = bb_tensor.tensor<float, DIM_SIZE_BBOXES>();
        for (short box_num = 0, total = dims.dim_size(DIM_INDX_N_OF_BBOXES); box_num < total; ++box_num) {

            const float y = bb_mapped_tensor(example, box_num, BBOX_TENSOR_INDEX_Y);
            const float x = bb_mapped_tensor(example, box_num, BBOX_TENSOR_INDEX_X);

            const float h = bb_mapped_tensor(example, box_num, BBOX_TENSOR_INDEX_H) - y;
            const float w = bb_mapped_tensor(example, box_num, BBOX_TENSOR_INDEX_W) - x;


            _boxes[example][box_num] = cv::Rect2f(x, y, w, h);
        }



        // Scores
        const Tensor& scores_tensor = _output_tensors[DIM_INDX_SCORES];
        dims = scores_tensor.shape();

        _scores[example].resize(dims.dim_size(DIM_INDX_N_OF_BBOXES));

        const auto& scores_mapped_tensor = scores_tensor.tensor<float, DIM_SIZE_SCORES>();
        for (short box_num = 0, total = dims.dim_size(DIM_INDX_N_OF_BBOXES); box_num < total; ++box_num) {
            _scores[example][box_num] = scores_mapped_tensor(example, box_num);
        }



        // Classes
        const Tensor& classes_tensor = _output_tensors[DIM_INDX_CLASSES];
        dims = classes_tensor.shape();

        _classes[example].resize(dims.dim_size(DIM_INDX_N_OF_BBOXES));

        const auto& classes_mapped_tensor = classes_tensor.tensor<float, DIM_SIZE_CLASSES>();
        for (short box_num = 0, total = dims.dim_size(DIM_INDX_N_OF_BBOXES); box_num < total; ++box_num) {
            _classes[example][box_num] = classes_mapped_tensor(example, box_num);
        }


        if (_classes.size() != _boxes.size() ||  _boxes.size() != _scores.size()) {
            tf_utils::DebugOutput("Error", "Output sctuctures has mismatched size!");
        }
    }


}

void TensorflowDetector::clearSession()
{
    TensorflowCNNInferencer::clearSession();

    _boxes.clear();
    _scores.clear();
    _classes.clear();
#ifdef TFDEBUG
    _bb_frames.clear();
    _frames.clear();
#endif
}

std::string TensorflowDetector::inference(const std::vector<cv::Mat> &imgs)
{

#ifdef TFDEBUG
    // Is it ok? They can be changed because cv::Mat is pointer?
    _frames.clear();
    _frames = imgs;
    _bb_frames.clear();
#endif

    return TensorflowCNNInferencer::inference(imgs);
}

std::vector<std::vector<float> > TensorflowDetector::getOutputScores()
{
    if (!_scores.size()) {
        parseOutput();
    }

    return _scores;
}

std::vector<std::vector<int> > TensorflowDetector::getOutputClasses(float tresh)
{
    if (!_classes.size()) {
        parseOutput();
    }

    std::vector<std::vector<int>> classes;
    classes.resize(_boxes.size());

    for (short example = 0, total = _boxes.size(); example < total; ++ example) {

        // Do not care about float comparison - close to impossible that some scores will be equal.
        auto it = std::lower_bound(_scores[example].begin(), _scores[example].end(), tresh,
                                   [](float first, float second){ return first > second; });

        std::vector<int> class_vec(_classes[example].begin(),
                                   _classes[example].begin() + (it - _scores[example].begin()));
        classes[example] = class_vec;
    }

    return _classes;
}

#ifdef TFDEBUG
std::vector<cv::Mat> TensorflowDetector::getFramesWithBoundingBoxes(float tresh)
{
    if (_bb_frames.size()) {
        return _bb_frames;
    }

    if (!_boxes.size()) {
        parseOutput();
    }

    const auto& boxes = getOutputBoundingBoxes(tresh);
    const auto& classes = getOutputClasses(tresh);

    for (short i = 0, total = _frames.size(); i < total; ++i) {
        cv::Mat frame;
        _frames[i].copyTo(frame);

        for (short j = 0; j < boxes[i].size(); ++j) {

            const int x = boxes[i][j].tl().x * (float)frame.cols;
            const int y = boxes[i][j].tl().y * (float)frame.rows;
            const int x1 = boxes[i][j].br().x * (float)frame.cols;
            const int y1 = boxes[i][j].br().y * (float)frame.rows;

            cv::Scalar color = cv::Scalar(250, 250, 250);

            if (_label_colours.size() >= classes[i][j]) {
                const auto& cl = _label_colours[classes[i][j]];
                color = cv::Scalar(cl[0], cl[1], cl[2]);
            }

            cv::rectangle(frame, cv::Point(x, y), cv::Point(x1, y1), color, 4);
            cv::putText(frame, _label_names[classes[i][j]],
                    cv::Point(x, y - 10), cv::FONT_HERSHEY_SIMPLEX,
                    2.0, color, 2);

        }

        _bb_frames.push_back(frame);

    }

    return _bb_frames;

}
#endif
