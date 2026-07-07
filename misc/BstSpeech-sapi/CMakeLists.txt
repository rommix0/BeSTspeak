cmake_minimum_required(VERSION 3.15)
project(BestspeechSAPI VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(BUILD_X64 TRUE)
    message(STATUS "Building x64 version")
else()
    set(BUILD_X64 FALSE)
    message(STATUS "Building x86 version")
endif()

if(NOT BUILD_X64)
    include(FetchContent)

    FetchContent_Declare(
        sonic
        GIT_REPOSITORY https://github.com/waywardgeek/sonic.git
        GIT_TAG master
    )
    FetchContent_MakeAvailable(sonic)

    add_library(sonic STATIC
        ${sonic_SOURCE_DIR}/sonic.c
    )
    target_include_directories(sonic PUBLIC ${sonic_SOURCE_DIR})

    FetchContent_Declare(
        minhook
        GIT_REPOSITORY https://github.com/TsudaKageyu/minhook.git
        GIT_TAG master
    )
    FetchContent_GetProperties(minhook)
    if(NOT minhook_POPULATED)
        FetchContent_Populate(minhook)
        add_subdirectory(${minhook_SOURCE_DIR} ${minhook_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()

    if(MSVC)
        set_property(TARGET sonic PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        set_property(TARGET minhook PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()

    add_library(b32_wrapper STATIC
        src/b32_wrapper.cpp
    )

    target_include_directories(b32_wrapper PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(b32_wrapper PUBLIC
        winmm
        sonic
        minhook
    )

    target_compile_definitions(b32_wrapper PRIVATE
        _CRT_SECURE_NO_WARNINGS
        UNICODE
        _UNICODE
    )

    add_library(BestspeechSAPI SHARED
        src/sapi_main.cpp
        src/com.cpp
        src/registry.cpp
        src/ISpDataKeyImpl.cpp
        src/IEnumSpObjectTokensImpl.cpp
        src/ISpTTSEngineImpl.cpp
        src/voice_token.cpp
        src/bestspeech_sapi.def
    )

    target_include_directories(BestspeechSAPI PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(BestspeechSAPI PRIVATE
        b32_wrapper
        ole32
        oleaut32
        advapi32
        shell32
    )

    target_compile_definitions(BestspeechSAPI PRIVATE
        _CRT_SECURE_NO_WARNINGS
        UNICODE
        _UNICODE
    )

    add_executable(BestspeechServer
        src/bestspeech_server.cpp
    )

    target_include_directories(BestspeechServer PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(BestspeechServer PRIVATE
        b32_wrapper
    )

    target_compile_definitions(BestspeechServer PRIVATE
        _CRT_SECURE_NO_WARNINGS
        UNICODE
        _UNICODE
    )

    if(MSVC)
        target_compile_options(b32_wrapper PRIVATE /W3 /MP)
        target_compile_options(BestspeechSAPI PRIVATE /W3 /MP)
        target_compile_options(BestspeechServer PRIVATE /W3 /MP)

        set_property(TARGET b32_wrapper PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        set_property(TARGET BestspeechSAPI PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        set_property(TARGET BestspeechServer PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()

    add_custom_command(TARGET BestspeechSAPI POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/bin/b32_tts.dll"
            "$<TARGET_FILE_DIR:BestspeechSAPI>/b32_tts.dll"
        COMMENT "Copying b32_tts.dll to output directory"
    )

    install(TARGETS BestspeechSAPI BestspeechServer
        RUNTIME DESTINATION .
        LIBRARY DESTINATION .
    )

    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/bin/b32_tts.dll"
        DESTINATION .
    )

    install(FILES "${CMAKE_BINARY_DIR}/bin/Release/x64/BestspeechSAPI.dll"
        DESTINATION x64
        OPTIONAL
    )

else()
    add_library(pipe_client STATIC
        src/pipe_client.cpp
    )

    target_include_directories(pipe_client PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(pipe_client PUBLIC
        shlwapi
    )

    target_compile_definitions(pipe_client PRIVATE
        _CRT_SECURE_NO_WARNINGS
        UNICODE
        _UNICODE
    )

    add_library(BestspeechSAPI SHARED
        src/sapi_main.cpp
        src/com.cpp
        src/registry.cpp
        src/ISpDataKeyImpl.cpp
        src/IEnumSpObjectTokensImpl.cpp
        src/ISpTTSEngineImpl.cpp
        src/voice_token.cpp
        src/bestspeech_sapi.def
    )

    target_include_directories(BestspeechSAPI PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(BestspeechSAPI PRIVATE
        pipe_client
        ole32
        oleaut32
        advapi32
        shell32
    )

    target_compile_definitions(BestspeechSAPI PRIVATE
        _CRT_SECURE_NO_WARNINGS
        UNICODE
        _UNICODE
        BUILD_X64
    )

    if(MSVC)
        target_compile_options(pipe_client PRIVATE /W3 /MP)
        target_compile_options(BestspeechSAPI PRIVATE /W3 /MP)

        set_property(TARGET pipe_client PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        set_property(TARGET BestspeechSAPI PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()

    install(TARGETS BestspeechSAPI
        RUNTIME DESTINATION .
        LIBRARY DESTINATION .
    )
endif()

set(CPACK_GENERATOR "INNOSETUP")
set(CPACK_PACKAGE_NAME "Bestspeech SAPI")
set(CPACK_PACKAGE_VENDOR "Gozaltech")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Bestspeech Text-to-Speech SAPI5 Engine")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Bestspeech")
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "BestspeechSAPI")
set(CPACK_PACKAGE_FILE_NAME "BestspeechSAPI_Setup")

set(CPACK_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/installer")
set(CPACK_INNOSETUP_USE_CMAKE_BOOL_FORMAT ON)

set(CPACK_INNOSETUP_SETUP_AppPublisherURL "http://gozaltech.org")
set(CPACK_INNOSETUP_SETUP_DisableWelcomePage "yes")
set(CPACK_INNOSETUP_SETUP_DisableDirPage "yes")
set(CPACK_INNOSETUP_SETUP_DisableReadyPage "yes")
set(CPACK_INNOSETUP_SETUP_DisableProgramGroupPage "yes")

include(CPack)
