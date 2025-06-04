set(SOURCES
	"src/display.c"
	"src/touch_controller.c"
	"src/ui.c"
	"src/mod_wifi.c"
)

idf_component_register(
	SRCS "main.c" ${SOURCES} "${CMAKE_BINARY_DIR}/generated_env.c"
	INCLUDE_DIRS "inc"
	REQUIRES esp_event esp_wifi nvs_flash
)

# Check if .env file exists and generate accordingly
if(EXISTS "${CMAKE_SOURCE_DIR}/.env")
	# Read .env file at configure time to get environment variables
	add_custom_command(
		OUTPUT "${CMAKE_BINARY_DIR}/generated_env.c"
		COMMAND ${CMAKE_COMMAND} -DENV_FILE=${CMAKE_SOURCE_DIR}/.env -DOUTPUT_FILE=${CMAKE_BINARY_DIR}/generated_env.c -P ${CMAKE_SOURCE_DIR}/main/generate_env.cmake
		DEPENDS ${CMAKE_SOURCE_DIR}/.env ${CMAKE_SOURCE_DIR}/main/generate_env.cmake
		COMMENT "Generating generated_env.c from .env"
	)

	add_custom_target(generate_env ALL DEPENDS "${CMAKE_BINARY_DIR}/generated_env.c")
	add_dependencies(${COMPONENT_LIB} generate_env)
	add_compile_definitions(SET_ENV)
else()
	# Generate empty generated_env.c directly when .env doesn't exist
	file(WRITE "${CMAKE_BINARY_DIR}/generated_env.c" "// Auto-generated - no .env file found\n")
endif()
