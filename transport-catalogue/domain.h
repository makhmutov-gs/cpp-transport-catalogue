#pragma once
#include <string>
#include <vector>
#include "geo.h"

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

}