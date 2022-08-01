add_library(jsoncpp)
target_include_directories(jsoncpp PUBLIC jsoncpp/include)
file(GLOB JSONCPP_C_CPP_FILES jsoncpp/src/lib_json/*.c jsoncpp/src/lib_json/*.cpp jsoncpp/src/lib_json/*.inl)
target_sources(jsoncpp PUBLIC ${JSONCPP_C_CPP_FILES})
