project('transport_catalogue', 'cpp', default_options: ['cpp_std=c++20', 'warning_level=3', 'werror=true'])

src = [
    'transport-catalogue/geo.h',
    'transport-catalogue/geo.cpp',
    'transport-catalogue/main.cpp',
    'transport-catalogue/transport_catalogue.cpp',
    'transport-catalogue/transport_catalogue.h',
    'transport-catalogue/json_builder.h',
    'transport-catalogue/json_builder.cpp',
    'transport-catalogue/json_reader.h',
    'transport-catalogue/json_reader.cpp',
    'transport-catalogue/json.h',
    'transport-catalogue/json.cpp',
    'transport-catalogue/domain.h',
    'transport-catalogue/map_renderer.h',
    'transport-catalogue/map_renderer.cpp',
    'transport-catalogue/request_handler.h',
    'transport-catalogue/request_handler.cpp',
    'transport-catalogue/svg.h',
    'transport-catalogue/svg.cpp',

]

transport_test = executable('main', 'transport-catalogue/main.cpp', sources : src)
