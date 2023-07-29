#include "domain.h"

namespace catalogue::domain {

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

}