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

#include "tensorflow_segmentator.hpp"

std::vector<cv::Mat> TensorflowSegmentator::getOutputIndices()
{
    if (!_is_loaded || _output_tensors.empty()) {
        return {};
    }

    if (_indices.empty()) {
        const auto& output = _output_tensors[0];
        _indices = tf_utils::convertSegmentTensorToMat(output);
    }

    return _indices;
}

std::vector<cv::Mat> TensorflowSegmentator::getOutputColored()
{
    if (!_is_loaded || _output_tensors.empty()) {
        return {};
    }

    if (_imgs.empty()) {
        const auto& output = _output_tensors[0];
        _imgs = tf_utils::convertSegmentTensorToMat(output, _label_colours);
    }

    return _imgs;
}

void TensorflowSegmentator::clearSession()
{
    TensorflowCNNInferencer::clearSession();

    _imgs.clear();
    _indices.clear();
}
