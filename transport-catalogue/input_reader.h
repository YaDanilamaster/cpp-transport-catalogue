#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "transport_catalogue.h"

namespace input_reader {

	struct DistanceToStop {
		std::string stop_name;
		int distance;
	};

	struct Stop {
		Stop(std::string& stop_name, const double stop_latitude, const double stop_longitude);
		std::string name;
		const double latitude;
		const double longitude;
		std::vector<DistanceToStop> distance_to_stop;
	};

	struct Bus {
		Bus(std::string& bus_name, const bool isring);
		std::vector<std::string> stop_for_bus;
		std::string name;
		const bool is_ring;
	};

	struct StopBusBuffer {
		std::vector<Stop> stops;
		std::vector<Bus> buses;
	};

	transport_catalogue::TransportCatalogue Load(std::istream& is);

	namespace detail {
		Stop ParseStop(std::string& line, const size_t start);
		Bus ParseBus(std::string& line, const size_t start);
	}

}