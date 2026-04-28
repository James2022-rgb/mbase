#pragma once

#define MBASE_ACCESSOR_GETV(type, name) \
  [[nodiscard]] type name() const { return name##_; }
#define MBASE_ACCESSOR_GETR(type, name) \
  [[nodiscard]] type& name() { return name##_; }
#define MBASE_ACCESSOR_GETCR(type, name) \
  [[nodiscard]] type const& name() const { return name##_; }
#define MBASE_ACCESSOR_GETCR_OPTIONAL(type, name) \
  MBASE_ACCESSOR_GETCR(std::optional<type>, name)

#define MBASE_ACCESSOR_ARRAY_PROXY(type, name) \
  [[nodiscard]] mbase::ArrayProxy<type const> name() const { return name##_; }

