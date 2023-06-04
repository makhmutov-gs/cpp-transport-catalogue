#include "map_renderer.h"

namespace catalogue::renderer {

MapRenderer::MapRenderer(Settings settings)
    : settings_(std::move(settings))
{
}

svg::Document MapRenderer::RenderRoutes(
    const std::vector<geo::Coordinates>& coords,
    const std::vector<const Bus*>& sorted_buses
) const {
    SphereProjector projector(
        coords.begin(),
        coords.end(),
        settings_.width,
        settings_.heigth,
        settings_.padding
    );

    svg::Document result;

    for (size_t i = 0; i < sorted_buses.size(); ++i) {
        svg::Polyline line;
        line.SetStrokeColor(GetCurrentColor(i));
        line.SetFillColor(svg::NoneColor);
        line.SetStrokeWidth(settings_.line_width);
        line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        for (const auto* stop : sorted_buses[i]->stops) {
            line.AddPoint(projector(stop->coords));
        }

        result.Add(line);
    }

    return result;
}

svg::Color MapRenderer::GetCurrentColor(size_t idx) const {
    return settings_.color_palette[idx % settings_.color_palette.size()];
}

}