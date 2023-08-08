# Modhook
add_executable (ModHook "ModHook.cpp" "ModHook.h")

target_include_directories(ModHook PRIVATE "${CMAKE_SOURCE_DIR}/extern/detours_lib/Detours/src")
target_link_libraries(ModHook PRIVATE libdetours)

# This doesn't work?
# target_include_directories(ModHook PRIVATE "${CMAKE_SOURCE_DIR}/extern/CLI11/include")
# target_link_libraries(ModHook PUBLIC CLI11::CLI11)

set_property(TARGET ModHook APPEND PROPERTY LINK_FLAGS "/MANIFEST:NO /NOIMPLIB /NOEXP")

# Injection
add_library(ModHookInjection SHARED "Injection.cpp" "Injection.h")

target_include_directories(ModHookInjection PRIVATE "${CMAKE_SOURCE_DIR}/extern/detours_lib/Detours/src")
target_link_libraries(ModHookInjection PRIVATE libdetours)

set_property(TARGET ModHookInjection APPEND PROPERTY LINK_FLAGS "/MANIFEST:NO /NOIMPLIB /NOEXP")


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ModHook PROPERTY CXX_STANDARD 20)
endif()
