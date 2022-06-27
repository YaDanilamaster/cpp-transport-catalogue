#include <iostream>

#include "json_reader.h"
#include "windows.h"
#include "psapi.h"


using namespace std;

void MemInfo() {
	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;
	SIZE_T physMemUsedByMe = pmc.WorkingSetSize;
	cerr << "virtualMemUsedByMe "s << pmc.PrivateUsage << '\n';
	cerr << "physMemUsedByMe "s << pmc.WorkingSetSize << '\n';
	cerr << "MemInfo"s << '\n';
}

int main() {

	transport_catalogue::TransportCatalogue db;
	jsonReader::JsonReader reader(db);
	reader.LoadJson(std::cin);
	// загружаем данные в базу
	reader.ProcessBaseRequests();

	// обрабатываем запросы к базе
	reader.ProcessStatRequests(std::cout);
	MemInfo();
	return 0;
}