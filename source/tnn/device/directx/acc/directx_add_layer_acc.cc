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

#include "tnn/device/directx/acc/directx_binary_layer_acc.h"
// #include "tnn/device/opencl/imagebuffer_convertor.h"

#include "tnn/core/macro.h"

namespace TNN_NS {

namespace directx {

DECLARE_DIRECTX_BINARY_ACC(Add);

Status DirectXAddLayerAcc::Init(Context *context, LayerParam *param, LayerResource *resource,
                               const std::vector<Blob *> &inputs, const std::vector<Blob *> &outputs) {
    LOGD("Init Add Acc\n");
    Status ret = DirectXBinaryLayerAcc::Init(context, param, resource, inputs, outputs);
    RETURN_ON_NEQ(ret, TNN_OK);

    op_name_ = "Add";

    /*
    // create kernel
    std::set<std::string> build_options;
    std::string kernel_name;
    std::string compute = "in0+in1";
    build_options.emplace(" -DOPERATOR=" + compute);
    ret = CreateExecuteUnit(execute_units_[0], "binary", kernel_name_, build_options);
    if (ret != TNN_OK) {
        LOGE("create execute unit failed!\n");
        return ret;
    }
    */

    return TNN_OK;
}

DirectXAddLayerAcc::~DirectXAddLayerAcc() {}

REGISTER_DIRECTX_ACC(Add, LAYER_ADD)
REGISTER_DIRECTX_LAYOUT(LAYER_ADD, DATA_FORMAT_NHC4W4);

} // namespace directx

}  // namespace TNN_NS