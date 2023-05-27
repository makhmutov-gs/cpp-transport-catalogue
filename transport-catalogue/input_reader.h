#pragma once
#include <iostream>
#include <string>
#include "transport_catalogue.h"

namespace catalogue::input {

namespace detail {

std::vector<std::string> SplitStops(std::string_view str, std::string delim);

}

class InputReader {

public:
    template <typename StreamIn>
    InputReader(StreamIn& in);

    void ProcessQueries(TransportCatalogue& cat);

private:
    std::vector<Stop> stop_queries_;
    std::vector<BusInput> bus_queries_;
    std::unordered_map<std::string, std::unordered_map<std::string, double>> stop_distances_;

    template <typename StreamIn>
    void ReadQueries(StreamIn& in);

    void AddStopQuery(const std::string& line);

    void AddBusQuery(const std::string& line);
};

template <typename StreamIn>
InputReader::InputReader(StreamIn& in) {
    ReadQueries(in);
}

template <typename StreamIn>
void InputReader::ReadQueries(StreamIn& in) {
    using namespace std::literals;

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