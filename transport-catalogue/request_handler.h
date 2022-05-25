#pragma once


// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)

#include <optional>
#include <string>

#include "transport_catalogue.h"
#include "domain.h"
#include "svg.h"
#include "map_renderer.h"

namespace requestHandler {

	class MapRequestHandler {
	public:
		// MapRenderer понадобится в следующей части итогового проекта
		MapRequestHandler(const transport_catalogue::TransportCatalogue& db, const renderer::SVG_Settings& svg_settings);

		// Этот метод будет нужен в следующей части итогового проекта
		svg::Document RenderMap();

	private:
		// RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
		const transport_catalogue::TransportCatalogue& db_;
		renderer::MapRenderer renderer_;
	};

	// Базовый класс, предоставляющий универсальный интерфейс для загрузки информации в справочник.
	// Модули загрузки (json, xml, yaml и прочие) наследуются от него. 
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
		// Обрабатывает пул запросов, загружает данные в транспортный справочник.
		void ProcessRequestPool(const Request_pool& req_pool);

		transport_catalogue::TransportCatalogue& data_base_;
	};



} // namespace requestHandler