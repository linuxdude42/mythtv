# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindPHPModules
--------

Find the listed PHP modules

Result variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

``PHPModules_FOUND``
  true if the PHPModules headers and libraries were found
``PHPModules_COMPONENTS_FOUND``
  A list of of all the modules that were found
``PHPModules_COMPONENTS_FOUND``
  A list of of all the modules that were found
``PHPModules_COMPONENTS_MISSING``
  A list of of all the modules that were not found (required or optional)
``PHPModules_COMPONENTS_REQUIRED_MISSING``
  A list of of all the required modules that were not found
``PHPModules_COMPONENTS_OPTIONAL_MISSING``
  A list of of all the optional modules that were not found

#]=======================================================================]

if(NOT PHP_EXECUTABLE)
  find_program(PHP_EXECUTABLE NAMES php)
  if(NOT PHP_EXECUTABLE)
    return()
  endif()
endif()

execute_process(
  COMMAND ${PHP_EXECUTABLE} -m RESULTS_VARIABLE _result
  OUTPUT_VARIABLE _output
  ERROR_VARIABLE _error
  ERROR_STRIP_TRAILING_WHITESPACE)
message(DEBUG "_result is ${_result}")
message(DEBUG "_output is ${_output}")
message(DEBUG "_error is ${_error}")
if(NOT _result EQUAL 0)
  return()
endif()

foreach(_module IN LISTS PHPModules_FIND_COMPONENTS)
  message(DEBUG "Checking module ${_module}")
  if("${_output}" MATCHES "\n${_module}\n.*")
    execute_process(COMMAND ${PHP_EXECUTABLE} --re ${_module} RESULTS_VARIABLE
                            _ext_result OUTPUT_VARIABLE _ext_output)
    if("${_ext_output}" MATCHES
       "Extension \\\[.*version[^0-9]*([0-9\.]+) \\\]")
      set(PHPModules_${module}_VERSION ${CMAKE_MATCH_1})
      set(_version_str "version ${CMAKE_MATCH_1}")
    endif()
    if(PHPModules_FIND_REQUIRED_${_module})
      message(STATUS "  Found required php module ${_module} (${_version_str})")
    else()
      message(STATUS "  Found optional php module ${_module} (${_version_str})")
    endif()
    set(PHPModules_${module}_FOUND TRUE)
    list(APPEND PHPModules_COMPONENTS_FOUND _{module})
  else()
    list(APPEND PHPModules_COMPONENTS_MISSING _{module})
    if(PHPModules_FIND_REQUIRED_${_module})
      list(APPEND PHPModules_COMPONENTS_REQUIRED_MISSING _{module})
      message(STATUS "  Missing required php module ${_module}")
    else()
      list(APPEND PHPModules_COMPONENTS_OPTIONAL_MISSING _{module})
      message(STATUS "  Missing optional php module ${_module}")
    endif()
  endif()
endforeach()

if(NOT PHPModules_COMPONENTS_MISSING)
  set(PHPModules_FOUND TRUE)
endif()

mark_as_advanced(
  PHPModules_FOUND PHPModules_COMPONENTS_FOUND PHPModules_COMPONENTS_MISSING
  PHPModules_COMPONENTS_REQUIRED_MISSING PHPModules_COMPONENTS_OPTIONAL_MISSING)
