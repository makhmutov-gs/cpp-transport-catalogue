#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"

namespace catalogue::reader {

class JsonReader {
public:
    JsonReader(std::istream& in, bool read_output_queries=true);

    void ProcessInQueries(TransportCatalogue& cat);
    void PrintOutQueries(
        TransportCatalogue& cat,
        const requests::RequestHandler& handler,
        std::ostream& out
    );
    renderer::Settings GetRenderSettings() const;

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
    };

    struct OutQuery {
        int id;
        OutQueryType type;
        std::string name;
    };

    std::vector<OutQuery> out_queries_;

    std::vector<Stop> stop_queries_;
    std::vector<BusQuery> bus_queries_;
    std::unordered_map<std::string, std::unordered_map<std::string, double>> stop_distances_;
    renderer::Settings render_settings_;

    void ReadDocument(std::istream& in, bool read_output_queries);
    void ReadInputQueries(const json::Array& in_queries);
    void ReadOutputQueries(const json::Array& out_queries);
    void ReadRenderSettings(const json::Dict& settings);

    void AddStopQuery(const json::Dict& query);
    void AddBusQuery(const json::Dict& query);

    json::Node FormStopQuery(const OutQuery& query, const TransportCatalogue& cat) const;
    json::Node FormBusQuery(const OutQuery& query, TransportCatalogue& cat) const;
    json::Node FormMapQuery(const OutQuery& query, const requests::RequestHandler& handler) const;
};

}
