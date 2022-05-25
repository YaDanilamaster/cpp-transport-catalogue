#include "domain.h"

namespace domain {

	Stop::Stop(const std::string_view stop_name, const double stop_latitude, const double stop_ongitude) :
		name(stop_name), latitude(stop_latitude), longitude(stop_ongitude), isRaw(true), isFinalStop(false)
	{
	}

	Bus::Bus(const std::string_view bus_name, const bool is_ring) 
		: name(bus_name)
		, secondFinalStop(nullptr)
		, is_ring(is_ring)
		, distance_by_geo(0)
		, distance_by_road(0)
	{
	}


}