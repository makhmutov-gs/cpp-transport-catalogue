#include <iostream>
#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"

#include "graph.h"
#include "router.h"

int main() {
    using namespace catalogue;
    using namespace catalogue::reader;
    using namespace catalogue::requests;
    using namespace catalogue::renderer;

    TransportCatalogue cat;

    JsonReader reader(std::cin);

    reader.ProcessInQueries(cat);

    MapRenderer renderer(reader.GetRenderSettings());
    RequestHandler handler(cat, renderer, reader.GetRoutingSettings());

    reader.PrintOutQueries(cat, handler, std::cout);
}