#
# Copyright (C) 2026 David Hampton
#
# See the file LICENSE_FSF for licensing information.
#
if(NOT CMAKE_CXX_COMPILER_ID)
  return()
endif()

include(CMakePushCheckState)

if(ENABLE_GCOV AND ENABLE_LLVM_COV)
  message(FATAL_ERROR "Can't enable both gcov and llvm-cov.")
  return()
endif()

if(ENABLE_GCOV)
  # This works with both gcc and clang, though llvm_cov is recommended
  # when using clang.
  cmake_push_check_state()
  set(CMAKE_REQUIRED_LIBRARIES gcov)
  check_cxx_compiler_flag("-fprofile-arcs" HAVE_CXX_fprofile-arcs)
  cmake_pop_check_state()
  check_cxx_compiler_flag("-ftest-coverage" HAVE_CXX_ftest-coverage)
  check_linker_flag(CXX "--coverage" HAVE_LINKER_coverage)
  if (HAVE_CXX_fprofile-arcs AND HAVE_CXX_ftest-coverage AND HAVE_LINKER_coverage)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} --coverage")
  else()
    message(WARNING "Disabling gcov. Missing compiler flag. (See the three previous tests.)")
  endif()
  return()
endif()

if(ENABLE_LLVM_COV)
  if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(WARNING "Disabling llvm-cov. Not compiling with the clang compiler.")
    return()
  endif()
  check_cxx_compiler_flag("-fprofile-instr-generate" HAVE_CXX_fprofile_instr_generate)
  check_cxx_compiler_flag("-fprofile-instr-generate -fcoverage-mapping" HAVE_CXX_fcoverage_mapping)
  check_linker_flag(CXX "--coverage" HAVE_LINKER_coverage)
  if (HAVE_CXX_fprofile_instr_generate AND HAVE_CXX_fcoverage_mapping AND HAVE_LINKER_coverage)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping -O0 -g")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fprofile-instr-generate --coverage -O0 -g")
  else()
    message(WARNING "Disabling llvm-cov. Missing compiler flag. (See the three previous tests.)")
  endif()
endif()
