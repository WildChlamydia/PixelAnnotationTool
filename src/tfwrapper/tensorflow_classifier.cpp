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

#include "tensorflow_classifier.hpp"

std::vector<int> TensorflowClassifier::getClassesOutputAsIndices(float tresh) const
{
    using namespace tensorflow;

    if (!_is_loaded) {
        return {};
    }

    const auto& probs = getClassesOutputAsProbs();

    if (probs.empty()) {
        return {};
    }

    std::vector<int> outputs(probs.size());

    for (short i = 0, total = probs.size(); i < total; ++i) {

        auto it = std::max_element(probs[i].cbegin(), probs[i].cend());
        const float probability = (*it);
        int max_index = -1;
        if (probability > tresh) {
            max_index  = static_cast<int>(it - probs[i].cbegin());
        }

        tf_utils::DebugOutput("Best class index: ", std::to_string(max_index));

        outputs[i] = (max_index);

    }

    return outputs;
}

std::vector<std::string> TensorflowClassifier::getClassesOutputAsStrings(float tresh) const
{
    if (!_is_loaded) {
        return {};
    }

    if (_label_names.empty()) {
        tf_utils::DebugOutput("Labels empty!", "");
        return {};
    }

    const std::vector<int>& results = getClassesOutputAsIndices(tresh);

    std::vector<std::string> str_results;

    for (const auto& res: results) {
        if (res >= 0) {
            str_results.push_back(_label_names[res]);
            tf_utils::DebugOutput("Best class: ", _label_names[res]);
        } else {
            str_results.push_back("");
        }
    }

    return str_results;
}

std::vector<std::vector<float>> TensorflowClassifier::getClassesOutputAsProbs() const
{
    using namespace tensorflow;

    // Only one output supported
    if (!_is_loaded || _output_tensors.size() != 1) {
        return {};
    }

    const Tensor& tensor = _output_tensors[0];
    const auto& dims = tensor.shape();

    std::vector<std::vector<float>> probs(dims.dim_size(DIM_INDX_N_OF_EXAMPLES));
    const auto& mapped_tensor = tensor.tensor<float, OUT_TENSOR_SIZE>();

    for (short example = 0, total = dims.dim_size(DIM_INDX_N_OF_EXAMPLES); example < total; ++example) {

        probs[example].resize(dims.dim_size(DIM_INDX_N_OF_CLASSES));
        for (short cl = 0; cl < dims.dim_size(DIM_INDX_N_OF_CLASSES); ++cl) {
            probs[example][cl] = mapped_tensor(example, cl);
        }

    }

    return probs;
}

void TensorflowClassifier::clearSession()
{
    TensorflowCNNInferencer::clearSession();
}
