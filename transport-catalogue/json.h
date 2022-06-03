#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <stdexcept>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    namespace detail {
        enum class ParentPype
        {
            Array,
            Map,
            NotOpening,
        };
        Node LoadString(std::istreambuf_iterator<char>& it);
        Node LoadDict(std::istreambuf_iterator<char>& it);
        Node LoadArray(std::istreambuf_iterator<char>& it);
        Node LoadNumber(std::istreambuf_iterator<char>& it);
        Node LoadString(std::istreambuf_iterator<char>& it);
        Node LoadBool(std::istreambuf_iterator<char>& it);
        Node LoadNull(std::istreambuf_iterator<char>& it);
        Node LoadNode(std::istreambuf_iterator<char>& it, const ParentPype parentType);
        bool IgnoreChar(const char);

    }    // namespace detail

    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const;
        PrintContext Indented() const;
    };

    class Node final
        : std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string, std::string_view> {
    public:
        using variant::variant;

        bool IsNull() const;
        bool IsArray() const;
        bool IsDict() const;
        bool IsMap() const;
        bool IsBool() const;
        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsString() const;
        bool IsString_view() const;

        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;
        const std::string& AsString() const;
        const std::string_view& AsString_view() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

        int AsInt();
        bool AsBool();
        double AsDouble();
        std::string& AsString();
        std::string_view& AsString_view();
        Array& AsArray();
        Dict& AsMap();

        bool operator==(const Node& other) const;
        bool operator!=(const Node& other) const;

        void Print(PrintContext output) const;

    private:

        void PrintValue(const int num, PrintContext) const;
        void PrintValue(const double num, PrintContext) const;
        void PrintValue(std::string str, PrintContext) const;
        void PrintValue(std::string_view str, PrintContext) const;
        void PrintValue(std::nullptr_t, PrintContext) const;
        void PrintValue(const bool bool_value, PrintContext) const;
        void PrintValue(const Dict dict, PrintContext) const;
        void PrintValue(const Array array, PrintContext) const;
    };

    class Document {
    public:
        explicit Document(Node root);

        Node& GetRoot();
        const Node& GetRoot() const;
        bool operator==(const Document& other);
        bool operator!=(const Document& other);

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json