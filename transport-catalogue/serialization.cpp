#include "serialization.h"

namespace transport_catalogue {
	namespace serialize {
		using namespace std;
		using namespace domain;

		namespace tcs = transport_catalogue_serialize;

		MainSerialize::MainSerialize(TransportCatalogue& tc, filesystem::path&& file)
			: db_(tc)
			, file_(std::move(file))
			, svgSettings_(nullopt)
			, routingSettings_(nullopt)
		{
		}


		Serialize::Serialize(TransportCatalogue& tc, filesystem::path&& file)
			: MainSerialize::MainSerialize(tc, move(file))
			, string_names_(db_.GetAllStrings())
			, graph_(nullptr)
			, router_(nullptr)
		{
		}

		void Serialize::SetSVGSettings(const renderer::SVG_Settings& svgSettings)
		{
			svgSettings_ = svgSettings;
		}

		void Serialize::SetRoutingSettings(const domain::RoutingSettings& routingSettings)
		{
			routingSettings_ = routingSettings;
		}

		void Serialize::SetGraph(graph::DirectedWeightedGraph<double>* graphRef)
		{
			graph_ = graphRef;
		}

		void Serialize::SetRouter(graph::Router<double>* router)
		{
			router_ = router;
		}

		Serialize::~Serialize()
		{
			google::protobuf::ShutdownProtobufLibrary();
		}

		void Serialize::Save()
		{
			SaveStrings();
			SaveStops();
			SaveBuses();
			SaveStopToStopDistance();
			if (svgSettings_) {
				SaveSVGSettings();
			}

			if (routingSettings_) {
				SaveRoutingSettings();
			}

			if (graph_ != nullptr) {
				SaveGraph();
			}

			if (router_ != nullptr) {
				SaveRouter();
			}

			ofstream out_file(file_, ios::out | ios::trunc | ios::binary);
			if (!out_file.is_open()) {
				return;
			}

			pbDataBase_.SerializeToOstream(&out_file);
			out_file.close();
		}

		void Serialize::SaveStrings()
		{
			for (const pair<std::string, size_t>& item : string_names_) {
				tcs::Strings_Stuct one_string;
				one_string.set_originstring(item.first);
				one_string.set_id(item.second);

				*pbDataBase_.add_strings_list() = move(one_string);
			}
		}

		void Serialize::SaveStops()
		{
			const deque<Stop>& stopsDb = db_.GetStopsList();

			for (const Stop& stop : stopsDb) {
				tcs::Stop pbStop;

				pbStop.set_name_id(string_names_.find(stop.name.data())->second);
				pbStop.set_latitude(stop.latitude);
				pbStop.set_longitude(stop.longitude);
				pbStop.set_israw(stop.isRaw);
				pbStop.set_isfinalstop(stop.isFinalStop);
				pbStop.set_stop_id(stop.id);

				*pbDataBase_.add_stops_list() = move(pbStop);
			}
		}

		void Serialize::SaveBuses()
		{
			const deque<Bus>& busesDb = db_.GetBusesList();

			for (const Bus& bus : busesDb) {
				tcs::Bus pbBus;

				for (Stop* stop : bus.stop_for_bus_forward) {
					pbBus.add_stop_id_for_bus(stop->id);
				}

				pbBus.set_name_id(string_names_.find(bus.name.data())->second);
				if (bus.secondFinalStop != nullptr) {
					pbBus.mutable_secondfinalstop_id()->set_value(bus.secondFinalStop->id);
				}
				pbBus.set_is_ring(bus.is_ring);
				pbBus.set_distance_by_geo(bus.distance_by_geo);
				pbBus.set_distance_by_road(bus.distance_by_road);
				pbBus.set_bus_id(bus.id);

				*pbDataBase_.add_buses_list() = move(pbBus);
			}
		}

		void Serialize::SaveStopToStopDistance()
		{
			for (const StopToStopDistance& item : db_.GetAllStopToStopDistance()) {
				tcs::Stop_to_stop_distance pbStopToStop;

				pbStopToStop.set_stop_a_id(item.stop_a);
				pbStopToStop.set_stop_b_id(item.stop_b);
				pbStopToStop.set_distance(item.distance);

				*pbDataBase_.add_distance_list() = move(pbStopToStop);
			}
		}

		void Serialize::SaveSVGSettings()
		{
			tcs::SVG_Settings pbSVGsetts;
			renderer::SVG_Settings& setts = svgSettings_.value();
			pbSVGsetts.set_width(setts.width);
			pbSVGsetts.set_height(setts.height);
			pbSVGsetts.set_padding(setts.padding);
			pbSVGsetts.set_line_width(setts.line_width);
			pbSVGsetts.set_stop_radius(setts.stop_radius);
			pbSVGsetts.set_bus_label_font_size(setts.bus_label_font_size);

			tcs::Point pointBus;
			pointBus.set_dx(setts.bus_label_offset.dx);
			pointBus.set_dy(setts.bus_label_offset.dy);
			*pbSVGsetts.mutable_bus_label_offset() = move(pointBus);

			tcs::Point pointStop;
			pointStop.set_dx(setts.stop_label_offset.dx);
			pointStop.set_dy(setts.stop_label_offset.dy);
			*pbSVGsetts.mutable_stop_label_offset() = move(pointStop);
			pbSVGsetts.set_stop_label_font_size(setts.stop_label_font_size);

			pbSVGsetts.set_underlayer_color(setts.underlayer_color);
			pbSVGsetts.set_underlayer_width(setts.underlayer_width);

			for (const string& color : setts.color_palette) {
				pbSVGsetts.add_color_palette(color);
			}

			*pbDataBase_.mutable_svgsettings() = move(pbSVGsetts);
		}

		void Serialize::SaveRoutingSettings()
		{
			tcs::RoutingSettings pbRs;
			const domain::RoutingSettings& rs = routingSettings_.value();
			pbRs.set_bus_wait_time(rs.bus_wait_time);
			pbRs.set_bus_velocity(rs.bus_velocity);
			*pbDataBase_.mutable_routingsettings() = move(pbRs);
		}

		void Serialize::SaveGraph()
		{
			tcs::DirectedWeightedGraph pbGraph;
			for (const auto& edge : graph_->GetEdges()) {
				tcs::Edge pbEdge;
				pbEdge.set_busid(edge.bus->id);
				pbEdge.set_count(edge.count);
				pbEdge.set_from(edge.from);
				pbEdge.set_to(edge.to);
				pbEdge.set_weight(edge.weight);

				*pbGraph.add_edges() = move(pbEdge);
			}

			for (const auto& incidenceList : graph_->GetIncidenceList()) {
				tcs::EdgeId edgesId;
				for (auto edgeId : incidenceList) {
					edgesId.add_edgeid(move(edgeId));
				}
				*pbGraph.add_incidencelist() = move(edgesId);
			}
			*pbDataBase_.mutable_graph() = move(pbGraph);
		}

		void Serialize::SaveRouter()
		{
			const auto& routerData = router_->GetRoutesInternalData();
			tcs::VertexFrom pbVertexFrom;
			for (const auto& from : routerData) {
				tcs::VertexTo pbVertexTo;
				for (const optional<graph::Router<double>::RouteInternalData>& to : from) {
					tcs::OptionalRouteInternalData pbOptionalRouteData;

					if (to) {
						tcs::RouteInternalData pbRouteData;
						pbRouteData.set_weight(to->weight);
						if (to->prev_edge) {
							pbRouteData.set_prev_edge(to->prev_edge.value());
						}
						else {
							pbRouteData.set_isnull(true);
						}
						*pbOptionalRouteData.mutable_route_internal_data() = move(pbRouteData);
					}
					else {
						pbOptionalRouteData.set_isnull(true);
					}
					*pbVertexTo.add_routes_internal_data() = move(pbOptionalRouteData);
				}
				*pbVertexFrom.add_from() = move(pbVertexTo);
			}

			*pbDataBase_.mutable_router() = move(pbVertexFrom);
		}






		Deserialize::Deserialize(TransportCatalogue& tc, filesystem::path&& file)
			: MainSerialize::MainSerialize(tc, move(file))
		{
		}

		Deserialize::~Deserialize()
		{
			google::protobuf::ShutdownProtobufLibrary();
		}

		void Deserialize::Load()
		{
			ifstream in_file(file_, ios::in | ios::binary);
			if (!in_file.is_open() || !pbDataBase_.ParseFromIstream(&in_file)) {
				return;
			}
			in_file.close();

			LoadStrings();
			LoadStops();
			LoadBuses();
			LoadStopToStopRoute();
			LoadSVGSettings();
			LoadRoutingSettings();
			LoadGraph();
			LoadRouter();

		}

		void Deserialize::LoadStrings()
		{
			for (size_t i = 0; i < pbDataBase_.strings_list_size(); ++i) {
				tcs::Strings_Stuct* pbstring = pbDataBase_.mutable_strings_list(i);
				stringViewId_[pbstring->id()] = db_.InsertString(move(*pbstring->mutable_originstring()), pbstring->id());
			}
		}

		void Deserialize::LoadStops()
		{
			for (size_t i = 0; i < pbDataBase_.stops_list_size(); ++i) {
				tcs::Stop* pbStop = pbDataBase_.mutable_stops_list(i);
				Stop newStop(
					stringViewId_.find(pbStop->name_id())->second,
					pbStop->latitude(),
					pbStop->longitude());
				newStop.isRaw = pbStop->israw();
				newStop.isFinalStop = pbStop->isfinalstop();
				newStop.id = pbStop->stop_id();

				db_.InsertStop(move(newStop));
			}
		}

		void Deserialize::LoadBuses()
		{
			for (size_t i = 0; i < pbDataBase_.buses_list_size(); ++i) {
				tcs::Bus* pbBus = pbDataBase_.mutable_buses_list(i);
				Bus newBus(stringViewId_.find(pbBus->name_id())->second, pbBus->is_ring());

				newBus.stop_for_bus_forward.reserve(pbBus->stop_id_for_bus_size());
				for (size_t j = 0; j < pbBus->stop_id_for_bus_size(); ++j) {
					newBus.stop_for_bus_forward.push_back(db_.MutableStopById(pbBus->stop_id_for_bus().Get(j)));
				}
				if (pbBus->has_secondfinalstop_id()) {
					newBus.secondFinalStop = db_.MutableStopById(pbBus->secondfinalstop_id().value());
				}
				else {
					newBus.secondFinalStop = nullptr;
				}
				newBus.distance_by_geo = pbBus->distance_by_geo();
				newBus.distance_by_road = pbBus->distance_by_road();
				newBus.id = pbBus->bus_id();

				db_.InsertBus(move(newBus));
			}
		}

		void Deserialize::LoadStopToStopRoute()
		{
			for (size_t i = 0; i < pbDataBase_.distance_list_size(); ++i) {
				tcs::Stop_to_stop_distance* distance = pbDataBase_.mutable_distance_list(i);
				const domain::StopToStopDistance stops(
					static_cast<size_t>(distance->stop_a_id()),
					static_cast<size_t>(distance->stop_b_id()),
					static_cast<size_t>(distance->distance()));
				db_.InsertStopToStopDistance(stops);
			}
		}

		void Deserialize::LoadSVGSettings()
		{
			if (pbDataBase_.has_svgsettings()) {
				renderer::SVG_Settings setts;

				const tcs::SVG_Settings& pbSetts = pbDataBase_.svgsettings();
				setts.width = pbSetts.width();
				setts.height = pbSetts.height();
				setts.padding = pbSetts.padding();
				setts.line_width = pbSetts.line_width();
				setts.stop_radius = pbSetts.stop_radius();
				setts.bus_label_font_size = pbSetts.bus_label_font_size();

				setts.bus_label_offset.dx = pbSetts.bus_label_offset().dx();
				setts.bus_label_offset.dy = pbSetts.bus_label_offset().dy();

				setts.stop_label_offset.dx = pbSetts.stop_label_offset().dx();
				setts.stop_label_offset.dy = pbSetts.stop_label_offset().dy();

				setts.stop_label_font_size = pbSetts.stop_label_font_size();

				setts.underlayer_color = pbSetts.underlayer_color();
				setts.underlayer_width = pbSetts.underlayer_width();

				for (size_t i = 0; i < pbSetts.color_palette_size(); ++i) {
					setts.color_palette.push_back(pbSetts.color_palette(i));
				}
				svgSettings_ = move(setts);
			}
		}

		void Deserialize::LoadRoutingSettings()
		{
			if (pbDataBase_.has_routingsettings()) {
				const tcs::RoutingSettings& pbRs = pbDataBase_.routingsettings();
				domain::RoutingSettings rs
				{
					pbRs.bus_wait_time(),
					pbRs.bus_velocity()
				};
				routingSettings_ = move(rs);
			}
		}

		void Deserialize::LoadGraph()
		{
			if (pbDataBase_.has_graph()) {
				const tcs::DirectedWeightedGraph* pbGraph = pbDataBase_.mutable_graph();

				edges_.reserve(pbGraph->edges_size());
				for (size_t i = 0; i < pbGraph->edges_size(); ++i) {
					const tcs::Edge& pbEdge = pbGraph->edges(i);
					graph::Edge<double> edge
					{
						pbEdge.from(),
						pbEdge.to(),
						pbEdge.weight(),
						pbEdge.count(),
						db_.MutableBusById(pbEdge.busid())
					};

					edges_.push_back(move(edge));
				}

				incidence_lists_.reserve(pbGraph->incidencelist_size());
				for (size_t i = 0; i < pbGraph->incidencelist_size(); ++i) {
					const tcs::EdgeId& pbEdgeId = pbGraph->incidencelist(i);
					vector<size_t> incidents(pbEdgeId.edgeid_size());
					for (size_t j = 0; j < pbEdgeId.edgeid_size(); ++j) {
						incidents[j] = pbEdgeId.edgeid(j);
					}
					incidence_lists_.push_back(move(incidents));
				}
			}
		}

		void Deserialize::LoadRouter()
		{
			if (pbDataBase_.has_router()) {
				tcs::VertexFrom* pbRouter = pbDataBase_.mutable_router();
				routes_internal_data_.resize(pbRouter->from_size());

				for (size_t i = 0; i < pbRouter->from_size(); ++i) {
					tcs::VertexTo* pbVertexTo = pbRouter->mutable_from(i);

					for (size_t j = 0; j < pbVertexTo->routes_internal_data_size(); ++j) {
						const tcs::OptionalRouteInternalData& pbRouteData = pbVertexTo->routes_internal_data(j);
						graph::Router<double>::RouteInternalData routeItem;
						routes_internal_data_[i].resize(pbVertexTo->routes_internal_data_size());

						if (pbRouteData.optional_route_internal_data_case() 
							== tcs::OptionalRouteInternalData::OptionalRouteInternalDataCase::kRouteInternalData) {

							const auto& data = pbRouteData.route_internal_data();
							routeItem.weight = data.weight();

							if (data.edgeId_case() == tcs::RouteInternalData::EdgeIdCase::kPrevEdge) {
								routeItem.prev_edge = data.prev_edge();
							}
							routes_internal_data_[i][j] = move(routeItem);
						}
						else {
							routes_internal_data_[i][j] = nullopt;
						}
					}
				}
			}
		}

		optional<renderer::SVG_Settings> Deserialize::GetSVGSettings() const
		{
			return svgSettings_;
		}

		optional<domain::RoutingSettings> Deserialize::GetRoutingSettings() const
		{
			return routingSettings_;
		}

		vector<graph::Edge<double>>& Deserialize::GetEdges()
		{
			return edges_;
		}

		vector<vector<size_t>>& Deserialize::GetIncidence_lists()
		{
			return incidence_lists_;
		}

		graph::Router<double>::RoutesInternalData& Deserialize::GetRoutesInternalData()
		{
			return routes_internal_data_;
		}



	} // namespace serialize
} // namespace transport_catalogue