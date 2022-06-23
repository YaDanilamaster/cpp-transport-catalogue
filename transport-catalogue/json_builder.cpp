#include "json_builder.h"

namespace json {
	Builder::Builder()
	{
		NodeItem tmp{ root_, ""s, NodeType::Root, {NodeType::Array, NodeType::Dict, NodeType::Value} };
		nodes_stack_.push(tmp);
	}

	DictItemContext Builder::StartDict()
	{
		CheckExpectedElement(NodeType::Dict);

		NodeItem& top_stack = nodes_stack_.top();

		if (top_stack.type == NodeType::Array) {
			nodes_stack_.push({ Dict{}, ""s, NodeType::Dict, {NodeType::Key} });
		}
		else {
			nodes_stack_.push({ nodes_stack_.top().node = Dict{}, ""s, NodeType::Dict, {NodeType::Key} });
		}

		++count_item_;
		return this;
	}

	EndContext Builder::EndDict()
	{
		if (nodes_stack_.top().type == NodeType::Dict) {
			Node curdict = std::move(nodes_stack_.top().node);
			nodes_stack_.pop();

			NodeItem& top_stack = nodes_stack_.top();
			switch (top_stack.type)
			{
			case NodeType::Key:
				top_stack.node.AsMap()[std::move(top_stack.key)] = std::move(curdict);
				top_stack.type = NodeType::Dict;
				top_stack.expected_element = { NodeType::Key };
				break;
			case NodeType::Array:
				top_stack.node.AsArray().emplace_back(std::move(curdict));
				break;
			case NodeType::Root:
				top_stack.node = std::move(curdict);
				top_stack.expected_element = { NodeType::Root };
				break;
			default:
				throw std::logic_error("unknown error"s);
			}

			return this;
		}
		throw std::logic_error("End dictionary completion is incorrect"s);
	}

	ArrayItemContext Builder::StartArray()
	{
		CheckExpectedElement(NodeType::Array);

		NodeItem& top_stack = nodes_stack_.top();

		switch (top_stack.type)
		{
		case NodeType::Key:
			nodes_stack_.push({ Array{}, ""s, NodeType::Array, {NodeType::Array, NodeType::Dict, NodeType::Value} });
			break;
		case NodeType::Array:
			top_stack.node.AsArray().emplace_back(Array{});
			nodes_stack_.push({ top_stack.node.AsArray().back(), ""s, NodeType::Array, {NodeType::Array, NodeType::Dict, NodeType::Value} });
			break;
		case NodeType::Root:
			nodes_stack_.push({ Array{}, ""s, NodeType::Array, {NodeType::Array, NodeType::Dict, NodeType::Value} });
			break;
		default:
			throw std::logic_error("unknown error"s);
		}

		++count_item_;
		return this;
	}

	EndContext Builder::EndArray()
	{
		if (nodes_stack_.size() != 0 && nodes_stack_.top().type == NodeType::Array) {
			Node curarray = std::move(nodes_stack_.top().node);
			nodes_stack_.pop();

			NodeItem& top_stack = nodes_stack_.top();
			switch (top_stack.type)
			{
			case NodeType::Key:
				top_stack.node.AsMap()[std::move(top_stack.key)] = std::move(curarray);
				top_stack.type = NodeType::Dict;
				top_stack.expected_element = { NodeType::Key };
				break;
			case NodeType::Array:
				top_stack.node.AsArray().emplace_back(std::move(curarray));
				break;
			case NodeType::Root:
				top_stack.node = std::move(curarray);
				top_stack.expected_element = { NodeType::Root };
				break;
			default:
				throw std::logic_error("unknown error"s);
			}

			return this;
		}
		throw std::logic_error("End array completion is incorrect."s);
	}

	KeyItemContext Builder::Key(std::string& str)
	{
		CheckExpectedElement(NodeType::Key);

		NodeItem& top_stack = nodes_stack_.top();
		top_stack.key = std::move(str);
		top_stack.type = NodeType::Key;
		top_stack.expected_element = { NodeType::Value, NodeType::Array, NodeType::Dict };
		++count_item_;
		return this;
	}

	ValueContext Builder::Value(Node val)
	{
		CheckExpectedElement(NodeType::Value);

		NodeItem& top_stack = nodes_stack_.top();
		switch (top_stack.type)
		{
		case NodeType::Array:
			top_stack.node.AsArray().emplace_back(std::move(val));
			break;
		case NodeType::Key:
			top_stack.node.AsMap()[std::move(top_stack.key)] = std::move(val);
			top_stack.type = NodeType::Dict;
			top_stack.expected_element = { NodeType::Key };
			break;
		case NodeType::Root:
			top_stack.node = std::move(val);
			top_stack.type = NodeType::Value;
			top_stack.expected_element = { NodeType::Root };
			break;
		default:
			throw std::logic_error("unknown error"s);
		}
		++count_item_;
		return this;
	}

	Node Builder::Build()
	{
		if (nodes_stack_.size() > 1) {
			throw std::logic_error("Unclosed element: "s + nodeTypeToStr_[static_cast<int>(nodes_stack_.top().type)]);
		}
		if (count_item_ == 0) {
			throw std::logic_error("There are no elements to build"s);
		}
		root_ = std::move(nodes_stack_.top().node);

		return root_;
	}


	void Builder::CheckExpectedElement(NodeType curent_type) const
	{
		bool good = false;
		for (const NodeType type : nodes_stack_.top().expected_element) {
			good = (good || type == curent_type);
		}

		if (!good) {
			std::string tmp;
			for (const NodeType type : nodes_stack_.top().expected_element) {
				tmp += " "s + nodeTypeToStr_[static_cast<int>(type)];
			}
			throw std::logic_error(nodeTypeToStr_[static_cast<int>(curent_type)] + " completion is incorrect. Expected:"s + tmp);
		}
	}


	DictItemContext KeyItemContext::Value(Node node)
	{
		ItemContext::Value(node);
		return ItemContext::GetBuilder();
	}

	ValueContext ItemContext::Value(Node node)
	{
		return builder_->Value(node);
	}

	DictItemContext ItemContext::StartDict()
	{
		return builder_->StartDict();
	}

	ArrayItemContext ItemContext::StartArray()
	{
		return builder_->StartArray();
	}

	EndContext ItemContext::EndArray()
	{
		return builder_->EndArray();
	}

	KeyItemContext ItemContext::Key(std::string str)
	{
		return builder_->Key(str);
	}

	EndContext ItemContext::EndDict()
	{
		return builder_->EndDict();
	}

	Node ItemContext::Build()
	{
		return builder_->Build();
	}

	Builder* ItemContext::GetBuilder()
	{
		return builder_;
	}


	ArrayItemContext ArrayItemContext::Value(Node node)
	{
		ItemContext::Value(node);
		return ItemContext::GetBuilder();
	}

} // namespace json