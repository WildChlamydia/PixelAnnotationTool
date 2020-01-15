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


#ifndef TENSORFLOW_CLASSIFIER_H
#define TENSORFLOW_CLASSIFIER_H

#include "tensorflow_cnn_inferencer.hpp"

///
/// \brief The TensorflowClassifier class
/// Specialisation of TensorflowCNNInferencer for classification tasks.
///
///
/// Expects:
///     Model that takes some image or batch of images of fixed size as input and gives probabilities of classes
///     as output.
///     For examples on input was given 1x64x64x3 (one image) and number of classes is 14.
///     Output must be 1x14 therefore, with probabilities for every class.
///
///
/// Usage example:
///     img = cv::imread(someimg);
///     classifier.load(".../frozen_mobilenet_v1.pb");
///     classifier.setOutputNodeName("MobilenetV1/Predictions/Reshape_1");
///     classifier.setPreprocessToFloat(true); // For now that is done in constructor for simplicity
///     classifier.inference({img});
///     std::string class = classifier.getClassesOutputAsStrings()[0];

class TensorflowClassifier : public TensorflowCNNInferencer
{
public:
    TensorflowClassifier() { setPreprocessToFloat(true); }
    virtual ~TensorflowClassifier() = default;

    std::vector<int> getClassesOutputAsIndices(float tresh = 0.5) const;
    std::vector<std::string> getClassesOutputAsStrings(float tresh = 0.5) const;
    std::vector<std::vector<float>> getClassesOutputAsProbs() const;

protected:
    virtual void clearSession();


    /// Some magic constants
    static constexpr short DIM_INDX_N_OF_EXAMPLES = 0;
    static constexpr short DIM_INDX_N_OF_CLASSES = 1;
    static constexpr short OUT_TENSOR_SIZE = 2;
};

#endif // TENSORFLOW_CLASSIFIER_H
