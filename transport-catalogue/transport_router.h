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



	// ���������� �������� �� ���������� ���������. ������� � ���� ���������� ���� (����� DirectedWeightedGraph) � ������������� (����� Router).
	class RouteHandler {
		/*������, ������ ��� �������� �����, ������� ������������ ����� ��������� ������ � ����.
		����������� � ���������� ����� ����� �������� ���������, � ��������� ������ ���������� ��� ��������������� ����������.
		������������� � ����� Router � ������� ������������� ������������ ���������� ������ ������ ������, �� ������ ������, ��������� ��� �������� ���� ���������.
		����������� �������������� ����� ��������� O(V^3 + E), ��� V � ���������� ������ �����, E � ���������� ����.
		������������� �� �������� � �������, �������� ���� �������������� ����.
		���������� �������� �� ������� �������������� ������� ������������ ���������� ���� � ��������.
		����� �������, �������� �������� ���������� ����������� ����� ������� �� ����������� ��������������.*/

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