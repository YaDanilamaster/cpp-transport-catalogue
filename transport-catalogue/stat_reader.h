#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <set>

#include"transport_catalogue.h"

namespace stat_reader {

	std::ostream& operator<<(std::ostream& os, const transport_catalogue::detail::BusInfo& info);

	void HandleRequests(const transport_catalogue::TransportCatalogue& database, std::istream& is, std::ostream& os);
	void PrintBusInfo(const transport_catalogue::TransportCatalogue& database, std::ostream& os);
	void PrintStopInfo(const transport_catalogue::TransportCatalogue& database, std::ostream& os);
}