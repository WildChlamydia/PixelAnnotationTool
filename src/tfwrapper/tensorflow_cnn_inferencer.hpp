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


#ifndef TENSORFLOW_SEGMENTATOR_H
#define TENSORFLOW_SEGMENTATOR_H
#include "tensorflow_inferencer.hpp"

///
/// \brief The TensorflowCNNInferencer class
/// Works with CNN model from .pb file.
/// For correct work specify input and output node names. Also if input size is not fixed
/// specify height, width and depth on input explicitly with setter methods. Otherwise image will be passed as is.
///
/// Also check load method documentation.
///
///
/// Expects:
///     See specialized classes of that base class: TensorflowSegmentator, TensorflowClassificator, TensorflowDetector, etc.
///
///
/// List of names of tf.constant in Graph that hepls with auto-deduction (if they are exists, therefore you do not need to
/// specify them manually):
///
/// * label_colours - flat list of tuples / lists of colors in RGB (!!!) format.
/// * label_names - flat list of strings
/// * input_size - list or tuple of size 2 or 3. Height x Width x Depth (optionally, default 3)
/// * output_name - list of size 1 with name of output node. For now only single output node supported, but it's
///   possible to add support for more.
///

class TensorflowCNNInferencer : public TensorflowInferencer
{
public:

    enum INPUT_TYPE {
        DT_FLOAT,
        DT_UINT8
    };

    TensorflowCNNInferencer();
    TensorflowCNNInferencer(TensorflowCNNInferencer &&that);
    virtual ~TensorflowCNNInferencer();

    ///
    /// \brief inference Inferences network with data passed in batch of images.
    /// \param imgs vector of images
    ///
    virtual std::string inference(const std::vector<cv::Mat> &imgs);

    ///
    /// \brief load Loads model from .pb file. Please, not that h, w, c of input, labels names and colors will be
    /// extracted from info in loaded Graph if that possible. So every load that info will change. If you want to
    /// control it or load can not deduce that info, set it manually with getter\setter methods accordingly.
    ///
    /// If you don't need some of that info, for example your CNN model is regressor, therefore just forget and use
    /// that feature.
    ///
    /// \param filename
    /// \return success or not. If not, you can not use model.
    ///
    virtual bool load(const std::string &filename);

    std::string getInputNodeName() const;
    void setInputNodeName(const std::string &getInputNodeName);

    std::vector<std::string> getOutputNodeName() const;
    void setOutputNodeName(const std::vector<std::string>& output_node_names);

    ///
    /// \brief warmUp Just inferences network with zeros input. That helps to move model weights to GPU or in RAM before
    /// workflow starts.
    ///
    void warmUp();

    short getInputHeight() const;
    void setInputHeight(const short &height);

    short getInputWidth() const;
    void setInputWidth(const short &width);

    short getInputDepth() const;
    void setInputDepth(const short &depth);

    std::vector<std::string> getLabelNames() const;
    void setLabelNames(const std::vector<std::string> &label_names);

    std::vector<std::vector<int> > getLabelColours() const;
    void setLabelColours(const std::vector<std::vector<int> > &label_colours);

    bool getPreprocessToFloat() const;
    ///
    /// \brief setPreprocessToFloat If true, therefore pixels will be devided by 255.0 during preprocess
    /// \param value
    ///
    void setPreprocessToFloat(bool value);

    INPUT_TYPE getInputDtype() const;
    void setInputDtype(const INPUT_TYPE &inputDtype);

protected:

    using ConvertFunctionType = decltype(&(tf_utils::convertMatToTensor<tensorflow::DT_FLOAT>));

    ConvertFunctionType getConvertFunction(INPUT_TYPE type) {
        if (type == INPUT_TYPE::DT_FLOAT) {
            return tf_utils::convertMatToTensor<tensorflow::DT_FLOAT>;
        } else if (type == INPUT_TYPE::DT_UINT8) {
            return tf_utils::convertMatToTensor<tensorflow::DT_UINT8>;
        } else throw std::invalid_argument("not implemented");
    }

    ///
    /// \brief deduceInputShapeIfPossible Input node size can be static or dynamic. If static
    /// then we will try to deduce it from Protobuf info. If it is not possible, therefore
    /// class will try to inference with size of image that will be passed to inference.
    /// \return Success or not
    ///
    bool deduceInputShapeIfPossible();

    bool getShapeFromGraphIfPossible();

    void getLabelsFromGraphIfPossible();

    void getColoursFromGraphIfPossible();

    void getOutputNodeNameFromGraphIfPossible();

    void getInputNodeNameFromGraphIfPossible();

    virtual void clearModel();
    virtual void clearSession();



    // Data
    INPUT_TYPE _input_type = INPUT_TYPE::DT_FLOAT;

    short _input_height = 0;
    short _input_width = 0;
    short _input_depth = 0;

    std::vector<std::string> _label_names;
    std::vector<std::vector<int>> _label_colours;

    // With 0, 0, 0 nothing changes. But if you will set some values mean will be extracted. Be carefull
    // make sure that mean extraction operation is not inside .pb graph already.
    std::vector<float> _mean = {0, 0, 0};
    bool _convert_to_float = false; // If true, image pixels will be divided by 255.0

    std::string _input_node_name = "input";
    std::vector<std::string> _output_node_names = {"output"};

    std::vector<tensorflow::Tensor> _output_tensors;
};

#endif // TENSORFLOW_SEGMENTATOR_H
