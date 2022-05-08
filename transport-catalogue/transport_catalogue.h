#pragma once

#include <string>
#include <deque>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <functional>
#include <string_view>
#include <cmath>

#include"geo.h"

namespace transport_catalogue {

	namespace detail {

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
	}


	class TransportCatalogue {

		struct Stop {
			Stop(const std::string_view stop_name, const double stop_latitude, const double stop_longitude);
			std::string_view name;
			double latitude;
			double longitude;
		};

		struct Bus {
			Bus(const std::string_view bus_name, const bool is_ring);
			std::vector<Stop*> stop_for_bus_forward;
			std::string_view name;
			const bool is_ring;
			double distance_by_geo;
			double distance_by_road;
		};

		struct KeyPairStops {
			const Stop* stop_a;
			const Stop* stop_b;
			bool operator==(const KeyPairStops other) const;
		};

		struct HasherPairStopStop {
			size_t operator()(const KeyPairStops&) const;
		};

		std::unordered_set<std::string> string_names_;
		std::deque<Stop> stops_list_;
		std::deque<Bus> buses_list_;

		std::unordered_map<std::string_view, Bus*> buses_pointers_;
		std::unordered_map<std::string_view, Stop*> stops_pointers_;

		std::unordered_map<Stop*, std::set<std::string_view>> stop_to_buses_name_;

		std::unordered_set<std::string_view> stops_indefinite_buffer_;
		std::unordered_map<KeyPairStops, int, HasherPairStopStop> stop_to_stop_route_;

		std::string_view PlaceStringName(std::string& name);
		void AddStopToBus(Bus& bus, const std::vector<std::string>& stopsname);
		double DistanceByRoad(Stop* const stop_a, Stop* const stop_b);
		[[maybe_unused]] Stop* CheckIndefiniteStopBuffer(std::string_view sv_newstop, detail::StopToAdd& stop_to_add);

	public:
		TransportCatalogue() = default;
		void AddStop(detail::StopToAdd&);
		void AddBus(std::string& name, const std::vector<std::string>& stopsname, const bool is_ring);

		const detail::BusInfo BusInfo(const std::string_view) const;
		const detail::StopInfo BusesInStop(const std::string_view) const;
	};
}