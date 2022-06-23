#pragma once

#include <algorithm>
#include <cstdlib>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <map>

#include "geo.h"
#include "svg.h"
#include "domain.h"

namespace renderer {

    inline const double EPSILON = 1e-6;

    struct Point {
        double dx = 0;
        double dy = 0;
    };

    struct SVG_Settings {
        double width = 0;
        double height = 0;

        double padding = 0;

        double line_width = 0;
        double stop_radius = 0;

        int bus_label_font_size = 10;
        Point bus_label_offset;

        int stop_label_font_size = 10;
        Point stop_label_offset;

        std::string underlayer_color;
        double underlayer_width = 0;

        std::vector<std::string> color_palette;
    };

    bool IsZero(double value);

    class SphereProjector {
    public:
        SphereProjector() = default;
        // points_begin � points_end ������ ������ � ����� ��������� ��������� geo::Coordinates
        template <typename PointInputIt>
        void SetSphereProjector(PointInputIt points_begin, PointInputIt points_end,
            double max_width, double max_height, double padding)
        {
            padding_ = padding;
            // ���� ����� ����������� ����� �� ������, ��������� ������
            if (points_begin == points_end) {
                return;
            }

            // ������� ����� � ����������� � ������������ ��������
            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // ������� ����� � ����������� � ������������ �������
            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // ��������� ����������� ��������������� ����� ���������� x
            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // ��������� ����������� ��������������� ����� ���������� y
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // ������������ ��������������� �� ������ � ������ ���������,
                // ���� ����������� �� ���
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                // ����������� ��������������� �� ������ ���������, ���������� ���
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                // ����������� ��������������� �� ������ ���������, ���������� ���
                zoom_coeff_ = *height_zoom;
            }
        }

        // ���������� ������ � ������� � ���������� ������ SVG-�����������
        svg::Point operator()(geo::Coordinates coords) const;

    private:
        double padding_ = 0;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };


    class MapRenderer {
    public:
        MapRenderer(const SVG_Settings& svg_settings);
        svg::Document DrawMap(const std::map<std::string_view, domain::Bus*>& buses);

    private:
        SVG_Settings svg_settings_;
        SphereProjector proj_;
        std::map<std::string_view, geo::Coordinates> stopsGeo_;
        int palette_index_;
        int max_pallete_index_;
        bool firstPallete_;

        int GetNextPaletteIndex();
        void PrepareSphereProjector(const std::map<std::string_view, domain::Bus*>& buses);
        void DrawRouteLines(const std::map<std::string_view, domain::Bus*>& buses, svg::Document& svgDoc);
        void DrawBusName(const std::map<std::string_view, domain::Bus*>& buses, svg::Document& svgDoc);
        void DrawStopPoint(svg::Document& svgDoc);
        void DrawStopName(svg::Document& svgDoc);
    };

}