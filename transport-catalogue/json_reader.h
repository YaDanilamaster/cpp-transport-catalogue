#pragma once

#include <map>
#include <string_view>
#include <sstream>

#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"

namespace jsonReader {
	using Base = requestHandler::LoadRequestHandler;

	class JsonReader final : public Base {
	public:
		explicit JsonReader(transport_catalogue::TransportCatalogue&);
		void LoadJson(std::istream&);
		void ProcessBaseRequests();
		void ProcessStatRequests(std::ostream&);

		const renderer::SVG_Settings GetRenderSettings() const;

		Base::Request_pool& GetRequestPool();
		json::Document& GetJsonDoc();

	private:
		Base::Request_pool request_pool_;
		json::Document jDoc_;

		void BaseRequests(const json::Node&);
		json::Document StatRequests(const json::Node&);

		const renderer::SVG_Settings RenderSettings(const std::map<std::string, json::Node>&) const;
		std::string GetColorAsString(const json::Node&) const;

		void LoadStopInfo(const std::map<std::string, json::Node>&);
		void LoadBusInfo(const std::map<std::string, json::Node>&);

		json::Node StopInfo(const std::map<std::string, json::Node>&);
		json::Node BusInfo(const std::map<std::string, json::Node>&);
		json::Node SvgMap(const std::map<std::string, json::Node>&);
	};

} // namespace jsonReader