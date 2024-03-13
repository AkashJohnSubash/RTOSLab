idf_component_register(SRCS "application.c" "edf_scheduler.c" "logger.c"
                    INCLUDE_DIRS "../includes")
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
