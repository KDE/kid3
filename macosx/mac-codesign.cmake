if(NOT DEFINED ENV{SIGNING_IDENTITY})
  message(WARNING "Environment variable SIGNING_IDENTITY not set, not signing")
  return()
endif()
set(SIGNING_IDENTITY $ENV{SIGNING_IDENTITY})
set(CODESIGN codesign --force --sign "${SIGNING_IDENTITY}" --options=runtime)
if(CMAKE_SCRIPT_MODE_FILE AND NOT CMAKE_PARENT_LIST_FILE)
  set(_pathToParent "${CMAKE_CURRENT_SOURCE_DIR}/./")
else()
  set(_pathToParent "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/./")
endif()
set(_bundleName "kid3.app")
set(_pathToBundle "${_pathToParent}/${_bundleName}")

execute_process(
  COMMAND xcrun -f codesign_allocate
  OUTPUT_STRIP_TRAILING_WHITESPACE
  OUTPUT_VARIABLE CODESIGN_ALLOCATE
)

execute_process(
  COMMAND find "${_bundleName}/Contents/PlugIns" -type f -d -print
  WORKING_DIRECTORY "${_pathToParent}"
  OUTPUT_VARIABLE _pluginFilesToSign
)

execute_process(
  COMMAND find "${_bundleName}" \( -name "*.framework" -or -name "*.dylib" \) -d -print
  WORKING_DIRECTORY "${_pathToParent}"
  OUTPUT_VARIABLE _libFilesToSign
)

string(STRIP ${_pluginFilesToSign} _pluginFilesToSign)
string(STRIP ${_libFilesToSign} _libFilesToSign)
string(REPLACE "\n" ";" _pluginFilesToSign ${_pluginFilesToSign})
string(REPLACE "\n" ";" _libFilesToSign ${_libFilesToSign})
set(_filesToSign ${_pluginFilesToSign} ${_libFilesToSign})
list(APPEND _filesToSign
  "${_bundleName}/Contents/MacOS/kid3"
  "${_bundleName}/Contents/MacOS/kid3-cli"
  "${_bundleName}"
)
list(REMOVE_DUPLICATES _filesToSign)

message(STATUS "Signing ${_bundleName}")
foreach(_fileToSign ${_filesToSign})
  execute_process(
    COMMAND cmake -E env CODESIGN_ALLOCATE=${CODESIGN_ALLOCATE} ${CODESIGN} "${_fileToSign}"
    WORKING_DIRECTORY "${_pathToParent}"
    RESULT_VARIABLE _result
  )
  if(NOT (${_result} EQUAL 0))
    message(WARNING "Could not sign file '${_fileToSign}'")
  endif()
endforeach()

execute_process(
  COMMAND codesign --verify "${_pathToParent}/${_bundleName}"
  RESULT_VARIABLE _result
)
if(NOT ${_result} EQUAL 0)
  message(FATAL_ERROR "Signature verification failed")
endif()

execute_process(
  COMMAND spctl --assess --type execute "${_pathToParent}/${_bundleName}"
  RESULT_VARIABLE _result
)
if(NOT ${_result} EQUAL 0)
  message(WARNING "Signature assessment failed")
endif()
