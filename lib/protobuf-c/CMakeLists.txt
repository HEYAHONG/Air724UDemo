add_library(protobuf-c)
target_include_directories(protobuf-c PUBLIC protobuf-c/protobuf-c)
file(GLOB PROTOBUF_C_C_CPP_FILES protobuf-c/protobuf-c/*.c)
target_sources(protobuf-c PUBLIC ${PROTOBUF_C_C_CPP_FILES})
