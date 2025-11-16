#pragma once

#define MBASE_PP_VA_ARGS(...) , ##__VA_ARGS__

#define MBASE_PP_CONCAT_IMPL(x, y) x##y
#define MBASE_PP_CONCAT(x, y) MBASE_PP_CONCAT_IMPL(x, y)
