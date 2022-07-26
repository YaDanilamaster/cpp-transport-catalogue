#include "map_renderer.h"

namespace renderer {
	using namespace std::literals;

	renderer::MapRenderer::MapRenderer(const SVG_Settings& svg_settings)
		: svg_settings_(svg_settings)
		, palette_index_(0)
		, max_pallete_index_(svg_settings.color_palette.size())
		, firstPallete_(true)
	{
	}

	svg::Document MapRenderer::DrawMap(const std::map<std::string_view, domain::Bus*>& buses) {
		svg::Document svgDoc;

		PrepareSphereProjector(buses);
		DrawRouteLines(buses, svgDoc);
		DrawBusName(buses, svgDoc);
		DrawStopPoint(svgDoc);
		DrawStopName(svgDoc);

		return svgDoc;
	}


	int MapRenderer::GetNextPaletteIndex()
	{
		if (!firstPallete_ && max_pallete_index_ > 0) {
			if (palette_index_ < max_pallete_index_ - 1) {
				++palette_index_;
			}
			else {
				palette_index_ = 0;
			}
			return palette_index_;
		}
		firstPallete_ = false;
		return 0;
	}

	// Настраивает проектор сферических координат на плоскость.
	void MapRenderer::PrepareSphereProjector(const std::map<std::string_view, domain::Bus*>& buses)
	{
		// нужен вектор всех гео локаций остановок
		std::vector<geo::Coordinates> geo_coords;

		for (const auto& [sv, bus] : buses) {
			for (const domain::Stop* stop : bus->stop_for_bus_forward) {
				if (stop != nullptr && !stop->isRaw) {
					geo_coords.push_back({ stop->latitude, stop->longitude });
					stopsGeo_[stop->name] = { stop->latitude, stop->longitude };
				}
			}
		}

		// Создаём проектор сферических координат на карту
		proj_.SetSphereProjector(
			geo_coords.begin(), geo_coords.end(), svg_settings_.width, svg_settings_.height, svg_settings_.padding);
	}

	void MapRenderer::DrawRouteLines(const std::map<std::string_view, domain::Bus*>& buses, svg::Document& svgDoc)
	{
		// задаем коодринаты остановкам
		for (const auto& [sv, bus] : buses) {
			svg::Polyline line;
			int noraw_count = 0;

			for (const domain::Stop* stop : bus->stop_for_bus_forward) {
				if (stop != nullptr && !stop->isRaw) {
					svg::Point screen_coord = proj_({ stop->latitude, stop->longitude });
					line.AddPoint({ screen_coord.x, screen_coord.y });
					++noraw_count;
				}
			}

			if (bus->stop_for_bus_forward.size() > 0 && noraw_count > 1) {
				// задаем настройки вывода
				line.SetFillColor("none");
				line.SetStrokeColor(svg_settings_.color_palette[GetNextPaletteIndex()]);
				line.SetStrokeWidth(svg_settings_.line_width);
				line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
				line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

				svgDoc.Add(line);
			}
		}
	}

	void MapRenderer::DrawBusName(const std::map<std::string_view, domain::Bus*>& buses, svg::Document& svgDoc)
	{
		palette_index_ = 0;
		firstPallete_ = true;
		int curentPalleteId = 0;
		for (const auto& [sv, bus] : buses) {
			if (bus->stop_for_bus_forward.size() > 0) {
				svg::Text busNameFirst;
				domain::Stop* firstStop = bus->stop_for_bus_forward[0];
				curentPalleteId = GetNextPaletteIndex();

				busNameFirst.SetData(std::string(bus->name));
				busNameFirst.SetPosition(proj_({ firstStop->latitude, firstStop->longitude }));
				busNameFirst.SetOffset({ svg_settings_.bus_label_offset.dx, svg_settings_.bus_label_offset.dy });
				busNameFirst.SetFontSize(svg_settings_.bus_label_font_size);
				busNameFirst.SetFontFamily("Verdana"s);
				busNameFirst.SetFontWeight("bold"s);
				busNameFirst.SetFillColor(svg_settings_.color_palette[curentPalleteId]);

				svg::Text busNameFirstBg(busNameFirst);
				busNameFirstBg.SetFillColor(svg_settings_.underlayer_color);
				busNameFirstBg.SetStrokeColor(svg_settings_.underlayer_color);
				busNameFirstBg.SetStrokeWidth(svg_settings_.underlayer_width);
				busNameFirstBg.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
				busNameFirstBg.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

				svgDoc.Add(busNameFirstBg);
				svgDoc.Add(busNameFirst);

				if (!bus->is_ring && (bus->stop_for_bus_forward[0]->name != bus->secondFinalStop->name)) {
					svg::Text busNameSecond;
					domain::Stop* secondStop = bus->secondFinalStop;

					busNameSecond.SetData(std::string(bus->name));
					busNameSecond.SetPosition(proj_({ secondStop->latitude, secondStop->longitude }));
					busNameSecond.SetOffset({ svg_settings_.bus_label_offset.dx, svg_settings_.bus_label_offset.dy });
					busNameSecond.SetFontSize(svg_settings_.bus_label_font_size);
					busNameSecond.SetFontFamily("Verdana"s);
					busNameSecond.SetFontWeight("bold"s);
					busNameSecond.SetFillColor(svg_settings_.color_palette[curentPalleteId]);

					svg::Text busNameSecondBg(busNameSecond);
					busNameSecondBg.SetFillColor(svg_settings_.underlayer_color);
					busNameSecondBg.SetStrokeColor(svg_settings_.underlayer_color);
					busNameSecondBg.SetStrokeWidth(svg_settings_.underlayer_width);
					busNameSecondBg.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
					busNameSecondBg.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

					svgDoc.Add(busNameSecondBg);
					svgDoc.Add(busNameSecond);
				}
			}
		}
	}

	void renderer::MapRenderer::DrawStopPoint(svg::Document& svgDoc)
	{
		for (const auto& [name, geo] : stopsGeo_) {
			svg::Circle cr;
			cr.SetCenter(proj_(geo));
			cr.SetRadius(svg_settings_.stop_radius);
			cr.SetFillColor("white");
			svgDoc.Add(cr);
		}
	}

	void renderer::MapRenderer::DrawStopName(svg::Document& svgDoc)
	{
		for (const auto& [name, geo] : stopsGeo_) {
			svg::Text textName;

			textName.SetPosition(proj_(geo));
			textName.SetOffset({ svg_settings_.stop_label_offset.dx, svg_settings_.stop_label_offset.dy });
			textName.SetFontSize(svg_settings_.stop_label_font_size);
			textName.SetFontFamily("Verdana");
			textName.SetData(std::string(name));
			textName.SetFillColor("black");

			svg::Text textNameBg(textName);
			textNameBg.SetFillColor(svg_settings_.underlayer_color);
			textNameBg.SetStrokeColor(svg_settings_.underlayer_color);
			textNameBg.SetStrokeWidth(svg_settings_.underlayer_width);
			textNameBg.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
			textNameBg.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

			svgDoc.Add(textNameBg);
			svgDoc.Add(textName);
		}
	}

	bool IsZero(double value) {
		return std::abs(value) < EPSILON;
	}

	svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
		return {
			(coords.lng - min_lon_) * zoom_coeff_ + padding_,
			(max_lat_ - coords.lat) * zoom_coeff_ + padding_
		};
	}



}