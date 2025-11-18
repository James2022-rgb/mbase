cmake_minimum_required(VERSION 3.22)

# --------------------------------------------------------------------------------
# Internal variables
#

set(TARGET_NAME "mbase")
set(SRC_DIR "src")
set(NATVIS_DIR "natvis")

# --------------------------------------------------------------------------------
# Project
#

project(${TARGET_NAME}
  VERSION 0.1.0.0
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=thread-safety")
endif()

if (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")
endif()

add_library(${TARGET_NAME} STATIC)
target_compile_features(${TARGET_NAME} PRIVATE cxx_std_23)

if(MSVC)
  target_compile_options(${TARGET_NAME} PRIVATE /W4)
else()
  target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -Werror)
endif()

# --------------------------------------------------------------------------------
# External libraries
#

#
# thirdpaty/source_location
#

target_include_directories(${TARGET_NAME}
  PUBLIC "thirdparty/source_location/include"
)

#
# thirdparty/spdlog
#

target_include_directories(${TARGET_NAME}
  PUBLIC "thirdparty/spdlog/include"
)

#
# thirdparty/xxHash
#

# We don't need to build the xxhsum binary.
option(XXHASH_BUILD_XXHSUM "Build the xxhsum binary" OFF)

add_subdirectory("thirdparty/xxHash/build/cmake" xxhash_build EXCLUDE_FROM_ALL)

set_target_properties(xxhash PROPERTIES FOLDER "external")

#
# thirdparty/cereal
#

set(THIRDPARTY_CEREAL_DIR "thirdparty/cereal")

set(BUILD_SANDBOX OFF)
set(SKIP_PERFORMANCE_COMPARISON ON)
add_subdirectory(${THIRDPARTY_CEREAL_DIR})

# --------------------------------------------------------------------------------
# Project library
#

#
# sources files
#

set(SOURCES_PUBLIC_DIR ${SRC_DIR}/mbase/public)

set(SOURCES_PUBLIC_ALGORITHM
  ${SOURCES_PUBLIC_DIR}/algorithm/my_ostream_joiner.h
)
source_group("Public/Algorithm" FILES ${SOURCES_PUBLIC_ALGORITHM})

set(SOURCES_PUBLIC_COM
  ${SOURCES_PUBLIC_DIR}/com/com.h
  ${SOURCES_PUBLIC_DIR}/com/com-helper.h
  ${SOURCES_PUBLIC_DIR}/com/com-refcount_template.h
  ${SOURCES_PUBLIC_DIR}/com/com-weak.h
)
source_group("Public/Com" FILES ${SOURCES_PUBLIC_COM})

set(SOURCES_PUBLIC_LOG
  ${SOURCES_PUBLIC_DIR}/log/log.h
)
source_group("Public/Log" FILES ${SOURCES_PUBLIC_LOG})

set(SOURCES_PUBLIC_MATH
  ${SOURCES_PUBLIC_DIR}/math/arithmetic.h
)
source_group("Public/Math" FILES ${SOURCES_PUBLIC_MATH})

set(SOURCES_PUBLIC_PLATFORM
  ${SOURCES_PUBLIC_DIR}/platform/platform.h
)
source_group("Public/Platform" FILES ${SOURCES_PUBLIC_PLATFORM})

set(SOURCES_PUBLIC_PROFILING
  ${SOURCES_PUBLIC_DIR}/profiling/scoped_timer.h
)
source_group("Public/Profiling" FILES ${SOURCES_PUBLIC_PROFILING})

set(SOURCES_PUBLIC_ROOT
  ${SOURCES_PUBLIC_DIR}/access.h
  ${SOURCES_PUBLIC_DIR}/array_proxy.h
  ${SOURCES_PUBLIC_DIR}/assert.h
  ${SOURCES_PUBLIC_DIR}/bitflags.h
  ${SOURCES_PUBLIC_DIR}/container.h
  ${SOURCES_PUBLIC_DIR}/hash.h
  ${SOURCES_PUBLIC_DIR}/format.h
  ${SOURCES_PUBLIC_DIR}/log.h
  ${SOURCES_PUBLIC_DIR}/memory.h
  ${SOURCES_PUBLIC_DIR}/platform.h
  ${SOURCES_PUBLIC_DIR}/profiling.h
  ${SOURCES_PUBLIC_DIR}/pp.h
  ${SOURCES_PUBLIC_DIR}/trap.h
  ${SOURCES_PUBLIC_DIR}/tsa.h
  ${SOURCES_PUBLIC_DIR}/type_safety.h
  ${SOURCES_PUBLIC_DIR}/type_util.h
)
source_group("Public" FILES ${SOURCES_PUBLIC_ROOT})

set(SOURCES_PRIVATE_DIR ${SRC_DIR}/mbase/private)

set(SOURCE_PRIVATE_LOG_DIR ${SOURCES_PRIVATE_DIR}/log)
set(SOURCES_PRIVATE_LOG
  ${SOURCE_PRIVATE_LOG_DIR}/log.cpp
)
source_group("Private/Log" FILES ${SOURCES_PRIVATE_LOG})

set(SOURCES_PRIVATE_PROFILING_DIR ${SOURCES_PRIVATE_DIR}/profiling)
set(SOURCES_PRIVATE_PROFILING
  ${SOURCES_PRIVATE_PROFILING_DIR}/scoped_timer.cpp
)
source_group("Private/Profiling" FILES ${SOURCES_PRIVATE_PROFILING})

set(SOURCES_PRIVATE_ROOT
  ${SOURCES_PRIVATE_DIR}/assert.cpp
  ${SOURCES_PRIVATE_DIR}/format.cpp
  ${SOURCES_PRIVATE_DIR}/hash.cpp
  ${SOURCES_PRIVATE_DIR}/memory.cpp
  ${SOURCES_PRIVATE_DIR}/trap.cpp
)
source_group("Private" FILES ${SOURCES_PRIVATE_ROOT})

#
# natvis files
#

set(NATVIS_FILES
  ${NATVIS_DIR}/container.natvis
)
source_group("Natvis Files" FILES ${NATVIS_FILES})

#
#
#

set(SOURCES
  ${SOURCES_PUBLIC_ALGORITHM}
  ${SOURCES_PUBLIC_COM}
  ${SOURCES_PUBLIC_LOG}
  ${SOURCES_PUBLIC_MATH}
  ${SOURCES_PUBLIC_PLATFORM}
  ${SOURCES_PUBLIC_PROFILING}
  ${SOURCES_PUBLIC_ROOT}
  ${SOURCES_PRIVATE_LOG}
  ${SOURCES_PRIVATE_PROFILING}
  ${SOURCES_PRIVATE_ROOT}
  ${NATVIS_FILES}
)

target_sources(${TARGET_NAME} PRIVATE ${SOURCES})

target_include_directories(${TARGET_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${SRC_DIR}/mbase/private
)
target_include_directories(${TARGET_NAME} PUBLIC
  ${SRC_DIR}
)

target_link_libraries(${TARGET_NAME} PUBLIC
  cereal::cereal
)

target_link_libraries(${TARGET_NAME} PRIVATE
  xxHash::xxhash
)
