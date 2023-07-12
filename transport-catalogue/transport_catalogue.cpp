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

std::optional<double> TransportCatalogue::GetRoadDistance(const std::string& from, const std::string& to) const {
    if (stopname_to_stop_.count(from) == 0 || stopname_to_stop_.count(to) == 0) {
        return std::nullopt;
    }
    auto p = std::pair{stopname_to_stop_.at(from), stopname_to_stop_.at(to)};
    if (road_distances_.count(p) == 1) {
        return road_distances_.at(p);
    } else {
        return road_distances_.at({p.second, p.first});
    }
}

void TransportCatalogue::AddBus(
    const std::string& name,
    const std::vector<std::string>& stops,
    bool is_roundtrip
) {
    Bus bus;
    bus.name = std::move(name);
    bus.is_roundtrip = is_roundtrip;

    for (const auto& stop_name : stops) {
        bus.stops.push_back(stopname_to_stop_[stop_name]);
    }

    buses_.push_back(std::move(bus));

    for (const auto& stop : buses_.back().stops) {
        stop_to_buses_[stop].insert(buses_.back().name);
    }

    busname_to_bus_[buses_.back().name] = &buses_.back();
}

std::optional<BusInfo> TransportCatalogue::GetBusInfo(const std::string& name) {
    if (busname_to_bus_.count(name) == 0) {
        return std::nullopt;
    }

    const Bus* bus = busname_to_bus_.at(name);

    size_t unique_stops = std::set(bus->stops.begin(), bus->stops.end()).size();
    double road_length = CalcRoadRouteLength(bus->stops);
    double curvature = road_length / CalcGeoRouteLength(bus->stops);

    return BusInfo{
        bus->stops.size(),
        unique_stops,
        road_length,
        curvature
    };
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