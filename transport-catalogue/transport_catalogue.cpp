#include "transport_catalogue.h"

namespace catalogue {

void TransportCatalogue::AddStop(const Stop& stop) {
    stops_.push_back(std::move(stop));
    stopname_to_stop_[stops_.back().name] = &stops_.back();
    stop_to_buses_[&stops_.back()] = {};
}

void TransportCatalogue::SetRoadDistance(
    const std::string& from,
    const std::string& to,
    double distance
) {
    road_distances_[
        {stopname_to_stop_[from], stopname_to_stop_[to]}
    ] = distance;
}

void TransportCatalogue::AddBus(const BusInput& bus_input) {
    Bus bus;
    bus.name = std::move(bus_input.name);

    for (auto& stop : bus_input.stops) {
        bus.stops.push_back(stopname_to_stop_[stop]);
    }

    bus.road_length = CalcRoadRouteLength(bus.stops);
    bus.curvature = bus.road_length /  CalcGeoRouteLength(bus.stops);
    bus.unique_stops = std::set(bus.stops.begin(), bus.stops.end()).size();

    buses_.push_back(std::move(bus));

    for (const auto& stop : buses_.back().stops) {
        stop_to_buses_[stop].insert(buses_.back().name);
    }

    busname_to_bus_[buses_.back().name] = &buses_.back();
}

const Bus* TransportCatalogue::GetBus(const std::string& name) const {
    if (busname_to_bus_.count(name) == 0) {
        return nullptr;
    }

    return busname_to_bus_.at(name);
}

std::optional<std::set<std::string_view>> TransportCatalogue::GetBusesByStop(const std::string& name) const {
    if (stopname_to_stop_.count(name) == 0) {
        return std::nullopt;
    }

    return stop_to_buses_.at(stopname_to_stop_.at(name));
}

double TransportCatalogue::CalcGeoRouteLength(const std::vector<const Stop*>& stops) {
    double result = 0;
    for (size_t i = 0; i < stops.size() - 1; ++i) {
        auto stop_pair = std::pair{stops[i], stops[i+1]};

        if (geo_distances_.count(stop_pair) == 0) {
            geo_distances_[stop_pair] = geo::ComputeDistance(
                {stops[i]->coords},
                {stops[i+1]->coords}
            );
        }
        result += geo_distances_.at(stop_pair);
    }
    return result;
}

double TransportCatalogue::CalcRoadRouteLength(const std::vector<const Stop*>& stops) const {
    double result = 0;
    for (size_t i = 0; i < stops.size() - 1; ++i) {
        auto stop_pair = std::pair{stops[i], stops[i+1]};

        if (road_distances_.count(stop_pair) != 0) {
            result += road_distances_.at(stop_pair);
        } else {
            result += road_distances_.at({stop_pair.second, stop_pair.first});
        }

    }
    return result;
}

}