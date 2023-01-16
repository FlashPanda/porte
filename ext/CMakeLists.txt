add_library(pugixml STATIC pugixml/src/pugixml.cpp)
set_property(TARGET pugixml PROPERTY ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set_property(TARGET pugixml PROPERTY FOLDER "dependencies")

add_library(tinyobjloader STATIC tinyobjloader/tiny_obj_loader.cc)
set_property(TARGET tinyobjloader PROPERTY ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set_property(TARGET tinyobjloader PROPERTY FOLDER "dependencies")

###########################################################################
# glog
SET(WITH_GFLAGS OFF CACHE BOOL "Use gflags")
SET(BUILD_SHARED_LIBS OFF CACHE BOOL " " FORCE)

ADD_SUBDIRECTORY(glog)
SET_PROPERTY(TARGET glog logging_unittest demangle_unittest utilities_unittest stl_logging_unittest PROPERTY FOLDER "dependencies")
INCLUDE_DIRECTORIES (
  src/ext/glog/src
  ${CMAKE_BINARY_DIR}/src/ext/glog
)

# 数据输出
MESSAGE(STATUS "【EXT】CMAKE_CURRENT_BINARY_DIR:			" ${CMAKE_CURRENT_BINARY_DIR})
MESSAGE(STATUS "【EXT】CMAKE_CURRENT_SOURCE_DIR:			" ${CMAKE_CURRENT_SOURCE_DIR})
MESSAGE(STATUS "【EXT】CMAKE_BINARY_DIR:			" ${CMAKE_BINARY_DIR})
MESSAGE(STATUS "【EXT】-------------------------------------")