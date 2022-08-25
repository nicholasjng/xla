/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

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

#ifndef TENSORFLOW_COMPILER_XLA_SERVICE_GPU_TRIANGULAR_SOLVE_THUNK_H_
#define TENSORFLOW_COMPILER_XLA_SERVICE_GPU_TRIANGULAR_SOLVE_THUNK_H_

#include <optional>

#include "tensorflow/compiler/xla/service/buffer_assignment.h"
#include "tensorflow/compiler/xla/service/gpu/buffer_allocations.h"
#include "tensorflow/compiler/xla/service/gpu/thunk.h"
#include "tensorflow/compiler/xla/service/hlo_instruction.h"
#include "tensorflow/compiler/xla/stream_executor/blas.h"
#include "tensorflow/compiler/xla/stream_executor/gpu/gpu_asm_opts.h"
#include "tensorflow/compiler/xla/types.h"
#include "tensorflow/compiler/xla/xla_data.pb.h"
#include "tensorflow/core/lib/core/status.h"
#include "tensorflow/core/platform/stream_executor_no_cuda.h"

namespace xla {
namespace gpu {

// This class stores everything that StreamExecutor needs to launch a triangular
// solve (BlasTrsm). It is generated by IrEmitter.
//
// Thread-compatible.
class TriangularSolveThunk : public Thunk {
 public:
  TriangularSolveThunk(ThunkInfo thunk_info,
                       const TriangularSolveOptions& options,
                       se::GpuAsmOpts asm_opts,
                       const BufferAllocation::Slice& a_buffer,
                       const BufferAllocation::Slice& b_buffer,
                       const BufferAllocation::Slice& temp_buffer,
                       PrimitiveType type, int64_t batch_size, int64_t m,
                       int64_t n, int64_t a_batch_stride,
                       int64_t b_batch_stride);

  TriangularSolveThunk(const TriangularSolveThunk&) = delete;
  TriangularSolveThunk& operator=(const TriangularSolveThunk&) = delete;

  Status ExecuteOnStream(const ExecuteParams& params) override;

 private:
  se::GpuAsmOpts asm_opts_;
  const se::blas::UpperLower uplo_;
  const se::blas::Side side_;
  const se::blas::Diagonal unit_diagonal_;
  se::blas::Transpose transpose_a_;

  const BufferAllocation::Slice a_buffer_;
  const BufferAllocation::Slice b_buffer_;
  const BufferAllocation::Slice temp_buffer_;

  const PrimitiveType type_;
  const int64_t batch_size_;
  const int64_t m_;
  const int64_t n_;
  const int64_t a_batch_stride_;
  const int64_t b_batch_stride_;
};

Status RunTriangulatSolve(se::DeviceMemoryBase a_data,
                          se::DeviceMemoryBase b_data,
                          se::DeviceMemoryBase temp_data,
                          se::GpuAsmOpts asm_opts, se::blas::UpperLower uplo,
                          se::blas::Side side, se::blas::Diagonal unit_diagonal,
                          se::blas::Transpose transpose_a, PrimitiveType type,
                          int64_t batch_size, int64_t m, int64_t n,
                          int64_t a_batch_stride, int64_t b_batch_stride,
                          se::Stream* stream);

}  // namespace gpu
}  // namespace xla

#endif  // TENSORFLOW_COMPILER_XLA_SERVICE_GPU_TRIANGULAR_SOLVE_THUNK_H_
