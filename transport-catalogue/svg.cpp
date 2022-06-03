#include "svg.h"

namespace svg {

    using namespace std::literals;

    std::ostream& operator<<(std::ostream& os, const StrokeLineCap val)
    {
        os << strokeLineCap_tostr[static_cast<int>(val)];
        return os;
    }
    std::ostream& operator<<(std::ostream& os, const StrokeLineJoin val)
    {
        os << strokeLineJoin_tostr[static_cast<int>(val)];
        return os;
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(context);
        out << "/>"sv;
    }

    // ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.emplace_back(std::move(point));
        return *this;
    }

    const std::vector<Point>& Polyline::GetPoints() const {
        return points_;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        bool first = true;
        out << "<polyline points=\""sv;
        for (const Point& point : points_) {
            if (!first) out << " ";
            out << point.x << "," << point.y;
            first = false;
        }
        out << "\"";
        RenderAttrs(context);
        out << "/>"sv;
    }

    // ---------- Text ------------------

    Text& Text::SetPosition(Point pos)
    {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset)
    {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size)
    {
        fontsize_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family)
    {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight)
    {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    Text& Text::SetData(std::string data)
    {
        data_ = std::move(data);
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        std::string safe_data;

        auto& out = context.out;
        out << "<text"sv;
        RenderAttrs(context);
        out << " x=\""sv << pos_.x << "\" "sv << "y=\""sv << pos_.y << "\""sv;
        out << " dx=\""sv << offset_.x << "\" "sv << "dy=\""sv << offset_.y << "\""sv;
        out << " font-size=\""sv << fontsize_ << "\""sv;
        if (!font_family_.empty()) {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }
        out << ">"sv << GetSafeData() << "<"sv;
        out << "/text>"sv;
    }

    std::string Text::GetSafeData() const
    {
        std::string result;
        for (char c : data_) {
            if (c == '"') {
                result += "&quot;";
            }
            else if (c == '\'') {
                result += "&apos;";
            }
            else if (c == '<') {
                result += "&lt;";
            }
            else if (c == '>') {
                result += "&gt;";
            }
            else if (c == '&') {
                result += "&amp;";
            }
            else {
                result += c;
            }
        }
        return result;
    }

    // ---------- Document ------------------

    void Document::AddPtr(std::unique_ptr<Object>&& obj)
    {
        objects_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const
    {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

        for (size_t i = 0; i < objects_.size(); ++i) {
            objects_[i]->Render(RenderContext{ out });
        }

        out << "</svg>"sv;
    }

}  // namespace svg

namespace shapes {
    void shapes::Triangle::Draw(svg::ObjectContainer& container) const
    {
        container.Add(svg::Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
    }

    shapes::Star::Star(svg::Point center, double outer_rad, double inner_rad, int num_rays)
        : center_(center)
        , outer_rad_(outer_rad)
        , inner_rad_(inner_rad)
        , num_rays_(num_rays)
    {
    }

    void shapes::Star::Draw(svg::ObjectContainer& container) const
    {
        svg::Polyline star = CreateStar(center_, outer_rad_, inner_rad_, num_rays_);
        star.SetFillColor("red").SetStrokeColor("black");
        container.Add(star);
    }

    svg::Polyline shapes::Star::CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays) const
    {
        using namespace svg;
        Polyline polyline;
        for (int i = 0; i <= num_rays; ++i) {
            double angle = 2 * M_PI * (i % num_rays) / num_rays;
            polyline.AddPoint({ center.x + outer_rad * sin(angle), center.y - outer_rad * cos(angle) });
            if (i == num_rays) {
                break;
            }
            angle += M_PI / num_rays;
            polyline.AddPoint({ center.x + inner_rad * sin(angle), center.y - inner_rad * cos(angle) });
        }
        return polyline;
    }

    shapes::Snowman::Snowman(svg::Point head_center, double head_rad)
        : head_center_(head_center)
        , head_rad_(head_rad)
    {
    }

    void shapes::Snowman::Draw(svg::ObjectContainer& container) const
    {
        svg::Circle legs = svg::Circle().SetCenter({ head_center_.x, (head_center_.y + head_rad_ * 3) + head_rad_ * 2 }).SetRadius(head_rad_ * 2);
        svg::Circle body = svg::Circle().SetCenter({ head_center_.x, head_center_.y + head_rad_ * 2 }).SetRadius(head_rad_ * 1.5);
        svg::Circle head = svg::Circle().SetCenter(head_center_).SetRadius(head_rad_);

        legs.SetFillColor("rgb(240,240,240)").SetStrokeColor("black");
        body.SetFillColor("rgb(240,240,240)").SetStrokeColor("black");
        head.SetFillColor("rgb(240,240,240)").SetStrokeColor("black");

        container.Add(legs);
        container.Add(body);
        container.Add(head);
    }

} // namespace shapes