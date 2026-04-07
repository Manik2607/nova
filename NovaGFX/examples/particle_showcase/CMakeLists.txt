add_executable(particle_showcase
    main.cpp
)

target_link_libraries(particle_showcase PRIVATE nova)

if(MSVC)
    target_compile_options(particle_showcase PRIVATE /W4)
else()
    target_compile_options(particle_showcase PRIVATE -Wall -Wextra)
endif()
