#include "transport_catalogue.h"
#include "svg.h"
#include <fstream>
#include <transport_catalogue.pb.h>

namespace catalogue {

void TransportCatalogue::AddStop(const Stop& stop) {
    stops_.push_back(std::move(stop));
    stopname_to_stop_[stops_.back().name] = &stops_.back();
    stop_to_buses_[&stops_.back()] = {};
}

void TransportCatalogue::SetRoadDistance(
    const std::string& from,
    const std::string& to,
    double distance
) {
    road_distances_[
        {stopname_to_stop_[from], stopname_to_stop_[to]}
    ] = distance;
}

std::optional<double> TransportCatalogue::GetRoadDistance(
    const std::string& from,
    const std::string& to
) const {
    if (stopname_to_stop_.count(from) == 0 || stopname_to_stop_.count(to) == 0) {
        return std::nullopt;
    }
    auto p = std::pair{stopname_to_stop_.at(from), stopname_to_stop_.at(to)};
    if (road_distances_.count(p) == 1) {
        return road_distances_.at(p);
    } else if (road_distances_.count({p.second, p.first})){
        return road_distances_.at({p.second, p.first});
    }
    return std::nullopt;
}

void TransportCatalogue::AddBus(
    const std::string& name,
    const std::vector<std::string>& stops,
    bool is_roundtrip
) {
    Bus bus;
    bus.name = std::move(name);
    bus.is_roundtrip = is_roundtrip;

    for (const auto& stop_name : stops) {
        bus.stops.push_back(stopname_to_stop_[stop_name]);
    }

    buses_.push_back(std::move(bus));

    for (const auto& stop : buses_.back().stops) {
        stop_to_buses_[stop].insert(buses_.back().name);
    }

    busname_to_bus_[buses_.back().name] = &buses_.back();
}

std::optional<BusInfo> TransportCatalogue::GetBusInfo(const std::string& name) {
    if (busname_to_bus_.count(name) == 0) {
        return std::nullopt;
    }

    const Bus* bus = busname_to_bus_.at(name);

    size_t unique_stops = std::set(bus->stops.begin(), bus->stops.end()).size();
    double road_length = CalcRoadRouteLength(bus->stops);
    double curvature = road_length / CalcGeoRouteLength(bus->stops);

    return BusInfo{
        bus->stops.size(),
        unique_stops,
        road_length,
        curvature
    };
}

std::optional<std::set<std::string_view>> TransportCatalogue::GetBusesByStop(const std::string& name) const {
    if (stopname_to_stop_.count(name) == 0) {
        return std::nullopt;
    }

    return stop_to_buses_.at(stopname_to_stop_.at(name));
}

double TransportCatalogue::CalcGeoRouteLength(const std::vector<const Stop*>& stops) {
    double result = 0;
    for (size_t i = 0; i < stops.size() - 1; ++i) {
        auto stop_pair = std::pair{stops[i], stops[i+1]};

        if (geo_distances_.count(stop_pair) == 0) {
            geo_distances_[stop_pair] = geo::ComputeDistance(
                {stops[i]->coords},
                {stops[i+1]->coords}
            );
        }
        result += geo_distances_.at(stop_pair);
    }
    return result;
}

double TransportCatalogue::CalcRoadRouteLength(const std::vector<const Stop*>& stops) const {
    double result = 0;
    for (size_t i = 0; i < stops.size() - 1; ++i) {
        auto stop_pair = std::pair{stops[i], stops[i+1]};

        if (road_distances_.count(stop_pair) != 0) {
            result += road_distances_.at(stop_pair);
        } else {
            result += road_distances_.at({stop_pair.second, stop_pair.first});
        }

    }
    return result;
}

struct ProtoColorGetter {
    proto_render::Color operator()(std::monostate) {
        proto_render::Color proto_color;
        proto_color.set_type(proto_render::Color_TYPE::Color_TYPE_NONE);
        return proto_color;
    }

    proto_render::Color operator()(const std::string& str) {
        proto_render::Color proto_color;
        proto_color.set_type(proto_render::Color_TYPE::Color_TYPE_STRING);
        proto_color.set_name(str);
        return proto_color;
    }

    proto_render::Color operator()(const svg::Rgb& rgb) {
        proto_render::Color proto_color;
        proto_color.set_type(proto_render::Color_TYPE::Color_TYPE_RGB);
        proto_color.set_red(rgb.red);
        proto_color.set_green(rgb.green);
        proto_color.set_blue(rgb.blue);
        return proto_color;
    }

    proto_render::Color operator()(const svg::Rgba& rgba) {
        proto_render::Color proto_color;
        proto_color.set_type(proto_render::Color_TYPE::Color_TYPE_RGBA);
        proto_color.set_red(rgba.red);
        proto_color.set_green(rgba.green);
        proto_color.set_blue(rgba.blue);
        proto_color.set_opacity(rgba.opacity);
        return proto_color;
    }
};

void TransportCatalogue::SaveWithSettings(
    const std::filesystem::path& path,
    const renderer::Settings& render_settings,
    const domain::RoutingSettings& routing_settings
) const {
    std::ofstream out_file(path, std::ios::binary);

    proto_transport::TransportCatalogue proto_cat;
    std::unordered_map<const Stop*, size_t> stop_to_id;
    size_t id = 0;
    for (const auto& s : stops_) {
        stop_to_id[&s] = id;

        proto_transport::Stop proto_stop;
        proto_stop.set_name(s.name);
        proto_stop.set_id(static_cast<int32_t>(id));

        proto_transport::Coordinates proto_coords;
        proto_coords.set_lat(s.coords.lat);
        proto_coords.set_lng(s.coords.lng);

        *proto_stop.mutable_coords() = proto_coords;

        *proto_cat.add_stop() = proto_stop;

        ++id;
    }

    for (const auto& s_from : stops_) {
        int32_t from = static_cast<int32_t>(stop_to_id[&s_from]);
        for (const auto& s_to : stops_) {
            if (&s_from == &s_to) {
                continue;
            }
            int32_t to = static_cast<int32_t>(stop_to_id[&s_to]);
            std::optional<double> meters = GetRoadDistance(s_from.name, s_to.name);

            if (meters.has_value()) {
                proto_transport::Distance proto_distance;
                proto_distance.set_from(from);
                proto_distance.set_to(to);
                proto_distance.set_meters(*meters);

                *proto_cat.add_distance() = proto_distance;
            }
        }
    }

    for (const auto& b : buses_) {
        proto_transport::Bus proto_bus;
        proto_bus.set_name(b.name);
        proto_bus.set_is_roundtrip(b.is_roundtrip);

        for (const auto s_ptr : b.stops) {
            id = stop_to_id[s_ptr];
            proto_bus.add_stop(static_cast<int32_t>(id));
        }

        *proto_cat.add_bus() = proto_bus;
    }

    proto_render::RendererSettings proto_render_settings;
    proto_render_settings.set_width(render_settings.width);
    proto_render_settings.set_height(render_settings.heigth);
    proto_render_settings.set_padding(render_settings.padding);
    proto_render_settings.set_line_width(render_settings.line_width);
    proto_render_settings.set_stop_radius(render_settings.stop_radius);
    proto_render_settings.set_bus_label_font_size(render_settings.bus_label_font_size);

    proto_render::Point bus_label_offset;
    bus_label_offset.set_x(render_settings.bus_label_offset.x);
    bus_label_offset.set_y(render_settings.bus_label_offset.y);

    *proto_render_settings.mutable_bus_label_offset() = bus_label_offset;

    proto_render::Point stop_label_offset;
    stop_label_offset.set_x(render_settings.stop_label_offset.x);
    stop_label_offset.set_y(render_settings.stop_label_offset.y);

    *proto_render_settings.mutable_stop_label_offset() = stop_label_offset;

    proto_render_settings.set_stop_label_font_size(render_settings.stop_label_font_size);

    *proto_render_settings.mutable_underlayer_color() = std::visit(ProtoColorGetter(), render_settings.underlayer_color);

    proto_render_settings.set_underlayer_width(render_settings.underlayer_width);

    for (const auto& c : render_settings.color_palette) {
        *proto_render_settings.add_color_palette() = std::visit(ProtoColorGetter(), c);
    }

    proto_router::RoutingSettings proto_routing_settings;
    proto_routing_settings.set_bus_wait_time(routing_settings.bus_wait_time);
    proto_routing_settings.set_bus_velocity(routing_settings.bus_velocity);

    *proto_cat.mutable_render_settings() = proto_render_settings;
    *proto_cat.mutable_routing_settings() = proto_routing_settings;

    proto_cat.SerializeToOstream(&out_file);
}



std::tuple<TransportCatalogue, renderer::Settings, domain::RoutingSettings> FromFile(const std::filesystem::path& path) {
    std::ifstream in_file(path, std::ios::binary);

    proto_transport::TransportCatalogue proto_cat;
    TransportCatalogue cat;

    proto_cat.ParseFromIstream(&in_file);

    std::unordered_map<int32_t, std::string> id_to_stopname;
    for (int i = 0; i < proto_cat.stop_size(); ++i) {
        proto_transport::Stop proto_stop = *proto_cat.mutable_stop(i);

        cat.AddStop(Stop{proto_stop.name(), {proto_stop.coords().lat(), proto_stop.coords().lng()}});
        id_to_stopname[proto_stop.id()] = proto_stop.name();
    }

    for (int i = 0; i < proto_cat.distance_size(); ++i) {
        proto_transport::Distance proto_distance = *proto_cat.mutable_distance(i);
        cat.SetRoadDistance(
            id_to_stopname[proto_distance.from()],
            id_to_stopname[proto_distance.to()],
            proto_distance.meters()
        );
    }

    for (int i = 0; i < proto_cat.bus_size(); ++i) {
        proto_transport::Bus proto_bus = *proto_cat.mutable_bus(i);

        std::vector<std::string> stops;
        for (int j = 0; j < proto_bus.stop_size(); ++j) {
            int id = proto_bus.stop(j);
            stops.push_back(id_to_stopname[id]);
        }
        cat.AddBus(proto_bus.name(), stops, proto_bus.is_roundtrip());
    }

    renderer::Settings render_settings;
    proto_render::RendererSettings proto_render_settings = proto_cat.render_settings();
    render_settings.width = proto_render_settings.width();
    render_settings.heigth = proto_render_settings.height();
    render_settings.padding = proto_render_settings.padding();
    render_settings.line_width = proto_render_settings.line_width();
    render_settings.stop_radius = proto_render_settings.stop_radius();
    render_settings.bus_label_font_size = proto_render_settings.bus_label_font_size();

    proto_render::Point bus_label_offset = proto_render_settings.bus_label_offset();
    render_settings.bus_label_offset.x = bus_label_offset.x();
    render_settings.bus_label_offset.y = bus_label_offset.y();

    proto_render::Point stop_label_offset = proto_render_settings.stop_label_offset();
    render_settings.stop_label_offset.x = stop_label_offset.x();
    render_settings.stop_label_offset.y = stop_label_offset.y();

    render_settings.stop_label_font_size = proto_render_settings.stop_label_font_size();

    render_settings.underlayer_color = domain::GetColorFromProto(proto_render_settings.underlayer_color());
    render_settings.underlayer_width = proto_render_settings.underlayer_width();

    for (int i = 0; i < proto_render_settings.color_palette_size(); ++i) {
        render_settings.color_palette.push_back(domain::GetColorFromProto(proto_render_settings.color_palette(i)));
    }

    domain::RoutingSettings routing_settings;
    proto_router::RoutingSettings proto_routing_settings = proto_cat.routing_settings();
    routing_settings.bus_wait_time = proto_routing_settings.bus_wait_time();
    routing_settings.bus_velocity = proto_routing_settings.bus_velocity();

    return {std::move(cat), render_settings, routing_settings};
}



}