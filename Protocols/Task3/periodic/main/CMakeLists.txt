idf_component_register(SRCS "application.c" "periodic_schedule.c"
                    INCLUDE_DIRS "../includes")
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
