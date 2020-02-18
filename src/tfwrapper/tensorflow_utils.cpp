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

#include "tensorflow_utils.hpp"
#include <QDebug>
std::vector<cv::Mat> tf_utils::convertSegmentTensorToMat(const tensorflow::Tensor& tensor,
                                                         const std::vector<std::vector<int>> &colours)
{
#ifdef TFDEBUG
    {
    PROFILE_BLOCK("convert tensor");
#endif
    const auto& temp_tensor = tensor.tensor<tensorflow::int64, 4>();
    const auto& dims = tensor.shape();

    // dims size - batch, height, width, channels

    std::vector<cv::Mat> imgs(size_t(dims.dim_size(0)));

    for(size_t example = 0; example < imgs.size(); ++example)
    {
        imgs[example] = cv::Mat(cv::Size_<int64>(dims.dim_size(2), dims.dim_size(1)), colours.size() ? CV_8UC3 : CV_8UC1);

        if(!colours.size())
            imgs[example].forEach<uchar>([&](uchar& pixel, const int position[]) -> void {
                pixel = uchar(temp_tensor(long(example), position[0], position[1], 0));
            });
        else
            imgs[example].forEach<cv::Vec3b>([&](cv::Vec3b& pixel, const int position[]) -> void {
                auto clrs(colours[size_t(temp_tensor(long(example), position[0], position[1], 0))]);
                pixel = cv::Vec3b(cv::Vec3i{clrs[0], clrs[1], clrs[2]});
            });
    }

    return imgs;
#ifdef TFDEBUG
    }
#endif
}

std::vector<int> tf_utils::getShapesFromMessage(tensorflow::AttrValue &val) {
    const std::string key = "size:";

    std::vector<int> shapes;
    std::string str = val.DebugString();

    auto pos = str.find(key);
    while ( pos != std::string::npos ) {
        pos += std::string(key).size() + 1;

        auto pos_end = str.find("}", pos);
        if (pos_end == std::string::npos) {
            return shapes;
        }
        pos_end -= 1;

        shapes.push_back(atoi(str.substr(pos, pos_end).c_str()));

        pos = str.find(key, pos);
    }
    throw std::logic_error("getShapesFromMessage didn't return");
}

void tf_utils::fastResizeIfPossible(const cv::Mat &in, cv::Mat *dist, const cv::Size& size)
{
    if (in.size() == size) {
        DebugOutput("sizes matches", std::to_string(in.cols) + "x" + std::to_string(in.rows) + "; "
                   + std::to_string(size.width) + "x" + std::to_string(size.height));
        in.copyTo(*dist);
        return;
    }

//    if (size.height < MAX_SIZE_FOR_PYR_METHOD && size.height < MAX_SIZE_FOR_PYR_METHOD) {
//        if (in.rows > size.height && in.cols > size.width) {

//            if ((std::abs(size.width * 2 - in.cols)) <= 2 && (std::abs(size.height * 2 - in.rows)) <= 2) {
//                DebugOutput("using pyrdown", "");
//                cv::pyrDown(in, *dist, size);
//                return;
//            }
//        }
//        if (in.rows < size.height && in.cols < size.width) {

//            if (std::abs(size.width - in.cols * 2) == size.width % 2
//                    && std::abs(size.height - in.rows * 2) == size.height % 2) {
//                DebugOutput("using pyrup", "");
//                cv::pyrUp(in, *dist, size);
//                return;
//            }
//        }
//    }

    cv::resize(in, *dist, size, 0, 0, cv::INTER_LINEAR);
    return;
}
