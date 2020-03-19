#include "tensorflow_utils.hpp"

std::vector<cv::Mat> tf_utils::convertSegmentTensorToMat(const tensorflow::Tensor& tensor,
                                                         const std::vector<std::vector<int>> &colours)
{
#ifdef TFDEBUG
    {
    PROFILE_BLOCK("convert tensor");
#endif
    const auto& temp_tensor = tensor.tensor<tensorflow::int64, 4>();
    const auto& dims = tensor.shape();
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


#include <QDebug>
std::vector<std::vector<cv::Mat>> tf_utils::getRCNNMasksFromTensor(const tensorflow::Tensor& tensor,
                                                                   std::vector<std::vector<int>> classes)
{

    const auto& temp_tensor = tensor.tensor<float, 5>();
    const auto& dims = tensor.shape();
    std::vector<std::vector<cv::Mat>> imgs(size_t(dims.dim_size(0)));

    for(size_t example = 0; example < imgs.size(); ++example) {
        imgs[example].resize(dims.dim_size(1));
        for(size_t mask_num = 0; mask_num < dims.dim_size(1); ++mask_num) {

            int cl = classes[example][mask_num];

            imgs[example][mask_num] = cv::Mat(cv::Size_<int64>(dims.dim_size(3), dims.dim_size(2)), CV_32FC1);
            imgs[example][mask_num].forEach<float>([&](float& pixel, const int position[]) -> void {

                pixel = temp_tensor(long(example), long(mask_num), position[0], position[1], cl);

            });


        }
    }

    return imgs;

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

    return shapes;
}

void tf_utils::fastResizeIfPossible(const cv::Mat &in, cv::Mat *dist, const cv::Size& size)
{
    if (in.size() == size) {
        DebugOutput("sizes matches", std::to_string(in.cols) + "x" + std::to_string(in.rows) + "; "
                   + std::to_string(size.width) + "x" + std::to_string(size.height));
        in.copyTo(*dist);
        return;
    }

    cv::resize(in, *dist, size, 0, 0, cv::INTER_LINEAR);
    return;
}

std::vector<std::vector< std::tuple<cv::Rect2f, float, int> > > tf_utils::getDetectionsFromTensor(const tensorflow::Tensor &tensor,
                                                                                                  const cv::Size& image_size)
{
    const auto& temp_tensor = tensor.tensor<float, 3>();
    const auto& dims = tensor.shape();


    std::vector<std::vector<std::tuple<cv::Rect2f, float, int>>> detections(size_t(dims.dim_size(0)));

    detections.resize(dims.dim_size(0));
    for(size_t example = 0; example < detections.size(); ++example) {
        detections[example].resize(dims.dim_size(1), std::tuple<cv::Rect2f, float, int>(cv::Rect(), 0, 0));
        for(size_t proporsal = 0; proporsal < dims.dim_size(1); ++proporsal) {

            cv::Rect2f det;
            det.y = temp_tensor(long(example), long(proporsal), 0);
            det.x = temp_tensor(long(example), long(proporsal), 1);
            det.height = (temp_tensor(long(example), long(proporsal), 2) - temp_tensor(long(example), long(proporsal), 0));
            det.width = (temp_tensor(long(example), long(proporsal), 3) - temp_tensor(long(example), long(proporsal), 1));

            int cl = round(temp_tensor(long(example), long(proporsal), 4));
            float score = temp_tensor(long(example), long(proporsal), 5);

            detections[example][proporsal] = {det, score, cl};
        }
    }

    return detections;
}
