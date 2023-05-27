#include <iostream>
#include <sstream>
#include <fstream>
#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

int main() {

    TransportCatalogue cat;
    InputReader reader;
    reader.Read(std::cin);

    for (auto& stop : reader.GetStopQueries()) {
        cat.AddStop(stop);
    }

    for (auto& stop_to_stop : reader.GetStopToStopDistances()) {
        cat.AddStopToStopDistance(stop_to_stop);
    }

    for (auto& bus : reader.GetBusQueries()) {
        cat.AddBus(bus);
    }

    StatReader statReader(std::cin, std::cout, cat);
}