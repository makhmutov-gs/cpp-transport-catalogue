#include <iostream>
#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"

#include "graph.h"
#include "router.h"

int main() {
    using namespace catalogue;
    using namespace catalogue::reader;
    using namespace catalogue::requests;
    using namespace catalogue::renderer;

    TransportCatalogue cat;

    JsonReader reader(std::cin);

    reader.ProcessInQueries(cat);

    MapRenderer renderer(reader.GetRenderSettings());
    RequestHandler handler(cat, renderer, reader.GetRoutingSettings());

    reader.PrintOutQueries(cat, handler, std::cout);

    /*
    auto router_settings = reader.GetRoutingSettings();
    auto stops = handler.GetSortedStops();
    graph::DirectedWeightedGraph<double> g(stops.size() * 2);

    struct EdgeInfo {
        std::string_view name;
        size_t span_count;
    };

    std::vector<EdgeInfo> edges_info;
    std::vector<graph::EdgeId> edges;

    for (size_t i = 0; i < stops.size(); ++i) {
        edges.push_back(g.AddEdge({i * 2, i * 2 + 1, static_cast<double>(router_settings.bus_wait_time)}));
        edges_info.push_back({stops[i]->name, 0});
    }

    auto buses = handler.GetSortedBuses();

    for (const auto bus : buses) {
        if (bus->is_roundtrip) {
            for (size_t i = 0; i < bus->stops.size() - 1; ++i) {
                size_t first_pos = std::distance(stops.begin(), std::find(stops.begin(), stops.end(), bus->stops[i]));
                double current_distance = 0;
                for (size_t j = i + 1; j < bus->stops.size(); ++j) {
                    if (i == 0 && j == bus->stops.size() - 1) {
                        continue;
                    }
                    size_t second_pos = std::distance(stops.begin(), std::find(stops.begin(), stops.end(), bus->stops[j]));
                    if (j == i + 1) {
                        current_distance += cat.GetRoadDistance(bus->stops[i]->name, bus->stops[j]->name).value();
                    } else {
                        current_distance += cat.GetRoadDistance(bus->stops[j-1]->name, bus->stops[j]->name).value();
                    }

                    g.AddEdge({first_pos * 2 + 1, second_pos * 2, current_distance / (router_settings.bus_velocity * 1000 / 60)});
                    edges_info.push_back({bus->name, j - i});
                }
            }
        } else {
            size_t end_stop_idx = bus->stops.size() / 2;

            for (size_t i = 0; i < end_stop_idx; ++i) {
                size_t first_pos = std::distance(stops.begin(), std::find(stops.begin(), stops.end(), bus->stops[i]));
                double current_distance = 0;
                for (size_t j = i + 1; j <= end_stop_idx; ++j) {
                    // if (i == 0 && j == bus->stops.size() - 1) {
                    //     continue;
                    // }
                    size_t second_pos = std::distance(stops.begin(), std::find(stops.begin(), stops.end(), bus->stops[j]));
                    if (j == i + 1) {
                        current_distance += cat.GetRoadDistance(bus->stops[i]->name, bus->stops[j]->name).value();
                    } else {
                        current_distance += cat.GetRoadDistance(bus->stops[j-1]->name, bus->stops[j]->name).value();
                    }

                    g.AddEdge({first_pos * 2 + 1, second_pos * 2, current_distance / (router_settings.bus_velocity * 1000 / 60)});
                    edges_info.push_back({bus->name, j - i});
                }
            }

            for (size_t i = end_stop_idx; i < bus->stops.size() - 1; ++i) {
                size_t first_pos = std::distance(stops.begin(), std::find(stops.begin(), stops.end(), bus->stops[i]));
                double current_distance = 0;
                for (size_t j = i + 1; j < bus->stops.size(); ++j) {
                    // if (i == 0 && j == bus->stops.size() - 1) {
                    //     continue;
                    // }
                    size_t second_pos = std::distance(stops.begin(), std::find(stops.begin(), stops.end(), bus->stops[j]));
                    if (j == i + 1) {
                        current_distance += cat.GetRoadDistance(bus->stops[i]->name, bus->stops[j]->name).value();
                    } else {
                        current_distance += cat.GetRoadDistance(bus->stops[j-1]->name, bus->stops[j]->name).value();
                    }

                    g.AddEdge({first_pos * 2 + 1, second_pos * 2, current_distance / (router_settings.bus_velocity * 1000 / 60)});
                    edges_info.push_back({bus->name, j - i});
                }
            }
        }

    }
    graph::Router router(g);
    auto t = router.BuildRoute(2, 4).value();
    */
}