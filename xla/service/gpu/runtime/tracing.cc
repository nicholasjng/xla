/* Copyright 2022 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "xla/service/gpu/runtime/tracing.h"

#include <memory>

#include "absl/status/statusor.h"
#include "absl/strings/str_format.h"
#include "xla/runtime/executable.h"
#include "xla/runtime/tracing.h"
#include "xla/service/gpu/runtime/support.h"
#include "tsl/profiler/lib/scoped_annotation_stack.h"

namespace xla {
namespace gpu {

using ::xla::runtime::CustomCall;
using ::xla::runtime::HloTrace;

using ::tsl::profiler::ScopedAnnotationStack;

//===----------------------------------------------------------------------===//

namespace {

struct ActivityStart {
  LLVM_ATTRIBUTE_ALWAYS_INLINE
  absl::StatusOr<int64_t> operator()(runtime::HloTrace annotation) const {
    return ScopedAnnotationStack::ActivityStart([&] {
      // We use the same tracing annotation scheme as the ThunkSequence (see
      // implementation of `GetThunkInfo` in `ir_emitter_unnested.cc`).
      return absl::StrFormat("Thunk:#hlo_op=%s,hlo_module=%s,program_id=%d#",
                             annotation.hlo_op, annotation.module,
                             annotation.program_id);
    });
  }

  static ActivityStart Handler() { return ActivityStart(); }
};

struct ActivityEnd {
  LLVM_ATTRIBUTE_ALWAYS_INLINE
  absl::Status operator()(int64_t activity_id) const {
    ScopedAnnotationStack::ActivityEnd(activity_id);
    return absl::OkStatus();
  }

  static ActivityEnd Handler() { return ActivityEnd(); }
};

}  // namespace

//===----------------------------------------------------------------------===//

XLA_RUNTIME_DEFINE_CUSTOM_CALL(Start, ActivityStart::Handler(), checks,
                               CustomCall::Bind("xla.trace.activity_start")
                                   .Attr<HloTrace>("annotation")
                                   .Ret<int64_t>());

XLA_RUNTIME_DEFINE_CUSTOM_CALL(
    End, ActivityEnd::Handler(), checks,
    CustomCall::Bind("xla.trace.activity_end").Arg<int64_t>());

//===----------------------------------------------------------------------===//

void RegisterTracingTypeIdNames(runtime::TypeIDNameRegistry& registry) {
  runtime::PopulateTraceTypeIdNames(registry);
}

void RegisterTracingCustomCalls(runtime::DirectCustomCallRegistry& registry) {
  registry.Register("xla.trace.activity_start", Start);
  registry.Register("xla.trace.activity_end", End);
}

}  // namespace gpu
}  // namespace xla
