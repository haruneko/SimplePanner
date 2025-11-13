cmake_minimum_required(VERSION 3.14.0)

project(SimplePanner VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ビルドタイプが指定されていない場合はReleaseに設定
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# ビルドタイプに応じて適切なマクロを定義
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(_DEBUG=1)
else()
    add_compile_definitions(NDEBUG=1 RELEASE=1)
endif()

# VST3 SDK のパスを設定 (環境変数またはオプションで指定)
set(VST3_SDK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vst3sdk" CACHE PATH "Path to VST3 SDK")

if(NOT EXISTS ${VST3_SDK_ROOT})
    message(STATUS "VST3 SDK not found at ${VST3_SDK_ROOT}")
    message(STATUS "Cloning VST3 SDK...")
    find_package(Git REQUIRED)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} clone --recursive https://github.com/steinbergmedia/vst3sdk.git ${VST3_SDK_ROOT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()

# VST3 SDK のオプションを設定（サンプルやホスティングをビルドしない）
set(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES OFF CACHE BOOL "")
set(SMTG_ENABLE_VST3_HOSTING_EXAMPLES OFF CACHE BOOL "")
set(SMTG_ENABLE_VSTGUI_SUPPORT OFF CACHE BOOL "")

# VST3 SDK を追加
add_subdirectory(${VST3_SDK_ROOT} ${CMAKE_CURRENT_BINARY_DIR}/vst3sdk)

# プラグインのソースファイル
set(PLUGIN_SOURCES
    source/pluginprocessor.cpp
    source/plugincontroller.cpp
    source/pluginfactory.cpp
)

set(PLUGIN_HEADERS
    include/pluginprocessor.h
    include/plugincontroller.h
    include/plugids.h
)

# VST3プラグインターゲットを作成
smtg_add_vst3plugin(SimplePanner
    ${PLUGIN_SOURCES}
    ${PLUGIN_HEADERS}
)

# プラットフォーム固有のエントリポイントを追加
if(SMTG_LINUX)
    target_sources(SimplePanner PRIVATE
        ${VST3_SDK_ROOT}/public.sdk/source/main/linuxmain.cpp
    )
elseif(SMTG_WIN)
    target_sources(SimplePanner PRIVATE
        ${VST3_SDK_ROOT}/public.sdk/source/main/dllmain.cpp
    )
elseif(SMTG_MAC)
    target_sources(SimplePanner PRIVATE
        ${VST3_SDK_ROOT}/public.sdk/source/main/macmain.cpp
    )
endif()

# インクルードディレクトリを追加
target_include_directories(SimplePanner PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${VST3_SDK_ROOT}
)

# VST3 SDKのライブラリをリンク
target_link_libraries(SimplePanner PRIVATE
    sdk
)

# プラグインの情報を設定
smtg_target_configure_version_file(SimplePanner)

if(SMTG_MAC)
    smtg_target_set_bundle(SimplePanner
        BUNDLE_IDENTIFIER "com.example.simplepanner"
        COMPANY_NAME "Example Company"
    )
elseif(SMTG_WIN)
    target_sources(SimplePanner PRIVATE resource/plugin.rc)
endif()

#------------------------------------------------------------------------
# Testing Setup
#------------------------------------------------------------------------
enable_testing()

# Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/release-1.12.1.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Helper function to add unit tests
function(add_simple_panner_test test_name test_file)
    add_executable(${test_name} ${test_file})
    target_link_libraries(${test_name} PRIVATE
        gtest_main
        pluginterfaces
        base
    )
    target_include_directories(${test_name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${VST3_SDK_ROOT}
    )

    # macOS specific: Link CoreFoundation framework for FUID::generate()
    if(APPLE)
        target_link_libraries(${test_name} PRIVATE
            "-framework CoreFoundation"
        )
    endif()

    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

#------------------------------------------------------------------------
# Unit Tests
#------------------------------------------------------------------------
add_simple_panner_test(test_parameter_conversion
    tests/unit/test_parameter_conversion.cpp
)
