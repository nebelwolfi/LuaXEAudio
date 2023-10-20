cmake_minimum_required(VERSION 3.26)
project(LuaSoLoudBinding)

set(CMAKE_CXX_STANDARD 23)

include_directories(Dependencies\\Include\\)
include_directories(C:\\LuaJIT-2.1.M.64\\src)
include_directories(D:\\LuaBindingGit\\)
include_directories(soloud)

file(GLOB_RECURSE soloud soloud/*.cpp soloud/*.c)

add_library(LuaSoLoudBinding SHARED main.cpp dll.def ${soloud})
add_executable(LuaSoLoudBindingTest main.cpp dll.def ${soloud})

find_library(LUAJ5164_3 lua51 lib C:\\LuaJIT-2.1.M.64\\src\\)

target_link_libraries(LuaSoLoudBinding LINK_PUBLIC ${LUAJ5164_3})
target_link_libraries(LuaSoLoudBindingTest LINK_PUBLIC ${LUAJ5164_3})

MESSAGE(STATUS "LUAJIT: " ${LUAJ5164_3})

target_compile_definitions(LuaSoLoudBinding PRIVATE WITH_MINIAUDIO)
target_compile_definitions(LuaSoLoudBindingTest PRIVATE WITH_MINIAUDIO)

target_link_libraries(LuaSoLoudBinding LINK_PUBLIC ${LUA51})
target_link_libraries(LuaSoLoudBindingTest LINK_PUBLIC ${LUA51})

set_property(TARGET LuaSoLoudBinding PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
add_compile_options(
        $<$<CONFIG:>:/MT> #---------|
        $<$<CONFIG:Debug>:/MTd> #---|-- Statically link the runtime libraries
        $<$<CONFIG:Release>:/MT> #--|
)

add_custom_command(TARGET LuaSoLoudBinding POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy LuaSoLoudBinding.dll "audio.dll")
