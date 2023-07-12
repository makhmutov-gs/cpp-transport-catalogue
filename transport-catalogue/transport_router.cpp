#include "transport_router.h"

namespace catalogue::router {


TransportRouter::TransportRouter(
    const std::vector<const Stop*>& stops,
    const std::vector<const Bus*>& buses,
    const TransportCatalogue& cat,
    domain::RoutingSettings settings
)   : stops_(stops)
    , buses_(buses)
    , cat_(cat)
    , settings_(std::move(settings))
    , graph_(InitGraph())
    , router_(graph_)
{
}

std::optional<RouteInfo> TransportRouter::BuildRoute(
    const std::string& from, const std::string& to
) const {
    size_t from_id = std::distance(
        stops_.begin(),
        std::find_if(stops_.begin(), stops_.end(),
            [&from](const Stop* s) { return s->name == from; }
        )
    );

    size_t to_id = std::distance(
        stops_.begin(),
        std::find_if(stops_.begin(), stops_.end(),
            [&to](const Stop* s) { return s->name == to; }
        )
    );

    auto route = router_.BuildRoute(from_id * 2, to_id * 2);
    if (!route) {
        return std::nullopt;
    }

    // TODO: сделать инициализацию через std::transform
    RouteInfo result;
    result.total_time = route->weight;

    for (const auto& edge_id : route->edges) {
        result.edges.push_back(edges_info_[edge_id]);
    }

    return result;
}

graph::DirectedWeightedGraph<double> TransportRouter::InitGraph() {
    graph::DirectedWeightedGraph<double> g(stops_.size() * 2);

    for (size_t i = 0; i < stops_.size(); ++i) {
        double time = static_cast<double>(settings_.bus_wait_time);
        edges_.push_back(g.AddEdge({i * 2, i * 2 + 1, time}));
        edges_info_.push_back({stops_[i]->name, 0, time});
    }

    for (const auto bus : buses_) {
        if (bus->is_roundtrip) {
            for (size_t i = 0; i < bus->stops.size() - 1; ++i) {
                size_t first_pos = std::distance(stops_.begin(), std::find(stops_.begin(), stops_.end(), bus->stops[i]));
                double current_distance = 0;
                for (size_t j = i + 1; j < bus->stops.size(); ++j) {
                    if (i == 0 && j == bus->stops.size() - 1) {
                        continue;
                    }
                    size_t second_pos = std::distance(stops_.begin(), std::find(stops_.begin(), stops_.end(), bus->stops[j]));
                    if (j == i + 1) {
                        current_distance += cat_.GetRoadDistance(bus->stops[i]->name, bus->stops[j]->name).value();
                    } else {
                        current_distance += cat_.GetRoadDistance(bus->stops[j-1]->name, bus->stops[j]->name).value();
                    }

                    double time = current_distance / (settings_.bus_velocity * 1000 / 60);
                    g.AddEdge({first_pos * 2 + 1, second_pos * 2, time});
                    edges_info_.push_back({bus->name, j - i, time});
                }
            }
        } else {
            size_t end_stop_idx = bus->stops.size() / 2;

            for (size_t i = 0; i < end_stop_idx; ++i) {
                size_t first_pos = std::distance(stops_.begin(), std::find(stops_.begin(), stops_.end(), bus->stops[i]));
                double current_distance = 0;
                for (size_t j = i + 1; j <= end_stop_idx; ++j) {
                    // if (i == 0 && j == bus->stops.size() - 1) {
                    //     continue;
                    // }
                    size_t second_pos = std::distance(stops_.begin(), std::find(stops_.begin(), stops_.end(), bus->stops[j]));
                    if (j == i + 1) {
                        current_distance += cat_.GetRoadDistance(bus->stops[i]->name, bus->stops[j]->name).value();
                    } else {
                        current_distance += cat_.GetRoadDistance(bus->stops[j-1]->name, bus->stops[j]->name).value();
                    }

                    double time = current_distance / (settings_.bus_velocity * 1000 / 60);
                    g.AddEdge({first_pos * 2 + 1, second_pos * 2, time});
                    edges_info_.push_back({bus->name, j - i, time});
                }
            }

            for (size_t i = end_stop_idx; i < bus->stops.size() - 1; ++i) {
                size_t first_pos = std::distance(stops_.begin(), std::find(stops_.begin(), stops_.end(), bus->stops[i]));
                double current_distance = 0;
                for (size_t j = i + 1; j < bus->stops.size(); ++j) {
                    // if (i == 0 && j == bus->stops.size() - 1) {
                    //     continue;
                    // }
                    size_t second_pos = std::distance(stops_.begin(), std::find(stops_.begin(), stops_.end(), bus->stops[j]));
                    if (j == i + 1) {
                        current_distance += cat_.GetRoadDistance(bus->stops[i]->name, bus->stops[j]->name).value();
                    } else {
                        current_distance += cat_.GetRoadDistance(bus->stops[j-1]->name, bus->stops[j]->name).value();
                    }

                    double time = current_distance / (settings_.bus_velocity * 1000 / 60);
                    g.AddEdge({first_pos * 2 + 1, second_pos * 2, time});
                    edges_info_.push_back({bus->name, j - i, time});
                }
            }
        }
    }
    return g;
}

}