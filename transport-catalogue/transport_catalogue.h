#pragma once

#include <string>
#include <deque>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <map>
#include <functional>
#include <string_view>
#include <cmath>

#include"geo.h"
#include"domain.h"

namespace transport_catalogue {
	using namespace domain;

	class TransportCatalogue {

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

		std::unordered_map<KeyPairStops, int, HasherPairStopStop> stop_to_stop_route_;

		std::string_view PlaceStringName(std::string& name);
		void AddStopToBusWithoutStat(Bus& bus, const std::vector<std::string>& stopsname);
		void GetBusStatistic(Bus& bus) const;

		double DistanceByRoad(Stop* const stop_a, Stop* const stop_b) const;

	public:
		TransportCatalogue() = default;
		void AddStop(StopToAdd&);
		void AddBus(std::string name, const std::vector<std::string>& stopsname, const bool is_ring);

		const BusInfo GetBusInfo(const std::string_view) const;
		const StopInfo GetBusesInStop(const std::string_view) const;

		const std::map<std::string_view, Bus*> GetAllBuses() const;
		const std::map<std::string_view, Stop*> GetAllStops() const;
	};
}