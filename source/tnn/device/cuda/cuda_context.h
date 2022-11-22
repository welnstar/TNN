// Tencent is pleased to support the open source community by making TNN available.
//
// Copyright (C) 2020 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// https://opensource.org/licenses/BSD-3-Clause
//
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

#ifndef TNN_SOURCE_TNN_DEVICE_CUDA_CUDA_CONTEXT_H_
#define TNN_SOURCE_TNN_DEVICE_CUDA_CUDA_CONTEXT_H_

#include <map>
#include <string>
#include <vector>
#include <cuda_runtime.h>

#include <cudnn.h>
#include <cublas_v2.h>

#include "tnn/core/context.h"
#include "tnn/interpreter/raw_buffer.h"

namespace TNN_NS {

class CudaContext : public Context {
public:
    // @brief deconstructor
    ~CudaContext();

    // @brief setup with specified device id
    Status Setup(int device_id);

    // @brief load library
    virtual Status LoadLibrary(std::vector<std::string> path) override;

    // @brief get tnn command queue
    // @param command_queue device command queue for forward
    virtual Status GetCommandQueue(void** command_queue) override;

    // @brief set tnn command queue
    // @param command_queue device command queue for forward
    virtual Status SetCommandQueue(void* command_queue) override;

    // @brief share tnn command queue to another context
    virtual Status ShareCommandQueue(Context* context);

    // @brief before instance forward
    virtual Status OnInstanceForwardBegin() override;

    // @brief after instance forward
    virtual Status OnInstanceForwardEnd() override;

    // @brief wait for jobs in the current context to complete
    virtual Status Synchronize() override;

    // @brief get cuda stream
    cudaStream_t& GetStream();

    // @brief get cudnn stream
    cudnnHandle_t& GetCudnnHandle();

    // @brief get cublas stream
    cublasHandle_t& GetCublasHandle();

    // @brief get workspace
    void* GetWorkspace();

    // @brief get worksapce size
    void SetWorkspaceSize(int size);

    // @brief set quant resource
    Status AddQuantResource(std::string name, std::shared_ptr<RawBuffer>);

    // @brief get quant resource
    std::shared_ptr<RawBuffer> GetQuantResource(std::string name);

private:
    cudnnHandle_t cudnn_handle_;
    cublasHandle_t cublas_handle_;
    cudaStream_t stream_;
    void* workspace_ = nullptr;
    int workspace_size_ = 0;
    int device_id_ = 0;
    bool own_stream_ = false;
    //TODO: share between in same thread(create instance and instance forward may in different threads).
    //lazy create
    bool own_cudnn_handle_ = false;
    bool own_cublas_handle_ = false;

    std::map<std::string, std::shared_ptr<RawBuffer>> quant_extra_res_;
};

}  //  namespace TNN_NS;

#endif  //  TNN_SOURCE_TNN_DEVICE_CUDA_CUDA_CONTEXT_H_
