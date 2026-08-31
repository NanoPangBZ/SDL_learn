if(NOT DEFINED INPUT_FILE OR NOT DEFINED OUTPUT_FILE OR NOT DEFINED SYMBOL_NAME)
    message(FATAL_ERROR "INPUT_FILE, OUTPUT_FILE and SYMBOL_NAME are required")
endif()

get_filename_component(output_directory "${OUTPUT_FILE}" DIRECTORY)
file(MAKE_DIRECTORY "${output_directory}")

# Convert every input byte to a C hexadecimal initializer. The generated object
# exposes stable symbol names regardless of the target operating system.
file(READ "${INPUT_FILE}" binary_hex HEX)
string(REGEX REPLACE "([0-9a-fA-F][0-9a-fA-F])" "0x\\1," binary_bytes "${binary_hex}")

file(WRITE "${OUTPUT_FILE}"
    "#include <stddef.h>\n\n"
    "const unsigned char __embedded_${SYMBOL_NAME}[] = {\n"
    "${binary_bytes}\n"
    "};\n\n"
    "const size_t __embedded_${SYMBOL_NAME}_size = sizeof(__embedded_${SYMBOL_NAME});\n"
)
