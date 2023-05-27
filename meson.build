project('transport_catalogue', 'cpp', default_options: ['cpp_std=c++20', 'warning_level=3', 'werror=true'])

src = [
    'transport-catalogue/geo.h',
    'transport-catalogue/input_reader.cpp',
    'transport-catalogue/input_reader.h',
    'transport-catalogue/main.cpp',
    'transport-catalogue/stat_reader.cpp',
    'transport-catalogue/stat_reader.h',
    'transport-catalogue/transport_catalogue.cpp',
    'transport-catalogue/transport_catalogue.h',
]

transport_test = executable('main', 'transport-catalogue/main.cpp', sources : src)
