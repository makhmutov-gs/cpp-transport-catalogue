#pragma once
#include "domain.h"
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"
#include <variant>

namespace catalogue::reader {

class JsonReader {
public:
    JsonReader(std::istream& in);

    void ProcessInQueries(TransportCatalogue& cat);
    void PrintOutQueries(
        TransportCatalogue& cat,
        const requests::RequestHandler& handler,
        std::ostream& out
    );
    renderer::Settings GetRenderSettings() const;
    domain::RoutingSettings GetRoutingSettings() const;
    std::string GetDbName() const;

private:
    struct BusQuery {
        std::string name;
        std::vector<std::string> stops;
        bool is_roundtrip = true;
    };

    enum class OutQueryType {
        BUS,
        STOP,
        MAP,
        ROUTE,
    };

    struct RouteQueryInfo {
        std::string from;
        std::string to;
    };

    struct OutQuery {
        int id;
        OutQueryType type;
        std::variant<std::string, RouteQueryInfo> payload;
    };

    std::string db_name_;

    std::vector<OutQuery> out_queries_;

    std::vector<Stop> stop_queries_;
    std::vector<BusQuery> bus_queries_;
    std::unordered_map<std::string, std::unordered_map<std::string, double>> stop_distances_;
    renderer::Settings render_settings_;
    domain::RoutingSettings routing_settings_;

    void ReadDocument(std::istream& in);
    void ReadInputQueries(const json::Array& in_queries);
    void ReadOutputQueries(const json::Array& out_queries);
    void ReadRenderSettings(const json::Dict& settings);
    void ReadRoutingSettings(const json::Dict& settings);
    void ReadSerializationSettings(const json::Dict& settings);

    void AddStopQuery(const json::Dict& query);
    void AddBusQuery(const json::Dict& query);

    json::Node FormStopQuery(const OutQuery& query, const TransportCatalogue& cat) const;
    json::Node FormBusQuery(const OutQuery& query, TransportCatalogue& cat) const;
    json::Node FormMapQuery(const OutQuery& query, const requests::RequestHandler& handler) const;
    json::Node FormRouteQuery(const OutQuery& query, const requests::RequestHandler& handler) const;
};

}
