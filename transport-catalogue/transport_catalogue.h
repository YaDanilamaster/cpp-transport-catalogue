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
#include <memory>

#include"geo.h"
#include"domain.h"

namespace transport_catalogue {

	class TransportCatalogue {

		struct KeyPairStops {
			const domain::Stop* stop_a;
			const domain::Stop* stop_b;
			bool operator==(const KeyPairStops other) const;
		};

		struct HasherPairStopStop {
			size_t operator()(const KeyPairStops&) const;
		};

		std::unordered_map<std::string, size_t> string_names_;
		std::deque<domain::Stop> stops_list_;
		std::deque<domain::Bus> buses_list_;

		std::unordered_map<std::string_view, domain::Bus*> buses_pointers_;
		std::unordered_map<std::string_view, domain::Stop*> stops_pointers_;
		std::unordered_map<domain::Stop*, std::set<std::string_view>> stop_to_buses_name_;
		std::unordered_map<KeyPairStops, int, HasherPairStopStop> stop_to_stop_route_;

		std::string_view PlaceStringName(std::string& name);

		//size_t stopVertexCoutn_ = 0;
		//size_t roundBusCount_ = 0;
		size_t stringCount_ = 0;

		void AddStopToBusWithoutStat(domain::Bus& bus, const std::vector<std::string>& stopsname);
		void GetBusStatistic(domain::Bus& bus) const;


	public:
		TransportCatalogue() = default;
		void AddStop(domain::StopToAdd&);
		void AddBus(std::string name, const std::vector<std::string>& stopsname, const bool is_ring);

		const domain::BusInfo GetBusInfo(const std::string_view) const;
		const domain::StopInfo GetBusesInStop(const std::string_view) const;
		const domain::Stop* GetStopByName(const std::string_view) const;

		double GetDistanceByRoad(domain::Stop* const stop_a, domain::Stop* const stop_b) const;
		const std::vector<domain::StopToStopDistance> GetAllStopToStopDistance() const;
		void InsertStopToStopDistance(const domain::StopToStopDistance&);

		const std::map<std::string_view, domain::Bus*> GetAllBuses() const;
		const std::unordered_map<std::string_view, domain::Bus*>& GetAllBusesRef() const;
		const std::deque<domain::Bus>& GetBusesList() const;
		domain::Bus* MutableBusById(size_t);
		void InsertBus(domain::Bus&& bus);

		const std::map<std::string_view, domain::Stop*> GetAllStops() const;
		const std::deque<domain::Stop>& GetStopsList() const;
		const domain::Stop& GetStopByID(size_t id) const;
		domain::Stop* MutableStopById(size_t);
		size_t StopsCount() const;
		void InsertStop(domain::Stop&& stop);

		std::string_view InsertString(std::string&& string, size_t id);

		const std::unordered_map<std::string, size_t>& GetAllStrings() const;

	};
}