#pragma once

#define _USE_MATH_DEFINES 
#include <cmath>

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>

namespace svg {

	struct Point {
		Point() = default;
		Point(double x, double y)
			: x(x)
			, y(y) {
		}
		double x = 0;
		double y = 0;
	};

	enum class StrokeLineCap {
		BUTT,
		ROUND,
		SQUARE,
	};
	inline const char* strokeLineCap_tostr[] = { "butt", "round", "square" };

	enum class StrokeLineJoin {
		ARCS,
		BEVEL,
		MITER,
		MITER_CLIP,
		ROUND,
	};
	inline const char* strokeLineJoin_tostr[] = { "arcs", "bevel", "miter", "miter-clip", "round" };

	std::ostream& operator<<(std::ostream& os, const StrokeLineCap val);
	std::ostream& operator<<(std::ostream& os, const StrokeLineJoin val);

	using Color = std::string;

	inline const Color NoneColor{ "none" };

	struct RenderContext {
		RenderContext(std::ostream& out)
			: out(out) {
		}

		RenderContext(std::ostream& out, int indent_step, int indent = 0)
			: out(out)
			, indent_step(indent_step)
			, indent(indent) {
		}

		RenderContext Indented() const {
			return { out, indent_step, indent + indent_step };
		}

		void RenderIndent() const {
			for (int i = 0; i < indent; ++i) {
				out.put(' ');
			}
		}

		std::ostream& out;
		int indent_step = 0;
		int indent = 0;
	};


	class Object {
	public:
		void Render(const RenderContext& context) const;

		virtual ~Object() = default;

	private:
		virtual void RenderObject(const RenderContext& context) const = 0;
	};

	class ObjectContainer {
	public:
		template <typename Obj>
		void Add(Obj obj) {
			AddPtr(std::make_unique<Obj>(std::move(obj)));
		}

		virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

		virtual ~ObjectContainer() = default;
	};

	class Drawable {
	public:
		virtual void Draw(ObjectContainer& container) const = 0;
		virtual ~Drawable() = default;
	};

	template <typename Owner>
	class PathProps {
	public:
		//����� �������� �������� fill � ���� �������. �� ��������� �������� �� ���������.
		Owner& SetFillColor(Color color) {
			fill_color_ = std::move(color);
			return AsOwner();
		}

		// ����� �������� �������� stroke � ���� �������. �� ��������� �������� �� ���������.
		Owner& SetStrokeColor(Color color) {
			stroke_color_ = std::move(color);
			return AsOwner();
		}

		//����� �������� �������� stroke-width � ������� �����. �� ��������� �������� �� ���������.
		Owner& SetStrokeWidth(double width) {
			stroke_width_ = width;
			return AsOwner();
		}

		//����� �������� �������� stroke-linecap � ��� ����� ����� �����. �� ��������� �������� �� ���������.
		Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
			stroke_linecap_ = line_cap;
			return AsOwner();
		}

		//����� �������� �������� stroke-linejoin � ��� ����� ���������� �����. �� ��������� �������� �� ���������.
		Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
			stroke_linejoin_ = line_join;
			return AsOwner();
		}

	protected:
		~PathProps() = default;

		void RenderAttrs(const RenderContext& context) const {
			using namespace std::literals;
			auto& out = context.out;


			if (fill_color_) {
				out << " fill=\""sv << *fill_color_ << "\""sv;
			}
			if (stroke_color_) {
				out << " stroke=\""sv << *stroke_color_ << "\""sv;
			}
			if (stroke_width_) {
				out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
			}
			if (stroke_linecap_) {
				out << " stroke-linecap=\""sv << strokeLineCap_tostr[static_cast<int>(*stroke_linecap_)] << "\""sv;
			}
			if (stroke_linejoin_) {
				out << " stroke-linejoin=\""sv << strokeLineJoin_tostr[static_cast<int>(*stroke_linejoin_)] << "\""sv;
			}
		}

	private:
		std::optional<Color> fill_color_;
		std::optional<Color> stroke_color_;
		std::optional<double> stroke_width_;
		std::optional<StrokeLineCap> stroke_linecap_;
		std::optional<StrokeLineJoin> stroke_linejoin_;

		Owner& AsOwner() {
			// static_cast ��������� ����������� *this � Owner&,
			// ���� ����� Owner � ��������� PathProps
			return static_cast<Owner&>(*this);
		}
	};


	/*
	 * ����� Circle ���������� ������� <circle> ��� ����������� �����
	 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
	 */
	class Circle final : public Object, public PathProps<Circle> {
	public:
		Circle& SetCenter(Point center);
		Circle& SetRadius(double radius);

	private:
		void RenderObject(const RenderContext& context) const override;

		Point center_;
		double radius_ = 1.0;
	};

	/*
	 * ����� Polyline ���������� ������� <polyline> ��� ����������� ������� �����
	 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
	 */
	class Polyline final : public Object, public PathProps<Polyline> {
	public:
		// ��������� ��������� ������� � ������� �����
		Polyline& AddPoint(Point point);

		/*
		 * ������ ������ � ������, ����������� ��� ���������� �������� <polyline>
		 */
		const std::vector<Point>& GetPoints() const;

	private:
		void RenderObject(const RenderContext& context) const override;

		std::vector<Point> points_;
	};

	/*
	 * ����� Text ���������� ������� <text> ��� ����������� ������
	 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
	 */
	class Text final : public Object, public PathProps<Text> {
	public:
		// ����� ���������� ������� ����� (�������� x � y)
		Text& SetPosition(Point pos);

		// ����� �������� ������������ ������� ����� (�������� dx, dy)
		Text& SetOffset(Point offset);

		// ����� ������� ������ (������� font-size)
		Text& SetFontSize(uint32_t size);

		// ����� �������� ������ (������� font-family)
		Text& SetFontFamily(std::string font_family);

		// ����� ������� ������ (������� font-weight)
		Text& SetFontWeight(std::string font_weight);

		// ����� ��������� ���������� ������� (������������ ������ ���� text)
		Text& SetData(std::string data);

		// ������ ������ � ������, ����������� ��� ���������� �������� <text>
	private:
		void RenderObject(const RenderContext& context) const override;
		std::string GetSafeData() const;

		Point pos_ = { 0, 0 };
		Point offset_ = { 0, 0 };
		uint32_t fontsize_ = 1;
		std::string font_family_ = "";
		std::string font_weight_ = "";
		std::string data_ = "";
	};

	class Document final : public ObjectContainer {
	public:

		// ��������� � svg-�������� ������-��������� svg::Object
		void AddPtr(std::unique_ptr<Object>&& obj) override;

		// ������� � ostream svg-������������� ���������
		void Render(std::ostream& out) const;

		// ������ ������ � ������, ����������� ��� ���������� ������ Document

	private:
		std::vector<std::unique_ptr<Object>> objects_;

	};

}  // namespace svg


namespace shapes {

	class Triangle : public svg::Drawable {
	public:
		Triangle(svg::Point p1, svg::Point p2, svg::Point p3)
			: p1_(p1)
			, p2_(p2)
			, p3_(p3) {
		}

		// ��������� ����� Draw ���������� svg::Drawable
		void Draw(svg::ObjectContainer& container) const override;

	private:
		svg::Point p1_, p2_, p3_;
	};

	class Star : public svg::Drawable { /* ���������� �������������� */
	public:
		Star(svg::Point center, double outer_rad, double inner_rad, int num_rays);
		void Draw(svg::ObjectContainer& container) const override;

	private:
		svg::Polyline CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays) const;
		svg::Point center_;
		double outer_rad_;
		double inner_rad_;
		int num_rays_;
	};

	class Snowman : public svg::Drawable { /* ���������� �������������� */
	public:
		Snowman(svg::Point head_center, double head_rad);
		void Draw(svg::ObjectContainer& container) const override;

	private:
		svg::Point head_center_;
		double head_rad_;
	};

} // namespace shapes


