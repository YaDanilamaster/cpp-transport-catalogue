#include "json_reader.h"

int main() {

	transport_catalogue::TransportCatalogue db;
	jsonReader::JsonReader reader(db);
	reader.LoadJson(std::cin);
	reader.ProcessBaseRequests();

	reader.ProcessStatRequests(std::cout);

	return 0;
}