#include "json_reader.h"
#include "json_builder.h"
#include <sstream>

namespace catalogue::reader {

using namespace std::literals;

inline const std::string BASE_REQUESTS = "base_requests";
inline const std::string STAT_REQUESTS = "stat_requests";
inline const std::string RENDER_SETTINGS = "render_settings";

JsonReader::JsonReader(std::istream& in, bool read_output_queries) {
    ReadDocument(in, read_output_queries);
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
        cat.AddBus(bus.name, bus.stops, bus.is_roundtrip);
    }
}

void JsonReader::PrintOutQueries(TransportCatalogue& cat, const requests::RequestHandler& handler, std::ostream& out) {
    json::Array to_print;
    for (const auto& query : out_queries_) {
        switch (query.type) {
            case OutQueryType::STOP:
                to_print.push_back(FormStopQuery(query, cat));
                break;
            case OutQueryType::BUS:
                to_print.push_back(FormBusQuery(query, cat));
                break;
            case OutQueryType::MAP:
                to_print.push_back(FormMapQuery(query, handler));
                break;
        }
    }
    json::Print(json::Document(to_print), out);
}

void JsonReader::ReadDocument(std::istream& in, bool read_output_queries) {
    json::Document doc = json::Load(in);

    if (!doc.GetRoot().IsDict()) {
        throw json::InvalidTypeError("Invalid json input.");
    }

    const auto root_map = doc.GetRoot().AsDict();
    if (root_map.count(BASE_REQUESTS) == 0 || root_map.count(STAT_REQUESTS) == 0) {
        throw json::InvalidTypeError("Invalid json input.");
    }

    ReadInputQueries(root_map.at(BASE_REQUESTS).AsArray());
    if (read_output_queries) {
        ReadOutputQueries(root_map.at(STAT_REQUESTS).AsArray());
    }
    ReadRenderSettings(root_map.at(RENDER_SETTINGS).AsDict());
}

void JsonReader::ReadInputQueries(const json::Array& in_queries) {
    for (const auto& query : in_queries) {
        std::string type = query.AsDict().at("type"s).AsString();
        if (type == "Stop"s) {
            AddStopQuery(query.AsDict());
        } else if (type == "Bus"s) {
            AddBusQuery(query.AsDict());
        }
    }
}

void JsonReader::ReadOutputQueries(const json::Array& out_queries) {
    for (const auto& query : out_queries) {
        int id = query.AsDict().at("id").AsInt();
        std::string type = query.AsDict().at("type"s).AsString();

        if (type == "Stop"s) {
            out_queries_.push_back(
                {id, OutQueryType::STOP, query.AsDict().at("name"s).AsString()}
            );
        } else if (type == "Bus"s) {
            out_queries_.push_back(
                {id, OutQueryType::BUS, query.AsDict().at("name"s).AsString()}
            );
        } else if (type == "Map"s) {
            out_queries_.push_back(
                {id, OutQueryType::MAP, {}}
            );
        }
    }
}

struct ColorSetter {
    svg::Color operator()(const json::Array& arr) {
        if (arr.size() == 3) {
            return svg::Rgb(
                arr[0].AsInt(),
                arr[1].AsInt(),
                arr[2].AsInt()
            );
        } else {
            return svg::Rgba(
                arr[0].AsInt(),
                arr[1].AsInt(),
                arr[2].AsInt(),
                arr[3].AsDouble()
            );
        }
    }

    svg::Color operator()(const std::string& name) {
        return name;
    }

    template <typename T>
    svg::Color operator()(const T&) {
        throw std::logic_error("Unexpected color type.");
    }
};

void JsonReader::ReadRenderSettings(const json::Dict& settings_dict) {
    render_settings_.width = settings_dict.at("width"s).AsDouble();
    render_settings_.heigth = settings_dict.at("height"s).AsDouble();
    render_settings_.padding = settings_dict.at("padding"s).AsDouble();
    render_settings_.line_width = settings_dict.at("line_width"s).AsDouble();
    render_settings_.stop_radius = settings_dict.at("stop_radius"s).AsDouble();
    render_settings_.bus_label_font_size = settings_dict.at("bus_label_font_size"s).AsInt();
    render_settings_.stop_label_font_size = settings_dict.at("stop_label_font_size"s).AsInt();

    std::vector<json::Node> offset = settings_dict.at("bus_label_offset"s).AsArray();
    render_settings_.bus_label_offset = svg::Point(offset[0].AsDouble(), offset[1].AsDouble());

    offset = settings_dict.at("stop_label_offset"s).AsArray();
    render_settings_.stop_label_offset = svg::Point(offset[0].AsDouble(), offset[1].AsDouble());

    render_settings_.underlayer_color = std::visit(ColorSetter(), settings_dict.at("underlayer_color"s).GetValue());
    render_settings_.underlayer_width = settings_dict.at("underlayer_width"s).AsDouble();

    for (const auto& color : settings_dict.at("color_palette").AsArray()) {
        render_settings_.color_palette.push_back(
            std::visit(ColorSetter(), color.GetValue())
        );
    }
}

renderer::Settings JsonReader::GetRenderSettings() const {
    return render_settings_;
}

void JsonReader::AddStopQuery(const json::Dict& query) {

    std::string name = query.at("name"s).AsString();
    double lat = query.at("latitude"s).AsDouble();
    double lon = query.at("longitude"s).AsDouble();

    stop_queries_.push_back({name, {lat, lon}});

    for (const auto& [stop_name, distance] : query.at("road_distances"s).AsDict()) {
        stop_distances_[name].insert({stop_name, distance.AsDouble()});
    }

}

void JsonReader::AddBusQuery(const json::Dict& query) {
    std::string name = query.at("name").AsString();
    std::vector<std::string> stops;
    bool is_roundtrip = true;
    for (const auto& stop_name : query.at("stops").AsArray()) {
        stops.push_back(stop_name.AsString());
    }

    if (!query.at("is_roundtrip").AsBool()) {
        std::vector<std::string> tmp{stops.rbegin() + 1, stops.rend()};
        stops.insert(stops.end(), tmp.begin(), tmp.end());
        is_roundtrip = false;
    }

    bus_queries_.push_back({name, stops, is_roundtrip});
}

json::Dict JsonReader::FormStopQuery(const OutQuery& query, const TransportCatalogue& cat) const {
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

json::Dict JsonReader::FormBusQuery(const OutQuery& query, TransportCatalogue& cat) const {
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

json::Dict JsonReader::FormMapQuery(
    const OutQuery& query,
    const requests::RequestHandler& handler
) const {
    svg::Document doc = handler.RenderMap();
    std::ostringstream ostr;

    doc.Render(ostr);

    return json::Dict{
        {"map"s, ostr.str()},
        {"request_id"s, query.id}
    };
}



}