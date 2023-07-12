#include "graph.h"
#include "router.h"
#include "domain.h"
#include "transport_catalogue.h"

namespace catalogue::router {

using catalogue::domain::Bus;
using catalogue::domain::Stop;
using catalogue::domain::RoutingSettings;
using catalogue::TransportCatalogue;

struct EdgeInfo {
    std::string_view name;
    size_t span_count;
    double time;
};

struct RouteInfo {
    double total_time;
    std::vector<EdgeInfo> edges;
};

class TransportRouter {

public:
    TransportRouter(
        const std::vector<const Stop*>& stops,
        const std::vector<const Bus*>& buses,
        const TransportCatalogue& cat,
        RoutingSettings settings
    );

    std::optional<RouteInfo> BuildRoute(
        const std::string& from, const std::string& to
    ) const;

private:
    std::vector<EdgeInfo> edges_info_;
    std::vector<graph::EdgeId> edges_;

    const std::vector<const Stop*>& stops_;
    const std::vector<const Bus*>& buses_;
    const TransportCatalogue& cat_;
    RoutingSettings settings_;
    graph::DirectedWeightedGraph<double> graph_;
    const graph::Router<double> router_;

    graph::DirectedWeightedGraph<double> InitGraph();
};

}