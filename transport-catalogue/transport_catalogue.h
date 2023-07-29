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
#include <filesystem>

#include "domain.h"
#include "map_renderer.h"

namespace catalogue {

using domain::Stop;
using domain::Bus;

struct BusInfo {
    size_t stop_count;
    size_t unique_stops;
    double road_length;
    double curvature;
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

    void SetRoadDistance(
        const std::string& from,
        const std::string& to,
        double distance
    );

    std::optional<double> GetRoadDistance(const std::string& from, const std::string& to) const;

    void AddBus(
        const std::string& name,
        const std::vector<std::string>& stops,
        bool is_roundtrip
    );

    const std::deque<Bus>& GetBuses() const {
        return buses_;
    }

    std::optional<BusInfo> GetBusInfo(const std::string& name);

    std::optional<std::set<std::string_view>> GetBusesByStop(const std::string& name) const;

    void SaveWithSettings(
        const std::filesystem::path& path,
        const renderer::Settings& render_settings,
        const domain::RoutingSettings& routing_settings
    ) const;

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

std::tuple<TransportCatalogue, renderer::Settings, domain::RoutingSettings> FromFile(const std::filesystem::path& path);

}