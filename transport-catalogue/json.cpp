#include "json.h"

using namespace std;

namespace json {

	namespace detail {

		Node LoadNode(std::istreambuf_iterator<char>&, const ParentPype);

		Node LoadDict(std::istreambuf_iterator<char>& it) {
			Dict result;
			auto end = std::istreambuf_iterator<char>();
			string key;

			while (it != end) {
				if (*it == '\"') {
					Node tmp_key = LoadNode(it, ParentPype::Map);
					if (tmp_key.IsString()) {
						key = (tmp_key.AsString());
					}
				}

				if (*it == ':') {
					result.insert({ move(key), LoadNode(++it, ParentPype::Map) });
				}

				if (*it == '}') {
					++it;
					return Node(move(result));
				}

				++it;
			}

			throw ParsingError("Dictionary parsing error");
		}

		Node LoadArray(std::istreambuf_iterator<char>& it) {
			Array result;
			auto end = std::istreambuf_iterator<char>();

			while (it != end && *it != ']') {
				if (*it == ',' || *it == '[') {
					++it;
					Node tmp_node = LoadNode(it, ParentPype::Array);
					if (!tmp_node.IsNull()) {
						result.push_back(move(tmp_node));
					}
				}
				else {
					++it;
				}
			}

			if (it != end && *it == ']') {
				++it;
				return Node(move(result));
			}
			throw ParsingError("Array parsing error");
		}

		Node LoadNumber(std::istreambuf_iterator<char>& it) {
			using namespace std::literals;
			auto end = std::istreambuf_iterator<char>();

			std::string parsed_num;

			// Считывает в parsed_num очередной символ из input
			auto read_char = [&parsed_num, &it, &end] {
				if (it == end) {
					throw ParsingError("Failed to read number from stream"s);
				}
				parsed_num += static_cast<char>(*it++);
			};

			// Считывает одну или более цифр в parsed_num из input
			auto read_digits = [read_char, &it, &end] {
				if (!std::isdigit(*it)) {
					throw ParsingError("A digit is expected"s);
				}
				while (it != end && std::isdigit(*it)) {
					read_char();
				}
			};

			if (*it == '-') {
				read_char();
			}
			// Парсим целую часть числа
			if (*it == '0') {
				read_char();
				// После 0 в JSON не могут идти другие цифры
			}
			else {
				read_digits();
			}

			bool is_int = true;

			if (it != end) {
				// Парсим дробную часть числа
				if (*it == '.') {
					read_char();
					read_digits();
					is_int = false;
				}

				if (it != end) {
					// Парсим экспоненциальную часть числа
					if (int ch = *it; ch == 'e' || ch == 'E') {
						read_char();
						if (ch = *it; ch == '+' || ch == '-') {
							read_char();
						}
						read_digits();
						is_int = false;
					}
				}
			}
			try {
				if (is_int) {
					// Сначала пробуем преобразовать строку в int
					try {
						return Node(std::stoi(parsed_num));
					}
					catch (...) {
						// В случае неудачи, например, при переполнении,
						// код ниже попробует преобразовать строку в double
					}
				}
				return Node(std::stod(parsed_num));
			}
			catch (...) {
				throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
			}
		}

		// Считывает содержимое строкового литерала JSON-документа
		// Функцию следует использовать после считывания открывающего символа ":
		Node LoadString(std::istreambuf_iterator<char>& it) {
			using namespace std::literals;
			auto end = std::istreambuf_iterator<char>();
			std::string s;
			while (true) {
				if (it == end) {
					// Поток закончился до того, как встретили закрывающую кавычку?
					throw ParsingError("String parsing error");
				}
				const char ch = *it;
				if (ch == '"') {
					// Встретили закрывающую кавычку
					++it;
					break;
				}
				else if (ch == '\\') {
					// Встретили начало escape-последовательности
					++it;
					if (it == end) {
						// Поток завершился сразу после символа обратной косой черты
						throw ParsingError("String parsing error");
					}
					const char escaped_char = *(it);
					// Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
					switch (escaped_char) {
					case 'n':
						s.push_back('\n');
						break;
					case 't':
						s.push_back('\t');
						break;
					case 'r':
						s.push_back('\r');
						break;
					case '"':
						s.push_back('"');
						break;
					case '\\':
						s.push_back('\\');
						break;
					default:
						// Встретили неизвестную escape-последовательность
						throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
					}
				}
				else if (ch == '\n' || ch == '\r') {
					// Строковый литерал внутри- JSON не может прерываться символами \r или \n
					throw ParsingError("Unexpected end of line"s);
				}
				else {
					// Просто считываем очередной символ и помещаем его в результирующую строку
					s.push_back(ch);
				}
				++it;
			}

			return Node(move(s));
		}

		Node LoadBool(std::istreambuf_iterator<char>& it) {
			string s;
			auto end = std::istreambuf_iterator<char>();

			while (it != end && *it != 'e') {
				s.push_back(*it);
				++it;
			}

			if (it != end && *it == 'e') {
				++it;
				if (s == "tru") {
					return Node(true);
				}
				else if (s == "fals") {
					return Node(false);
				}
			}
			throw ParsingError("Bool parsing error"s);
		}

		Node LoadNull(std::istreambuf_iterator<char>& it) {
			string s;
			auto end = std::istreambuf_iterator<char>();

			while (it != end && *it != ' ' && *it != ',') {
				s.push_back(*it);
				++it;
			}
			if (s == "null") {
				return Node();
			}
			throw ParsingError("Null parsing error"s);
		}

		Node LoadNode(std::istreambuf_iterator<char>& it, const detail::ParentPype parentType) {
			char c;
			auto end = std::istreambuf_iterator<char>();

			while (it != end) {
				c = *it;
				if (IgnoreChar(c)) {
					++it;
					continue;
				}

				if (c == ']' || c == '}') {
					if (parentType != detail::ParentPype::NotOpening) {
						return nullptr;
					}
					else {
						throw ParsingError(
							(parentType == detail::ParentPype::Array ? "Array "s : "Map "s) + "parsing error"s);
					}
				}

				if (c == '[') {
					return LoadArray(it);
				}
				else if (c == '{') {
					return LoadDict(++it);
				}
				else if (c == '"') {
					return LoadString(++it);
				}
				else if (c == 't' || c == 'f') {
					return LoadBool(it);
				}
				else if (c == 'n' || c == 'N' || c == ',') {
					return LoadNull(it);
				}
				else {
					return LoadNumber(it);
				}

				++it;
			}
			throw ParsingError("Unexpected ending of a stream"s);
		}

		bool IgnoreChar(const char c)
		{
			return c == '\t' || c == '\r' || c == '\n' || c == ' ';
		}

	}  // namespace detail

	//------------------- Node constructor ----------------------------

	//------------------- Node Is metod --------------------

	bool Node::IsNull() const
	{
		return holds_alternative<nullptr_t>(*this);
	}

	bool Node::IsArray() const
	{
		return holds_alternative<Array>(*this);
	}

	bool Node::IsDict() const
	{
		return holds_alternative<Dict>(*this);
	}

	bool Node::IsMap() const
	{
		return holds_alternative<Dict>(*this);
	}

	bool Node::IsBool() const
	{
		return holds_alternative<bool>(*this);
	}

	bool Node::IsInt() const
	{
		return holds_alternative<int>(*this);
	}

	bool Node::IsDouble() const
	{
		return holds_alternative<double>(*this) || holds_alternative<int>(*this);
	}

	bool Node::IsPureDouble() const
	{
		return holds_alternative<double>(*this);
	}

	bool Node::IsString() const
	{
		return holds_alternative<string>(*this);
	}

	bool Node::IsString_view() const
	{
		return holds_alternative<string_view>(*this);
	}

	//------------------- Node As metod --------------------

	int Node::AsInt() const {
		if (!IsInt()) {
			throw logic_error("Ahtung!");
		}
		return get<int>(*this);
	}

	bool Node::AsBool() const
	{
		if (!IsBool()) {
			throw logic_error("Ahtung!");
		}
		return get<bool>(*this);
	}

	double Node::AsDouble() const
	{
		if (!IsDouble()) {
			throw logic_error("Ahtung!");
		}
		if (holds_alternative<double>(*this)) {
			return get<double>(*this);
		}
		return get<int>(*this);
	}

	const std::string& Node::AsString() const
	{
		if (!IsString()) {
			throw logic_error("Ahtung!");
		}
		return get<string>(*this);
	}

	const std::string_view& Node::AsString_view() const
	{
		if (!IsString_view()) {
			throw logic_error("Ahtung!");
		}
		return get<string_view>(*this);
	}

	const Array& Node::AsArray() const
	{
		if (!IsArray()) {
			throw logic_error("Ahtung!");
		}
		return get<Array>(*this);
	}

	const Dict& Node::AsMap() const
	{
		if (!IsDict()) {
			throw logic_error("Ahtung!");
		}
		return get<Dict>(*this);
	}


	int Node::AsInt()
	{
		if (!IsInt()) {
			throw logic_error("Ahtung!");
		}
		return get<int>(*this);
	}

	bool Node::AsBool()
	{
		if (!IsBool()) {
			throw logic_error("Ahtung!");
		}
		return get<bool>(*this);
	}

	double Node::AsDouble()
	{
		if (!IsDouble()) {
			throw logic_error("Ahtung!");
		}
		if (holds_alternative<double>(*this)) {
			return get<double>(*this);
		}
		return get<int>(*this);
	}

	std::string& Node::AsString()
	{
		if (!IsString()) {
			throw logic_error("Ahtung!");
		}
		return get<string>(*this);
	}

	std::string_view& Node::AsString_view()
	{
		if (!IsString_view()) {
			throw logic_error("Ahtung!");
		}
		return get<string_view>(*this);
	}

	Array& Node::AsArray()
	{
		if (!IsArray()) {
			throw logic_error("Ahtung!");
		}
		return get<Array>(*this);
	}

	Dict& Node::AsMap()
	{
		if (!IsDict()) {
			throw logic_error("Ahtung!");
		}
		return get<Dict>(*this);
	}


	//------------------- Node Compare --------------------

	bool Node::operator==(const Node& other) const
	{
		return static_cast<variant>(*this) == static_cast<variant>(other);
	}

	bool Node::operator!=(const Node& other) const
	{
		return static_cast<variant>(*this) != static_cast<variant>(other);
	}

	//------------------- Print ----------------------------

		// Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
	void PrintContext::PrintIndent() const {
		for (int i = 0; i < indent; ++i) {
			out.put(' ');
		}
	}

	PrintContext PrintContext::Indented() const {
		return { out, indent_step, indent_step + indent };
	}

	void Node::Print(PrintContext output) const
	{
		std::visit(
			[&output, this](const auto& value) { PrintValue(value, output); },
			static_cast<variant>(*this));
	}

	void Node::PrintValue(const int num, PrintContext output) const
	{
		output.out << num;
	}

	void Node::PrintValue(const double num, PrintContext output) const
	{
		output.out << num;
	}

	void Node::PrintValue(string str, PrintContext output) const { // string
		// \r,\n,\\,\"
		output.out << "\"";
		for (const char ch : str) {
			switch (ch)
			{
			case '\r':
				output.out << "\\r";
				break;
			case '\n':
				output.out << "\\n";
				break;
			case '\\':
				output.out << "\\\\";
				break;
			case '"':
				output.out << "\\\"";
				break;
			default:
				output.out << ch;
				break;
			}
		}
		output.out << "\"";
	}

	void Node::PrintValue(std::string_view str, PrintContext output) const
	{
		// \r,\n,\\,\"
		output.out << "\"";
		for (const char ch : str) {
			switch (ch)
			{
			case '\r':
				output.out << "\\r";
				break;
			case '\n':
				output.out << "\\n";
				break;
			case '\\':
				output.out << "\\\\";
				break;
			case '"':
				output.out << "\\\"";
				break;
			default:
				output.out << ch;
				break;
			}
		}
		output.out << "\"";
	}

	void Node::PrintValue(std::nullptr_t, PrintContext output) const { // null
		output.out << "null"sv;
	}

	void Node::PrintValue(const bool bool_value, PrintContext output) const { // bool
		output.out << std::boolalpha << bool_value;
	}

	void Node::PrintValue(const Dict dict, PrintContext output) const { // map / Dict
		bool first = true;
		output.out << "{ \n";
		PrintContext out2 = output.Indented();
		for (const auto item : dict) {
			if (!first) {
				out2.out << ",\n";
			}
			out2.PrintIndent();
			out2.out << "\"" << item.first << "\": ";
			item.second.Print(out2);
			first = false;
		}
		output.out << "\n";
		output.PrintIndent();
		output.out << "}";
	}

	void Node::PrintValue(const Array array, PrintContext output) const { // Array
		bool first = true;
		output.out << "\n";
		output.PrintIndent();
		output.out << "[\n";
		PrintContext out2 = output.Indented();
		for (const auto& item : array) {
			if (!first) {
				out2.out << ",\n";
			}
			out2.PrintIndent();
			item.Print(out2);
			first = false;
		}
		output.out << "\n";
		output.PrintIndent();
		output.out << "]";
	}


	//------------------- / Node / ----------------------------


	Document::Document(Node root)
		: root_(move(root)) {
	}

	Node& Document::GetRoot() {
		return root_;
	}

	const Node& Document::GetRoot() const {
		return root_;
	}

	bool Document::operator==(const Document& other)
	{
		return this->GetRoot() == other.GetRoot();
	}

	bool Document::operator!=(const Document& other)
	{
		return this->GetRoot() != other.GetRoot();
	}

	Document Load(istream& input) {
		auto it = std::istreambuf_iterator<char>(input);
		return Document{ detail::LoadNode(it, detail::ParentPype::NotOpening) };
	}


	void Print(const Document& doc, std::ostream& output) {
		doc.GetRoot().Print(PrintContext{ output });
	}

}  // namespace json