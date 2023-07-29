#include <iostream>
#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"

#include <fstream>
#include <iostream>
#include <string_view>

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    using namespace catalogue;
    using namespace catalogue::reader;
    using namespace catalogue::requests;
    using namespace catalogue::renderer;

    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        TransportCatalogue cat;

        JsonReader reader(std::cin, false);

        reader.ProcessInQueries(cat);
        cat.SaveTo(std::filesystem::path(reader.GetDbName()));

    } else if (mode == "process_requests"sv) {

        JsonReader reader(std::cin);
        TransportCatalogue cat = *FromFile(std::filesystem::path(reader.GetDbName()));
        reader.ProcessInQueries(cat);

        MapRenderer renderer({});
        RequestHandler handler(cat, renderer, {6, 40 * 1000 / 60});

        reader.PrintOutQueries(cat, handler, std::cout);

    } else {
        PrintUsage();
        return 1;
    }
}
