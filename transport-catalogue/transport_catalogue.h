#pragma once
#include <unordered_map>
#include <deque>
#include <unordered_set>
#include <string>
#include <string_view>
#include <vector>
#include <utility>
#include <set>
#include <optional>
#include "geo.h"

namespace catalogue {

struct Stop {
    std::string name;
    geo::Coordinates coords;
};

struct Bus {
    std::string name;
    std::vector<const Stop*> stops;
    size_t unique_stops;
    double road_length;
    double curvature;
};

struct BusInput {
    std::string name;
    std::vector<std::string> stops;
};

struct StopPairHash {
    size_t operator()(const std::pair<const Stop*, const Stop*>& stop_pair) const {
        return hasher_(stop_pair.first) + 37 * hasher_(stop_pair.second);
    }

private:
    std::hash<const Stop*> hasher_;
};

class TransportCatalogue {
public:
    TransportCatalogue() {}

    void AddStop(const Stop& stop);

    void AddStopDistances(
        const std::string& name,
        const std::unordered_map<std::string, double>& distances
    );

    void AddBus(const BusInput& bus_input);

    const Bus* GetBus(const std::string& name) const;

    std::optional<std::set<std::string_view>> GetBusesByStop(const std::string& name) const;

private:
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
    std::unordered_map<const Stop*, std::set<std::string_view>> stop_to_buses_;
    std::unordered_map<std::pair<const Stop*, const Stop*>, double, StopPairHash> geo_distances_;
    std::unordered_map<std::pair<const Stop*, const Stop*>, double, StopPairHash> road_distances_;

    double CalcGeoRouteLength(const std::vector<const Stop*>& stops);
    double CalcRoadRouteLength(const std::vector<const Stop*>& stops) const;
};

}