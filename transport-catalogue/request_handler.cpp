#include "request_handler.h"

namespace catalogue::requests {

template <typename T>
void SortByName(std::vector<T*>& container) {
    std::sort(
        container.begin(),
        container.end(),
        [] (const T* lhs, const T* rhs) {
            return lhs->name < rhs->name;
        }
    );
}

RequestHandler::RequestHandler(
    const TransportCatalogue& cat,
    renderer::MapRenderer& renderer
) : cat_(cat)
  , renderer_(renderer) {}

svg::Document RequestHandler::RenderMap() const {
    auto sorted_buses = GetSortedBuses();
    auto sorted_stops = GetSortedStopsOnRoutes();
    renderer_.SetProjectorFromCoords(GetCoordsOnRoutes());

    svg::Document result;
    for (const auto& route : renderer_.RenderLines(sorted_buses)) {
        result.Add(route);
    }

    for (const auto& text : renderer_.RenderBusNames(sorted_buses)) {
        result.Add(text);
    }

    for (const auto& circles : renderer_.RenderStopCircles(sorted_stops)) {
        result.Add(circles);
    }

    for (const auto& text : renderer_.RenderStopNames(sorted_stops)) {
        result.Add(text);
    }

    return result;
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

std::vector<const Stop*> RequestHandler::GetSortedStopsOnRoutes() const {
    std::set<const Stop*> stops_set;
    for (const auto& bus : cat_.GetBuses()) {
        stops_set.insert(bus.stops.begin(), bus.stops.end());
    }
    std::vector<const Stop*> stops(stops_set.begin(), stops_set.end());
    SortByName(stops);

    return stops;
}

std::vector<const Bus*> RequestHandler::GetSortedBuses() const {
    std::vector<const Bus*> buses;

    for (const auto& bus : cat_.GetBuses()) {
        buses.push_back(&bus);
    }

    SortByName(buses);

    return buses;
}


}