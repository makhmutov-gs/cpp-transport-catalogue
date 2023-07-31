#include <filesystem>

#include "svg.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "domain.h"

#include <transport_catalogue.pb.h>

namespace catalogue::serialization {

void SaveWithSettings(
    const std::filesystem::path& path,
    const TransportCatalogue& cat,
    const renderer::Settings& render_settings,
    const domain::RoutingSettings& routing_settings
);

std::tuple<TransportCatalogue, renderer::Settings, domain::RoutingSettings> FromFile(const std::filesystem::path& path);

proto_transport::Stop StopToProto(const Stop& s, int32_t id);
proto_router::RoutingSettings RoutingSettingsToProto(const domain::RoutingSettings& settings);

struct ProtoColorGetter {
    proto_render::Color operator()(std::monostate);
    proto_render::Color operator()(const std::string& str);
    proto_render::Color operator()(const svg::Rgb& rgb);
    proto_render::Color operator()(const svg::Rgba& rgba);
};

svg::Color GetColorFromProto(const proto_render::Color& proto_color);
domain::RoutingSettings RoutingSettingsFromProto(const proto_router::RoutingSettings& proto_settings);

proto_render::RendererSettings RendererSettingsToProto(const renderer::Settings& settings);
renderer::Settings RendererSettingFromProto(const proto_render::RendererSettings& proto_settings);

}