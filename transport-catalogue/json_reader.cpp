#include "json_reader.h"

namespace jsonReader {
	using namespace std;

	JsonReader::JsonReader(transport_catalogue::TransportCatalogue& db)
		: LoadRequestHandler(db), jDoc_(json::Node())
	{
	}

	void jsonReader::JsonReader::LoadJson(std::istream& is)
	{
		jDoc_ = json::Load(is);
	}

	// Обрабатывает base_requests. Запросы на добавление данных в справочник.
	void JsonReader::ProcessBaseRequests()
	{
		if (jDoc_.GetRoot().IsDict() && jDoc_.GetRoot().AsMap().size() != 0) {
			// Загрузка данных
			const json::Node& base_requests_node = jDoc_.GetRoot().AsMap().find("base_requests")->second;
			BaseRequests(base_requests_node);

			ProcessRequestPool(request_pool_);
		}
	}

	// Обрабатывает stat_requests. Запросы на получение информации о маршруте.
	void JsonReader::ProcessStatRequests(std::ostream& os)
	{
		// Т.к. основная нагрузка построения оптимальных путей ложится на конструктор маршрутизатора
		// маршрутизатор инициализируется при чтении настроек маршрутизации, если они указаны.
		
		//GetRoutingSettings(); - не актуально для режима process_requests

		auto it = jDoc_.GetRoot().AsMap().find("stat_requests");
		if (it != jDoc_.GetRoot().AsMap().end()) {
			const json::Node& stat_requests_node = it->second;
			json::Document jDocStatResult = StatRequests(stat_requests_node);

			json::Print(jDocStatResult, os);
		}
	}

	// Сериализует базу данных в бинарный файл
	void JsonReader::ProcessSerialization()
	{
		auto it = jDoc_.GetRoot().AsMap().find("serialization_settings");
		if (it != jDoc_.GetRoot().AsMap().end()) {
			json::Node& fileName = it->second.AsMap().find("file")->second;

			transport_catalogue::serialize::Serialize serialize(data_base_, filesystem::path(fileName.AsString()));
			serialize.SetSVGSettings(GetRenderSettings());
			GetRoutingSettings();
			serialize.SetRoutingSettings(routingSettings_);
			serialize.SetGraph(router_->GetGrahpPtr());
			serialize.SetRouter(router_->GetRouterPtr());
			serialize.Save();
		}
	}

	// Десериализует данные из бинарного файла в справочник
	void JsonReader::ProcessDeserialization()
	{
		auto it = jDoc_.GetRoot().AsMap().find("serialization_settings");
		if (it != jDoc_.GetRoot().AsMap().end()) {
			json::Node& fileName = it->second.AsMap().find("file")->second;
			transport_catalogue::serialize::Deserialize deserialize(data_base_, filesystem::path(fileName.AsString()));
			deserialize.Load();

			optional<renderer::SVG_Settings> svgSetts = deserialize.GetSVGSettings();
			if (svgSetts) {
				svgSettings_ = move(svgSetts.value());
			}

			std::optional<domain::RoutingSettings> routigSettings = deserialize.GetRoutingSettings();
			if (routigSettings) {
				routingSettings_ = move(routigSettings.value());

				transport_router::GraphBuilder graphBuilder(
					data_base_,
					routingSettings_,
					move(deserialize.GetEdges()),
					move(deserialize.GetIncidence_lists())
					);

				router_ = make_unique<transport_router::RouteHandler>(
					data_base_,
					routingSettings_,
					forward<transport_router::GraphBuilder>(graphBuilder),
					forward<graph::Router<double>::RoutesInternalData>(deserialize.GetRoutesInternalData())
					);
			}
		}
	}


	// Загружает настройки визуализации
	const renderer::SVG_Settings JsonReader::GetRenderSettings() const
	{
		auto it = jDoc_.GetRoot().AsMap().find("render_settings");
		if (it != jDoc_.GetRoot().AsMap().end()) {
			const map<string, json::Node>& render_settings_node = it->second.AsMap();
			return RenderSettings(render_settings_node);
		}
		return svgSettings_;
	}

	// Загружает настройки маршрутизации и инициализирует построение маршрутизатора.
	void JsonReader::GetRoutingSettings()
	{
		auto it = jDoc_.GetRoot().AsMap().find("routing_settings");
		if (it != jDoc_.GetRoot().AsMap().end()) {
			const map<string, json::Node>& routing_settings_node = it->second.AsMap();
			routingSettings_.bus_velocity = routing_settings_node.at("bus_velocity").AsInt();
			routingSettings_.bus_wait_time = routing_settings_node.at("bus_wait_time").AsDouble();

			router_ = make_unique<transport_router::RouteHandler>(data_base_, routingSettings_);
		}
	}

	Base::Request_pool& JsonReader::GetRequestPool()
	{
		return request_pool_;
	}

	json::Document& JsonReader::GetJsonDoc()
	{
		return jDoc_;
	}

	void JsonReader::BaseRequests(const json::Node& requests)
	{
		for (const json::Node& request : requests.AsArray()) {
			const map<string, json::Node>& item = request.AsMap();
			auto it = item.find("type");
			if (it != item.end()) {
				string request_type = it->second.AsString();
				if (request_type == "Stop") {
					LoadStopInfo(item);
				}
				else if (request_type == "Bus") {
					LoadBusInfo(item);
				}
				// ... новые типы запросов
			}
		}
	}
	// Обрабатывает json массив с запросами к базе
	json::Document JsonReader::StatRequests(const json::Node& requests)
	{
		json::Builder jbuild = json::Builder{};
		auto jarray = jbuild.StartArray();

		for (const json::Node& request : requests.AsArray()) {
			const map<string, json::Node>& item = request.AsMap();
			auto it = item.find("type");
			if (it != item.end()) {
				string request_type = it->second.AsString();
				if (request_type == "Stop") {
					jarray.Value(StopInfo(item));
				}
				else if (request_type == "Bus") {
					jarray.Value(BusInfo(item));
				}
				else if (request_type == "Map") {
					jarray.Value(SvgMap(item));
				}
				else if (request_type == "Route") {
					jarray.Value(RouteInfo(item));
				}

				// ... новые типы запросов
			}
		}
		return json::Document(jarray.EndArray().Build());
	}

	const renderer::SVG_Settings JsonReader::RenderSettings(const map<string, json::Node>& jsetings) const
	{
		renderer::SVG_Settings settings;

		settings.width = jsetings.at("width").AsDouble();
		settings.height = jsetings.at("height").AsDouble();

		settings.padding = jsetings.at("padding").AsDouble();

		settings.line_width = jsetings.at("line_width").AsDouble();
		settings.stop_radius = jsetings.at("stop_radius").AsDouble();

		settings.bus_label_font_size = jsetings.at("bus_label_font_size").AsInt();

		const vector<json::Node>& jpointB = jsetings.at("bus_label_offset").AsArray();
		settings.bus_label_offset.dx = jpointB[0].AsDouble();
		settings.bus_label_offset.dy = jpointB[1].AsDouble();

		settings.stop_label_font_size = jsetings.at("stop_label_font_size").AsInt();
		const vector<json::Node>& jpointS = jsetings.at("stop_label_offset").AsArray();
		settings.stop_label_offset.dx = jpointS[0].AsDouble();
		settings.stop_label_offset.dy = jpointS[1].AsDouble();

		settings.underlayer_color = GetColorAsString(jsetings.at("underlayer_color"));
		settings.underlayer_width = jsetings.at("underlayer_width").AsDouble();

		if (jsetings.count("color_palette") == 1) {
			const vector<json::Node>& jcolor = jsetings.at("color_palette").AsArray();
			for (const json::Node& jitem : jcolor) {
				settings.color_palette.push_back(GetColorAsString(jitem));
			}
		}
		return settings;
	}

	std::string JsonReader::GetColorAsString(const json::Node& color_node) const
	{
		std::stringstream ss;

		if (color_node.IsString()) {
			return color_node.AsString();
		}
		else if (color_node.IsArray()) {
			const vector<json::Node>& jcolor = color_node.AsArray();
			if (jcolor.size() == 3) {
				ss << "rgb("s
					<< jcolor[0].AsInt() << ","s
					<< jcolor[1].AsInt() << ","s
					<< jcolor[2].AsInt() << ")"s;
				return ss.str();
			}
			else if (jcolor.size() == 4) {
				ss << "rgba("s
					<< jcolor[0].AsInt() << ","s
					<< jcolor[1].AsInt() << ","s
					<< jcolor[2].AsInt() << ","s
					<< jcolor[3].AsDouble() << ")"s;
				return ss.str();
			}
		}
		return "none"s;
	}

	// Загружает в базу информацию об остановке
	void JsonReader::LoadStopInfo(const map<string, json::Node>& stop)
	{
		Stop newStop(stop.at("name"s).AsString(), stop.at("latitude"s).AsDouble(), stop.at("longitude"s).AsDouble());
		auto it = stop.find("road_distances"s);
		if (it != stop.end()) {
			const map<string, json::Node>& road_distances = it->second.AsMap();
			for (const auto& [stopName, dist] : road_distances) {
				newStop.distance_to_stop.push_back({ stopName, dist.AsInt() });
			}
		}
		request_pool_.stops.push_back(newStop);
	}

	// Загружает в базу информацию о маршруте
	void JsonReader::LoadBusInfo(const map<string, json::Node>& bus)
	{
		Bus newBus(bus.at("name"s).AsString(), bus.at("is_roundtrip"s).AsBool());
		for (const json::Node& jstop : bus.at("stops"s).AsArray()) {
			newBus.stop_for_bus.push_back(jstop.AsString());
		}
		request_pool_.buses.push_back(newBus);
	}

	// Формирует json ветку с информацией об остановке
	json::Node JsonReader::StopInfo(const std::map<std::string, json::Node>& stop)
	{
		domain::StopInfo stopInfo = this->GetStopInfo(stop.at("name"s).AsString());
		json::Builder jbuilder = json::Builder{};
		auto jdict = jbuilder.StartDict();

		if (stopInfo.buses_on_stop) {
			jdict.Key("request_id"s).Value(stop.at("id"s).AsInt());
			auto jstop = jdict.Key("buses"s).StartArray();
			for (string_view sv : *(stopInfo.buses_on_stop)) {
				jstop.Value(std::string{ sv });
			}
			jstop.EndArray();
		}
		else {
			if (!stopInfo.stop_found) {
				jdict.Key("request_id"s).Value(stop.at("id"s).AsInt());
				jdict.Key("error_message"s).Value("not found"s);
			}
			else {
				jdict.Key("request_id"s).Value(stop.at("id"s).AsInt());
				jdict.Key("buses"s).StartArray().EndArray();
			}
		}
		return jdict.EndDict().Build();
	}

	// Формирует json ветку с информацией о маршруте
	json::Node JsonReader::BusInfo(const std::map<std::string, json::Node>& bus)
	{
		domain::BusInfo busInfo = this->GetBusInfo(bus.at("name").AsString());
		json::Builder jbuilder = json::Builder{};
		auto jresult = jbuilder.StartDict();

		if (busInfo.stop_count != 0) {
			jresult.Key("request_id"s).Value(bus.at("id"s).AsInt());
			jresult.Key("curvature"s).Value(busInfo.curvature);
			jresult.Key("route_length"s).Value(busInfo.lenght);
			jresult.Key("stop_count"s).Value(static_cast<int>(busInfo.stop_count));
			jresult.Key("unique_stop_count"s).Value(static_cast<int>(busInfo.unique_stop_count));
		}
		else {
			jresult.Key("request_id"s).Value(bus.at("id"s).AsInt());
			jresult.Key("error_message"s).Value("not found"s);
		}
		return jresult.EndDict().Build();
	}

	// Формирует json ветку с картой всех маршрутов в формате svg
	json::Node jsonReader::JsonReader::SvgMap(const std::map<std::string, json::Node>& item)
	{
		json::Builder jbuilder = json::Builder{};
		auto jresult = jbuilder.StartDict();

		renderer::SVG_Settings svgSet = GetRenderSettings();
		requestHandler::MapRequestHandler mapreq(data_base_, svgSet);
		svg::Document svgDoc = mapreq.RenderMap();

		std::stringstream ss;
		svgDoc.Render(ss);

		jresult.Key("map"s).Value(ss.str());
		jresult.Key("request_id"s).Value(item.at("id"s).AsInt());

		return jresult.EndDict().Build();;
	}

	// Обрабатывает запрос на построение маршрута из точки А в точку Б
	json::Node JsonReader::RouteInfo(const std::map<std::string, json::Node>& route) {
		json::Builder jbuilder = json::Builder{};
		auto jresult = jbuilder.StartDict();
		jresult.Key("request_id"s).Value(route.at("id"s).AsInt());

		const optional<transport_router::Route> routeItems = router_->BuildRoute(route.at("from").AsString(), route.at("to").AsString());
		if (routeItems) {
			//&& !holds_alternative<domain::RouteItem_NoWay>(routeItems->route_items[0] ?
			jresult.Key("total_time"s).Value(routeItems->total_time);
			auto jarray = jresult.Key("items"s).StartArray();
			for (const transport_router::Items& item : routeItems->route_items) {
				visit([&jarray](const auto& value) {
					JsonReader::RouteItem{}(value, jarray);
					}, item);
			}
			jarray.EndArray();
		}
		else {
			jresult.Key("error_message"s).Value("not found"s);
		}

		return jresult.EndDict().Build();;
	}

	void JsonReader::RouteItem::operator()(const domain::RouteItem_Wait& value, json::ArrayItemContext& jitem)
	{
		auto jdict = jitem.StartDict();
		jdict.Key("stop_name"s).Value(value.stop_name);
		jdict.Key("time"s).Value(value.time);
		jdict.Key("type"s).Value("Wait"s);
		jdict.EndDict();
	}

	void JsonReader::RouteItem::operator()(const domain::RouteItem_Bus& value, json::ArrayItemContext& jitem)
	{
		auto jdict = jitem.StartDict();
		jdict.Key("bus"s).Value(value.bus_name);
		jdict.Key("span_count"s).Value(value.span_count);
		jdict.Key("time"s).Value(value.time);
		jdict.Key("type"s).Value("Bus"s);
		jdict.EndDict();
	}

	void JsonReader::RouteItem::operator()([[maybe_unused]] const domain::RouteItem_NoWay value, [[maybe_unused]] json::ArrayItemContext& jitem)
	{
		// Может быть позже...
	}

} // namespace jsonReader