#include "svg.h"

namespace svg {

using namespace std::literals;

Rgb::Rgb(uint8_t r, uint8_t g, uint8_t b)
    : red(r)
    , green(g)
    , blue(b)
{
}

Rgba::Rgba(uint8_t r, uint8_t g, uint8_t b, double op)
    : red(r)
    , green(g)
    , blue(b)
    , opacity(op)
{
}

void ColorPrinter::operator()(std::monostate) {
    using namespace std::literals;
    out << "none"sv;
}

void ColorPrinter::operator()(std::string color) {
    out << color;
}

void ColorPrinter::operator()(svg::Rgb rgb) {
    using namespace std::literals;
    out << "rgb("sv
        << static_cast<unsigned int>(rgb.red) << ","sv
        << static_cast<unsigned int>(rgb.green) << ","sv
        << static_cast<unsigned int>(rgb.blue) << ")"sv;
}

std::ostream& operator<<(std::ostream& out, Color color) {
    std::visit(ColorPrinter{out}, color);
    return out;
}

void ColorPrinter::operator()(svg::Rgba rgba) {
    using namespace std::literals;
    out << "rgba("sv
        << static_cast<unsigned int>(rgba.red) << ","sv
        << static_cast<unsigned int>(rgba.green) << ","sv
        << static_cast<unsigned int>(rgba.blue) << ","sv
        << rgba.opacity << ")"sv;
}

std::ostream& operator<<(std::ostream& out, StrokeLineCap stroke_line_cap) {
    out << line_cap_literals.at(stroke_line_cap);
    return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineJoin stroke_line_join) {
    out << line_join_literals.at(stroke_line_join);
    return out;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

RenderContext::RenderContext(std::ostream& out)
    : out(out) {
}

RenderContext::RenderContext(std::ostream& out, int indent_step, int indent)
    : out(out)
    , indent_step(indent_step)
    , indent(indent) {
}

RenderContext RenderContext::Indented() const {
    return {out, indent_step, indent + indent_step};
}

void RenderContext::RenderIndent() const {
    for (int i = 0; i < indent; ++i) {
        out.put(' ');
    }
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\" "sv;

    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    bool is_first = true;
    for (const auto& p : points_) {
        if (is_first) {
            is_first = false;
        } else {
            out << " "sv;
        }
        out << p.x << ","sv << p.y;
    }
    out << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Text ------------------
// Задаёт координаты опорной точки (атрибуты x и y)
Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}

// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

// Задаёт размеры шрифта (атрибут font-size)
Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}

// Задаёт название шрифта (атрибут font-family)
Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}

// Задаёт толщину шрифта (атрибут font-weight)
Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}

// Задаёт текстовое содержимое объекта (отображается внутри тега text)
Text& Text::SetData(std::string data) {
    data_ = Process(data);
    return *this;
}

std::string Text::Process(std::string data) {
    size_t i = 0;
    size_t len = data.size();

    while(i < len) {
        if (special_symbols_.count(data[i])) {
            char symbol = data[i];
            data.erase(i);
            data.insert(i, special_symbols_.at(symbol));
            i += special_symbols_.at(symbol).size();
            len += special_symbols_.at(symbol).size() - 1;
        } else {
            ++i;
        }
    }
    return data;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text";
    RenderAttrs(out);
    out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\""sv;
    out << " dx=\""sv << offset_.x << "\" dy=\"" << offset_.y << "\""sv;
    out << " font-size=\""sv << font_size_ << "\""sv;

    if (font_family_) {
        out << " font-family=\""sv << font_family_.value() << "\""sv;
    }
    if (font_weight_) {
        out << " font-weight=\""sv << font_weight_.value() << "\""sv;
    }
    out << ">"sv;
    out << data_;
    out << "</text>"sv;
}

// ---------- Document ------------------
void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.push_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n";
    RenderContext context(out);
    for (const auto& obj : objects_) {
        obj->Render(context);
    }
    out << "</svg>";
}

}  // namespace svg