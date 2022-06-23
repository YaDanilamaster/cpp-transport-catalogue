#pragma once

#include <string_view>
#include <set>
#include <vector>
#include <string>
#include <unordered_map>

namespace domain {

	struct Stop {
		Stop(const std::string_view stop_name, const double stop_latitude, const double stop_longitude);
		std::string_view name;
		double latitude;
		double longitude;
		bool isRaw;
		bool isFinalStop;
		size_t id;
	};

	struct Bus {
		Bus(const std::string_view bus_name, const bool is_ring);
		std::vector<Stop*> stop_for_bus_forward;
		std::string_view name;
		Stop* secondFinalStop;
		const bool is_ring;
		double distance_by_geo;
		double distance_by_road;
		size_t id;
	};

	struct BusInfo {
		std::string_view name;
		size_t stop_count;
		size_t unique_stop_count;
		double lenght;
		double curvature;
	};

	struct StopInfo {
		const std::set<std::string_view>* const buses_on_stop;
		bool stop_found;
	};

	struct DistanceToStop {
		std::string stop_name;
		int distance;
	};

	struct StopToAdd {
		std::string stop_name;
		double latitude;
		double longitude;
		std::vector<DistanceToStop> distance_to_stop;
	};

	// Transport router items

	struct RoutingSettings {
		double bus_wait_time;
		int bus_velocity;
	};

	struct RouteItem_Wait {
		double time;
		std::string_view stop_name;
	};
	struct RouteItem_Bus {
		std::string_view bus_name;
		double time = 0;
		int span_count = 0;
	};
	struct RouteItem_NoWay {
	};

}