#include <iostream>
#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

int main() {
    using namespace catalogue;

    TransportCatalogue cat;

    input::InputReader reader(std::cin);

    reader.ProcessQueries(cat);

    output::StatReader statReader(std::cin, std::cout);

    statReader.PrintQueries(cat);
}