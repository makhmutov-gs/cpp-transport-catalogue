#pragma once
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace catalogue::requests {

class RequestHandler {
public:
    RequestHandler(
        const TransportCatalogue& cat,
        renderer::MapRenderer& renderer,
        domain::RoutingSettings routing_settings
    );

    svg::Document RenderMap() const;

    std::optional<catalogue::router::TransportRouter::RouteInfo> FormRoute(
        const std::string& from, const std::string& to
    ) const;

private:
    const TransportCatalogue& cat_;
    const std::vector<const Stop*> sorted_stops_;
    const std::vector<const Bus*> sorted_buses_;
    renderer::MapRenderer& renderer_;
    const catalogue::router::TransportRouter transport_router_;

    std::vector<const Stop*> GetSortedStops() const;
    std::vector<const Bus*> GetSortedBuses() const;
};

}
