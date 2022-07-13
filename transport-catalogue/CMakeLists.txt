cmake_minimum_required(VERSION 3.11)

set ("project_name" Transport_catalog)

project (Transport_catalog CXX)

set(CMAKE_CXX_STANDARD 17)

#set(SOURCE_DIR src/)

set(FILES_H
 "${SOURCE_DIR/}domain.h"
 "${SOURCE_DIR/}geo.h"
 "${SOURCE_DIR/}graph.h"
 "${SOURCE_DIR/}json.h"
 "${SOURCE_DIR/}json_builder.h"
 "${SOURCE_DIR/}json_reader.h"
 "${SOURCE_DIR/}map_renderer.h"
 "${SOURCE_DIR/}ranges.h"
 "${SOURCE_DIR/}request_handler.h"
 "${SOURCE_DIR/}router.h"
 "${SOURCE_DIR/}svg.h"
 "${SOURCE_DIR/}transport_catalogue.h"
 "${SOURCE_DIR/}transport_router.h"
)

set(FILES_CPP
 "${SOURCE_DIR/}main.cpp"
 "${SOURCE_DIR/}domain.cpp"
 "${SOURCE_DIR/}geo.cpp"
 "${SOURCE_DIR/}json.cpp"
 "${SOURCE_DIR/}json_builder.cpp"
 "${SOURCE_DIR/}json_reader.cpp"
 "${SOURCE_DIR/}map_renderer.cpp"
 "${SOURCE_DIR/}request_handler.cpp"
 "${SOURCE_DIR/}svg.cpp"
 "${SOURCE_DIR/}transport_catalogue.cpp"
 "${SOURCE_DIR/}transport_router.cpp"
)

source_group("Headers" FILES ${FILES_H})
source_group("Sources" FILES ${FILES_CPP}) 

add_executable(transport-catalogue ${FILES_H} ${FILES_CPP}) 
