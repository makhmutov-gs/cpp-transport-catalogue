#include <iostream>
#include "transport_catalogue.h"
#include "json_reader.h"

int main() {
    using namespace catalogue;
    using namespace catalogue::reader;

    TransportCatalogue cat;

    JsonReader reader(std::cin);

    reader.ProcessInQueries(cat);
    reader.PrintOutQueries(cat, std::cout);
}