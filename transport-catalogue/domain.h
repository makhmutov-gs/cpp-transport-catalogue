#pragma once
#include <string>
#include <vector>
#include "geo.h"
#include "svg.h"

#include <transport_catalogue.pb.h>

namespace catalogue::domain {

struct Stop {
    std::string name;
    geo::Coordinates coords;
};

struct Bus {
    std::string name;
    std::vector<const Stop*> stops;
    bool is_roundtrip;
};

struct RoutingSettings {
    int bus_wait_time;
    double bus_velocity;
};

svg::Color GetColorFromProto(const proto_render::Color& proto_color);

struct ProtoColorGetter {
    proto_render::Color operator()(std::monostate);
    proto_render::Color operator()(const std::string& str);
    proto_render::Color operator()(const svg::Rgb& rgb);
    proto_render::Color operator()(const svg::Rgba& rgba);
};

}