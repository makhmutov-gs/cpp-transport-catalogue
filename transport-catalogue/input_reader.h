#pragma once
#include <iostream>
#include <string>
#include "transport_catalogue.h"


std::vector<std::string> SplitStops(std::string_view str, std::string delim) {
    std::vector<std::string> result;
    while (true) {
        const auto delim_pos = str.find(delim);
        const auto last_character = str.find_last_not_of(' ', delim_pos - 1);
        const auto word = str.substr(str.find_first_not_of(' '), last_character);
        if (word != delim) {
            result.push_back(std::string(word));
        }
        if (delim_pos == str.npos) {
            break;
        }
        else {
            str.remove_prefix(delim_pos + 1);
        }
    }
    return result;
}
using namespace std::literals;
class InputReader {

public:
    template <typename Stream>
    void Read(Stream& in) {
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

    std::vector<Stop> GetStopQueries() const {
        return stop_queries_;
    }

    std::vector<BusInput> GetBusQueries() const {
        return bus_queries_;
    }

    auto GetStopToStopDistances() const {
        return stop_to_stop_distances_;
    }


private:
    std::vector<Stop> stop_queries_;
    std::vector<BusInput> bus_queries_;
    std::unordered_map<std::string, std::unordered_map<std::string, double>> stop_to_stop_distances_;

    void AddStopQuery(const std::string& line) {
        size_t semicolon_pos = line.find(':');
        size_t name_begin = line.find_first_not_of(' ');


        std::string name = line.substr(name_begin, semicolon_pos - name_begin);

        size_t comma_pos = line.find(',');
        size_t lat_begin = line.find_first_not_of(' ', semicolon_pos + 1);
        size_t comma_after = line.find(',', comma_pos + 1);

        std::string lat_str = line.substr(lat_begin, comma_pos - lat_begin);
        std::string lon_str = line.substr(comma_pos + 1, comma_after - comma_pos - 1);

        double lat = std::stod(lat_str);
        double lon = std::stod(lon_str);

        stop_queries_.push_back({name, lat, lon});

        while (comma_after != std::string::npos) {
            size_t m_pos = line.find('m', comma_after);
            double meters = std::stod(line.substr(comma_after + 1, m_pos - comma_after - 1));
            size_t to_pos = line.find("to", m_pos);

            comma_after = line.find(',', to_pos);
            size_t stop_name_begin = line.find_first_not_of(' ', to_pos + 3);
            std::string stop_name = line.substr(stop_name_begin, comma_after - stop_name_begin);
            stop_to_stop_distances_[name].insert({stop_name, meters});
        }

    }

    void AddBusQuery(const std::string& line) {
        bool isStraight = true;
        std::string delim = "-"s;

        if (line.find(">"s) != std::string::npos) {
            isStraight = false;
            delim = ">"s;
        }

        size_t semicolon_pos = line.find(':');
        size_t name_begin = line.find_first_not_of(' ');
        std::string name = line.substr(name_begin, semicolon_pos - name_begin);
        std::vector<std::string> stops = SplitStops(line.substr(semicolon_pos + 1), delim);

        size_t len = stops.size();

        if (isStraight) {
            for (size_t i = 0; i < len - 1; ++i) {
                stops.push_back(stops[len - 2 - i]);
            }
        }

        bus_queries_.push_back({name, stops});
    }


};
