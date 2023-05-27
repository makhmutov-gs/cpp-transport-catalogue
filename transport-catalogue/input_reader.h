#pragma once
#include <iostream>
#include <string>
#include "transport_catalogue.h"

namespace catalogue {

std::vector<std::string> SplitStops(std::string_view str, std::string delim);

using namespace std::literals;
class InputReader {

public:
    template <typename Stream>
    InputReader(Stream& in);

    template <typename Stream>
    void Read(Stream& in);

    void ProcessQueries(TransportCatalogue& cat);


private:
    std::vector<Stop> stop_queries_;
    std::vector<BusInput> bus_queries_;
    std::unordered_map<std::string, std::unordered_map<std::string, double>> stop_to_stop_distances_;

    void AddStopQuery(const std::string& line);

    void AddBusQuery(const std::string& line);
};

template <typename Stream>
InputReader::InputReader(Stream& in) {
    Read(in);
}

template <typename Stream>
void InputReader::Read(Stream& in) {
    size_t query_count;
    in >> query_count;

    for (size_t i = 0; i < query_count; ++i) {
        std::string operation;
        in >> operation;

        std::string line;
        std::getline(in, line);

        if (operation == "Stop"s) {
            AddStopQuery(line);
        }

        if (operation == "Bus"s) {
            AddBusQuery(line);
        }
    }
}

}