// Copyright 2021 The TensorStore Authors
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

#ifndef TENSORSTORE_INTERNAL_DIMENSION_INDEXED_JSON_BINDER_H_
#define TENSORSTORE_INTERNAL_DIMENSION_INDEXED_JSON_BINDER_H_

#include <algorithm>

#include "tensorstore/index_space/index_transform.h"
#include "tensorstore/internal/json.h"
#include "tensorstore/rank.h"
#include "tensorstore/util/status.h"

namespace tensorstore {
namespace internal {
namespace json_binding {

/// JSON binder for arrays indexed by dimensions (length limited by `kMaxRank`).
///
/// Example usage:
///
///     [](auto is_loading, const auto &options, auto *obj, auto *j) {
///       DimensionIndex rank = dynamic_rank;
///       return jb::Sequence(
///           jb::Member("a", jb::Projection(
///                               &X::a,
///                               jb::DimensionIndexedVector(&rank))),
///           jb::Member("b", jb::Projection(
///                               &X::b,
///                               jb::DimensionIndexedVector(&rank))))
///           (is_loading, options, obj, j);
///     }
///
/// \param rank Pointer to common rank constraint.  Ignored when converting *to*
///     JSON.  When converting *from* JSON and `rank` is non-null: if
///     `*rank != dynamic_rank`, the length of the array must equal `*rank`;
///     otherwise, `*rank` is set to the length, and may serve as a constraint
///     for subsequent uses of other `DimensionIndexedVector` binders.
/// \param element_binder Binder used for elements of the array.
template <typename ElementBinder = decltype(DefaultBinder<>)>
constexpr auto DimensionIndexedVector(
    DimensionIndex* rank, ElementBinder element_binder = DefaultBinder<>) {
  return json_binding::Array(
      [](auto& c) { return c.size(); },
      [rank](auto& c, size_t size) {
        TENSORSTORE_RETURN_IF_ERROR(ValidateRank(size));
        if (rank) {
          if (*rank == dynamic_rank) {
            *rank = size;
          } else if (*rank != static_cast<DimensionIndex>(size)) {
            return internal::JsonValidateArrayLength(size, *rank);
          }
        }
        c.resize(size);
        return absl::OkStatus();
      },
      [](auto& c, size_t i) -> decltype(auto) { return c[i]; },
      std::move(element_binder));
}

/// JSON binder for dimension-indexed shape arrays, where each element must be
/// an integer in `[0, max_size]`.
///
/// Refer to the documentation of `DimensionIndexedVector` for details on the
/// `rank` parameter.
constexpr auto ShapeVector(DimensionIndex* rank,
                           Index max_size = kInfSize - 1) {
  return DimensionIndexedVector(rank,
                                json_binding::Integer<Index>(0, max_size));
}

/// JSON binder for dimension-indexed shape arrays, where each element must be
/// an integer in `[1, max_size]`.
///
/// Refer to the documentation of `DimensionIndexedVector` for details on the
/// `rank` parameter.
constexpr auto ChunkShapeVector(DimensionIndex* rank,
                                Index max_size = kInfSize - 1) {
  return DimensionIndexedVector(rank,
                                json_binding::Integer<Index>(1, max_size));
}

/// JSON binder for dimension-indexed label arrays, where each element is an
/// empty or unique non-empty string.
///
/// Refer to the documentation of `DimensionIndexedVector` for details on the
/// `rank` parameter.
///
/// When converting from JSON, if `rank` is non-null and
/// `*rank != dynamic_rank`, the JSON value is allowed to be discarded
/// (i.e. unspecified), in which case the array the C++ object is initialized to
/// an array of `*rank` empty strings.
///
/// When converting to JSON, if all labels are empty strings, a discarded JSON
/// value is returned.
constexpr auto DimensionLabelVector(DimensionIndex* rank) {
  return [rank](auto is_loading, const auto& options, auto* obj, auto* j) {
    if constexpr (is_loading) {
      if (rank && *rank != dynamic_rank) {
        if (j->is_discarded()) {
          obj->resize(*rank);
          return absl::OkStatus();
        }
      }
      TENSORSTORE_RETURN_IF_ERROR(
          DimensionIndexedVector(rank)(is_loading, options, obj, j));
      return internal_index_space::ValidateLabelsAreUnique(*obj);
    } else {
      if (std::all_of(obj->begin(), obj->end(),
                      [](const auto& v) { return v.empty(); })) {
        return absl::OkStatus();
      }
      return DimensionIndexedVector(nullptr)(is_loading, options, obj, j);
    }
  };
}

}  // namespace json_binding
}  // namespace internal
}  // namespace tensorstore

#endif  // TENSORSTORE_INTERNAL_DIMENSION_INDEXED_JSON_BINDER_H_
