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

#ifndef TENSORFLOW_INFERENCER_H
#define TENSORFLOW_INFERENCER_H

///
/// \brief The TensorflowInferencer class
/// Basic abstract class for all types of Tensorflow inferencers. For now there are three types: classifier,
/// detector and segmentator. Also it can be possible regressor or clusterisator, for example, and other types.
///
/// For now hierarchy is like TensorflowInferencer -> TensorflowCNNInferencer
///                                                                          -> Detector
///                                                                          -> Segmentator
///                                                                          -> Classificator
///                                                -> Here can be created classes for Not CNN models

#include <string>
#include <vector>

#include "tensorflow_utils.hpp"

class TensorflowInferencer
{
public:
    TensorflowInferencer() = default;
    virtual ~TensorflowInferencer() = default;

    // No reason to support copy constructor
    TensorflowInferencer(const TensorflowInferencer&) = delete;
    TensorflowInferencer(TensorflowInferencer&& that);

    ///
    /// \brief load
    /// Loading model from file on disk. Format must be frozen .pb.
    /// During loading name of model will be set to extructed from filename string (pure filename without path and extension).
    /// If you need to change it, do it after load with setName.
    /// \return successfull load or not
    ///
    virtual bool load(const std::string& filename);
    ///
    /// \brief isLoaded
    /// Checks if model is loaded. If not, you can not use this model.
    /// \return is model loaded or not
    ///
    virtual inline bool isLoaded() const { return _is_loaded; }

    // Not necessary to use name, just hepler
    virtual inline std::string getName() const { return _name; }
    virtual void setName(const std::string& name);

    ///
    /// \brief warmUp
    /// warmUpializes resources and prepares model for fast inference. Not necessary to call it
    /// but otherwise first model inference call can be slow.
    ///
    virtual void warmUp() = 0;

    std::string getPath() const;

    bool getAggressiveOptimizationGPUEnabled() const;
    void setAggressiveOptimizationGPUEnabled(bool enabled);

    bool getAllowSoftPlacement() const;
    void setAllowSoftPlacement(bool allowSoftPlacement);

    bool getCpuOnly() const;
    void setCpuOnly(bool cpu_only);

    bool getAggressiveOptimizationCPUEnabled() const;
    ///
    /// \brief setAgressiveOptimizationCPUEnabled JIT optimizations for CPU. Only for CPU Only mode.
    ///
    void setAggressiveOptimizationCPUEnabled(bool enabled);


    int getGpuNumber() const;
    // If -1 may use all visible GPUs. Otherwise that GPU number that was set. Override with default device in the model
    void setGpuNumber(int value);

    double getGpuMemoryFraction() const;
    void setGpuMemoryFraction(double gpu_memory_fraction);

    std::string getVisibleDevices() const;
    void setVisibleDevices(const std::string &visible_devices);

    bool getAllowGrowth() const;
    void setAllowGrowth(bool allow_growth);

protected:

    virtual void clearModel() = 0;
    virtual void clearSession() = 0;

    void parseName(const std::string& filename);
    tensorflow::SessionOptions configureSession();
    void configureGraph();

    ///
    /// \brief getTensorFromGraph Method for extracting tensors from graph. For usage, model must be loaded and Session must be active.
    /// \param tensor_name Name in the Graph
    /// \return Empty Tensor if failed, otherwise extructed Tensor
    ///
    tensorflow::Tensor getTensorFromGraph(const std::string& tensor_name);


    bool _is_loaded = false;
    bool _agres_optim_enabled = false;

    /// Mostly that nedeed for XLA, because it's not possible to enable XLA for CPU on session level
    /// But is possible manually. Works only if cpu only mode.
    bool _agres_optim_cpu_enabled = false;

    bool _allow_soft_placement = true;
    bool _cpu_only = false;

    std::string _name = "UnknownModel";
    std::string _path = "";
    std::string _visible_devices = "";

    tensorflow::GraphDef _graph_def;
    tensorflow::Session* _session = nullptr;

    int _gpu_number = -1;
    double _gpu_memory_fraction = 0.;
    bool _allow_growth = false;
};

#endif // TENSORFLOW_INFERENCER_H
