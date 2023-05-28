#pragma once
#include <iostream>
#include <string>
#include "transport_catalogue.h"

namespace catalogue::output {

class StatReader {
public:
    template <typename StreamIn>
    StatReader(StreamIn& in, std::ostream& out);

    void PrintQueries(TransportCatalogue& cat);

private:
    enum class QueryType {
        BUS,
        STOP
    };

    struct Query {
        std::string name;
        QueryType type;
    };

    std::ostream& out_;

    std::vector<Query> queries_;

    template <typename StreamIn>
    void ReadQueries(StreamIn& in);

    void PrintBusQuery(const std::string& bus_name, TransportCatalogue& cat);

    void PrintStopQuery(const std::string& stop_name, const TransportCatalogue& cat);

    void AddQuery(const std::string& line, QueryType type);
};

template <typename StreamIn>
StatReader::StatReader(StreamIn& in, std::ostream& out) : out_(out) {
    ReadQueries(in);
}

template <typename StreamIn>
void StatReader::ReadQueries(StreamIn& in) {
    using namespace std::literals;

    size_t query_count;
    in >> query_count;

    for (size_t i = 0; i < query_count; ++i) {
        std::string operation;
        in >> operation;

        std::string line;
        std::getline(in, line);

        if (operation == "Bus"s) {
            AddQuery(line, QueryType::BUS);
        }

        if (operation == "Stop"s) {
            AddQuery(line, QueryType::STOP);
        }
    }
}

}