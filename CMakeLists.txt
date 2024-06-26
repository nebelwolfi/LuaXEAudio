cmake_minimum_required(VERSION 3.26)
project(LuaSoLoudBinding)

set(CMAKE_CXX_STANDARD 23)

include_directories(Dependencies\\Include\\)
include_directories(C:\\LuaJIT-2.1.M.64\\src)
include_directories(D:\\LuaXE\\shared\\include)
include_directories(soloud)

file(GLOB_RECURSE soloud soloud/*.cpp soloud/*.c)

add_library(LuaSoLoudBinding SHARED main.cpp dll.def ${soloud})

find_library(LUAJ lua51dyn lib C:\\LuaJIT-2.1.M.64\\src\\)

target_link_libraries(LuaSoLoudBinding LINK_PUBLIC ${LUAJ})

MESSAGE(STATUS "LUAJIT: " ${LUAJ5164_4})

target_compile_definitions(LuaSoLoudBinding PRIVATE WITH_XAUDIO2)

target_link_libraries(LuaSoLoudBinding LINK_PUBLIC ${LUA51})

set_property(TARGET LuaSoLoudBinding PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
add_compile_options(
        $<$<CONFIG:>:/MT> #---------|
        $<$<CONFIG:Debug>:/MTd> #---|-- Statically link the runtime libraries
        $<$<CONFIG:Release>:/MT> #--|
)

target_compile_options(LuaSoLoudBinding PRIVATE "/Zi" PRIVATE "/O2" PRIVATE "/showIncludes")
target_link_options(LuaSoLoudBinding PRIVATE "/nologo" PRIVATE "/OPT:REF" PRIVATE "/OPT:ICF" PRIVATE "/INCREMENTAL" PRIVATE "/MACHINE:X64" PRIVATE "/LARGEADDRESSAWARE" PRIVATE "/LTCG")
target_link_options(LuaSoLoudBinding PRIVATE "/DEBUG")
target_link_options(LuaSoLoudBinding PRIVATE "/OPT:REF")
target_link_options(LuaSoLoudBinding PRIVATE "/OPT:ICF")

add_custom_command(TARGET LuaSoLoudBinding POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy LuaSoLoudBinding.dll "audio.dll")
