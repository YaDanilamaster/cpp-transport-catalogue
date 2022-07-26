#include <iostream>
#include <fstream>
#include <string_view>

#include "json_reader.h"


using namespace std;

void PrintUsage(std::ostream& stream = std::cerr) {
	stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		PrintUsage();
		return 1;
	}

	const std::string_view mode(argv[1]);

	transport_catalogue::TransportCatalogue db;
	jsonReader::JsonReader reader(db);

	if (mode == "make_base"sv) {
		// make_base: создание базы транспортного справочника по запросам base_requests и её сериализация в файл.
		reader.LoadJson(std::cin);

		// загружаем данные в базу
		reader.ProcessBaseRequests();

		// обрабатываем запросы к базе
		reader.ProcessSerialization();
	}
	else if (mode == "process_requests"sv) {
		// process_requests: десериализация базы из файла и использование её для ответов на запросы stat_requests.
		reader.LoadJson(std::cin);

		// загружаем информацию из бинарного файла
		reader.ProcessDeserialization();

		// обрабатываем запросы к базе
		reader.ProcessStatRequests(std::cout);
	}
	else {
		PrintUsage();
		return 1;
	}
}
