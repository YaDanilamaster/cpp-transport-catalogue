#include "stat_reader.h"

using namespace std;
using namespace transport_catalogue;

ostream& stat_reader::operator<<(ostream& os, const detail::BusInfo& info)
{
	cout << "Bus " << info.name << ": ";
	cout << info.stop_count << " stops on route, ";
	cout << info.unique_stop_count << " unique stops, ";
	cout << info.lenght << " route length, ";
	cout << info.curvature << " curvature";
	return os;
}

void stat_reader::HandleRequests(const TransportCatalogue& database, istream& is, ostream& os)
{
	int n;
	is >> n;
	string str_cmd;
	getline(is, str_cmd);

	for (size_t i = 0; i < n; i++) {
		cin >> str_cmd;

		if (str_cmd == "Bus") {
			PrintBusInfo(database, os);
		}
		else if (str_cmd == "Stop") {
			PrintStopInfo(database, os);
		}

	}

}

void stat_reader::PrintBusInfo(const TransportCatalogue& database, ostream& os)
{
	string str_line;
	getline(cin, str_line);
	str_line = str_line.substr(1);

	detail::BusInfo bus_info = database.BusInfo(str_line);

	if (bus_info.stop_count != 0) {
		cout << bus_info << endl;
	}
	else {
		cout << "Bus " << str_line << ": not found" << endl;
	}
}

void stat_reader::PrintStopInfo(const TransportCatalogue& database, ostream& os)
{
	string str_line;
	getline(cin, str_line);
	str_line = str_line.substr(1);

	const detail::StopInfo buses = database.BusesInStop(str_line);

	cout << "Stop " << str_line << ": ";

	if (buses.stop_found) {
		if (buses.buses_on_stop != nullptr) {
			cout << "buses";
			for (const string_view sv : *buses.buses_on_stop) {
				cout << " " << sv;
			}
		}
		else {
			cout << "no buses";
		}
	}
	else {
		cout << "not found";
	}
	cout << endl;
}