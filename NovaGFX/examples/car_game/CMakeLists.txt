add_executable(car_game 
    main.cpp
    car.cpp
    terrain.cpp
)

target_link_libraries(car_game PRIVATE nova box2d)

if(MSVC)
    target_compile_options(car_game PRIVATE /W4)
else()
    target_compile_options(car_game PRIVATE -Wall -Wextra -Wpedantic)
endif()
