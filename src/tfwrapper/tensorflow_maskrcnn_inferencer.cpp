#include "tensorflow_maskrcnn_inferencer.h"

TensorflowMaskRCNNInferencer::TensorflowMaskRCNNInferencer()
{

}

bool TensorflowMaskRCNNInferencer::load(const std::string &filename)
{
    bool success = TensorflowCNNInferencer::load(filename);

    ////
    _input_node_name = "input_image";
    _output_node_names = {
        "mrcnn_mask/Reshape_1", "mrcnn_detection/Reshape_1"
    };

    // BGR
    _mean = {
        103.9f, 116.8f, 123.7f
    };

    return success;
}

std::string TensorflowMaskRCNNInferencer::inference(const std::vector<cv::Mat>& imgs)
{
    using namespace tensorflow;

    if (!isLoaded()) {
        tf_utils::DebugOutput("Error", "You trying to inference not loaded model! Aborting...");
        return "You trying to inference not loaded model! Aborting...";
    }

    clearSession();
    _probs_masks.clear();
    _detections.clear();

    if (!imgs.size()) {
        tf_utils::DebugOutput("Error", "Empty batch of images passed in inference! Aborting...");
        return "Empty batch of images passed in inference! Aborting...";
    }

    if (_cpu_only) {
        tf_utils::DebugOutput("Warning", "CPU only mode is enabled, be carefull!");
    }

    if (_anchors.empty()) {
        return "No anchors were loaded!";
    }

    if (_metas.empty()) {
        return "No image metas were loaded!";
    }

    _last_inferenced_input.clear();
    _last_inferenced_input.resize(imgs.size(), cv::Mat());
    for (int i = 0; i < imgs.size(); ++i) {
        if (imgs[i].cols <= 0 || imgs[i].rows <= 0) {
            tf_utils::DebugOutput("Error", "Empty image passed in inference!");
            return "Empty image passed in inference!";
        }
        imgs[i].copyTo(_last_inferenced_input[i]);
    }

    Tensor input = getConvertFunction(_input_type)(imgs, _input_height, _input_width, _input_depth, false, _mean);
    Tensor anchor_input = tf_utils::convertAnchorsToTensor(_anchors);
    Tensor metas_input = tf_utils::convertMetasToTensor(_metas);

    std::vector<std::pair<string, tensorflow::Tensor>> inputs = {
        { _input_node_name, input },
        { _anchor_node_name, anchor_input },
        { _meta_node_name, metas_input }
    };

    tensorflow::Status status;
#ifdef TFDEBUG
    {
    PROFILE_BLOCK("inference network");
#endif
    status = _session->Run(inputs, _output_node_names, {}, &_output_tensors);
#ifdef TFDEBUG
    }
#endif


    tf_utils::DebugOutput("Sucessfully run graph! Status is: ", status.ToString());

    _detections = tf_utils::getDetectionsFromTensor(_output_tensors[1], cv::Size(this->getInputWidth(), this->getInputHeight()));
    std::vector<std::vector<int>> classes;

    classes.resize(_detections.size());
    for (size_t i = 0; i < _detections.size(); ++i){

        classes[i].resize(_detections[i].size());

        for (size_t j = 0; j < classes[i].size(); ++j){
            classes[i][j] = std::get<2>(_detections[i][j]);
        }

    }

    _probs_masks = tf_utils::getRCNNMasksFromTensor(_output_tensors[0], classes);

    return status.ToString();
}

void TensorflowMaskRCNNInferencer::drawBBOnFrame(cv::Mat output, const cv::Scalar& color, const std::string& objTitle, const cv::Rect& det_rect) const
{
    auto radius = rounded_rectangle(output, det_rect, color, THICKNESS, cv::LINE_AA, 8);

    cv::Point2d titlePoint = det_rect.tl();
    titlePoint.x = det_rect.tl().x + det_rect.width / 2;
    titlePoint.y -= 4; // Object rect thickness x2
    titlePoint.x = titlePoint.x + radius;

    cv::Rect name_rect;

    cv::Size tsize = cv::getTextSize(objTitle,
                                     FONT, FONT_SCALE, 0.3, 0);

    name_rect.x = radius + det_rect.tl().x + det_rect.width / 2;
    name_rect.y = det_rect.y - tsize.height - 5;
    name_rect.width = tsize.width + 5;
    name_rect.x = name_rect.x - name_rect.width / 2;
    titlePoint.x = titlePoint.x - name_rect.width / 2;
    name_rect.height = tsize.height + 10;
    name_rect.y = name_rect.y - name_rect.height * 0.1;
    titlePoint.y = titlePoint.y - name_rect.height * 0.1;

    name_rect.y -= THICKNESS * 2;
    cv::rectangle(output,
                  name_rect,
                  color,
                  cv::FILLED);

    cv::putText(output,
                objTitle,
                titlePoint,
                FONT,
                FONT_SCALE,
                cv::Scalar(255, 255, 255),
                0.3,
                cv::LINE_AA);
}

std::vector<cv::Mat> TensorflowMaskRCNNInferencer::getOutputMasks(cv::Size target_size) const
{

    if (target_size.area() < 10) {
        target_size = cv::Size(this->getInputWidth(), this->getInputHeight());
    }

    std::vector<cv::Mat> answers;
    for (int example = 0; example < _detections.size(); ++example) {

        cv::Mat output(cv::Size_<int64>(target_size.width, target_size.height), CV_8UC3);
        output.setTo(cv::Scalar(0, 0, 0));

        for (int i = 0; i < _detections[example].size(); ++i) {

            float score = std::get<1>(_detections[example][i]);

            int cl = std::get<2>(_detections[example][i]);

            if (score < _thresh) continue;

            auto det_rect = std::get<0>(_detections[example][i]);

            det_rect.x = det_rect.x * target_size.width;
            det_rect.y = det_rect.y * target_size.height;
            det_rect.width = det_rect.width * target_size.width;
            det_rect.height = det_rect.height * target_size.height;

            auto sz = cv::Size(output(det_rect).cols, output(det_rect).rows);
            cv::Mat temp_mask;
            _probs_masks[example][i].copyTo(temp_mask);

            if (_nice_draw) {
                cv::resize(temp_mask, temp_mask, sz, cv::INTER_LINEAR);
                cv::blur(temp_mask, temp_mask, cv::Size(25, 25));
            } else {
                cv::resize(temp_mask, temp_mask, sz, cv::INTER_NEAREST);
            }

            cv::Mat binary_mask;
            cv::threshold( temp_mask, binary_mask, _thresh, 255, cv::THRESH_BINARY );
            binary_mask.convertTo(binary_mask, CV_8U);

            cv::Mat temp(sz, CV_8UC3);
            cv::resize(temp, temp, sz);
            temp.setTo(_label_colours[cl]);
            temp.copyTo(output(det_rect), binary_mask);

        }

        answers.push_back(output);
    }

    return answers;

}

void TensorflowMaskRCNNInferencer::drawDetectionsOnFrames(std::vector<cv::Mat> outputs) const
{
    for (int example = 0; example < _detections.size(); ++example) {
        for (int i = 0; i < _detections[example].size(); ++i) {

            float score = std::get<1>(_detections[example][i]);
            int cl = std::get<2>(_detections[example][i]);

            if (score < _thresh) continue;

            auto det_rect = std::get<0>(_detections[example][i]);
            det_rect.x = det_rect.x * outputs[example].cols;
            det_rect.y = det_rect.y * outputs[example].rows;
            det_rect.width = det_rect.width * outputs[example].cols;
            det_rect.height = det_rect.height * outputs[example].rows;

            cv::Scalar color = cv::Scalar(_label_colours[cl][0], _label_colours[cl][1], _label_colours[cl][2]);
            std::string objTitle = _label_names[cl];

            drawBBOnFrame(outputs[example], color, objTitle, det_rect);
        }
    }
}

bool TensorflowMaskRCNNInferencer::loadAnchors(const std::string& npy_filename)
{

    if (!QFile::exists(QString::fromStdString(npy_filename))) {
        return false;
    }

    cnpy::NpyArray anchor_array = cnpy::npy_load(npy_filename);
    float* loaded_data = anchor_array.data<float>();

    if (anchor_array.shape.size() != 3) {
        return false;
    }

    size_t Nx = (size_t)anchor_array.shape[0];
    size_t Ny = (size_t)anchor_array.shape[1];
    size_t Nz = (size_t)anchor_array.shape[2];

    _anchors.clear();

    _anchors.resize(Nx);

    for (size_t i = 0; i < Nx; ++i) {

        _anchors[i].resize(Ny);
        for (size_t k = 0; k < Ny; ++k) {

            _anchors[i][k].resize(Nz);
            for (size_t z = 0; z < Nz; ++z) {
                _anchors[i][k][z] = loaded_data[(i * Nz * Ny) + (k * Nz) + z];
            }

        }

    }

    return true;

}

bool TensorflowMaskRCNNInferencer::loadMetas(const std::string &npy_filename)
{

    if (!QFile::exists(QString::fromStdString(npy_filename))) {
        return false;
    }

    cnpy::NpyArray anchor_array = cnpy::npy_load(npy_filename);
    int64* loaded_data = anchor_array.data<int64>();

    if (anchor_array.shape.size() != 2) {
        return false;
    }

    size_t Nx = (size_t)anchor_array.shape[0];
    size_t Ny = (size_t)anchor_array.shape[1];

    _metas.clear();

    _metas.resize(Nx);

    for (size_t i = 0; i < Nx; ++i) {

        _metas[i].resize(Ny);
        for (size_t k = 0; k < Ny; ++k) {

            _metas[i][k] = (float)loaded_data[(i * Ny) + k];

        }

    }

    return true;

}

std::string TensorflowMaskRCNNInferencer::getAnchorsNpyFilename() const
{
    return _anchors_npy_filename;
}

void TensorflowMaskRCNNInferencer::setAnchorsNpyFilename(const std::string &anchors_npy_filename)
{
    _anchors_npy_filename = anchors_npy_filename;
}

std::string TensorflowMaskRCNNInferencer::getMetasNpyFilename() const
{
    return _metas_npy_filename;
}

void TensorflowMaskRCNNInferencer::setMetasNpyFilename(const std::string &metas_npy_filename)
{
    _metas_npy_filename = metas_npy_filename;
}

float TensorflowMaskRCNNInferencer::getThresh() const
{
    return _thresh;
}

void TensorflowMaskRCNNInferencer::setThresh(float thresh)
{
    _thresh = thresh;
}

bool TensorflowMaskRCNNInferencer::getNiceDraw() const
{
    return _nice_draw;
}

void TensorflowMaskRCNNInferencer::setNiceDraw(bool nice_draw)
{
    _nice_draw = nice_draw;
}

bool TensorflowMaskRCNNInferencer::getDrawBoxesOnImage() const
{
    return _draw_boxes_on_image;
}

void TensorflowMaskRCNNInferencer::setDrawBoxesOnImage(bool draw_boxes_on_image)
{
    _draw_boxes_on_image = draw_boxes_on_image;
}

int TensorflowMaskRCNNInferencer::rounded_rectangle(cv::Mat &src, const cv::Rect rect, const cv::Scalar lineColor, const int thickness, const int lineType, const int cornerRadius) const
{
    using namespace cv;
    /* corners:
     * p1 - p2
     * |     |
     * p4 - p3
     */

    const cv::Point topLeft = rect.tl();
    const cv::Point bottomRight = rect.br();
    //const int cornerRadius = (rect.width + rect.height) * cr;
    Point p1 = topLeft;
    Point p2 = Point (bottomRight.x, topLeft.y);
    Point p3 = bottomRight;
    Point p4 = Point (topLeft.x, bottomRight.y);

    // draw straight lines
    line(src, Point (p1.x+cornerRadius,p1.y), Point (p2.x-cornerRadius,p2.y), lineColor, thickness, lineType);
    line(src, Point (p2.x,p2.y+cornerRadius), Point (p3.x,p3.y-cornerRadius), lineColor, thickness, lineType);
    line(src, Point (p4.x+cornerRadius,p4.y), Point (p3.x-cornerRadius,p3.y), lineColor, thickness, lineType);
    line(src, Point (p1.x,p1.y+cornerRadius), Point (p4.x,p4.y-cornerRadius), lineColor, thickness, lineType);

    // draw arcs
    ellipse( src, p1+Point(cornerRadius, cornerRadius),
             Size( cornerRadius, cornerRadius ), 180.0, 0, 90, lineColor, thickness, lineType );
    ellipse( src, p2+Point(-cornerRadius, cornerRadius),
             Size( cornerRadius, cornerRadius ), 270.0, 0, 90, lineColor, thickness, lineType );
    ellipse( src, p3+Point(-cornerRadius, -cornerRadius),
             Size( cornerRadius, cornerRadius ), 0.0, 0, 90, lineColor, thickness, lineType );
    ellipse( src, p4+Point(cornerRadius, -cornerRadius),
             Size( cornerRadius, cornerRadius ), 90.0, 0, 90, lineColor, thickness, lineType );

    return cornerRadius;
}

std::vector<std::vector<std::tuple<cv::Rect2f, float, int> > > TensorflowMaskRCNNInferencer::getOutputBoundingBoxes() const
{
    return _detections;
}

std::vector<cv::Mat> TensorflowMaskRCNNInferencer::getOutputColored(cv::Size target_size, bool draw_only_boxes ) const
{
    if (draw_only_boxes) {

        std::vector<cv::Mat> answers;
        answers.resize(_detections.size(), cv::Mat());
        for (int example = 0; example < _detections.size(); ++example) {

            _last_inferenced_input[example].copyTo(answers[example]);

            for (int i = 0; i < _detections[example].size(); ++i) {

                float score = std::get<1>(_detections[example][i]);

                int cl = std::get<2>(_detections[example][i]);

                if (score < _thresh) continue;

                auto det_rect = std::get<0>(_detections[example][i]);

                det_rect.x = det_rect.x * target_size.width;
                det_rect.y = det_rect.y * target_size.height;
                det_rect.width = det_rect.width * target_size.width;
                det_rect.height = det_rect.height * target_size.height;

                cv::Scalar color = cv::Scalar(_label_colours[cl][0], _label_colours[cl][1], _label_colours[cl][2]);
                std::string objTitle = _label_names[cl];

                drawBBOnFrame(answers[example], color, objTitle, det_rect);

            }

        }

        return answers;

    } else {

        return getOutputMasks(target_size);

    }
}

