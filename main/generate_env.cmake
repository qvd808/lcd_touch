# generate_env.cmake
file(READ "${ENV_FILE}" CONTENTS)
file(WRITE "${OUTPUT_FILE}" "// Auto-generated from .env\n\n")

# Convert to list of lines properly
string(REGEX REPLACE "\r?\n" ";" LINES "${CONTENTS}")

foreach(line IN LISTS LINES)
    string(STRIP "${line}" line)
    if(NOT line MATCHES "^#.*" AND line MATCHES "^([^=]+)=(.*)$")
        string(REGEX REPLACE "^([^=]+)=(.*)$" "\\1;\\2" VAR_MATCH "${line}")
        list(GET VAR_MATCH 0 KEY)
        list(GET VAR_MATCH 1 VALUE)
        # Escape double quotes in VALUE
        string(REPLACE "\"" "\\\"" ESCAPED_VALUE "${VALUE}")
        file(APPEND "${OUTPUT_FILE}" "const char* ${KEY} = \"${ESCAPED_VALUE}\";\n")
        message(STATUS "Adding env var: ${KEY}=${ESCAPED_VALUE}")
    endif()
endforeach()
