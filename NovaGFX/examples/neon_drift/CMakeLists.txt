add_executable(neon_drift
    main.cpp
    game.cpp
    car.cpp
    track.cpp
    drift_scorer.cpp
    hud.cpp
    neon_renderer.cpp
)

target_link_libraries(neon_drift PRIVATE nova box2d)

if(MSVC)
    target_compile_options(neon_drift PRIVATE /W4)
else()
    target_compile_options(neon_drift PRIVATE -Wall -Wextra -Wpedantic)
endif()
