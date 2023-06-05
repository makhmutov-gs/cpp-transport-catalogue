#pragma once

#include "geo.h"
#include "svg.h"
#include "domain.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>

namespace catalogue::renderer {

using namespace domain;

struct Settings {
    double width;
    double heigth;
    double padding;
    double line_width;
    double stop_radius;
    int bus_label_font_size;
    svg::Point bus_label_offset;
    svg::Point stop_label_offset;
    int stop_label_font_size;
    svg::Color underlayer_color;
    double underlayer_width;
    std::vector<svg::Color> color_palette;
};

inline const double EPSILON = 1e-6;
inline bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

class SphereProjector {
public:

    SphereProjector() = default;

    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding);

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

class MapRenderer {
public:
    MapRenderer(Settings settings);

    svg::Document Render(
        const std::vector<const Bus*>& sorted_buses,
        const std::vector<const Stop*>& sorted_stops
    );

private:
    Settings settings_;
    SphereProjector projector_;

    svg::Color GetCurrentColor(size_t idx) const;

    void VisualiseBusTexts(
        std::vector<svg::Text>& to,
        geo::Coordinates coords,
        svg::Color color,
        const std::string& data
    ) const;

    void VisualiseStopTexts(
        std::vector<svg::Text>& to,
        geo::Coordinates coords,
        svg::Color color,
        const std::string& data
    ) const;

    std::vector<svg::Polyline> RenderLines(
        const std::vector<const Bus*>& sorted_buses
    ) const;

    std::vector<svg::Text> RenderBusNames(
        const std::vector<const Bus*>& sorted_buses
    ) const;

    std::vector<svg::Circle> RenderStopCircles(
        const std::vector<const Stop*>& sorted_stops
    ) const;

    std::vector<svg::Text> RenderStopNames(
        const std::vector<const Stop*>& sorted_stops
    ) const;

    void InitProjector(const std::vector<const Stop*>& stops);
};


template <typename PointInputIt>
SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                double max_width, double max_height, double padding)
    : padding_(padding) //
{
    // Если точки поверхности сферы не заданы, вычислять нечего
    if (points_begin == points_end) {
        return;
    }

    // Находим точки с минимальной и максимальной долготой
    const auto [left_it, right_it] = std::minmax_element(
        points_begin, points_end,
        [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
    min_lon_ = left_it->lng;
    const double max_lon = right_it->lng;

    // Находим точки с минимальной и максимальной широтой
    const auto [bottom_it, top_it] = std::minmax_element(
        points_begin, points_end,
        [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
    const double min_lat = bottom_it->lat;
    max_lat_ = top_it->lat;

    // Вычисляем коэффициент масштабирования вдоль координаты x
    std::optional<double> width_zoom;
    if (!IsZero(max_lon - min_lon_)) {
        width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
    }

    // Вычисляем коэффициент масштабирования вдоль координаты y
    std::optional<double> height_zoom;
    if (!IsZero(max_lat_ - min_lat)) {
        height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
    }

    if (width_zoom && height_zoom) {
        // Коэффициенты масштабирования по ширине и высоте ненулевые,
        // берём минимальный из них
        zoom_coeff_ = std::min(*width_zoom, *height_zoom);
    } else if (width_zoom) {
        // Коэффициент масштабирования по ширине ненулевой, используем его
        zoom_coeff_ = *width_zoom;
    } else if (height_zoom) {
        // Коэффициент масштабирования по высоте ненулевой, используем его
        zoom_coeff_ = *height_zoom;
    }
}

}