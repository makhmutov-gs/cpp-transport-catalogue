#pragma once
#include <iostream>
#include <iomanip>
#include <string>
#include "transport_catalogue.h"

namespace catalogue {

template <typename StreamOut>
class StatReader {
public:
    template <typename StreamIn>
    StatReader(StreamIn& in, StreamOut& out) : out_(out) {
        ReadQueries(in);
    }

    void ProcessQueries(const TransportCatalogue& cat) {
        for (const auto& query : queries_) {
            switch (query.second) {
                case QueryType::BUS:
                    ProcessBusQuery(query.first, cat);
                    break;
                case QueryType::STOP:
                    ProcessStopQuery(query.first, cat);
                    break;
            }
        }
    }

private:
    enum class QueryType {
        BUS,
        STOP
    };

    StreamOut& out_;

    std::vector<std::pair<std::string, QueryType>> queries_;

    template <typename StreamIn>
    void ReadQueries(StreamIn& in) {
        using namespace std::literals;

        size_t query_count;
        in >> query_count;

        for (size_t i = 0; i < query_count; ++i) {
            std::string operation;
            in >> operation;

            std::string line;
            std::getline(in, line);

            if (operation == "Bus"s) {
                AddBusQuery(line);
            }

            if (operation == "Stop"s) {
                AddStopQuery(line);
            }
        }
    }

    void ProcessBusQuery(const std::string& bus_name, const TransportCatalogue& cat) {
        using namespace std::literals;

        auto bus = cat.GetBus(bus_name);
        if (bus) {
            out_ << std::setprecision(6);
            out_ << "Bus "s << bus->name << ": "s
                << bus->stops.size() << " stops on route, "s
                << bus->unique_stops << " unique stops, "s
                << bus->road_length << " route length, "s
                << bus->curvature << " curvature\n"s;
        } else {
            out_ << "Bus "s << bus_name << ": not found\n"s;
        }
    }

    void ProcessStopQuery(const std::string& stop_name, const TransportCatalogue& cat) {
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

    void AddBusQuery(const std::string& line) {
        std::string name = line.substr(line.find_first_not_of(' '));
        queries_.push_back({name, QueryType::BUS});
    }

    void AddStopQuery(const std::string& line) {
        std::string name = line.substr(line.find_first_not_of(' '));
         queries_.push_back({name, QueryType::STOP});
    }

};

}