#include "map_renderer.h"

namespace catalogue::renderer {

proto_render::RendererSettings RendererSettingsToProto(const Settings& settings) {
    proto_render::RendererSettings proto_render_settings;
    proto_render_settings.set_width(settings.width);
    proto_render_settings.set_height(settings.heigth);
    proto_render_settings.set_padding(settings.padding);
    proto_render_settings.set_line_width(settings.line_width);
    proto_render_settings.set_stop_radius(settings.stop_radius);
    proto_render_settings.set_bus_label_font_size(settings.bus_label_font_size);

    proto_render::Point proto_bus_label_offset;
    proto_bus_label_offset.set_x(settings.bus_label_offset.x);
    proto_bus_label_offset.set_y(settings.bus_label_offset.y);

    *proto_render_settings.mutable_bus_label_offset() = proto_bus_label_offset;

    proto_render::Point proto_stop_label_offset;
    proto_stop_label_offset.set_x(settings.stop_label_offset.x);
    proto_stop_label_offset.set_y(settings.stop_label_offset.y);

    *proto_render_settings.mutable_stop_label_offset() = proto_stop_label_offset;

    proto_render_settings.set_stop_label_font_size(settings.stop_label_font_size);

    *proto_render_settings.mutable_underlayer_color() = std::visit(domain::ProtoColorGetter(), settings.underlayer_color);

    proto_render_settings.set_underlayer_width(settings.underlayer_width);

    for (const auto& c : settings.color_palette) {
        *proto_render_settings.add_color_palette() = std::visit(domain::ProtoColorGetter(), c);
    }

    return proto_render_settings;
}

Settings RendererSettingFromProto(const proto_render::RendererSettings& proto_settings) {
    Settings render_settings;
    render_settings.width = proto_settings.width();
    render_settings.heigth = proto_settings.height();
    render_settings.padding = proto_settings.padding();
    render_settings.line_width = proto_settings.line_width();
    render_settings.stop_radius = proto_settings.stop_radius();
    render_settings.bus_label_font_size = proto_settings.bus_label_font_size();

    proto_render::Point proto_bus_label_offset = proto_settings.bus_label_offset();
    render_settings.bus_label_offset.x = proto_bus_label_offset.x();
    render_settings.bus_label_offset.y = proto_bus_label_offset.y();

    proto_render::Point proto_stop_label_offset = proto_settings.stop_label_offset();
    render_settings.stop_label_offset.x = proto_stop_label_offset.x();
    render_settings.stop_label_offset.y = proto_stop_label_offset.y();

    render_settings.stop_label_font_size = proto_settings.stop_label_font_size();

    render_settings.underlayer_color = domain::GetColorFromProto(proto_settings.underlayer_color());
    render_settings.underlayer_width = proto_settings.underlayer_width();

    for (int i = 0; i < proto_settings.color_palette_size(); ++i) {
        render_settings.color_palette.push_back(domain::GetColorFromProto(proto_settings.color_palette(i)));
    }

    return render_settings;
}

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {
        (coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_
    };
}

MapRenderer::MapRenderer(Settings settings)
    : settings_(std::move(settings))
{
}

svg::Document MapRenderer::Render(
    const std::vector<const Bus*>& sorted_buses,
    const std::vector<const Stop*>& sorted_stops
) {
    InitProjector(sorted_stops);

    svg::Document result;

    for (const auto& route : RenderLines(sorted_buses)) {
        result.Add(route);
    }

    for (const auto& text : RenderBusNames(sorted_buses)) {
        result.Add(text);
    }

    for (const auto& circles : RenderStopCircles(sorted_stops)) {
        result.Add(circles);
    }

    for (const auto& text : RenderStopNames(sorted_stops)) {
        result.Add(text);
    }

    return result;
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
        VisualiseBusTexts(
            result,
            sorted_buses[i]->stops[0]->coords,
            GetCurrentColor(i),
            sorted_buses[i]->name
        );

        if (!sorted_buses[i]->is_roundtrip) {
            size_t end_stop_idx = sorted_buses[i]->stops.size() / 2;
            if (sorted_buses[i]->stops[0] != sorted_buses[i]->stops[end_stop_idx]) {
                VisualiseBusTexts(
                    result,
                    sorted_buses[i]->stops[end_stop_idx]->coords,
                    GetCurrentColor(i),
                    sorted_buses[i]->name
                );
            }
        }
    }

    return result;
}

void MapRenderer::VisualiseBusTexts(
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
    under_text.SetStrokeWidth(settings_.underlayer_width);
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

std::vector<svg::Text> MapRenderer::RenderStopNames(
    const std::vector<const Stop*>& sorted_stops
) const {
    using namespace std::literals;
    std::vector<svg::Text> result;

    for (const auto stop : sorted_stops) {
        VisualiseStopTexts(
            result,
            stop->coords,
            "black"s,
            stop->name
        );
    }

    return result;
}

void MapRenderer::InitProjector(const std::vector<const Stop*>& stops) {
    std::vector<geo::Coordinates> coords;
    coords.reserve(stops.size());

    std::transform(stops.begin(), stops.end(), std::back_inserter(coords),
        [](const Stop* s){
            return s->coords;
        }
    );

    projector_ = SphereProjector(
        coords.begin(),
        coords.end(),
        settings_.width,
        settings_.heigth,
        settings_.padding
    );
}

void MapRenderer::VisualiseStopTexts(
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
        t->SetOffset(settings_.stop_label_offset);
        t->SetFontSize(settings_.stop_label_font_size);
        t->SetFontFamily("Verdana"s);
        t->SetData(data);
    }

    under_text.SetFillColor(settings_.underlayer_color);
    under_text.SetStrokeColor(settings_.underlayer_color);
    under_text.SetStrokeWidth(settings_.underlayer_width);
    under_text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    under_text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    text.SetFillColor(color);

    to.push_back(under_text);
    to.push_back(text);
}

svg::Color MapRenderer::GetCurrentColor(size_t idx) const {
    return settings_.color_palette[idx % settings_.color_palette.size()];
}

}