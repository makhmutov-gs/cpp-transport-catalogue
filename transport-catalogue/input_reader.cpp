#include "input_reader.h"

namespace catalogue::input {

namespace detail {

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

}

void InputReader::ProcessQueries(TransportCatalogue& cat) {
    for (auto& stop : stop_queries_) {
        cat.AddStop(stop);
    }

    for (const auto& [stop, distances] : stop_distances_) {
        cat.AddStopDistances(stop, distances);
    }

    for (auto& bus : bus_queries_) {
        cat.AddBus(bus);
    }
}

void InputReader::AddStopQuery(const std::string& line) {
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
        stop_distances_[name].insert({stop_name, meters});
    }
}

void InputReader::AddBusQuery(const std::string& line) {
    using namespace std::literals;

    bool isStraight = true;
    std::string delim = "-"s;

    if (line.find(">"s) != std::string::npos) {
        isStraight = false;
        delim = ">"s;
    }

    size_t semicolon_pos = line.find(':');
    size_t name_begin = line.find_first_not_of(' ');
    std::string name = line.substr(name_begin, semicolon_pos - name_begin);
    std::vector<std::string> stops = detail::SplitStops(line.substr(semicolon_pos + 1), delim);

    size_t len = stops.size();

    if (isStraight) {
        for (size_t i = 0; i < len - 1; ++i) {
            stops.push_back(stops[len - 2 - i]);
        }
    }

    bus_queries_.push_back({name, stops});
}


}