add_library(Qwiic_OTOS_Library STATIC
        src/sfeQwiicOtos.cpp
        src/utils.cpp
)
target_link_libraries(Qwiic_OTOS_Library
        pico_stdlib 
        hardware_i2c
)
target_include_directories(Qwiic_OTOS_Library PUBLIC
        include
)
