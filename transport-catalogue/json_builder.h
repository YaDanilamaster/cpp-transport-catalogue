#pragma once
#include "json.h"

#include <stack>
#include <vector>
#include <map>
#include <string>
#include <utility>


namespace json {
	using namespace std::literals;

	class Builder;
	class KeyItemContext;
	class EndContext;
	class ValueContext;
	class ArrayItemContext;
	class DictItemContext;

	class ItemContext {
	public:
		ItemContext(Builder* bld) : builder_(bld) {
		}
		ValueContext Value(Node node);
		KeyItemContext Key(std::string str);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		EndContext EndArray();
		EndContext EndDict();
		Node Build();
		Builder* GetBuilder();
	private:
		Builder* builder_;
	};

	class DictItemContext : private ItemContext
	{
	public:
		using ItemContext::ItemContext;
		using ItemContext::Key;
		using ItemContext::EndDict;
	};

	class ArrayItemContext : private ItemContext
	{
	public:
		using ItemContext::ItemContext;
		ArrayItemContext Value(Node node);
		using ItemContext::StartDict;
		using ItemContext::StartArray;
		using ItemContext::EndArray;
	};

	class KeyItemContext : private ItemContext
	{
	public:
		using ItemContext::ItemContext;
		DictItemContext Value(Node node);
		using ItemContext::StartDict;
		using ItemContext::StartArray;
	};

	class EndContext : private ItemContext
	{
	public:
		using ItemContext::ItemContext;
		using ItemContext::Value;
		using ItemContext::Key;
		using ItemContext::StartDict;
		using ItemContext::EndDict;
		using ItemContext::Build;
	};

	class ValueContext : private ItemContext
	{
	public:
		using ItemContext::ItemContext;
		using ItemContext::StartDict;
		using ItemContext::Value;
		using ItemContext::EndArray;
		using ItemContext::Build;
	};


	class Builder
	{
	public:

		Builder();
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		ValueContext Value(Node);

		Builder& operator==(Builder&) = delete;
		Builder& operator=(Builder&) = delete;
		Builder(Builder&) = delete;

	private:
		KeyItemContext Key(std::string&);
		EndContext EndDict();
		EndContext EndArray();
		Node Build();

		friend ItemContext;
	private:
		enum class NodeType {
			Array,
			Key,
			Value,
			Dict,
			Root,
		};
		std::string nodeTypeToStr_[5]{ "Array"s, "Key"s, "Value"s, "Dict"s, "Root"s };

		struct NodeItem {
			Node node;
			std::string key;
			NodeType type;
			std::vector<NodeType> expected_element;
		};

		std::stack<NodeItem> nodes_stack_;
		Node root_;
		size_t count_item_ = 0;

		void CheckExpectedElement(NodeType) const;

	};

} // namespace json