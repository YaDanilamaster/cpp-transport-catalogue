#include "input_reader.h"

using namespace std;

input_reader::Stop::Stop(std::string& stop_name, const double stop_latitude, const double stop_longitude) :
	name(stop_name), latitude(stop_latitude), longitude(stop_longitude)
{
}

input_reader::Bus::Bus(string& bus_name, const bool isring) :
	name(bus_name), is_ring(isring)
{
}

input_reader::Output input_reader::Load(const size_t count)
{
	Output result;
	string str_line;
	getline(cin, str_line);

	for (size_t i = 0; i < count; i++) {
		getline(cin, str_line);

		size_t pos_cmd;
		pos_cmd = str_line.find(' ');
		string cmd = str_line.substr(0, pos_cmd);

		if (cmd == "Stop") {
			result.stops.push_back(detail::ParseStop(str_line, pos_cmd + 1));
		}
		else if (cmd == "Bus") {
			result.buses.push_back(detail::ParseBus(str_line, pos_cmd + 1));
		}
	}

	return result;
}

input_reader::Stop input_reader::detail::ParseStop(std::string& line, const size_t start)
{
	size_t pos1, pos2, pos_name;
	pos_name = line.find(':');
	string name = line.substr(start, pos_name - start);

	pos1 = line.find(',', pos_name);
	double latitude = stod(line.substr(pos_name + 2, pos1 - (pos_name + 2)));
	pos2 = line.find(',', pos1 + 1);
	double longitude = stod(line.substr(pos1 + 2, pos2 - (pos1 + 2))); // ?

	Stop stop(name, latitude, longitude);

	while (pos2 != line.npos) {
		pos1 = line.find('m', pos2);
		string distance = line.substr(pos2 + 2, pos1 - (pos2 + 2));
		int dist = stoi(distance);

		pos2 = line.find(',', pos1 + 1);
		string stop_name = line.substr(pos1 + 5, pos2 - (pos1 + 5));
		stop.distance_to_stop.push_back(DistanceToStop{ move(stop_name), dist });
	}

	return stop;
}

input_reader::Bus input_reader::detail::ParseBus(std::string& line, const size_t start)
{
	size_t pos1, pos2, pos_name;
	pos_name = line.find(':');
	string name = line.substr(start, pos_name - start);

	pos1 = pos_name + 2;
	pos2 = line.find_first_of(">-", pos1);
	bool isring = line[pos2] == '>';
	input_reader::Bus result(name, isring);

	while (pos2 != line.npos) {
		result.stop_for_bus.push_back(line.substr(pos1, pos2 - (pos1 + 1)));
		pos1 = pos2 + 2;
		pos2 = line.find_first_of(">-", pos1);
	}
	result.stop_for_bus.push_back(line.substr(pos1));

	return result;
}
