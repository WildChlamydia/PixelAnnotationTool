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

#ifndef TENSORFLOWSEGMENTATOR_H
#define TENSORFLOWSEGMENTATOR_H

#include "tensorflow_cnn_inferencer.hpp"

///
/// \brief The TensorflowSegmentator class
/// Specification of TensorflowCNNInferencer for segmentation purposes
///
///
/// Expects:
///     Model that takes image or batch of images of fixed size and outputs image OF THE SAME SIZE with indices.
///     For example takes on input 1x1024x1024x3 and outputs classes indices with size 1x1024x1024x1 or 1x1024x1024x3.
///     In last case indices expected to be found in channel 0.
///
///
/// Typical workflow example:
///
///  ```cv::Mat orig_img = cv::imread('.../img.png');
///     TensorflowSegmentator segmentator;
///     segmentator.load(".../model_frozen.pb");
///     segmentator.warmUp(); // Not necessary, just for further speedup
///     segmentator.inference({orig_img});
///     cv::Mat signle_batch_result = segmentator.getOutputColored()[0];```
///

class TensorflowSegmentator : public TensorflowCNNInferencer
{
public:
    TensorflowSegmentator() = default;
    virtual ~TensorflowSegmentator() = default;

    std::vector<cv::Mat> getOutputIndices();
    std::vector<cv::Mat> getOutputColored();

protected:
    virtual void clearSession();

    std::vector<cv::Mat> _imgs;
    std::vector<cv::Mat> _indices;
};

#endif // TENSORFLOWSEGMENTATOR_H
