add_executable(ui_demo main.cpp)
target_link_libraries(ui_demo PRIVATE nova)

target_compile_definitions(ui_demo PRIVATE NOVA_BUILD_EXAMPLES)
if(NOT NOVA_BUILD_SHARED)
    target_compile_definitions(ui_demo PRIVATE NOVA_STATIC)
endif()

# We might want to copy assets just in case the engine needs it in the working dir
# Add any required copy steps if there were assets (not needed since we use built in shapes/lines)
