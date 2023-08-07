add_executable (ModHook "ModHook.cpp" "ModHook.h")

add_library(ModHookInjection SHARED "Injection.cpp" "Injection.h")

target_link_libraries(ModHookInjection PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/lib/MinHook.x86.lib")

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/lib/" DESTINATION "${CMAKE_BINARY_DIR}/")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/resources" DESTINATION "${CMAKE_BINARY_DIR}/")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ModHook PROPERTY CXX_STANDARD 20)
endif()
