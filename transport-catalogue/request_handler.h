#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"

namespace catalogue::requests {

class RequestHandler {
public:
    RequestHandler(const TransportCatalogue& cat, renderer::MapRenderer& renderer);

    svg::Document RenderMap() const;

private:
    std::vector<geo::Coordinates> GetCoordsOnRoutes() const;
    std::vector<const Bus*> GetSortedBuses() const;
    std::vector<const Stop*> GetSortedStopsOnRoutes() const;

    const TransportCatalogue& cat_;
    renderer::MapRenderer& renderer_;
};

}
