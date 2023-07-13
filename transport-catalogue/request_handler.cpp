#include "request_handler.h"

namespace catalogue::requests {

RequestHandler::RequestHandler(
    const TransportCatalogue& cat,
    renderer::MapRenderer& renderer,
    domain::RoutingSettings routing_settings
) : cat_(cat)
  , sorted_stops_(GetSortedStops())
  , sorted_buses_(GetSortedBuses())
  , renderer_(renderer)
  , transport_router_(sorted_stops_, sorted_buses_, cat_, routing_settings)
{
}

svg::Document RequestHandler::RenderMap() const {
    return renderer_.Render(sorted_buses_, sorted_stops_);
}

std::optional<catalogue::router::TransportRouter::RouteInfo> RequestHandler::FormRoute(
    const std::string& from, const std::string& to
) const {
    return transport_router_.BuildRoute(from, to);
}

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

std::vector<const Bus*> RequestHandler::GetSortedBuses() const {
    const auto& buses = cat_.GetBuses();
    std::vector<const Bus*> sorted_buses;
    sorted_buses.reserve(buses.size());

    std::transform(buses.begin(), buses.end(), std::back_inserter(sorted_buses),
        [](const Bus& b) {
            return &b;
        }
    );

    SortByName(sorted_buses);

    return sorted_buses;
}

std::vector<const Stop*> RequestHandler::GetSortedStops() const {
    std::set<const Stop*> stops_set;

    for (const auto& bus : cat_.GetBuses()) {
        stops_set.insert(bus.stops.begin(), bus.stops.end());
    }

    std::vector<const Stop*> sorted_stops(stops_set.begin(), stops_set.end());

    SortByName(sorted_stops);

    return sorted_stops;
}

}