// Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <Eigen/Core>
#include "paddle/fluid/framework/eigen.h"
#include "paddle/fluid/framework/operator.h"
#include "paddle/fluid/lite/core/kernel.h"
#include "paddle/fluid/lite/core/op_lite.h"
#include "paddle/fluid/lite/core/op_registry.h"
#include "paddle/fluid/lite/core/type_system.h"
#include "paddle/fluid/lite/operators/relu_op.h"

namespace paddle {
namespace lite {
namespace kernels {
namespace x86 {

template <typename T>
class ReluCompute : public KernelLite<TARGET(kX86), PRECISION(kFloat)> {
 public:
  using param_t = operators::ReluParam;

  void Run() override {
    auto& param = *param_.get_mutable<param_t>();
    auto n = param.input->dims().production();
    const float* input = param.input->data<float>();
    float* output = param.output->mutable_data<float>();
    for (int i = 0; i < n; i++) {
      output[i] = std::max(0.f, input[i]);
    }
  }

  virtual ~ReluCompute() = default;
};

}  // namespace x86
}  // namespace kernels
}  // namespace lite
}  // namespace paddle

REGISTER_LITE_KERNEL(relu, kX86, kFloat, kNCHW,
                     paddle::lite::kernels::x86::ReluCompute<float>, def)
    .BindInput("X", {LiteType::GetTensorTy(TARGET(kX86))})
    .BindOutput("Out", {LiteType::GetTensorTy(TARGET(kX86))})
    .Finalize();
