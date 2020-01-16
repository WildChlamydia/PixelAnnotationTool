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

#ifndef TENSORFLOWUTILS_H
#define TENSORFLOWUTILS_H

//#include "opencv/cv.h"
//#include "opencv/cv.hpp"

#include <opencv2/opencv.hpp>
#include "tensorflow/core/public/session.h"
#include "tensorflow/core/graph/default_device.h"

#define TFDEBUG

namespace tf_utils
{

////
/// \brief MAX_SIZE_FOR_PYR_METHOD Max size in fastResize function for that pyrDown or pyrUp will be used
/// (because method is slower then resize on big images).
///
constexpr int MAX_SIZE_FOR_PYR_METHOD = 500;
///

/// For time measure
struct profiler;
/// Profile helper
#define PROFILE_BLOCK(pbn) tf_utils::profiler _pfinstance(pbn)

inline void DebugOutput(const std::string& header, const std::string& msg) {
#ifdef TFDEBUG
    std::cerr << header << ": " << msg << "\n";
#endif
}


void fastResizeIfPossible(const cv::Mat& in, cv::Mat *dist, const cv::Size& size);

///
/// \brief convertTensorToMat Converts batch of Tensors or single Tensor (shape must be still size of 4) to batch of Mats
/// \param tensor
/// \param colours If set will be returned 3 channel Mat with image, otherwise Mat with indices of classes
/// \return Batch of images
///
std::vector<cv::Mat> convertSegmentTensorToMat(const tensorflow::Tensor &tensor,
                                               const std::vector<std::vector<int>> &colours = {});

///
/// \brief getShapesFromMessage Allows to get shape from Graph Def
/// \param val tensorflow::AttrValue. For example: graph_def.node(0).attr()["shape"];
/// \return vector with dims
///
std::vector<int> getShapesFromMessage(tensorflow::AttrValue& val);


/// Template functions and structures


///
/// \brief convertTensorToMat Converts batch of Mats to Tensor format
/// \param imgs Vector of images (batch) of cv::Mat format, they must be uchar type
/// \param height Output Tensor height
/// \param width Output Tensor width
/// \param depth Output Tensor depth
/// \return Ready to inference Tensor with batch of images
///
template<tensorflow::DataType T>
tensorflow::Tensor convertMatToTensor(const std::vector<cv::Mat>& imgs, int height, int width, int depth,
                                       bool normalize, const std::vector<float>& mean) {

    using namespace tensorflow;

    if (mean.size() != depth) {
        std::cerr << "convertMatToTensor: wrong mean or channel size!" << std::endl;
        return Tensor();
    }

    Tensor input(T, TensorShape({(long long)imgs.size(), height, width, depth}));

    using POD_type = typename tensorflow::EnumToDataType<T>::Type;
    auto input_tensor_mapped = input.tensor<POD_type, 4>();

    const auto batch_size = imgs.size();
    for(size_t i = 0; i < batch_size; ++i)
    {
        cv::Mat img;
#ifdef TFDEBUG
    {
    PROFILE_BLOCK("resize img");
#endif
        fastResizeIfPossible(imgs[i], &img, cv::Size(width, height));
#ifdef TFDEBUG
     }
#endif

#ifdef TFDEBUG
    {
    PROFILE_BLOCK("convert img");
#endif
        img.forEach<cv::Vec3b>([&](cv::Vec3b& pixel, const int position[]) -> void {
            for (short c = 0; c < 3; ++c) {
                POD_type val(pixel[c]);
                val -= mean[c];
                if(normalize)
                {
                    val /= 255.0;
                    val -= 0.5;
                    val *= 2.0;
                }
                // "2 - c" performs cv::COLOR_BGR2RGB
                input_tensor_mapped(i, position[0], position[1], 2 - c) = val;
            }
        });
#ifdef TFDEBUG
    }
#endif
    }

    return input;
}

struct profiler {
    std::string name;
    std::chrono::high_resolution_clock::time_point p;
    profiler(std::string const &n) :
        name(n), p(std::chrono::high_resolution_clock::now()) { }
    ~profiler()
    {
        using dura = std::chrono::duration<double>;
        auto d = std::chrono::high_resolution_clock::now() - p;
        std::cout << name << ": "
            << std::chrono::duration_cast<dura>(d).count()
            << std::endl;
    }
};

}

#endif // TENSORFLOWUTILS_H
