cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0026 OLD) # allow use of the LOCATION target property

# store the current source directory for future use
set(QT_ANDROID_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR})

# check the JAVA_HOME environment variable
if(JAVA_HOME)
    # Set JAVA_HOME environment variable.
    set(ENV{JAVA_HOME} ${JAVA_HOME})
else()
    set(JAVA_HOME $ENV{JAVA_HOME})
    if(NOT JAVA_HOME)
        message(FATAL_ERROR "The JAVA_HOME environment variable is not set. Please set it or the JAVA_HOME CMake variable to the root directory of the JDK.")
    endif()
endif()

# make sure that the Android toolchain is used
if(NOT ANDROID)
    message(FATAL_ERROR "Trying to use the CMake Android package without the Android toolchain. Please use the provided toolchain (toolchain/android.toolchain.cmake)")
endif()

# find the Qt root directory
if(NOT Qt${QT_VERSION_MAJOR}Core_DIR)
    find_package(Qt${QT_VERSION_MAJOR}Core REQUIRED)
endif()
get_filename_component(QT_ANDROID_QT_ROOT "${Qt${QT_VERSION_MAJOR}Core_DIR}/../../.." ABSOLUTE)
message(STATUS "Found Qt for Android: ${QT_ANDROID_QT_ROOT}")

# find the Android SDK
if(NOT QT_ANDROID_SDK_ROOT)
    set(QT_ANDROID_SDK_ROOT $ENV{ANDROID_SDK})
    if(NOT QT_ANDROID_SDK_ROOT)
        message(FATAL_ERROR "Could not find the Android SDK. Please set either the ANDROID_SDK environment variable, or the QT_ANDROID_SDK_ROOT CMake variable to the root directory of the Android SDK")
    endif()
endif()
string(REPLACE "\\" "/" QT_ANDROID_SDK_ROOT ${QT_ANDROID_SDK_ROOT}) # androiddeployqt doesn't like backslashes in paths
message(STATUS "Found Android SDK: ${QT_ANDROID_SDK_ROOT}")

# find the Android NDK
if(NOT QT_ANDROID_NDK_ROOT)
    set(QT_ANDROID_NDK_ROOT $ENV{ANDROID_NDK})
    if(NOT QT_ANDROID_NDK_ROOT)
        set(QT_ANDROID_NDK_ROOT ${ANDROID_NDK})
        if(NOT QT_ANDROID_NDK_ROOT)
        message(FATAL_ERROR "Could not find the Android NDK. Please set either the ANDROID_NDK environment or CMake variable, or the QT_ANDROID_NDK_ROOT CMake variable to the root directory of the Android NDK")
        endif()
    endif()
endif()
string(REPLACE "\\" "/" QT_ANDROID_NDK_ROOT ${QT_ANDROID_NDK_ROOT}) # androiddeployqt doesn't like backslashes in paths
message(STATUS "Found Android NDK: ${QT_ANDROID_NDK_ROOT}")

option(APK_ALL_TARGET "generate apk package with target 'all'" ON)
if(APK_ALL_TARGET)
    SET(APK_ALL ALL)
endif()

include(CMakeParseArguments)

# define a macro to create an Android APK target
#
# example:
# add_qt_android_apk(my_app_apk my_app
#     NAME "My App"
#     VERSION_CODE 12
#     PACKAGE_NAME "org.mycompany.myapp"
#     PACKAGE_SOURCES ${CMAKE_CURRENT_LIST_DIR}/my-android-sources
#     KEYSTORE ${CMAKE_CURRENT_LIST_DIR}/mykey.keystore myalias
#     KEYSTORE_PASSWORD xxxx
#     DEPENDS a_linked_target "path/to/a_linked_library.so" ...
#     INSTALL
#)
#
macro(add_qt_android_apk TARGET SOURCE_TARGET)

    # parse the macro arguments
    cmake_parse_arguments(ARG "INSTALL" "NAME;VERSION_CODE;PACKAGE_NAME;PACKAGE_SOURCES;KEYSTORE_PASSWORD" "DEPENDS;KEYSTORE;DEPLOYMENT_DEPENDS" ${ARGN})

    # extract the full path of the source target binary
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        get_property(QT_ANDROID_APP_PATH TARGET ${SOURCE_TARGET} PROPERTY DEBUG_LOCATION)
    else()
        get_property(QT_ANDROID_APP_PATH TARGET ${SOURCE_TARGET} PROPERTY LOCATION)
    endif()

    # define the application name
    if(ARG_NAME)
        set(QT_ANDROID_APP_NAME ${ARG_NAME})
    else()
        set(QT_ANDROID_APP_NAME ${SOURCE_TARGET})
    endif()

    # define the application package name
    if(ARG_PACKAGE_NAME)
        set(QT_ANDROID_APP_PACKAGE_NAME ${ARG_PACKAGE_NAME})
    else()
        set(QT_ANDROID_APP_PACKAGE_NAME org.qtproject.${SOURCE_TARGET})
    endif()

    # set sdkBuildToolsRevision in qtdeploy.json
    if(QT_ANDROID_BUILD_TOOLS_REVISION)
        set(QT_ANDROID_SDK_BUILDTOOLS_REVISION ${QT_ANDROID_BUILD_TOOLS_REVISION})
    else()
        # detect latest Android SDK build-tools revision
        set(QT_ANDROID_SDK_BUILDTOOLS_REVISION "0.0.0")
        file(GLOB ALL_BUILD_TOOLS_VERSIONS RELATIVE ${QT_ANDROID_SDK_ROOT}/build-tools ${QT_ANDROID_SDK_ROOT}/build-tools/*)
        foreach(BUILD_TOOLS_VERSION ${ALL_BUILD_TOOLS_VERSIONS})
            # find subfolder with greatest version
            if (${BUILD_TOOLS_VERSION} VERSION_GREATER ${QT_ANDROID_SDK_BUILDTOOLS_REVISION})
                set(QT_ANDROID_SDK_BUILDTOOLS_REVISION ${BUILD_TOOLS_VERSION})
            endif()
        endforeach()
        message("Detected Android SDK build tools version ${QT_ANDROID_SDK_BUILDTOOLS_REVISION}")
    endif()

    # define the application source package directory
    if(ARG_PACKAGE_SOURCES)
        set(QT_ANDROID_APP_PACKAGE_SOURCE_ROOT ${ARG_PACKAGE_SOURCES})
    else()
        # get version code from arguments, or generate a fixed one if not provided
        if(NOT QT_ANDROID_APP_VERSION_CODE)
            set(QT_ANDROID_APP_VERSION_CODE ${ARG_VERSION_CODE})
        endif()
        if(NOT QT_ANDROID_APP_VERSION_CODE)
            set(QT_ANDROID_APP_VERSION_CODE 1)
        endif()

        # try to extract the app version from the target properties, or use the version code if not provided
        get_property(QT_ANDROID_APP_VERSION TARGET ${SOURCE_TARGET} PROPERTY VERSION)
        if(NOT QT_ANDROID_APP_VERSION)
            set(QT_ANDROID_APP_VERSION ${QT_ANDROID_APP_VERSION_CODE})
        endif()

        # create a subdirectory for the extra package sources
        set(QT_ANDROID_APP_PACKAGE_SOURCE_ROOT "${CMAKE_CURRENT_BINARY_DIR}/package")

        # generate a manifest from the template
        configure_file(${QT_ANDROID_SOURCE_DIR}/AndroidManifest.xml.in ${QT_ANDROID_APP_PACKAGE_SOURCE_ROOT}/AndroidManifest.xml @ONLY)
    endif()

    # define the STL shared library path
    if(ANDROID_STL_SHARED_LIBRARIES)
        list(GET ANDROID_STL_SHARED_LIBRARIES 0 STL_LIBRARY_NAME) # we can only give one to androiddeployqt
        if(ANDROID_STL_PATH)
            set(QT_ANDROID_STL_PATH "${ANDROID_STL_PATH}/libs/${ANDROID_ABI}/lib${STL_LIBRARY_NAME}.so")
        else()
            set(QT_ANDROID_STL_PATH "${ANDROID_NDK}/sources/cxx-stl/${ANDROID_STL_PREFIX}/libs/${ANDROID_ABI}/lib${STL_LIBRARY_NAME}.so")
        endif()
    else()
        set(QT_ANDROID_STL_PATH "${ANDROID_NDK}/sources/cxx-stl/gnu-libstdc++/4.9/libs/${ANDROID_ABI}/libgnustl_shared.so")
        if(NOT EXISTS "${QT_ANDROID_STL_PATH}")
            set(QT_ANDROID_STL_PATH "${ANDROID_NDK}/sources/cxx-stl/llvm-libc++/libs/${ANDROID_ABI}/libc++_shared.so")
        endif()
        if(NOT EXISTS "${QT_ANDROID_STL_PATH}")
            set(QT_ANDROID_STL_PATH)
        endif()
    endif()

    # set the list of dependent libraries
    if(ARG_DEPENDS)
        foreach(LIB ${ARG_DEPENDS})
            if(TARGET ${LIB})
                # item is a CMake target, extract the library path
                if(CMAKE_BUILD_TYPE STREQUAL "Debug")
                    get_property(LIB_PATH TARGET ${LIB} PROPERTY DEBUG_LOCATION)
                else()
                    get_property(LIB_PATH TARGET ${LIB} PROPERTY LOCATION)
                endif()
                set(LIB ${LIB_PATH})
            endif()
        if(EXTRA_LIBS)
            set(EXTRA_LIBS "${EXTRA_LIBS},${LIB}")
        else()
            set(EXTRA_LIBS "${LIB}")
        endif()
        endforeach()
        set(QT_ANDROID_APP_EXTRA_LIBS "\"android-extra-libs\": \"${EXTRA_LIBS}\",")
    endif()

    # set some toolchain variables used by androiddeployqt;
    # unfortunately, Qt tries to build paths from these variables although these full paths
    # are already available in the toochain file, so we have to parse them
    string(REGEX MATCH "${ANDROID_NDK}/toolchains/(.*)-(.*)/prebuilt/.*" ANDROID_TOOLCHAIN_PARSED ${ANDROID_TOOLCHAIN_ROOT})
    if(ANDROID_TOOLCHAIN_PARSED)
        set(QT_ANDROID_TOOLCHAIN_PREFIX ${CMAKE_MATCH_1})
        set(QT_ANDROID_TOOLCHAIN_VERSION ${CMAKE_MATCH_2})
    else()
        string(REGEX MATCH "${ANDROID_NDK}/toolchains/([^/]*)/prebuilt/([^/]*)" ANDROID_TOOLCHAIN_PARSED ${ANDROID_TOOLCHAIN_ROOT})
        if(ANDROID_TOOLCHAIN_PARSED)
            set(QT_ANDROID_TOOLCHAIN_PREFIX ${CMAKE_MATCH_1})
        else()
            message(FATAL_ERROR "Failed to parse ANDROID_TOOLCHAIN_ROOT to get toolchain prefix and version")
        endif()
    endif()
    if(QT_ANDROID_TOOLCHAIN_PREFIX STREQUAL "llvm")
        set(QT_ANDROID_USE_LLVM "true")
    else()
        set(QT_ANDROID_USE_LLVM "false")
    endif()
    if(QT_ANDROID_TOOLCHAIN_PREFIX STREQUAL "x86")
        set(QT_ANDROID_TOOL_PREFIX "i686-linux-android")
    else()
        set(QT_ANDROID_TOOL_PREFIX ${QT_ANDROID_TOOLCHAIN_PREFIX})
    endif()

    # set the list of deployment-dependencies
    if(ARG_DEPLOYMENT_DEPENDS)
        set(DEPLOYMENT_DEPENDENCIES)
        foreach(DEP ${ARG_DEPLOYMENT_DEPENDS})
            if(DEPLOYMENT_DEPENDENCIES)
                set(DEPLOYMENT_DEPENDENCIES "${DEPLOYMENT_DEPENDENCIES},${DEP}")
            else()
                set(DEPLOYMENT_DEPENDENCIES "${DEP}")
            endif()
        endforeach()
        if(QT_ANDROID_APP_EXTRA_LIBS)
            set(QT_ANDROID_APP_EXTRA_LIBS "${QT_ANDROID_APP_EXTRA_LIBS}\n ")
        endif()
        set(QT_ANDROID_APP_EXTRA_LIBS "${QT_ANDROID_APP_EXTRA_LIBS}\"deployment-dependencies\": \"${DEPLOYMENT_DEPENDENCIES}\",")
    endif()

    # make sure that the output directory for the Android package exists
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/libs/${ANDROID_ABI})

    # create the configuration file that will feed androiddeployqt
    configure_file(${QT_ANDROID_SOURCE_DIR}/qtdeploy.json.in ${CMAKE_CURRENT_BINARY_DIR}/qtdeploy.json @ONLY)

    # check if the apk must be signed
    if(ARG_KEYSTORE)
        if(ARG_KEYSTORE STREQUAL "UNSIGNED_RELEASE")
            set(SIGN_OPTIONS --release)
        else()
            set(SIGN_OPTIONS --release --sign ${ARG_KEYSTORE} --tsa http://timestamp.digicert.com)
            if(ARG_KEYSTORE_PASSWORD)
                set(SIGN_OPTIONS ${SIGN_OPTIONS} --storepass ${ARG_KEYSTORE_PASSWORD})
            endif()
        endif()
    endif()

    # check if the apk must be installed to the device
    if(ARG_INSTALL)
        set(INSTALL_OPTIONS --reinstall)
    endif()

    # specify the Android API level
    if(ANDROID_PLATFORM_LEVEL)
        set(TARGET_LEVEL_OPTIONS --android-platform android-${ANDROID_PLATFORM_LEVEL})
    endif()

    # create a custom command that will run the androiddeployqt utility to prepare the Android package
    add_custom_target(
        ${TARGET}
        ALL
        DEPENDS ${SOURCE_TARGET}
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/libs/${ANDROID_ABI} # it seems that recompiled libraries are not copied if we don't remove them first
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/libs/${ANDROID_ABI}
        COMMAND ${CMAKE_COMMAND} -E copy ${QT_ANDROID_APP_PATH} ${CMAKE_CURRENT_BINARY_DIR}/libs/${ANDROID_ABI}
        COMMAND ${QT_ANDROID_QT_ROOT}/bin/androiddeployqt --verbose --output ${CMAKE_CURRENT_BINARY_DIR} --input ${CMAKE_CURRENT_BINARY_DIR}/qtdeploy.json --gradle ${TARGET_LEVEL_OPTIONS} ${INSTALL_OPTIONS} ${SIGN_OPTIONS}
    )

endmacro()
