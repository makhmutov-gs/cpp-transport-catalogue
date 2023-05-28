#include <iomanip>
#include "stat_reader.h"

namespace catalogue::output {

void StatReader::PrintQueries(TransportCatalogue& cat) {
    for (const auto& query : queries_) {
        switch (query.type) {
            case QueryType::BUS:
                PrintBusQuery(query.name, cat);
                break;
            case QueryType::STOP:
                PrintStopQuery(query.name, cat);
                break;
        }
    }
}

void StatReader::PrintBusQuery(const std::string& bus_name, TransportCatalogue& cat) {
    using namespace std::literals;

    auto bus_info = cat.GetBusInfo(bus_name);

    if (bus_info) {
        out_ << std::setprecision(6);
        out_ << "Bus "s << bus_name << ": "s
            << bus_info.value().stop_count << " stops on route, "s
            << bus_info.value().unique_stops << " unique stops, "s
            << bus_info.value().road_length << " route length, "s
            << bus_info.value().curvature << " curvature\n"s;
    } else {
        out_ << "Bus "s << bus_name << ": not found\n"s;
    }
}

void StatReader::PrintStopQuery(const std::string& stop_name, const TransportCatalogue& cat) {
    using namespace std::literals;

    auto bus_list = cat.GetBusesByStop(stop_name);
    if (bus_list) {
        if (bus_list.value().empty()) {
            out_ << "Stop "s << stop_name << ": no buses\n"s;
        } else {
            out_ << std::setprecision(6);
            out_ << "Stop "s << stop_name << ": buses"s;
            for (const auto& bus_name : bus_list.value()) {
                out_ << " "s << bus_name;
            }
            out_ << "\n";
        }
    } else {
        out_ << "Stop "s << stop_name << ": not found\n"s;
    }
}

void StatReader::AddQuery(const std::string& line, QueryType type) {
    std::string name = line.substr(line.find_first_not_of(' '));
    queries_.push_back({name, type});
}

}