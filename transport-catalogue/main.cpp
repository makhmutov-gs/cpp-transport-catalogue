// #include <iostream>
// #include "transport_catalogue.h"
// #include "json_reader.h"
// #include "map_renderer.h"
// #include "request_handler.h"

// int main() {
//     using namespace catalogue;
//     using namespace catalogue::reader;
//     using namespace catalogue::requests;
//     using namespace catalogue::renderer;

//     TransportCatalogue cat;

//     JsonReader reader(std::cin);

//     reader.ProcessInQueries(cat);

//     MapRenderer renderer(reader.GetRenderSettings());
//     RequestHandler handler(cat, renderer, reader.GetRoutingSettings());

//     reader.PrintOutQueries(cat, handler, std::cout);
// }

#include <fstream>
#include <iostream>
#include <string_view>

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {

        // make base here

    } else if (mode == "process_requests"sv) {

        // process requests here

    } else {
        PrintUsage();
        return 1;
    }
}