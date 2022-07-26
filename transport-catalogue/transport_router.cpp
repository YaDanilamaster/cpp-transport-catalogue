#include "transport_router.h"

namespace transport_router {
	using namespace std;
	using namespace domain;

	transport_router::GraphBuilder::GraphBuilder(
		transport_catalogue::TransportCatalogue& db,
		domain::RoutingSettings& routing_sett)
		: db_(db), routing_settings_(routing_sett)
		, dwGraph_(db.StopsCount())
	{
		BuildGraph();
	}

	GraphBuilder::GraphBuilder(
		transport_catalogue::TransportCatalogue& db,
		domain::RoutingSettings& routing_sett,
		std::vector<graph::Edge<double>>&& edges,
		std::vector<std::vector<size_t>>&& incList
	)
		: db_(db)
		, routing_settings_(routing_sett)
		, dwGraph_(
			forward<std::vector<graph::Edge<double>>>(edges),
			forward<std::vector<std::vector<size_t>>>(incList)
		)
	{
		
	}

	graph::DirectedWeightedGraph<double>* GraphBuilder::GetGrahpPtr()
	{
		return &dwGraph_;
	}

	optional<Route> GraphBuilder::GetItemsFromRouteInfo(const std::optional<graph::Router<double>::RouteInfo>& routeInfo) const
	{
		if (!routeInfo) {
			return nullopt;
		}
		Route result;
		const size_t edges_size = routeInfo->edges.size();
		double total_time = routeInfo->weight;

		for (size_t edgeID = 0; edgeID < edges_size; edgeID++) {
			const graph::Edge edge = dwGraph_.GetEdge(routeInfo->edges[edgeID]);

			if (edge.count > 0) {
				result.route_items.push_back(RouteItem_Wait{ routing_settings_.bus_wait_time, db_.GetStopByID(edge.from).name});
				result.route_items.push_back(RouteItem_Bus{ edge.bus->name, edge.weight - routing_settings_.bus_wait_time, edge.count });
			}
			else {
				result.route_items.push_back(RouteItem_Wait{ routing_settings_.bus_wait_time, db_.GetStopByID(edge.from).name });
			}
		}
		result.total_time = total_time;
		if (result.total_time == 0) {
			result.route_items.push_back(RouteItem_NoWay{});
		}
		return result;
	}

	double GraphBuilder::TakeWeightEdge(domain::Stop* const stop_a, domain::Stop* const stop_b) const
	{
		const double distance = db_.GetDistanceByRoad(stop_a, stop_b) / 1000;
		return (distance / routing_settings_.bus_velocity) * TIME_SPAN;
	}

	// Рисует из маршрутов направленный взвешенный граф.
	void transport_router::GraphBuilder::BuildGraph()
	{
		const unordered_map<string_view, domain::Bus*>& allBuses = db_.GetAllBusesRef();
		for (const auto& [bus_name, bus_ptr] : allBuses) {
			if (!bus_ptr->is_ring) {
				DrawEdgeForSimpleRoute(bus_ptr);
			}
			else {
				DrawEdgeForRoundRoute(bus_ptr);
			}
		}
	}

	// Прокладываем ребра между вершинами прямого маршрута
	void GraphBuilder::DrawEdgeForSimpleRoute(domain::Bus* bus)
	{
		const vector<domain::Stop*> stops = bus->stop_for_bus_forward;
		const size_t stopsCount = stops.size();

		for (size_t i = 0; i < stopsCount - 1; i++) {
			double weightFrom = routing_settings_.bus_wait_time;
			double weightTo = weightFrom;
			int count = 0; // ++ будет быстрее, чем вычислять разницу между j и i

			for (size_t j = i + 1; j < stopsCount; j++) {
				weightFrom += TakeWeightEdge(stops[j - 1], stops[j]);
				weightTo += TakeWeightEdge(stops[j], stops[j - 1]);
				++count;

				dwGraph_.AddEdge({ stops[i]->id, stops[j]->id, weightFrom, count, bus });
				dwGraph_.AddEdge({ stops[j]->id, stops[i]->id, weightTo, count, bus });
			}
		}
		// Приехали. Дальше только пересадка
		dwGraph_.AddEdge({ stops[0]->id, stops[stopsCount - 1]->id, routing_settings_.bus_wait_time, 0, bus });
	}

	// Прокладываем ребра между вершинами кругового маршрута
	void GraphBuilder::DrawEdgeForRoundRoute(domain::Bus* bus)
	{
		const vector<domain::Stop*> stops = bus->stop_for_bus_forward;
		const size_t stopsCount = stops.size();

		for (size_t i = 0; i < stopsCount - 1; i++) {
			double weight = routing_settings_.bus_wait_time;
			int count = 0; // ++ будет быстрее, чем вычислять разницу j и i

			for (size_t j = i + 1; j < stopsCount; j++) {
				weight += TakeWeightEdge(stops[j - 1], stops[j]);
				++count;

				dwGraph_.AddEdge({ stops[i]->id, stops[j]->id, weight, count, bus });
			}
		}
		// Приехали. Дальше только пересадка
		dwGraph_.AddEdge({ stops[0]->id, stops[stopsCount - 1]->id, routing_settings_.bus_wait_time, 0, bus });
	}

	RouteHandler::RouteHandler(transport_catalogue::TransportCatalogue& db, domain::RoutingSettings& route_sett)
		: graph_builder_(db, route_sett), db_(db)
		, routing_settings_(route_sett)
		, router_(*graph_builder_.GetGrahpPtr())
	{
		
	}

	RouteHandler::RouteHandler(
		transport_catalogue::TransportCatalogue& db,
		domain::RoutingSettings& route_sett,
		GraphBuilder&& graphBuilder, 
		graph::Router<double>::RoutesInternalData&& routes_data
	)
		: db_(db)
		, routing_settings_(route_sett)
		, graph_builder_(forward<GraphBuilder>(graphBuilder))
		, router_(*graph_builder_.GetGrahpPtr(), forward<graph::Router<double>::RoutesInternalData>(routes_data))
	{
	}


	optional<Route> RouteHandler::BuildRoute(const std::string_view from_sv, const std::string_view to_sv) const
	{
		const domain::Stop* stop_from = db_.GetStopByName(from_sv);
		const domain::Stop* stop_to = db_.GetStopByName(to_sv);
		
		if (stop_from != nullptr && stop_to != nullptr) {
			return graph_builder_.GetItemsFromRouteInfo(router_.BuildRoute(stop_from->id, stop_to->id));
		}
		return nullopt;
	}

	graph::Router<double>* RouteHandler::GetRouterPtr()
	{
		return &router_;
	}

	graph::DirectedWeightedGraph<double>* RouteHandler::GetGrahpPtr()
	{
		return graph_builder_.GetGrahpPtr();
	}

} // namespace transport_router