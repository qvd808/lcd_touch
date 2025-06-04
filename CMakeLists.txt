cmake_minimum_required(VERSION 3.16)

# Set target to esp32c6
set(IDF_TARGET "esp32c6")

# Function to load environment variables from .env file
function(load_env_file)
	if(EXISTS "${CMAKE_SOURCE_DIR}/.env")
		# Read value from .env and load it
		file(READ "${CMAKE_SOURCE_DIR}/.env" ENV_FILE_CONTENT)

		# Parse key-value pairs (handle multi-line content properly)
		string(REGEX REPLACE "\r?\n" ";" ENV_LINES "${ENV_FILE_CONTENT}")
		
		# Loop through each line
		foreach(LINE ${ENV_LINES})
			# Skip empty lines and comments
			if(LINE AND NOT LINE MATCHES "^#.*")
				if(LINE MATCHES "^([^=]+)=(.*)$")
					string(REGEX REPLACE "^([^=]+)=(.*)$" "\\1;\\2" VAR_MATCH "${LINE}")
					list(GET VAR_MATCH 0 KEY)
					list(GET VAR_MATCH 1 VALUE)
					
					# Remove leading/trailing whitespace
					string(STRIP "${KEY}" KEY)
					string(STRIP "${VALUE}" VALUE)
					
					# Escape quotes and backslashes in the value
					string(REPLACE "\\" "\\\\" VALUE_ESCAPED "${VALUE}")
					string(REPLACE "\"" "\\\"" VALUE_ESCAPED "${VALUE_ESCAPED}")
					
					# Set as compile definition with proper quoting
					add_compile_definitions(${KEY}="${VALUE_ESCAPED}")
					message(STATUS "Loaded env variable as define: ${KEY}=\"${VALUE_ESCAPED}\"")
				endif()
			endif()
		endforeach()
	else()
		message(WARNING ".env file not found at ${CMAKE_SOURCE_DIR}/.env")
	endif()
endfunction()

# Load environment variables initially (when CMake configures)
load_env_file()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Custom target that depends on .env file
add_custom_target(load_env
	COMMAND ${CMAKE_COMMAND} -E echo "Reloading .env file..."
	DEPENDS "${CMAKE_SOURCE_DIR}/.env"
	COMMENT "Environment file dependency"
	VERBATIM
)

# "Trim" the build. Include the minimal set of components, main, and anything it depends on.
idf_build_set_property(MINIMAL_BUILD ON)
project(spi_lcd_touch)
