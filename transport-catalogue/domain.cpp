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

}