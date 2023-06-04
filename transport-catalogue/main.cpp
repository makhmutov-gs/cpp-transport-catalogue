#include <iostream>
#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"

int main() {
    using namespace catalogue;
    using namespace catalogue::reader;
    using namespace catalogue::requests;
    using namespace catalogue::renderer;

    TransportCatalogue cat;

    JsonReader reader(std::cin, false);

    reader.ProcessInQueries(cat);
    //reader.PrintOutQueries(cat, std::cout);

    MapRenderer renderer(reader.GetRenderSettings());
    RequestHandler handler(cat, renderer);

    handler.RenderMap().Render(std::cout);
}