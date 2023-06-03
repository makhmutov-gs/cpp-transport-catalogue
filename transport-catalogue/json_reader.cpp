#include "json_reader.h"

namespace catalogue::reader {

inline const std::string BASE_REQUESTS = "base_requests";
inline const std::string STAT_REQUESTS = "stat_requests";

JsonReader::JsonReader(std::istream& in) {
    ReadDocument(in);
}

void JsonReader::ProcessInQueries(TransportCatalogue& cat) {
    for (auto& stop : stop_queries_) {
        cat.AddStop(stop);
    }

    for (const auto& [from, distances_to_stops] : stop_distances_) {
        for (const auto& [to, distance] : distances_to_stops) {
            cat.SetRoadDistance(from, to, distance);
        }
    }

    for (auto& bus : bus_queries_) {
        cat.AddBus(bus.name, bus.stops);
    }
}

void JsonReader::PrintOutQueries(TransportCatalogue& cat, std::ostream& out) {
    json::Array to_print;
    for (const auto& query : out_queries_) {
        switch (query.type) {
            case OutQueryType::STOP:
                to_print.push_back(FormStopQuery(query, cat));
                break;
            case OutQueryType::BUS:
                to_print.push_back(FormBusQuery(query, cat));
                break;
        }
    }
    json::Print(json::Document(to_print), out);
}

void JsonReader::ReadDocument(std::istream& in) {
    json::Document doc = json::Load(in);

    if (!doc.GetRoot().IsMap()) {
        throw json::InvalidTypeError("Invalid json input.");
    }

    const auto root_map = doc.GetRoot().AsMap();
    if (root_map.count(BASE_REQUESTS) == 0 || root_map.count(STAT_REQUESTS) == 0) {
        throw json::InvalidTypeError("Invalid json input.");
    }

    ReadInputQueries(root_map.at(BASE_REQUESTS).AsArray());
    ReadOutputQueries(root_map.at(STAT_REQUESTS).AsArray());
}

void JsonReader::ReadInputQueries(const json::Array& in_queries) {
    using namespace std::literals;
    for (const auto& query : in_queries) {
        std::string type = query.AsMap().at("type"s).AsString();
        if (type == "Stop"s) {
            AddStopQuery(query.AsMap());
        } else if (type == "Bus"s) {
            AddBusQuery(query.AsMap());
        }
    }
}

void JsonReader::ReadOutputQueries(const json::Array& out_queries) {
    using namespace std::literals;
    for (const auto& query : out_queries) {
        int id = query.AsMap().at("id").AsInt();
        std::string type = query.AsMap().at("type"s).AsString();
        std::string name = query.AsMap().at("name"s).AsString();

        if (type == "Stop"s) {
            out_queries_.push_back({id, OutQueryType::STOP, name});
        } else if (type == "Bus"s) {
            out_queries_.push_back({id, OutQueryType::BUS, name});
        }
    }
}

void JsonReader::AddStopQuery(const json::Dict& query) {
    std::string name = query.at("name").AsString();
    double lat = query.at("latitude").AsDouble();
    double lon = query.at("longitude").AsDouble();

    stop_queries_.push_back({name, {lat, lon}});

    for (const auto& [stop_name, distance] : query.at("road_distances").AsMap()) {
        stop_distances_[name].insert({stop_name, distance.AsDouble()});
    }

}

void JsonReader::AddBusQuery(const json::Dict& query) {
    std::string name = query.at("name").AsString();
    std::vector<std::string> stops;
    for (const auto& stop_name : query.at("stops").AsArray()) {
        stops.push_back(stop_name.AsString());
    }

    if (!query.at("is_roundtrip").AsBool()) {
        std::vector<std::string> tmp{stops.rbegin() + 1, stops.rend()};
        stops.insert(stops.end(), tmp.begin(), tmp.end());
    }

    bus_queries_.push_back({name, stops});
}

json::Dict JsonReader::FormStopQuery(const OutQuery& query, const TransportCatalogue& cat) {
    using namespace std::literals;

    auto bus_list = cat.GetBusesByStop(query.name);
    if (bus_list) {
        std::set<std::string> buses_string;
        for (const auto bus_sv : *bus_list) {
            buses_string.insert(std::string(bus_sv));
        }

        json::Array buses{buses_string.begin(), buses_string.end()};
        return json::Dict{
            {"buses"s, buses},
            {"request_id"s, query.id}
        };
    } else {
        return json::Dict{
            {"error_message"s, "not found"s},
            {"request_id"s, query.id}
        };
    }
}

json::Dict JsonReader::FormBusQuery(const OutQuery& query, TransportCatalogue& cat) {
    using namespace std::literals;

    auto bus_info = cat.GetBusInfo(query.name);

    if (bus_info) {
        return json::Dict{
            {"curvature"s, bus_info->curvature},
            {"route_length"s, bus_info->road_length},
            {"stop_count"s, static_cast<int>(bus_info->stop_count)},
            {"unique_stop_count"s, static_cast<int>(bus_info->unique_stops)},
            {"request_id"s, query.id}
        };
    } else {
        return json::Dict{
            {"error_message"s, "not found"s},
            {"request_id"s, query.id}
        };
    }
}



}