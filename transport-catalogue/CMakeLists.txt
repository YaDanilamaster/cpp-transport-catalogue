cmake_minimum_required (VERSION 3.7)

set ("project_name" Transport_catalog)

project ("${project_name}")

add_executable ("${project_name}" 
main.cpp 
domain.h
domain.cpp 
geo.h
geo.cpp
graph.h
json.h
json.cpp
json_builder.h
json_builder.cpp
json_reader.h
json_reader.cpp
map_renderer.h
map_renderer.cpp
ranges.h
request_handler.h
request_handler.cpp
router.h
svg.h
svg.cpp
transport_catalogue.h
transport_catalogue.cpp
transport_router.h
transport_router.cpp )


set_target_properties("${project_name}" PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO )