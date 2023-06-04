#include "map_renderer.h"

namespace catalogue::renderer {

MapRenderer::MapRenderer(Settings settings)
    : settings_(std::move(settings))
{
}

std::vector<svg::Polyline> MapRenderer::RenderLines(
    const std::vector<const Bus*>& sorted_buses
) const {
    std::vector<svg::Polyline> result;

    for (size_t i = 0; i < sorted_buses.size(); ++i) {
        svg::Polyline line;
        line.SetStrokeColor(GetCurrentColor(i));
        line.SetFillColor(svg::NoneColor);
        line.SetStrokeWidth(settings_.line_width);
        line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        for (const auto* stop : sorted_buses[i]->stops) {
            line.AddPoint(projector_(stop->coords));
        }

        result.push_back(line);
    }

    return result;
}

std::vector<svg::Text> MapRenderer::RenderBusNames(
    const std::vector<const Bus*>& sorted_buses
) const {
    std::vector<svg::Text> result;

    for (size_t i = 0; i < sorted_buses.size(); ++i) {
        AddTexts(
            result,
            sorted_buses[i]->stops[0]->coords,
            GetCurrentColor(i),
            sorted_buses[i]->name
        );

        if (!sorted_buses[i]->is_roundtrip) {
            size_t end_stop_idx = sorted_buses[i]->stops.size() / 2;
            AddTexts(
                result,
                sorted_buses[i]->stops[end_stop_idx]->coords,
                GetCurrentColor(i),
                sorted_buses[i]->name
            );
        }
    }

    return result;
}

void MapRenderer::AddTexts(
    std::vector<svg::Text>& to,
    geo::Coordinates coords,
    svg::Color color,
    const std::string& data
) const {
    using namespace std::literals;
    svg::Text under_text;
    svg::Text text;

    for (auto t : {&under_text, &text}) {
        t->SetPosition(projector_(coords));
        t->SetOffset(settings_.bus_label_offset);
        t->SetFontSize(settings_.bus_label_font_size);
        t->SetFontFamily("Verdana"s);
        t->SetFontWeight("bold"s);
        t->SetData(data);
    }

    under_text.SetFillColor(settings_.underlayer_color);
    under_text.SetStrokeColor(settings_.underlayer_color);
    under_text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    under_text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    text.SetFillColor(color);

    to.push_back(under_text);
    to.push_back(text);
}

std::vector<svg::Circle> MapRenderer::RenderStopCircles(
        const std::vector<const Stop*>& sorted_stops
) const {
    using namespace std::literals;
    std::vector<svg::Circle> result;

    for (const auto stop : sorted_stops) {
        svg::Circle c;
        c.SetCenter(projector_(stop->coords));
        c.SetRadius(settings_.stop_radius);
        c.SetFillColor("white"s);

        result.push_back(c);
    }

    return result;
}

svg::Color MapRenderer::GetCurrentColor(size_t idx) const {
    return settings_.color_palette[idx % settings_.color_palette.size()];
}

}