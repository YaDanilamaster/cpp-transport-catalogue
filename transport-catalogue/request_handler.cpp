#include "request_handler.h"

namespace requestHandler {


	LoadRequestHandler::Stop::Stop(const std::string& stop_name, const double stop_latitude, const double stop_longitude)
		: name(stop_name), latitude(stop_latitude), longitude(stop_longitude)
	{
	}

	LoadRequestHandler::Bus::Bus(const std::string& bus_name, const bool isring) :
		name(bus_name), is_ring(isring)
	{
	}

	requestHandler::LoadRequestHandler::LoadRequestHandler(transport_catalogue::TransportCatalogue& db)
		: data_base_(db)
	{
	}

	const domain::BusInfo LoadRequestHandler::GetBusInfo(const std::string_view bus) const
	{
		return data_base_.GetBusInfo(bus);
	}

	const domain::StopInfo LoadRequestHandler::GetStopInfo(const std::string_view stop) const
	{
		return data_base_.GetBusesInStop(stop);
	}

	void requestHandler::LoadRequestHandler::ProcessRequestPool(const Request_pool& req_pool)
	{
		for (const LoadRequestHandler::Stop& stop : req_pool.stops) {
			domain::StopToAdd new_stop{ stop.name, stop.latitude, stop.longitude, {} };
			for (auto distance_to_stop : stop.distance_to_stop) {
				new_stop.distance_to_stop.push_back({ move(distance_to_stop.stop_name), distance_to_stop.distance });
			}
			data_base_.AddStop(new_stop);
		}

		for (const LoadRequestHandler::Bus& bus : req_pool.buses) {
			data_base_.AddBus(bus.name, bus.stop_for_bus, bus.is_ring);
		}
	}



	MapRequestHandler::MapRequestHandler(const transport_catalogue::TransportCatalogue& db, const renderer::SVG_Settings& svg_settings)
		: db_(db)
		, renderer_(svg_settings)
	{

	}

	svg::Document MapRequestHandler::RenderMap()
	{
		return renderer_.DrawMap(db_.GetAllBuses());
	}

} // namespace requestHandler