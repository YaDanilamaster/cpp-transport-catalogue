#include "transport_catalogue.h"

using namespace transport_catalogue;

TransportCatalogue::Stop::Stop(const std::string_view stop_name, const double stop_latitude, const double stop_ongitude) :
	name(stop_name), latitude(stop_latitude), longitude(stop_ongitude)
{
}

TransportCatalogue::Bus::Bus(const std::string_view bus_name, const bool is_ring) :
	name(bus_name), is_ring(is_ring), distance_by_geo(0), distance_by_road(0)
{
}

std::string_view TransportCatalogue::PlaceStringName(std::string& name)
{
	auto [item, _] = string_names_.insert(std::move(name));
	return std::string_view(*item);
}

void TransportCatalogue::AddStopToBus(Bus& bus, const std::vector<std::string>& stopsname)
{
	size_t stop_count = stopsname.size();
	bus.stop_for_bus_forward.resize(bus.stop_for_bus_forward.size() + stop_count + (bus.is_ring ? 0 : stop_count - 1));

	size_t forward = 0;
	bool first = true;

	for (; forward < stop_count; forward++) {
		Stop* stop = stops_pointers_.find(stopsname[forward])->second;
		bus.stop_for_bus_forward[forward] = stop;

		Stop* stop_a = bus.stop_for_bus_forward[forward - (!first)];
		Stop* stop_b = bus.stop_for_bus_forward[forward];

		if (!first) {
			bus.distance_by_geo += geo::ComputeDistance(
				{ stop_a->latitude, stop_a->longitude },
				{ stop_b->latitude, stop_b->longitude }
			);
		}

		bus.distance_by_road += DistanceByRoad(stop_a, stop_b);

		first = false;
		stop_to_buses_name_[stop].insert(bus.name);
	}

	if (!bus.is_ring) {
		// Конечная, если маршрут не кольцевой.
		bus.distance_by_road += DistanceByRoad(bus.stop_for_bus_forward[forward - 1], bus.stop_for_bus_forward[forward - 1]);

		for (size_t backward = stop_count - 1; backward > 0; backward--, forward++) {
			bus.stop_for_bus_forward[forward] = stops_pointers_.find(stopsname[backward - 1])->second;

			Stop* stop_a = bus.stop_for_bus_forward[forward - 1];
			Stop* stop_b = bus.stop_for_bus_forward[forward];

			bus.distance_by_road += DistanceByRoad(stop_a, stop_b);
		}
		bus.distance_by_geo *= 2;
	}

}

double TransportCatalogue::DistanceByRoad(Stop* const stop_a, Stop* const stop_b)
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

// Проверяем остановку в реестре неопределенных остановок и корректируем координаты, при необходимости.
// Возвращает указатеь на ранее добавленную остановку.
TransportCatalogue::Stop* TransportCatalogue::CheckIndefiniteStopBuffer(std::string_view sv_newstop, detail::StopToAdd& stop_to_add)
{
	const auto it_newstop = stops_indefinite_buffer_.find(sv_newstop);
	if (it_newstop != stops_indefinite_buffer_.end()) {
		// Остановка ранее добавлялась и была помечена как неопределенная.
		// Нужно поправить координаты и удалить из реестра неопределенных.
		auto it_edit_stop = stops_pointers_.find(sv_newstop);
		it_edit_stop->second->latitude = stop_to_add.latitude;
		it_edit_stop->second->longitude = stop_to_add.longitude;

		stops_indefinite_buffer_.erase(it_newstop);
		return it_edit_stop->second;
	}
	return nullptr;
}

void TransportCatalogue::AddStop(detail::StopToAdd& stop_to_add)
{
	std::string_view sv_newstop = PlaceStringName(stop_to_add.stop_name);

	Stop* new_stop_pointer = CheckIndefiniteStopBuffer(sv_newstop, stop_to_add);
	if (new_stop_pointer == nullptr) {
		stops_list_.push_back(Stop(sv_newstop, stop_to_add.latitude, stop_to_add.longitude));
		new_stop_pointer = &stops_list_.back();
		stops_pointers_[sv_newstop] = new_stop_pointer;
	}

	if (stop_to_add.distance_to_stop.size() != 0) {
		// Указано расстояние до других остановок.
		for (detail::DistanceToStop& stop_b : stop_to_add.distance_to_stop) {
			std::string_view sv_stop_b = PlaceStringName(stop_b.stop_name);
			const auto it_stop_b = stops_pointers_.find(sv_stop_b);

			if (it_stop_b != stops_pointers_.end()) {
				// Остановка, до которой указано расстояние - ранее была добавлена.
				// Проверяем реестр неопределенных остановок.
				CheckIndefiniteStopBuffer(sv_stop_b, stop_to_add);

				stop_to_stop_route_[{new_stop_pointer, it_stop_b->second}] = stop_b.distance;
			}
			else {
				// Остановка, до которой указано расстояние - неопределена.
				// Сохраняем ее без координат и помещаем в реестр неопределенных остановок.
				stops_list_.push_back(Stop(sv_stop_b, 0, 0));
				stops_pointers_[sv_stop_b] = &stops_list_.back();

				stop_to_stop_route_[{new_stop_pointer, & stops_list_.back()}] = stop_b.distance;

				stops_indefinite_buffer_.insert(sv_stop_b);
			}
		}
	}

}

void TransportCatalogue::AddBus(std::string& name, const std::vector<std::string>& stopsname, const bool is_ring)
{
	std::string_view sv = PlaceStringName(name);
	Bus addbus(sv, is_ring);

	AddStopToBus(addbus, stopsname);

	buses_list_.push_back(std::move(addbus));
	buses_pointers_[sv] = &buses_list_.back();
}

const detail::BusInfo TransportCatalogue::BusInfo(const std::string_view bus_name) const
{
	auto it = buses_pointers_.find(bus_name);
	if (it != buses_pointers_.end()) {
		Bus* bus = buses_pointers_.find(bus_name)->second;
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

const detail::StopInfo TransportCatalogue::BusesInStop(const std::string_view stop_name) const
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

size_t TransportCatalogue::HasherPairStopStop::operator()(const KeyPairStops& key) const
{
	size_t h1 = std::hash<std::string_view>{}(key.stop_a->name);
	size_t h2 = std::hash<std::string_view>{}(key.stop_b->name);
	return h1 + 37 * h2;
}

bool TransportCatalogue::KeyPairStops::operator==(const KeyPairStops other) const
{
	return stop_a->name == other.stop_a->name && stop_b->name == other.stop_b->name;
}
