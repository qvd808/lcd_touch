file(GLOB_RECURSE SOURCES "src/*.c")

idf_component_register(
    SRCS "main.c" ${SOURCES} "lvgl_demo_ui.c"
    INCLUDE_DIRS "inc"
)
