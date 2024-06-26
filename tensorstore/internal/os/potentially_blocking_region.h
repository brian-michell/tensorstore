// Copyright 2022 The TensorStore Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef TENSORSTORE_INTERNAL_OS_POTENTIALLY_BLOCKING_REGION_H_
#define TENSORSTORE_INTERNAL_OS_POTENTIALLY_BLOCKING_REGION_H_

namespace tensorstore {
namespace internal {

// Extension point used internally at Google to support lightweight fibers.
class [[maybe_unused]] PotentiallyBlockingRegion {
 public:
  ~PotentiallyBlockingRegion() {}
};

}  // namespace internal
}  // namespace tensorstore

#endif  // TENSORSTORE_INTERNAL_OS_POTENTIALLY_BLOCKING_REGION_H_
