cmake_minimum_required(VERSION 3.12)

project(sffscfggen C CXX ASM)


#添加可执行文件
add_executable(sffscfggen)

#设置C++标准
set_property(TARGET sffscfggen PROPERTY CXX_STANDARD 20)

#添加源代码
file(GLOB  C_FILES *.c *.cpp)
target_sources(sffscfggen PUBLIC ${C_FILES})


include(FindPkgConfig)

#jsoncpp库
pkg_check_modules(JSONCPP REQUIRED IMPORTED_TARGET jsoncpp)
target_link_libraries(sffscfggen  PkgConfig::JSONCPP)
