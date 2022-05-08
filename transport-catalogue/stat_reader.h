#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <set>

#include"transport_catalogue.h"

namespace stat_reader {

	std::ostream& operator<<(std::ostream& os, const transport_catalogue::detail::BusInfo& info);

	void Processing(const transport_catalogue::TransportCatalogue& database);
	void PrintBusInfo(const transport_catalogue::TransportCatalogue& database);
	void PrintStopInfo(const transport_catalogue::TransportCatalogue& database);
}