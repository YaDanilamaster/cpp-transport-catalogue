#pragma once
#include<vector>
#include<unordered_map>
#include<map>
#include<string_view>
#include<optional>
#include<variant>
#include<algorithm>
#include<memory>

#include <iostream>

#include "domain.h"
#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"

namespace transport_router {
	
	using Items = std::variant<domain::RouteItem_Wait, domain::RouteItem_Bus, domain::RouteItem_NoWay>;

	struct Route {
		std::vector<Items> route_items;
		double total_time = 0;
	};

	class GraphBuilder 
	{
	public:
		GraphBuilder(transport_catalogue::TransportCatalogue&, domain::RoutingSettings&);
		const graph::DirectedWeightedGraph<double>& GetGrahp() const;
		std::optional<Route> GetItemsFromRouteInfo(const std::optional<graph::Router<double>::RouteInfo>& routeInfo) const;
		
		void PrintDiagnosticInfo() const;

	private:
		const int TIME_SPAN = 60;

		transport_catalogue::TransportCatalogue& db_;
		domain::RoutingSettings& routing_settings_;
		graph::DirectedWeightedGraph<double> dwGraph_;

		double TakeWeightEdge(domain::Stop* const stop_a, domain::Stop* const stop_b) const;
		void BuildGraph();

		void DrawEdgeForSimpleRoute(domain::Bus* bus);
		void DrawEdgeForRoundRoute(domain::Bus* bus);

	};



	// Обработчик запросов на построение маршрутов. Включае в себя взвешенный граф (класс DirectedWeightedGraph) и маршрутизатор (класс Router).
	class RouteHandler {
		/*Память, нужная для хранения графа, линейна относительно суммы количеств вершин и рёбер.
		Конструктор и деструктор графа имеют линейную сложность, а остальные методы константны или амортизированно константны.
		Маршрутизатор — класс Router — требует квадратичного относительно количества вершин объёма памяти, не считая памяти, требуемой для хранения кэша маршрутов.
		Конструктор маршрутизатора имеет сложность O(V^3 + E), где V — количество вершин графа, E — количество рёбер.
		Маршрутизатор не работает с графами, имеющими рёбра отрицательного веса.
		Построение маршрута на готовом маршрутизаторе линейно относительно количества рёбер в маршруте.
		Таким образом, основная нагрузка построения оптимальных путей ложится на конструктор маршрутизатора.*/

	public:
		RouteHandler(transport_catalogue::TransportCatalogue&, domain::RoutingSettings&);

		std::optional<Route> BuildRoute(const std::string_view from, const std::string_view to) const;

	private:
		GraphBuilder graph_builder_;
		std::unique_ptr<graph::Router<double>> router_ptr_;
		transport_catalogue::TransportCatalogue& db_;
		domain::RoutingSettings& routing_settings_;
	};

} // namespace transport_router