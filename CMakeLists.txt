cmake_minimum_required(VERSION 3.14.0)

project(SimplePanner)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
smtg_add_vst3plugin(SimplePanner ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})

# インクルードディレクトリを追加
target_include_directories(SimplePanner PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
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
