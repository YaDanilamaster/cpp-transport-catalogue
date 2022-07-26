#include "transport_catalogue.h"

using namespace transport_catalogue;
using namespace domain;

std::string_view TransportCatalogue::PlaceStringName(std::string& name)
{
	auto [item, isNew] = string_names_.insert({ std::move(name), stringCount_ });
	stringCount_ += isNew;
	return std::string_view(item->first);
}

void TransportCatalogue::AddStopToBusWithoutStat(Bus& bus, const std::vector<std::string>& stopsname)
{
	size_t stop_count = stopsname.size();
	if (stop_count == 0) {
		return;
	}
	bus.stop_for_bus_forward.resize(bus.stop_for_bus_forward.size() + stop_count + (bus.is_ring ? 0 : stop_count - 1));
	Stop* stop = nullptr;
	size_t forward = 0;
	for (; forward < stop_count; forward++) {
		auto it = stops_pointers_.find(stopsname[forward]);
		if (it != stops_pointers_.end()) {
			stop = it->second;
			bus.stop_for_bus_forward[forward] = stop;

			stop_to_buses_name_[stop].insert(bus.name);
			//++stopVertexCoutn_;
		}
	}


	if (!bus.is_ring) {
		bus.stop_for_bus_forward[forward - 1]->isFinalStop = true;
		bus.distance_by_road += GetDistanceByRoad(bus.stop_for_bus_forward[forward - 1], bus.stop_for_bus_forward[forward - 1]);
		auto it = stops_pointers_.find(stopsname[forward - 1]);
		bus.secondFinalStop = it->second;

		for (size_t backward = stop_count - 1; backward > 0; backward--, forward++) {
			it = stops_pointers_.find(stopsname[backward - 1]);
			if (it != stops_pointers_.end()) {
				bus.stop_for_bus_forward[forward] = it->second;
				//++stopVertexCoutn_;
			}
		}
	}
	else {
		stop->isFinalStop = true;
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
		bus.distance_by_road += GetDistanceByRoad(stop_a, stop_b);
		first = false;
	}

}


double TransportCatalogue::GetDistanceByRoad(Stop* const stop_a, Stop* const stop_b) const
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

const std::vector<StopToStopDistance> transport_catalogue::TransportCatalogue::GetAllStopToStopDistance() const
{
	std::vector<StopToStopDistance> result;
	result.reserve(stop_to_stop_route_.size());
	for (auto& item : stop_to_stop_route_) {
		result.emplace_back(item.first.stop_a->id, item.first.stop_b->id, item.second);
	}
	return result;
}

void transport_catalogue::TransportCatalogue::InsertStopToStopDistance(const StopToStopDistance& stops)
{
	stop_to_stop_route_[{
		&stops_list_[stops.stop_a],
		&stops_list_[stops.stop_b]}] = stops.distance;
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
		thisstop->id = stops_list_.size() - 1;

		thisstop->isRaw = false;
		stops_pointers_[sv_newstop] = thisstop;
	}


	for (DistanceToStop& stop_b : stop_to_add.distance_to_stop) {
		std::string_view sv_newstop_d = PlaceStringName(stop_b.stop_name);
		auto it_b = stops_pointers_.find(sv_newstop_d);
		if (it_b == stops_pointers_.end()) {
			stops_list_.push_back(Stop(sv_newstop_d, 0, 0));
			auto pointer_stop_b = &stops_list_.back();
			pointer_stop_b->id = stops_list_.size() - 1;

			stops_pointers_[sv_newstop_d] = pointer_stop_b;
			stop_to_stop_route_[{thisstop, pointer_stop_b}] = stop_b.distance;
		}
		else
		{
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
	Bus& newBus = buses_list_.back();
	newBus.id = buses_list_.size() - 1;
	buses_pointers_[sv] = &newBus;
	//roundBusCount_ += is_ring;

	GetBusStatistic(newBus);
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

const Stop* transport_catalogue::TransportCatalogue::GetStopByName(const std::string_view stop_name) const
{
	auto it = stops_pointers_.find(stop_name);
	if (it != stops_pointers_.end()) {
		return it->second;
	}
	return nullptr;
}

const std::map<std::string_view, Bus*> transport_catalogue::TransportCatalogue::GetAllBuses() const
{
	std::map<std::string_view, Bus*> result(buses_pointers_.begin(), buses_pointers_.end());
	return result;
}

const std::unordered_map<std::string_view, Bus*>& transport_catalogue::TransportCatalogue::GetAllBusesRef() const
{
	return buses_pointers_;
}

const std::deque<Bus>& transport_catalogue::TransportCatalogue::GetBusesList() const
{
	return buses_list_;
}

domain::Bus* transport_catalogue::TransportCatalogue::MutableBusById(size_t busId)
{
	return &buses_list_[busId];
}

void transport_catalogue::TransportCatalogue::InsertBus(Bus&& bus)
{
	buses_list_.push_back(std::move(bus));
	Bus& ref = buses_list_.back();
	buses_pointers_[ref.name] = &ref;

	for (Stop* stop : ref.stop_for_bus_forward) {
		stop_to_buses_name_[stop].insert(ref.name);
	}
}

const std::map<std::string_view, Stop*> transport_catalogue::TransportCatalogue::GetAllStops() const
{
	std::map<std::string_view, Stop*> result(stops_pointers_.begin(), stops_pointers_.end());
	return result;
}

const std::deque<Stop>& transport_catalogue::TransportCatalogue::GetStopsList() const
{
	return stops_list_;
}

void transport_catalogue::TransportCatalogue::InsertStop(Stop&& stop)
{
	stops_list_.push_back(std::move(stop));
	Stop& ref = stops_list_.back();
	stops_pointers_[ref.name] = &ref;
}

std::string_view TransportCatalogue::InsertString(std::string&& string, size_t string_id)
{
	auto [item, isNew] = string_names_.insert({ std::move(string), string_id });
	stringCount_ += isNew;
	return std::string_view(item->first);
}

const std::unordered_map<std::string, size_t>& transport_catalogue::TransportCatalogue::GetAllStrings() const
{
	return string_names_;
}

const Stop& transport_catalogue::TransportCatalogue::GetStopByID(size_t id) const
{
	return stops_list_.at(id);
}

Stop* transport_catalogue::TransportCatalogue::MutableStopById(size_t stop_id)
{
	return &stops_list_[stop_id];
}

size_t transport_catalogue::TransportCatalogue::StopsCount() const
{
	return stops_list_.size();
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