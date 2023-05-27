#pragma once
#include <iostream>
#include <iomanip>
#include <string>
#include "transport_catalogue.h"

using namespace std::literals;

template <typename StreamIn, typename StreamOut>
class StatReader {

public:

    enum class QueryType {
        Bus,
        Stop
    };

    StatReader(StreamIn& in, StreamOut& out, TransportCatalogue cat) : cat_(cat), out_(out) {
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

        for (const auto& query : queries_) {
            switch (query.second) {
                case QueryType::Bus:
                    ProcessBusQuery(query.first);
                    break;
                case QueryType::Stop:
                    ProcessStopQuery(query.first);
                    break;
            }

        }
    }

private:
    TransportCatalogue cat_;
    StreamOut& out_;

    std::vector<std::pair<std::string, QueryType>> queries_;

    void ProcessBusQuery(const std::string& bus_name) {
        auto bus = cat_.GetBusInfo(bus_name);
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

    void ProcessStopQuery(const std::string& stop_name) {
        auto bus_list = cat_.GetBusListByStop(stop_name);
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
        queries_.push_back({name, QueryType::Bus});
    }

    void AddStopQuery(const std::string& line) {
        std::string name = line.substr(line.find_first_not_of(' '));
         queries_.push_back({name, QueryType::Stop});
    }


};
