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

#pragma once
#include <glog/logging.h>
#include <iostream>
#include <sstream>

namespace paddle {
namespace lite {

enum class TargetType : int {
  kUnk = 0,
  kHost,
  kX86,
  kCUDA,
  kAny,  // any target
  kLastAsPlaceHolder,
};
enum class PrecisionType : int {
  kUnk = 0,
  kFloat,
  kInt8,
  kAny,  // any precision
  kLastAsPlaceHolder,
};
enum class DataLayoutType : int {
  kUnk = 0,
  kNCHW,
  kAny,  // any data layout
  kLastAsPlaceHolder,
};

// Some helper macro to get a specific TargetType.
#define TARGET(item__) paddle::lite::TargetType::item__
#define TARGET_VAL(item__) static_cast<int>(TARGET(item__))
// Some helper macro to get a specific PrecisionType.
#define PRECISION(item__) paddle::lite::PrecisionType::item__
#define PRECISION_VAL(item__) static_cast<int>(PRECISION(item__))
#define DATALAYOUT(item__) paddle::lite::DataLayoutType::item__

constexpr const int kNumPrecisions =
    PRECISION_VAL(kLastAsPlaceHolder) - PRECISION_VAL(kFloat);
constexpr const int kNumTargets =
    TARGET_VAL(kLastAsPlaceHolder) - TARGET_VAL(kHost);

static const std::string target2string[] = {"unk", "host", "x86", "cuda",
                                            "any"};
static const std::string& TargetToStr(TargetType target) {
  return target2string[static_cast<int>(target)];
}

static const std::string precision2string[] = {"unk", "float", "int8", "any"};
static const std::string& PrecisionToStr(PrecisionType precision) {
  return precision2string[static_cast<int>(precision)];
}

static const std::string datalayout2string[] = {"unk", "NCHW", "any"};
static const std::string& DataLayoutToStr(DataLayoutType x) {
  return datalayout2string[static_cast<int>(x)];
}

/*
 * Place specifies the execution context of a Kernel or input/output for a
 * kernel. It is used to make the analysis of the MIR more clear and accurate.
 */
struct Place {
  TargetType target{TARGET(kUnk)};
  PrecisionType precision{PRECISION(kUnk)};
  DataLayoutType layout{DATALAYOUT(kUnk)};
  short device{0};  // device ID

  Place() = default;
  Place(TargetType target, PrecisionType precision,
        DataLayoutType layout = DATALAYOUT(kNCHW), short device = 0)
      : target(target), precision(precision), layout(layout), device(device) {}

  bool is_valid() const {
    return target != TARGET(kUnk) && precision != PRECISION(kUnk) &&
           layout != DATALAYOUT(kUnk);
  }

  size_t hash() const;

  bool operator==(const Place& other) const {
    return target == other.target && precision == other.precision &&
           layout == other.layout && device == other.device;
  }

  bool operator!=(const Place& other) const { return !(*this == other); }

  friend bool operator<(const Place& a, const Place& b);

  friend std::ostream& operator<<(std::ostream& os, const Place& other) {
    os << other.DebugString();
    return os;
  }

  std::string DebugString() const;
};

// Memory copy directions.
enum class IoDirection {
  HtoH = 0,  // Host to host
  HtoD,      // Host to device
  DtoH,      // Device to host
  DtoD,      // Device to device
};

// This interface should be specified by each kind of target.
template <TargetType Target, typename StreamTy = int, typename EventTy = int>
class TargetWrapper {
 public:
  using stream_t = StreamTy;
  using event_t = EventTy;

  static size_t num_devices() { return 0; }
  static size_t maximum_stream() { return 0; }

  static void CreateStream(stream_t* stream) {}
  static void DestroyStream(const stream_t& stream) {}

  static void CreateEvent(event_t* event) {}
  static void DestroyEvent(const event_t& event) {}

  static void RecordEvent(const event_t& event) {}
  static void SyncEvent(const event_t& event) {}

  static void StreamSync(const stream_t& stream) {}

  static void* Malloc(size_t size) {
    LOG(FATAL) << "Unimplemented malloc for " << TargetToStr(Target);
    return nullptr;
  }
  static void Free(void* ptr) { LOG(FATAL) << "Unimplemented"; }

  static void MemcpySync(void* dst, const void* src, size_t size,
                         IoDirection dir) {
    LOG(FATAL) << "Unimplemented";
  }
  static void MemcpyAsync(void* dst, const void* src, size_t size,
                          IoDirection dir, const stream_t& stream) {
    MemcpySync(dst, src, size, dir);
  }
};

// This interface should be specified by each kind of target.
template <>
class TargetWrapper<TARGET(kHost)> {
 public:
  using stream_t = int;
  using event_t = int;

  static size_t num_devices() { return 0; }
  static size_t maximum_stream() { return 0; }

  static void CreateStream(stream_t* stream) {}
  static void DestroyStream(const stream_t& stream) {}

  static void CreateEvent(event_t* event) {}
  static void DestroyEvent(const event_t& event) {}

  static void RecordEvent(const event_t& event) {}
  static void SyncEvent(const event_t& event) {}

  static void StreamSync(const stream_t& stream) {}

  static void* Malloc(size_t size);
  static void Free(void* ptr);

  static void MemcpySync(void* dst, const void* src, size_t size,
                         IoDirection dir);
  static void MemcpyAsync(void* dst, const void* src, size_t size,
                          IoDirection dir, const stream_t& stream) {
    MemcpySync(dst, src, size, dir);
  }
};

}  // namespace lite
}  // namespace paddle