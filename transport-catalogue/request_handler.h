#pragma once


// ����� RequestHandler ������ ���� ������, ����������� �������������� JSON reader-�
// � ������� ������������ ����������.
// ��. ������� �������������� �����: https://ru.wikipedia.org/wiki/�����_(������_��������������)

#include <optional>
#include <string>

#include "transport_catalogue.h"
#include "domain.h"
#include "svg.h"
#include "map_renderer.h"
#include "transport_router.h"

namespace requestHandler {

	class MapRequestHandler {
	public:
		// MapRenderer ����������� � ��������� ����� ��������� �������
		MapRequestHandler(const transport_catalogue::TransportCatalogue& db, const renderer::SVG_Settings& svg_settings);

		// ���� ����� ����� ����� � ��������� ����� ��������� �������
		svg::Document RenderMap();

	private:
		// RequestHandler ���������� ��������� �������� "������������ ����������" � "������������ �����"
		const transport_catalogue::TransportCatalogue& db_;
		renderer::MapRenderer renderer_;
	};

	// ������� �����, ��������������� ������������� ��������� ��� �������� ���������� � ����������.
	// ������ �������� (json, xml, yaml � ������) ����������� �� ����. 
	//
	//			LoadRequestHandler
	//				/
	//		   JsonReader
	//
	class LoadRequestHandler {
	public:
		struct DistanceToStop {
			std::string stop_name;
			int distance;
		};

		struct Stop {
			Stop(const std::string& stop_name, const double stop_latitude, const double stop_longitude);
			const std::string name;
			const double latitude;
			const double longitude;
			std::vector<DistanceToStop> distance_to_stop;
		};

		struct Bus {
			Bus(const std::string& bus_name, const bool isring);
			std::vector<std::string> stop_for_bus;
			const std::string name;
			const bool is_ring;
		};

		struct Request_pool {
			std::vector<Stop> stops;
			std::vector<Bus> buses;
		};

		explicit LoadRequestHandler(transport_catalogue::TransportCatalogue& db);
		const domain::BusInfo GetBusInfo(const std::string_view) const;
		const domain::StopInfo GetStopInfo(const std::string_view) const;

	protected:
		// ������������ ��� ��������, ��������� ������ � ������������ ����������.
		void ProcessRequestPool(const Request_pool& req_pool);

		transport_catalogue::TransportCatalogue& data_base_;
		domain::RoutingSettings routingSettings_;
		std::unique_ptr<transport_router::RouteHandler> router_;
	};



} // namespace requestHandler