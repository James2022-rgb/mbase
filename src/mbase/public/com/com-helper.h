#pragma once

#include <span>
#include <vector>
#include <algorithm>
#include <iterator>

#include "mbase/public/com/com.h"

namespace mbase {

template<class T>
std::vector<ComPtr<T>> ToComPtrVector(std::span<T* const> span) {
  std::vector<ComPtr<T>> result;
  result.reserve(span.size());
  std::transform(
    span.begin(), span.end(),
    std::back_inserter(result),
    [](T* ptr) { return ComPtr<T>(ptr); }
  );

  return result;
}

template<class T>
std::vector<T*> ToRawPtrVector(std::span<ComPtr<T>> span) {
  std::vector<T*> result;
  result.reserve(span.size());
  std::transform(
    span.begin(), span.end(),
    std::back_inserter(result),
    [](ComPtr<T> const& ptr) { return ptr.Get(); }
  );
  return result;
}

template<class T>
std::vector<T*> ToRawPtrVector(std::vector<ComPtr<T>> span) {
  std::vector<T*> result;
  result.reserve(span.size());
  std::transform(
    span.begin(), span.end(),
    std::back_inserter(result),
    [](ComPtr<T> const& ptr) { return ptr.Get(); }
  );
  return result;
}

} // namespace mbase
