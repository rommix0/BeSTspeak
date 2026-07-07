if(NOT EXISTS "${ISS_FILE}")
    message(FATAL_ERROR "ISS_FILE does not exist: ${ISS_FILE}")
endif()

file(READ "${ISS_FILE}" ISS_CONTENT)

string(REGEX REPLACE "LicenseFile=[^\r\n]+[\r\n]+" "" ISS_CONTENT "${ISS_CONTENT}")

string(REGEX REPLACE "\\{ No extra code files specified \\}" "" ISS_CONTENT "${ISS_CONTENT}")

string(FIND "${ISS_CONTENT}" "[Code]" CODE_POS)
if(CODE_POS EQUAL -1)
    message(FATAL_ERROR "[Code] section not found in ISS file")
endif()

math(EXPR CODE_END "${CODE_POS} + 6")
string(SUBSTRING "${ISS_CONTENT}" 0 ${CODE_END} BEFORE_CODE)
string(SUBSTRING "${ISS_CONTENT}" ${CODE_END} -1 AFTER_CODE)

file(READ "${CMAKE_CURRENT_LIST_DIR}/innosetup_code.iss" CUSTOM_CODE)

set(ISS_CONTENT "${BEFORE_CODE}\n${CUSTOM_CODE}${AFTER_CODE}")

file(WRITE "${ISS_FILE}" "${ISS_CONTENT}")

message(STATUS "Post-processed InnoSetup script: ${ISS_FILE}")
