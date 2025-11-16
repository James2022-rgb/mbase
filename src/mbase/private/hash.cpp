// my header --------------------------------------------
#include "mbase/public/hash.h"

// external headers -------------------------------------
#define XXH_STATIC_LINKING_ONLY
#define XXH_IMPLEMENTATION

#include "xxhash.h"

// project headers --------------------------------------


namespace mbase {

namespace detail {

static_assert(HasherStateTraits<uint32_t>::kStorageSize == sizeof(XXH32_state_s));
static_assert(HasherStateTraits<uint32_t>::kStorageAlign == alignof(XXH32_state_s));

static_assert(HasherStateTraits<uint64_t>::kStorageSize == sizeof(XXH64_state_s));
static_assert(HasherStateTraits<uint64_t>::kStorageAlign == alignof(XXH64_state_s));

}

template<>
HasherN<uint32_t>::ValueType HasherN<uint32_t>::ComputeBytes(void const* s, uint64_t length) {
  return XXH32(s, length, 0);
}

template<>
void HasherN<uint32_t>::Reset();

template<>
HasherN<uint32_t>::HasherN() {
  new(&storage_)XXH32_state_s {};
  Reset();
}
template<>
HasherN<uint32_t>::~HasherN() {
  reinterpret_cast<XXH32_state_s*>(&storage_)->~XXH32_state_s();
}

template<>
HasherN<uint32_t>::ValueType HasherN<uint32_t>::Finish() {
  value_ = XXH32_digest(reinterpret_cast<XXH32_state_s const*>(&storage_));
  return value_;
}

template<>
void HasherN<uint32_t>::Reset() {
  XXH32_reset(reinterpret_cast<XXH32_state_s*>(&storage_), 0);
}

template<>
void HasherN<uint32_t>::DoBytes(void const* s, uint64_t length) {
  XXH32_update(reinterpret_cast<XXH32_state_s*>(&storage_), s, size_t(length));
}

template<>
HasherN<uint64_t>::ValueType HasherN<uint64_t>::ComputeBytes(void const* s, uint64_t length) {
  return XXH64(s, length, 0);
}

template<>
void HasherN<uint64_t>::Reset();

template<>
HasherN<uint64_t>::HasherN() {
  new(&storage_)XXH64_state_s {};
  Reset();
}
template<>
HasherN<uint64_t>::~HasherN() {
  reinterpret_cast<XXH64_state_s*>(&storage_)->~XXH64_state_s();
}

template<>
HasherN<uint64_t>::ValueType HasherN<uint64_t>::Finish() {
  value_ = XXH64_digest(reinterpret_cast<XXH64_state_s const*>(&storage_));
  return value_;
}

template<>
void HasherN<uint64_t>::Reset() {
  XXH64_reset(reinterpret_cast<XXH64_state_s*>(&storage_), 0);
}

template<>
void HasherN<uint64_t>::DoBytes(void const* s, uint64_t length) {
  XXH64_update(reinterpret_cast<XXH64_state_s*>(&storage_), s, size_t(length));
}

template class HasherN<uint32_t>;
template class HasherN<uint64_t>;


}
