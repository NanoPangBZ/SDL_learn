if(NOT DEFINED INPUT_FILE OR NOT DEFINED OUTPUT_FILE)
    message(FATAL_ERROR "INPUT_FILE and OUTPUT_FILE are required")
endif()

get_filename_component(output_directory "${OUTPUT_FILE}" DIRECTORY)
file(MAKE_DIRECTORY "${output_directory}")

# Convert every input byte to a C hexadecimal initializer. The generated object
# exposes stable symbol names regardless of the target operating system.
file(READ "${INPUT_FILE}" binary_hex HEX)
string(REGEX REPLACE "([0-9a-fA-F][0-9a-fA-F])" "0x\\1," binary_bytes "${binary_hex}")

file(WRITE "${OUTPUT_FILE}"
    "#include <stddef.h>\n\n"
    "const unsigned char __embedded_font[] = {\n"
    "${binary_bytes}\n"
    "};\n\n"
    "const size_t __embedded_font_size = sizeof(__embedded_font);\n"
)
