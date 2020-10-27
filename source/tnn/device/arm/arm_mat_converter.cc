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

#include "tnn/device/arm/arm_mat_converter.h"

#include "tnn/device/arm/arm_mat_util.h"
#include "tnn/device/arm/arm_util.h"

#include "tnn/utils/dims_vector_utils.h"
#include "tnn/utils/mat_converter_utils.h"

namespace TNN_NS {

Status ArmMatConverterAcc::Copy(Mat& src, Mat& dst, void* command_queue) {
    Status ret = TNN_OK;

    ret = CheckMatConverterParams(src, dst, false);
    if (ret != TNN_OK)
        return ret;

    auto elem_num = DimsVectorUtils::Count(src.GetDims());

    if (src.GetMatType() == NGRAY || src.GetMatType() == NNV21 || src.GetMatType() == NNV12 || 
        src.GetMatType() == N8UC3 || src.GetMatType() == N8UC4) {
        memcpy(dst.GetData(), src.GetData(), elem_num * sizeof(uint8_t));
    } else if(src.GetMatType() == NCHW_FLOAT) {
        memcpy(dst.GetData(), src.GetData(), elem_num * sizeof(float));
    } else {
        return Status(TNNERR_PARAM_ERR, "convert type not support yet");
    }
    return ret;
}

Status ArmMatConverterAcc::Resize(Mat& src, Mat& dst, ResizeParam param, void* command_queue) {
    Status ret = TNN_OK;

    ret = CheckMatConverterParams(src, dst, true);
    if (ret != TNN_OK)
        return ret;

    int dst_width  = dst.GetWidth();
    int dst_height = dst.GetHeight();

    if (dst_width == 0 || dst_height == 0) {
        return Status(TNNERR_INVALID_INPUT, "dst size is zero");
    }

    if (src.GetMatType() == NGRAY) {
        if (param.type == INTERP_TYPE_LINEAR) {
            ResizeBilinearC1((uint8_t*)src.GetData(), src.GetBatch(), src.GetWidth(), src.GetHeight(),
                             (uint8_t*)dst.GetData(), dst_width, dst_height);
        } else if (param.type == INTERP_TYPE_NEAREST) {
            ResizeNearestC1((uint8_t*)src.GetData(), src.GetBatch(), src.GetWidth(), src.GetHeight(),
                            (uint8_t*)dst.GetData(), dst_width, dst_height);
        } else {
            return Status(TNNERR_PARAM_ERR, "interpolation type not support yet");
        }
    } else if (src.GetMatType() == N8UC3) {
        if (param.type == INTERP_TYPE_LINEAR) {
            ResizeBilinearC3((uint8_t*)src.GetData(), src.GetBatch(), src.GetWidth(), src.GetHeight(),
                             (uint8_t*)dst.GetData(), dst_width, dst_height);
        } else if (param.type == INTERP_TYPE_NEAREST) {
            ResizeNearestC3((uint8_t*)src.GetData(), src.GetBatch(), src.GetWidth(), src.GetHeight(),
                            (uint8_t*)dst.GetData(), dst_width, dst_height);
        } else {
            return Status(TNNERR_PARAM_ERR, "interpolation type not support yet");
        }
    } else if (src.GetMatType() == N8UC4) {
        if (param.type == INTERP_TYPE_LINEAR) {
            ResizeBilinearC4((uint8_t*)src.GetData(), src.GetBatch(), src.GetWidth(), src.GetHeight(),
                             (uint8_t*)dst.GetData(), dst_width, dst_height);
        } else if (param.type == INTERP_TYPE_NEAREST) {
            ResizeNearestC4((uint8_t*)src.GetData(), src.GetBatch(), src.GetWidth(), src.GetHeight(),
                            (uint8_t*)dst.GetData(), dst_width, dst_height);
        } else {
            return Status(TNNERR_PARAM_ERR, "interpolation type not support yet");
        }
    } else if (src.GetMatType() == NNV21 || src.GetMatType() == NNV12) {
        if (param.type == INTERP_TYPE_LINEAR) {
            ResizeBilinearYUV420sp((uint8_t*)src.GetData(), src.GetBatch(), src.GetWidth(), src.GetHeight(),
                                   (uint8_t*)dst.GetData(), dst_width, dst_height);
        } else if (param.type == INTERP_TYPE_NEAREST) {
            ResizeNearestYUV420sp((uint8_t*)src.GetData(), src.GetBatch(), src.GetWidth(), src.GetHeight(),
                                  (uint8_t*)dst.GetData(), dst_width, dst_height);
        } else {
            return Status(TNNERR_PARAM_ERR, "interpolation type not support yet");
        }
    } else {
        return Status(TNNERR_PARAM_ERR, "convert type not support yet");
    }

    return ret;
}

Status ArmMatConverterAcc::Crop(Mat& src, Mat& dst, CropParam param, void* command_queue) {
    Status ret = TNN_OK;

    ret = CheckMatConverterParams(src, dst, true);
    if (ret != TNN_OK)
        return ret;

    if (src.GetMatType() == NGRAY) {
        // element size 1
        for (int b = 0; b < src.GetBatch(); ++b) {
            auto src_ptr = GET_OFFSET_PTR(src.GetData(), b * src.GetHeight() * src.GetWidth() +
                                          param.top_left_x + param.top_left_y * src.GetWidth());
            auto dst_ptr = GET_OFFSET_PTR(dst.GetData(), b * dst.GetHeight() * dst.GetWidth());
            MatMemcpy2D(src_ptr, dst_ptr, param.width, param.height, src.GetWidth(), dst.GetWidth());
        }
    } else if (src.GetMatType() == N8UC3) {
        // element size 3
        for (int b = 0; b < src.GetBatch(); ++b) {
            auto src_ptr = GET_OFFSET_PTR(src.GetData(), b * src.GetHeight() * src.GetWidth() * 3 +
                                          (param.top_left_x + param.top_left_y * src.GetWidth()) * 3);
            auto dst_ptr = GET_OFFSET_PTR(dst.GetData(), b * dst.GetHeight() * dst.GetWidth() * 3);
            MatMemcpy2D(src_ptr, dst_ptr, param.width * 3, param.height, src.GetWidth() * 3, dst.GetWidth() * 3);
        }
    } else if (src.GetMatType() == N8UC4) {
        // element size 4
        for (int b = 0; b < src.GetBatch(); ++b) {
            auto src_ptr = GET_OFFSET_PTR(src.GetData(), b * src.GetHeight() * src.GetWidth() * 4 +
                                          (param.top_left_x + param.top_left_y * src.GetWidth()) * 4);
            auto dst_ptr = GET_OFFSET_PTR(dst.GetData(), b * dst.GetHeight() * dst.GetWidth() * 4);
            MatMemcpy2D(src_ptr, dst_ptr, param.width * 4, param.height, src.GetWidth() * 4, dst.GetWidth() * 4);
        }
    } else if (src.GetMatType() == NNV21 || src.GetMatType() == NNV12) {
        if (param.top_left_x % 2 || param.top_left_y % 2 || param.width % 2 || param.height % 2) {
            return Status(TNNERR_PARAM_ERR, "corp param can not be odd");
        }
        for (int b = 0; b < src.GetBatch(); ++b) {
            // crop y
            auto src_ptr = GET_OFFSET_PTR(src.GetData(), b * src.GetHeight() * src.GetWidth() * 3 / 2 +
                                          param.top_left_x + param.top_left_y * src.GetWidth());
            auto dst_ptr = GET_OFFSET_PTR(dst.GetData(), b * dst.GetHeight() * dst.GetWidth() * 3 / 2);
            MatMemcpy2D(src_ptr, dst_ptr, param.width, param.height, src.GetWidth(), dst.GetWidth());
            // crop uv
            src_ptr = GET_OFFSET_PTR(src.GetData(), b * src.GetHeight() * src.GetWidth() * 3 / 2 +
                      src.GetWidth() * src.GetHeight() + param.top_left_x + param.top_left_y * src.GetWidth() / 2);
            dst_ptr = GET_OFFSET_PTR(dst.GetData(), b * dst.GetHeight() * dst.GetWidth() * 3 / 2 +
                      dst.GetWidth() * dst.GetHeight());
            MatMemcpy2D(src_ptr, dst_ptr, param.width, param.height / 2, src.GetWidth(), dst.GetWidth());
        }
    } else {
        return Status(TNNERR_PARAM_ERR, "convert type not support yet");
    }

    return ret;
}

Status ArmMatConverterAcc::WarpAffine(Mat& src, Mat& dst, WarpAffineParam param, void* command_queue) {
    Status ret = TNN_OK;

    ret = CheckMatConverterParams(src, dst, true);
    if (ret != TNN_OK)
        return ret;

    int dst_width  = dst.GetWidth();
    int dst_height = dst.GetHeight();

    if (dst_width == 0 || dst_height == 0) {
        return Status(TNNERR_INVALID_INPUT, "dst size is zero");
    }

    if (src.GetMatType() == NGRAY) {
        if (param.interp_type == INTERP_TYPE_LINEAR && param.border_type == BORDER_TYPE_CONSTANT) {
            WarpAffineBilinearC1((uint8_t*)src.GetData(), src.GetBatch(), src.GetWidth(), src.GetHeight(),
                                 (uint8_t*)dst.GetData(), dst_width, dst_height,
                                 param.transform, param.border_val);
        } else {
            return Status(TNNERR_PARAM_ERR, "warpaffine type not support yet");
        }
    } else if (src.GetMatType() == N8UC3) {
        if (param.interp_type == INTERP_TYPE_LINEAR && param.border_type == BORDER_TYPE_CONSTANT) {
            WarpAffineBilinearC3((uint8_t*)src.GetData(), src.GetBatch(), src.GetWidth(), src.GetHeight(),
                                 (uint8_t*)dst.GetData(), dst_width, dst_height,
                                 param.transform, param.border_val);
        } else {
            return Status(TNNERR_PARAM_ERR, "warpaffine type not support yet");
        }
    } else if (src.GetMatType() == N8UC4) {
        if (param.interp_type == INTERP_TYPE_LINEAR && param.border_type == BORDER_TYPE_CONSTANT) {
            WarpAffineBilinearC4((uint8_t*)src.GetData(), src.GetBatch(), src.GetWidth(), src.GetHeight(),
                                 (uint8_t*)dst.GetData(), dst_width, dst_height,
                                 param.transform, param.border_val);
        } else {
            return Status(TNNERR_PARAM_ERR, "warpaffine type not support yet");
        }
    } else if (src.GetMatType() == NNV21 || src.GetMatType() == NNV12) {
        if (param.interp_type == INTERP_TYPE_LINEAR && param.border_type == BORDER_TYPE_CONSTANT) {
            WarpAffineBilinearYUV420sp((uint8_t*)src.GetData(), src.GetBatch(), src.GetWidth(), src.GetHeight(),
                                       (uint8_t*)dst.GetData(), dst_width, dst_height,
                                       param.transform, param.border_val);
        } else {
            return Status(TNNERR_PARAM_ERR, "warpaffine type not support yet");
        }
    } else {
        return Status(TNNERR_PARAM_ERR, "convert type not support yet");
    }

    return ret;
}

Status ArmMatConverterAcc::CvtColor(Mat& src, Mat& dst, ColorConversionType type, void* command_queue) {
    Status ret = TNN_OK;

    ret = CheckMatConverterParams(src, dst, true);
    if (ret != TNN_OK)
        return ret;

    if (type == COLOR_CONVERT_NV12TOBGR) {
        NV12ToBGR((uint8_t*)src.GetData(), (uint8_t*)dst.GetData(), src.GetBatch()*src.GetHeight(), src.GetWidth());
    } else if (type == COLOR_CONVERT_NV21TOBGR) {
        NV21ToBGR((uint8_t*)src.GetData(), (uint8_t*)dst.GetData(), src.GetBatch()*src.GetHeight(), src.GetWidth());
    } else if (type == COLOR_CONVERT_NV12TOBGRA) {
        NV12ToBGRA((uint8_t*)src.GetData(), (uint8_t*)dst.GetData(), src.GetBatch()*src.GetHeight(), src.GetWidth());
    } else if (type == COLOR_CONVERT_NV21TOBGRA) {
        NV21ToBGRA((uint8_t*)src.GetData(), (uint8_t*)dst.GetData(), src.GetBatch()*src.GetHeight(), src.GetWidth());
    } else if (type == COLOR_CONVERT_BGRTOGRAY) {
        BGRToGray((uint8_t*)src.GetData(), (uint8_t*)dst.GetData(), src.GetBatch()*src.GetHeight(), src.GetWidth());
    } else if (type == COLOR_CONVERT_BGRATOGRAY) {
        BGRAToGray((uint8_t*)src.GetData(), (uint8_t*)dst.GetData(), src.GetBatch()*src.GetHeight(), src.GetWidth());
    } else {
        return Status(TNNERR_PARAM_ERR, "color conversion type not support yet");
    }

    return ret;
}

Status ArmMatConverterAcc::ResizeAndPaste(Mat& src, Mat& dst, ResizeParam param, PasteParam paste_param, void* command_queue) {
    return Status(TNNERR_PARAM_ERR, "arm not support ResizeAndPaste conversion");
}

Status ArmMatConverterAcc::ConcatMatWithBatch(std::vector<Mat>& src_vec, Mat& dst, void* command_queue) {
    return Status(TNNERR_PARAM_ERR, "arm not support ConcatMatWithBatch conversion");
}


DECLARE_MAT_CONVERTER_CREATER(Arm);
REGISTER_MAT_CONVERTER(Arm, DEVICE_ARM);

}  // namespace TNN_NS
