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

#include "tensorflow_inferencer.hpp"

TensorflowInferencer::TensorflowInferencer(TensorflowInferencer &&that)
{
    this->_session = that._session;
    this->_is_loaded = that._is_loaded;

    this->_name = std::move(that._name);
    this->_path = std::move(that._path);
    this->_graph_def = std::move(that._graph_def);
}

tensorflow::SessionOptions TensorflowInferencer::configureSession()
{
    using namespace tensorflow;

    SessionOptions opts;
    opts.config.set_allow_soft_placement(_allow_soft_placement);

    GPUOptions *gpu_options = new GPUOptions;
#ifdef TFDEBUG
    //opts.config.set_log_device_placement(true);
#endif
    if (_cpu_only) {
        auto device_map = opts.config.mutable_device_count();
        if (device_map) {
            tf_utils::DebugOutput("Warning", "Disabling GPU!!!");
            (*device_map)["GPU"] = 0;
        }
    } else {
        if (!_visible_devices.empty()) {
            gpu_options->set_visible_device_list(_visible_devices);
        }
        gpu_options->set_per_process_gpu_memory_fraction(_gpu_memory_fraction);
        gpu_options->set_allow_growth(_allow_growth);
    }

    GraphOptions *graph_opts = new GraphOptions;
    /// TODO: Needs tests, maybe not all options is ok
    OptimizerOptions *optim_opts = new OptimizerOptions;
    // OptimizerOptions_GlobalJitLevel_ON_2 turn on compilation, with higher values being
    // more aggressive.  Higher values may reduce opportunities for parallelism
    // and may use more memory.  (At present, there is no distinction, but this
    // is expected to change.)
    optim_opts->set_global_jit_level( (_agres_optim_enabled ? OptimizerOptions_GlobalJitLevel_ON_2
                                                    : OptimizerOptions_GlobalJitLevel_OFF) );
    optim_opts->set_do_common_subexpression_elimination(_agres_optim_enabled ? true : false);
    optim_opts->set_do_constant_folding(_agres_optim_enabled ? true : false);
    optim_opts->set_do_function_inlining(_agres_optim_enabled ? true : false);

    graph_opts->set_allocated_optimizer_options(optim_opts);

    opts.config.set_allocated_graph_options(graph_opts);
    opts.config.set_allocated_gpu_options(gpu_options);

    return opts;
}

void TensorflowInferencer::configureGraph()
{
    using namespace tensorflow;
    if(_cpu_only && _agres_optim_cpu_enabled)
        graph::SetDefaultDevice("/job:localhost/replica:0/task:0/device:XLA_CPU:0", &_graph_def);
    if(!_cpu_only && _gpu_number >= 0)
        graph::SetDefaultDevice("/job:localhost/replica:0/task:0/device:GPU:" + std::to_string(_gpu_number), &_graph_def);
}

bool TensorflowInferencer::load(const std::string &filename)
{
    using namespace tensorflow;

    // Configuration for session
    SessionOptions opts = configureSession();

    if (_session) {
        _session->Close();
        delete _session;
        _session = nullptr;
    }
    _graph_def.Clear();

    Status status = NewSession(opts, &_session);

    if (!status.ok()) {
        tf_utils::DebugOutput("tf error: ", status.ToString());

        return _is_loaded = false;
    }

    status = ReadBinaryProto(Env::Default(), filename, &_graph_def);
    if (!status.ok()) {
        tf_utils::DebugOutput("tf error: ", status.ToString());

        return _is_loaded = false;
    }
    configureGraph();

    status = _session->Create(_graph_def);
    if (!status.ok()) {
        tf_utils::DebugOutput("tf error: ", status.ToString());
        return _is_loaded = false;
    } else {
        tf_utils::DebugOutput("Graph successfully loaded!", "");
    }

    parseName(filename);

    _path = filename;
    return _is_loaded = true;
}

void TensorflowInferencer::setName(const std::string& name)
{
    _name = name;
}

void TensorflowInferencer::parseName(const std::string &filename)
{
    auto last_slash = filename.rfind("/");
    if (last_slash == std::string::npos) {
        last_slash = 0;
    }

    auto last_dot = filename.rfind(".");
    if (last_dot == std::string::npos) {
        _name = "UnknownModel";
        return;
    }

    if (last_slash > last_dot) {
        _name = "UnknownModel";
        return;
    }

    _name = filename.substr(last_slash + 1, (last_dot - last_slash) - 1);
}

tensorflow::Tensor TensorflowInferencer::getTensorFromGraph(const std::string &tensor_name)
{
    using namespace tensorflow;

    if (tensor_name.empty()) {
        return Tensor();
    }

    if (!_is_loaded) {
        return Tensor();
    }

    tensorflow::Status status;
    std::vector<tensorflow::Tensor> tensors;

    status = _session->Run({}, {tensor_name}, {}, &tensors);

    tf_utils::DebugOutput("Sucessfully run graph! Status is: ", status.ToString());

    if (!status.ok()) {
        return Tensor();
    }

    return tensors[0];
}

bool TensorflowInferencer::getAllowGrowth() const
{
    return _allow_growth;
}

void TensorflowInferencer::setAllowGrowth(bool allow_growth)
{
    _allow_growth = allow_growth;
}

std::string TensorflowInferencer::getVisibleDevices() const
{
    return _visible_devices;
}

void TensorflowInferencer::setVisibleDevices(const std::string &visible_devices)
{
    _visible_devices = visible_devices;
}

double TensorflowInferencer::getGpuMemoryFraction() const
{
    return _gpu_memory_fraction;
}

void TensorflowInferencer::setGpuMemoryFraction(double gpu_memory_fraction)
{
    if (gpu_memory_fraction > 1.0)
        gpu_memory_fraction = 1.0;
    if (gpu_memory_fraction < 0.0)
        gpu_memory_fraction = 0.1;

    _gpu_memory_fraction = gpu_memory_fraction;
}

int TensorflowInferencer::getGpuNumber() const
{
    return _gpu_number;
}

void TensorflowInferencer::setGpuNumber(int value)
{
    _gpu_number = value;
}

bool TensorflowInferencer::getAggressiveOptimizationCPUEnabled() const
{
    return _agres_optim_cpu_enabled;
}

void TensorflowInferencer::setAggressiveOptimizationCPUEnabled(bool enabled)
{
    _agres_optim_cpu_enabled = enabled;
}

bool TensorflowInferencer::getCpuOnly() const
{
    return _cpu_only;
}

void TensorflowInferencer::setCpuOnly(bool cpu_only)
{
    _cpu_only = cpu_only;
}

bool TensorflowInferencer::getAllowSoftPlacement() const
{
    return _allow_soft_placement;
}

void TensorflowInferencer::setAllowSoftPlacement(bool allowSoftPlacement)
{
    _allow_soft_placement = allowSoftPlacement;
}

bool TensorflowInferencer::getAggressiveOptimizationGPUEnabled() const
{
    return _agres_optim_enabled;
}

void TensorflowInferencer::setAggressiveOptimizationGPUEnabled(bool enabled)
{
    _agres_optim_enabled = enabled;
}

std::string TensorflowInferencer::getPath() const
{
    return _path;
}
