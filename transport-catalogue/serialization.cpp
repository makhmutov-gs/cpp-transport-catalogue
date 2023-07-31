#include "serialization.h"
#include <fstream>

namespace catalogue::serialization {

void SaveWithSettings(
    const std::filesystem::path& path,
    const TransportCatalogue& cat,
    const renderer::Settings& render_settings,
    const domain::RoutingSettings& routing_settings
) {
    std::ofstream out_file(path, std::ios::binary);

    proto_transport::RoutesInfo proto_routes_info;
    std::unordered_map<const Stop*, int32_t> stop_to_id;

    const auto& stops = cat.GetStops();

    int32_t id = 0;
    for (const auto& s : stops) {
        stop_to_id[&s] = id;
        *proto_routes_info.add_stop() = StopToProto(s, id);
        ++id;
    }

    for (const auto& s_from : stops) {
        int32_t from = stop_to_id[&s_from];
        for (const auto& s_to : stops) {
            if (&s_from == &s_to) {
                continue;
            }
            int32_t to = stop_to_id[&s_to];
            std::optional<double> meters = cat.GetRoadDistance(s_from.name, s_to.name);

            if (!meters.has_value()) {
                continue;
            }

            proto_transport::Distance proto_distance;
            proto_distance.set_from(from);
            proto_distance.set_to(to);
            proto_distance.set_meters(*meters);

            *proto_routes_info.add_distance() = proto_distance;
        }
    }

    for (const auto& b : cat.GetBuses()) {
        proto_transport::Bus proto_bus;
        proto_bus.set_name(b.name);
        proto_bus.set_is_roundtrip(b.is_roundtrip);

        for (const auto s_ptr : b.stops) {
            id = stop_to_id[s_ptr];
            proto_bus.add_stop(id);
        }

        *proto_routes_info.add_bus() = proto_bus;
    }

    proto_transport::TransportCatalogue proto_cat;

    *proto_cat.mutable_routes_info() = proto_routes_info;
    *proto_cat.mutable_render_settings() = RendererSettingsToProto(render_settings);
    *proto_cat.mutable_routing_settings() = RoutingSettingsToProto(routing_settings);

    proto_cat.SerializeToOstream(&out_file);
}

std::tuple<TransportCatalogue, renderer::Settings, domain::RoutingSettings> FromFile(const std::filesystem::path& path) {
    std::ifstream in_file(path, std::ios::binary);

    proto_transport::TransportCatalogue proto_cat;
    TransportCatalogue cat;

    proto_cat.ParseFromIstream(&in_file);

    proto_transport::RoutesInfo proto_routes_info = *proto_cat.mutable_routes_info();

    std::unordered_map<int32_t, std::string> id_to_stopname;
    for (int i = 0; i < proto_routes_info.stop_size(); ++i) {
        proto_transport::Stop proto_stop = *proto_routes_info.mutable_stop(i);

        cat.AddStop(Stop{proto_stop.name(), {proto_stop.coords().lat(), proto_stop.coords().lng()}});
        id_to_stopname[proto_stop.id()] = proto_stop.name();
    }

    for (int i = 0; i < proto_routes_info.distance_size(); ++i) {
        proto_transport::Distance proto_distance = *proto_routes_info.mutable_distance(i);
        cat.SetRoadDistance(
            id_to_stopname[proto_distance.from()],
            id_to_stopname[proto_distance.to()],
            proto_distance.meters()
        );
    }

    for (int i = 0; i < proto_routes_info.bus_size(); ++i) {
        proto_transport::Bus proto_bus = *proto_routes_info.mutable_bus(i);

        std::vector<std::string> stops;
        for (int j = 0; j < proto_bus.stop_size(); ++j) {
            int id = proto_bus.stop(j);
            stops.push_back(id_to_stopname[id]);
        }
        cat.AddBus(proto_bus.name(), stops, proto_bus.is_roundtrip());
    }



    return {
        std::move(cat),
        RendererSettingFromProto(proto_cat.render_settings()),
        RoutingSettingsFromProto(proto_cat.routing_settings())
    };
}

proto_transport::Stop StopToProto(const Stop& s, int32_t id) {
    proto_transport::Stop proto_stop;
    proto_stop.set_name(s.name);
    proto_stop.set_id(id);

    proto_transport::Coordinates proto_coords;
    proto_coords.set_lat(s.coords.lat);
    proto_coords.set_lng(s.coords.lng);

    *proto_stop.mutable_coords() = proto_coords;

    return proto_stop;
}

proto_router::RoutingSettings RoutingSettingsToProto(const domain::RoutingSettings& settings) {
    proto_router::RoutingSettings proto_routing_settings;
    proto_routing_settings.set_bus_wait_time(settings.bus_wait_time);
    proto_routing_settings.set_bus_velocity(settings.bus_velocity);
    return proto_routing_settings;
}

proto_render::Color ProtoColorGetter::operator()(std::monostate) {
    proto_render::Color proto_color;
    proto_color.set_type(proto_render::Color_TYPE::Color_TYPE_NONE);
    return proto_color;
}

proto_render::Color ProtoColorGetter::operator()(const std::string& str) {
    proto_render::Color proto_color;
    proto_color.set_type(proto_render::Color_TYPE::Color_TYPE_STRING);
    proto_color.set_name(str);
    return proto_color;
}

proto_render::Color ProtoColorGetter::operator()(const svg::Rgb& rgb) {
    proto_render::Color proto_color;
    proto_color.set_type(proto_render::Color_TYPE::Color_TYPE_RGB);
    proto_color.set_red(rgb.red);
    proto_color.set_green(rgb.green);
    proto_color.set_blue(rgb.blue);
    return proto_color;
}

proto_render::Color ProtoColorGetter::operator()(const svg::Rgba& rgba) {
    proto_render::Color proto_color;
    proto_color.set_type(proto_render::Color_TYPE::Color_TYPE_RGBA);
    proto_color.set_red(rgba.red);
    proto_color.set_green(rgba.green);
    proto_color.set_blue(rgba.blue);
    proto_color.set_opacity(rgba.opacity);
    return proto_color;
}

svg::Color GetColorFromProto(const proto_render::Color& proto_color) {
    switch (proto_color.type()) {
        case proto_render::Color_TYPE::Color_TYPE_NONE:
            return std::monostate();
            break;
        case proto_render::Color_TYPE::Color_TYPE_STRING:
            return proto_color.name();
            break;
        case proto_render::Color_TYPE::Color_TYPE_RGB:
            return svg::Rgb(proto_color.red(), proto_color.green(), proto_color.blue());
            break;
        case proto_render::Color_TYPE::Color_TYPE_RGBA:
            return svg::Rgba(proto_color.red(), proto_color.green(), proto_color.blue(), proto_color.opacity());
            break;
    }
}

domain::RoutingSettings RoutingSettingsFromProto(const proto_router::RoutingSettings& proto_settings) {
    return {
         proto_settings.bus_wait_time(),
         proto_settings.bus_velocity()
    };
}

proto_render::RendererSettings RendererSettingsToProto(const renderer::Settings& settings) {
    proto_render::RendererSettings proto_render_settings;
    proto_render_settings.set_width(settings.width);
    proto_render_settings.set_height(settings.heigth);
    proto_render_settings.set_padding(settings.padding);
    proto_render_settings.set_line_width(settings.line_width);
    proto_render_settings.set_stop_radius(settings.stop_radius);
    proto_render_settings.set_bus_label_font_size(settings.bus_label_font_size);

    proto_render::Point proto_bus_label_offset;
    proto_bus_label_offset.set_x(settings.bus_label_offset.x);
    proto_bus_label_offset.set_y(settings.bus_label_offset.y);

    *proto_render_settings.mutable_bus_label_offset() = proto_bus_label_offset;

    proto_render::Point proto_stop_label_offset;
    proto_stop_label_offset.set_x(settings.stop_label_offset.x);
    proto_stop_label_offset.set_y(settings.stop_label_offset.y);

    *proto_render_settings.mutable_stop_label_offset() = proto_stop_label_offset;

    proto_render_settings.set_stop_label_font_size(settings.stop_label_font_size);

    *proto_render_settings.mutable_underlayer_color() = std::visit(ProtoColorGetter(), settings.underlayer_color);

    proto_render_settings.set_underlayer_width(settings.underlayer_width);

    for (const auto& c : settings.color_palette) {
        *proto_render_settings.add_color_palette() = std::visit(ProtoColorGetter(), c);
    }

    return proto_render_settings;
}

renderer::Settings RendererSettingFromProto(const proto_render::RendererSettings& proto_settings) {
    renderer::Settings render_settings;
    render_settings.width = proto_settings.width();
    render_settings.heigth = proto_settings.height();
    render_settings.padding = proto_settings.padding();
    render_settings.line_width = proto_settings.line_width();
    render_settings.stop_radius = proto_settings.stop_radius();
    render_settings.bus_label_font_size = proto_settings.bus_label_font_size();

    proto_render::Point proto_bus_label_offset = proto_settings.bus_label_offset();
    render_settings.bus_label_offset.x = proto_bus_label_offset.x();
    render_settings.bus_label_offset.y = proto_bus_label_offset.y();

    proto_render::Point proto_stop_label_offset = proto_settings.stop_label_offset();
    render_settings.stop_label_offset.x = proto_stop_label_offset.x();
    render_settings.stop_label_offset.y = proto_stop_label_offset.y();

    render_settings.stop_label_font_size = proto_settings.stop_label_font_size();

    render_settings.underlayer_color = GetColorFromProto(proto_settings.underlayer_color());
    render_settings.underlayer_width = proto_settings.underlayer_width();

    for (int i = 0; i < proto_settings.color_palette_size(); ++i) {
        render_settings.color_palette.push_back(GetColorFromProto(proto_settings.color_palette(i)));
    }

    return render_settings;
}

}