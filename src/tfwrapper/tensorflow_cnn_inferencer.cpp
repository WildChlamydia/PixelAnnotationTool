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

#include "tensorflow_cnn_inferencer.hpp"

TensorflowCNNInferencer::TensorflowCNNInferencer()
{

}

TensorflowCNNInferencer::TensorflowCNNInferencer(TensorflowCNNInferencer && that)
{

    this->_input_type = that._input_type;
    this->_input_height = that._input_height;
    this->_input_width = that._input_width;
    this->_convert_to_float = that._convert_to_float;

    this->_label_names = std::move(that._label_names);
    this->_label_colours = std::move(that._label_colours);
    this->_mean = std::move(that._mean);
    this->_input_node_name = std::move(_input_node_name);
    this->_output_node_names = std::move(_output_node_names);
    this->_output_tensors = std::move(_output_tensors);
}

TensorflowCNNInferencer::~TensorflowCNNInferencer()
{
    if (_session) {
        _session->Close();
        delete _session;
    }
}

std::string TensorflowCNNInferencer::inference(const std::vector<cv::Mat>& imgs)
{
    using namespace tensorflow;

    if (!isLoaded()) {
        tf_utils::DebugOutput("Error", "You trying to inference not loaded model! Aborting...");
        return "You trying to inference not loaded model! Aborting...";
    }

    clearSession();

    if (!imgs.size()) {
        tf_utils::DebugOutput("Error", "Empty batch of images passed in inference! Aborting...");
        return "Empty batch of images passed in inference! Aborting...";
    }

    if (_cpu_only) {
        tf_utils::DebugOutput("Warning", "CPU only mode is enabled, be carefull!");
    }

    for (const auto& img : imgs) {
        if (img.cols <= 0 || img.rows <= 0) {
            tf_utils::DebugOutput("Error", "Empty image passed in inference!");
            return "Empty image passed in inference!";
        }
    }

    Tensor input = getConvertFunction(_input_type)(imgs, _input_height, _input_width, _input_depth, _convert_to_float, _mean);

    std::vector<std::pair<string, tensorflow::Tensor>> inputs = { { _input_node_name, input } };

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

    return status.ToString();
}

void TensorflowCNNInferencer::clearModel()
{
    clearSession();

    _label_names.clear();
    _label_colours.clear();
    _output_node_names.clear();
}

void TensorflowCNNInferencer::clearSession()
{
    _output_tensors.clear();
}

bool TensorflowCNNInferencer::load(const std::string &filename)
{
    clearModel();

    using namespace tensorflow;

    TensorflowInferencer::load(filename);

    if (!deduceInputShapeIfPossible()) {
        tf_utils::DebugOutput("Can not deduce input shape, will work as is!", "");
    }

    tf_utils::DebugOutput("Current shape: ", std::to_string(_input_width)
                          + "x" + std::to_string(_input_height) + "x" + std::to_string(_input_depth));

    getLabelsFromGraphIfPossible();
    getColoursFromGraphIfPossible();
    getOutputNodeNameFromGraphIfPossible();
    getInputNodeNameFromGraphIfPossible();

    return _is_loaded;
}

bool TensorflowCNNInferencer::deduceInputShapeIfPossible()
{

    if (getShapeFromGraphIfPossible())
        return true;

    // Seems that there is no tensor with shape in graph.
    // Then try to deduce it from Protocol Buffer from Graph Def information
    for (int i = 0, total = _graph_def.node_size(); i < total; ++i) {
        if (_graph_def.node(i).name() == _input_node_name) {
            auto att = _graph_def.node(i).attr();
            auto shape_message = att["shape"];

            std::vector<int> shape = tf_utils::getShapesFromMessage(shape_message);

            // Batch case: 1x1024x1023x3, for example
            if (shape.size() == 4) {
                _input_height = shape[1];
                _input_width = shape[2];
                _input_depth = shape[3];


                // If -1 somewhere, therefore graph expects dynamic size
                if (_input_height > 0 && _input_width > 0 && _input_depth > 0)
                    return true;
                else
                    return false;
            }

            // Non-batch case: 1024x1023x3, for example
            if (shape.size() == 3) {
                _input_height = shape[0];
                _input_width = shape[1];
                _input_depth = shape[2];

                if (_input_height > 0 && _input_width > 0 && _input_depth > 0)
                    return true;
                else
                    return false;
            }
        }
    }
    return false;
}

bool TensorflowCNNInferencer::getShapeFromGraphIfPossible()
{
    using namespace tensorflow;

    const Tensor& shape_tensor = getTensorFromGraph("input_size");
    if (shape_tensor.NumElements() >= 2) {
        const auto& shape_mapped = shape_tensor.tensor<int, 1>();
        _input_height = shape_mapped(0);
        _input_width = shape_mapped(1);
        if (shape_tensor.NumElements() == 3)
            _input_depth = shape_mapped(2);

#ifdef TFDEBUG
        std::cerr << "Shape:\n------------------" << std::endl;
        for (short i = 0; i < shape_tensor.dim_size(0); ++i) {
            std::cerr << shape_mapped(i) << std::endl;
        }
        std::cerr << "------------------\nShape loaded" << std::endl;
#endif
        return true;
    }

    return false;
}

void TensorflowCNNInferencer::getLabelsFromGraphIfPossible()
{
    using namespace tensorflow;

    const Tensor& names_tensor = getTensorFromGraph("label_names");
    if (names_tensor.NumElements()) {
        const auto& names_mapped = names_tensor.tensor<std::string, 1>();
#ifdef TFDEBUG
        std::cerr << "Labels:\n------------------" << std::endl;
#endif
        for (short i = 0; i < names_tensor.NumElements(); ++i) {
            _label_names.push_back(names_mapped(i));
#ifdef TFDEBUG
            std::cerr << names_mapped(i) << std::endl;
#endif
        }
#ifdef TFDEBUG
        std::cerr << "------------------\nLabels loaded" << std::endl;
#endif
    }
}

void TensorflowCNNInferencer::getColoursFromGraphIfPossible()
{
    using namespace tensorflow;

    const Tensor& colors_tensor = getTensorFromGraph("label_colours");
    if (colors_tensor.NumElements()) {

        if (colors_tensor.dim_size(1) == 3) {

            const auto& color_mapped = colors_tensor.tensor<int, 2>();

#ifdef TFDEBUG
            std::cerr << "Colors:\n------------------" << std::endl;
#endif
            for (short i = 0; i < colors_tensor.dim_size(0); ++i) {

                /// Must be BGR, stored in Graph is RGB
                _label_colours.push_back({color_mapped(i, 2), color_mapped(i, 1), color_mapped(i, 0)});

#ifdef TFDEBUG
                std::cerr << color_mapped(i, 0) << " " << color_mapped(i, 1) << " " << color_mapped(i, 2) << std::endl;
#endif
            }
#ifdef TFDEBUG
            std::cerr << "------------------\nColors loaded" << std::endl;
#endif

        }
    }
}

void TensorflowCNNInferencer::getOutputNodeNameFromGraphIfPossible()
{
    _output_node_names.clear();

    using namespace tensorflow;

    const Tensor& names_tensor = getTensorFromGraph("output_name");

    if (names_tensor.dims() > 1) {
        tf_utils::DebugOutput("Wrong shape of tensor for output names", "");
        return;
    }

#ifdef TFDEBUG
        std::cerr << "Output node names:\n------------------" << std::endl;
#endif
    for (short i = 0; i < names_tensor.dim_size(0); ++i) {
        const auto& names_mapped = names_tensor.tensor<std::string, 1>();
        _output_node_names.push_back(names_mapped(i));
#ifdef TFDEBUG
        std::cerr << names_mapped(i) << std::endl;
#endif
    }
#ifdef TFDEBUG
        std::cerr << "------------------\nOutput node names loaded" << std::endl;
#endif
}

void TensorflowCNNInferencer::getInputNodeNameFromGraphIfPossible()
{
    using namespace tensorflow;

    const Tensor& names_tensor = getTensorFromGraph("input_name");
    if (names_tensor.NumElements() == 1) {
        const auto& names_mapped = names_tensor.tensor<std::string, 1>();
#ifdef TFDEBUG
        std::cerr << "Input node name:\n------------------" << std::endl;
#endif
        _input_node_name = names_mapped(0);
#ifdef TFDEBUG
        std::cerr << names_mapped(0) << std::endl;
#endif

#ifdef TFDEBUG
        std::cerr << "------------------\nInput node name loaded" << std::endl;
#endif
    }
}

TensorflowCNNInferencer::INPUT_TYPE TensorflowCNNInferencer::getInputDtype() const
{
    return _input_type;
}

void TensorflowCNNInferencer::setInputDtype(const INPUT_TYPE &inputDtype)
{
    _input_type = inputDtype;
}

bool TensorflowCNNInferencer::getPreprocessToFloat() const
{
    return _convert_to_float;
}

void TensorflowCNNInferencer::setPreprocessToFloat(bool value)
{
    _convert_to_float = value;
}

std::vector<std::vector<int> > TensorflowCNNInferencer::getLabelColours() const
{
    return _label_colours;
}

void TensorflowCNNInferencer::setLabelColours(const std::vector<std::vector<int> > &label_colours)
{
    _label_colours = label_colours;
}

std::vector<std::string> TensorflowCNNInferencer::getLabelNames() const
{
    return _label_names;
}

void TensorflowCNNInferencer::setLabelNames(const std::vector<std::string> &label_names)
{
    _label_names = label_names;
}

short TensorflowCNNInferencer::getInputDepth() const
{
    return _input_depth;
}

void TensorflowCNNInferencer::setInputDepth(const short &depth)
{
    _input_depth = depth;
}

short TensorflowCNNInferencer::getInputWidth() const
{
    return _input_width;
}

void TensorflowCNNInferencer::setInputWidth(const short &width)
{
    _input_width = width;
}

short TensorflowCNNInferencer::getInputHeight() const
{
    return _input_height;
}

void TensorflowCNNInferencer::setInputHeight(const short &height)
{
    _input_height = height;
}

std::string TensorflowCNNInferencer::getInputNodeName() const
{
    return _input_node_name;
}

void TensorflowCNNInferencer::setInputNodeName(const std::string &input_node_name)
{
    _input_node_name = input_node_name;
}

std::vector<std::string> TensorflowCNNInferencer::getOutputNodeName() const
{
    return _output_node_names;
}

void TensorflowCNNInferencer::setOutputNodeName(const std::vector<std::string> &output_node_names)
{
    _output_node_names = output_node_names;
}

void TensorflowCNNInferencer::warmUp()
{
    cv::Mat img;
    if (_input_depth == 1) {
        img = cv::Mat(_input_height, _input_width, CV_8UC1, cv::Scalar(0.));
    }
    if (_input_depth == 3) {
        img = cv::Mat(_input_height, _input_width, CV_8UC3, cv::Scalar(0.));
    }
    inference({img});
}
