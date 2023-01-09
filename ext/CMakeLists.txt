add_library(pugixml SHARED pugixml/src/pugixml.cpp)
set_property(TARGET pugixml PROPERTY
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/pugixml")
set_property(TARGET pugixml PROPERTY FOLDER "dependencies")
set(PUGIXML_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/pugixml/src PARENT_SCOPE)
target_compile_options(pugixml PRIVATE -DPUGIXML_BUILD_DLL)
#MESSAGE(STATUS "CMAKE_CURRENT_BINARY_DIR:			" ${CMAKE_CURRENT_BINARY_DIR})