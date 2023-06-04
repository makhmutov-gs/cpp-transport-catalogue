#include "request_handler.h"

namespace catalogue::requests {

RequestHandler::RequestHandler(
    const TransportCatalogue& cat,
    const renderer::MapRenderer& renderer
) : cat_(cat)
  , renderer_(renderer) {}

svg::Document RequestHandler::RenderMap() const {
    std::vector<geo::Coordinates> coords = GetCoordsOnRoutes();

    return renderer_.RenderRoutes(GetCoordsOnRoutes(), GetSortedBuses());
}

std::vector<geo::Coordinates> RequestHandler::GetCoordsOnRoutes() const {
    std::vector<geo::Coordinates> coords;
    for (const auto& bus : cat_.GetBuses()) {
        for (const auto stop : bus.stops) {
            coords.push_back(stop->coords);
        }
    }
    return coords;
}

std::vector<const Bus*> RequestHandler::GetSortedBuses() const {
    std::vector<const Bus*> buses;

    for (const auto& bus : cat_.GetBuses()) {
        buses.push_back(&bus);
    }

    std::sort(
        buses.begin(),
        buses.end(),
        [] (const Bus* lhs, const Bus* rhs) {
            return lhs->name < rhs->name;
        }
    );

    return buses;
}


}