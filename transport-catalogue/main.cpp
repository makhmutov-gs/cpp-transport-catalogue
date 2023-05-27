#include <iostream>
#include <sstream>
#include <fstream>
#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

int main() {
    using namespace catalogue;
    TransportCatalogue cat;
    InputReader reader(std::cin);

    reader.ProcessQueries(cat);

    StatReader statReader(std::cin, std::cout, cat);
}