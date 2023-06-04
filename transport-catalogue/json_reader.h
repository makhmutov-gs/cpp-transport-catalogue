#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

namespace catalogue::reader {

class JsonReader {
public:
    JsonReader(std::istream& in, bool read_output_queries);

    void ProcessInQueries(TransportCatalogue& cat);
    void PrintOutQueries(TransportCatalogue& cat, std::ostream& out);
    renderer::Settings GetRenderSettings() const;

private:
    struct BusQuery {
        std::string name;
        std::vector<std::string> stops;
        bool is_roundtrip = true;
    };

    enum class OutQueryType {
        BUS,
        STOP
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

    json::Dict FormStopQuery(const OutQuery& query, const TransportCatalogue& cat);
    json::Dict FormBusQuery(const OutQuery& query, TransportCatalogue& cat);
};

}