#include "domain.h"

namespace catalogue::domain {

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

proto_router::RoutingSettings RoutingSettingsToProto(const RoutingSettings& settings) {
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


}