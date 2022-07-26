#pragma once
#include <fstream>
#include <filesystem>
#include <optional>

#include "transport_catalogue.h"
#include <transport_catalogue.pb.h>
#include "map_renderer.h"
#include "graph.h"
#include "router.h"

namespace transport_catalogue {
	namespace serialize {

		class MainSerialize {
		public:
			MainSerialize(TransportCatalogue& tc, std::filesystem::path&& file);
			virtual ~MainSerialize() = default;

		protected:
			TransportCatalogue& db_;
			std::filesystem::path file_;
			transport_catalogue_serialize::DataBase pbDataBase_;
			std::optional<renderer::SVG_Settings> svgSettings_;
			std::optional<domain::RoutingSettings> routingSettings_;

		};

		class Serialize final : public MainSerialize {
		public:
			Serialize(TransportCatalogue& tc, std::filesystem::path&& file);
			void SetSVGSettings(const renderer::SVG_Settings& svgSettings);
			void SetRoutingSettings(const domain::RoutingSettings& routingSettings);
			void SetGraph(graph::DirectedWeightedGraph<double>* graphRef);
			void SetRouter(graph::Router<double>* router);

			~Serialize() override;

			void Save();

		private:
			const std::unordered_map<std::string, size_t>& string_names_;
			graph::DirectedWeightedGraph<double>* graph_;
			graph::Router<double>* router_;

			void SaveStrings();
			void SaveStops();
			void SaveBuses();
			void SaveStopToStopDistance();
			void SaveSVGSettings();
			void SaveRoutingSettings();
			void SaveGraph();
			void SaveRouter();
		};


		class Deserialize final : public MainSerialize{
		public:
			Deserialize(TransportCatalogue& tc, std::filesystem::path&& file);
			std::optional<renderer::SVG_Settings> GetSVGSettings() const;
			std::optional<domain::RoutingSettings> GetRoutingSettings() const;
			std::vector<graph::Edge<double>>& GetEdges();
			std::vector<std::vector<size_t>>& GetIncidence_lists();
			graph::Router<double>::RoutesInternalData& GetRoutesInternalData();

			~Deserialize() override;

			void Load();

		private:
			std::unordered_map<size_t, std::string_view> stringViewId_;
			std::vector<graph::Edge<double>> edges_;
			std::vector<std::vector<size_t>> incidence_lists_;
			graph::Router<double>::RoutesInternalData routes_internal_data_;

			void LoadStrings();
			void LoadStops();
			void LoadBuses();
			void LoadStopToStopRoute();
			void LoadSVGSettings();
			void LoadRoutingSettings();
			void LoadGraph();
			void LoadRouter();
		};


	} // namespace serialize
} // namespace transport_catalogue