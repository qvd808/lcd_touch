set(SOURCES
    "src/display.c"
    "src/touch_controller.c"
    "src/ui.c"
)

idf_component_register(
    SRCS "main.c" ${SOURCES}
    INCLUDE_DIRS "inc"
)
