#include <string>
#include<unordered_set>
#include<iostream>

#include"transport_catalogue.h"
#include"input_reader.h"
#include"stat_reader.h"

using namespace std;

int main() {
	int n = 0;
	cin >> n;
	auto result = input_reader::Load(n);

	transport_catalogue::TransportCatalogue database;
	for (input_reader::Stop& stop : result.stops) {
		transport_catalogue::detail::StopToAdd new_stop{ stop.name, stop.latitude, stop.longitude, {} };
		for (auto distance_to_stop : stop.distance_to_stop) {
			new_stop.distance_to_stop.push_back({ move(distance_to_stop.stop_name), distance_to_stop.distance });
		}
		database.AddStop(new_stop);
	}

	for (input_reader::Bus& bus : result.buses) {
		database.AddBus(bus.name, bus.stop_for_bus, bus.is_ring);
	}

	stat_reader::Processing(database);

}