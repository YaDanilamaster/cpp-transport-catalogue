#include "transport_catalogue.h"

using namespace transport_catalogue;

std::string_view TransportCatalogue::PlaceStringName(std::string& name)
{
	auto [item, _] = string_names_.insert(std::move(name));
	return std::string_view(*item);
}

void TransportCatalogue::AddStopToBusWithoutStat(Bus& bus, const std::vector<std::string>& stopsname)
{
	size_t stop_count = stopsname.size();
	if (stop_count == 0) {
		return;
	}
		bus.stop_for_bus_forward.resize(bus.stop_for_bus_forward.size() + stop_count + (bus.is_ring ? 0 : stop_count - 1));

	size_t forward = 0;

	for (; forward < stop_count; forward++) {
		auto it = stops_pointers_.find(stopsname[forward]);
		if (it != stops_pointers_.end()) {
			Stop* stop = it->second;
			bus.stop_for_bus_forward[forward] = stop;

			stop_to_buses_name_[stop].insert(bus.name);
		}
	}

	if (!bus.is_ring) {
		// Конечная, если маршрут не кольцевой.
		bus.stop_for_bus_forward[forward - 1]->isFinalStop = true;
		bus.distance_by_road += DistanceByRoad(bus.stop_for_bus_forward[forward - 1], bus.stop_for_bus_forward[forward - 1]);
		auto it = stops_pointers_.find(stopsname[forward - 1]);
		bus.secondFinalStop = it->second;

		// Обратный трекер
		for (size_t backward = stop_count - 1; backward > 0; backward--, forward++) {
			it = stops_pointers_.find(stopsname[backward - 1]);
			if (it != stops_pointers_.end()) {
				bus.stop_for_bus_forward[forward] = it->second;
			}
		}
	}

}

void TransportCatalogue::GetBusStatistic(Bus& bus) const {

	size_t stop_count = bus.stop_for_bus_forward.size();

	size_t forward = 0;
	bool first = true;

	for (; forward < stop_count; forward++) {

		Stop* stop_a = bus.stop_for_bus_forward[forward - (!first)];
		Stop* stop_b = bus.stop_for_bus_forward[forward];

		if (!first) {
			if (stop_a != nullptr && stop_b != nullptr) {
				bus.distance_by_geo += geo::ComputeDistance(
					{ stop_a->latitude, stop_a->longitude },
					{ stop_b->latitude, stop_b->longitude }
				);
			}
		}

		bus.distance_by_road += DistanceByRoad(stop_a, stop_b);

		first = false;
	}

}


double TransportCatalogue::DistanceByRoad(Stop* const stop_a, Stop* const stop_b) const
{
	auto it_stop_to_stop = stop_to_stop_route_.find({ stop_a, stop_b });
	if (it_stop_to_stop == stop_to_stop_route_.end()) {
		it_stop_to_stop = stop_to_stop_route_.find({ stop_b, stop_a });
		if (it_stop_to_stop == stop_to_stop_route_.end()) {
			return 0;
		}
	}
	return it_stop_to_stop->second;
}

void TransportCatalogue::AddStop(StopToAdd& stop_to_add)
{
	std::string_view sv_newstop = PlaceStringName(stop_to_add.stop_name);
	Stop* thisstop;

	auto it = stops_pointers_.find(sv_newstop);
	if (it != stops_pointers_.end()) {
		if (it->second->isRaw) {
			it->second->latitude = stop_to_add.latitude;
			it->second->longitude = stop_to_add.longitude;
			it->second->isRaw = false;
		}
		thisstop = it->second;
	}
	else {
		stops_list_.push_back(Stop(sv_newstop, stop_to_add.latitude, stop_to_add.longitude));
		thisstop = &stops_list_.back();

		thisstop->isRaw = false;
		stops_pointers_[sv_newstop] = thisstop;
	}


	for (DistanceToStop& stop_b : stop_to_add.distance_to_stop) {
		std::string_view sv_newstop_d = PlaceStringName(stop_b.stop_name);
		auto it_b = stops_pointers_.find(sv_newstop_d);
		if (it_b == stops_pointers_.end()) {
			stops_list_.push_back(Stop(sv_newstop_d, 0, 0));
			auto pointer_stop_b = &stops_list_.back();

			stops_pointers_[sv_newstop_d] = pointer_stop_b;
			// добавить дистанции между остановками
			stop_to_stop_route_[{thisstop, pointer_stop_b}] = stop_b.distance;
		}
		else
		{
			// добавить дистанции между остановками
			stop_to_stop_route_[{thisstop, it_b->second}] = stop_b.distance;
		}
	}

}

void TransportCatalogue::AddBus(std::string name, const std::vector<std::string>& stopsname, const bool is_ring)
{
	std::string_view sv = PlaceStringName(name);
	Bus addbus(sv, is_ring);

	AddStopToBusWithoutStat(addbus, stopsname);

	buses_list_.push_back(std::move(addbus));
	buses_pointers_[sv] = &buses_list_.back();

	GetBusStatistic(buses_list_.back());
}

const BusInfo TransportCatalogue::GetBusInfo(const std::string_view bus_name) const
{
	auto it = buses_pointers_.find(bus_name);
	if (it != buses_pointers_.end()) {
		Bus* bus = buses_pointers_.find(bus_name)->second;

		if (bus->distance_by_road < 0.001) {
			GetBusStatistic(*bus);
		}
		std::unordered_set<std::string_view> unique;
		for (const Stop* item : bus->stop_for_bus_forward) {
			unique.insert(item->name);
		}
		return {
			bus->name,
			bus->stop_for_bus_forward.size(),
			unique.size(),
			bus->distance_by_road,
			bus->distance_by_road / bus->distance_by_geo
		};

	}
	return {};
}

const StopInfo TransportCatalogue::GetBusesInStop(const std::string_view stop_name) const
{
	auto it = stops_pointers_.find(stop_name);
	if (it != stops_pointers_.end()) {
		auto it_buses = stop_to_buses_name_.find(it->second);
		if (it_buses != stop_to_buses_name_.end()) {
			return { &it_buses->second, true };
		}
		return { nullptr, true };
	}
	return { nullptr, false };
}

const std::map<std::string_view, Bus*> transport_catalogue::TransportCatalogue::GetAllBuses() const
{
	std::map<std::string_view, Bus*> result(buses_pointers_.begin(), buses_pointers_.end());
	return result;
}

const std::map<std::string_view, Stop*> transport_catalogue::TransportCatalogue::GetAllStops() const
{
	std::map<std::string_view, Stop*> result(stops_pointers_.begin(), stops_pointers_.end());
	return result;
}

size_t TransportCatalogue::HasherPairStopStop::operator()(const KeyPairStops& key) const
{
	if (key.stop_a != nullptr && key.stop_b != nullptr) {
		size_t h1 = std::hash<std::string_view>{}(key.stop_a->name);
		size_t h2 = std::hash<std::string_view>{}(key.stop_b->name);
		return h1 + 37 * h2;
	}
	return 0;
}

bool TransportCatalogue::KeyPairStops::operator==(const KeyPairStops other) const
{
	return stop_a->name == other.stop_a->name && stop_b->name == other.stop_b->name;
}
