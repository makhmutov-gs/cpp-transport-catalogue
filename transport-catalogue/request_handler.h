#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"

namespace catalogue::requests {

class RequestHandler {
public:
    RequestHandler(const TransportCatalogue& cat, const renderer::MapRenderer& renderer);

    svg::Document RenderMap() const;

private:
    std::vector<geo::Coordinates> GetCoordsOnRoutes() const;
    std::vector<const Bus*> GetSortedBuses() const;
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const TransportCatalogue& cat_;
    const renderer::MapRenderer& renderer_;
};

}
