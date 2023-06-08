#include "json_builder.h"

namespace json {

BaseContext::BaseContext(Builder& b) : b_(b) {}

DictKeyContext BaseContext::Key(std::string key) {
    return b_.Key(key);
}

BaseContext BaseContext::Value(Node::Value value) {
    return {b_.Value(value)};
}

DictItemContext BaseContext::StartDict() {
    return b_.StartDict();
}

ArrayItemContext BaseContext::StartArray() {
    return b_.StartArray();
}

BaseContext BaseContext::EndDict() {
    return b_.EndDict();
}

BaseContext BaseContext::EndArray() {
    return b_.EndArray();
}

Node BaseContext::Build() {
    return b_.Build();
}

Builder& BaseContext::GetBuilder() {
    return b_;
}

DictItemContext DictKeyContext::Value(Node::Value value) {
    return {GetBuilder().Value(value)};
}

ArrayItemContext ArrayItemContext::Value(Node::Value value) {
    return {GetBuilder().Value(value)};
}

DictKeyContext Builder::Key(std::string key) {
    CheckIfBuilt();
    if (!nodes_stack_.back()->IsDict() || current_key_) {
        throw std::logic_error("Invalid key usage.");
    }
    current_key_ = std::move(key);
    return DictKeyContext{*this};
}

Builder& Builder::Value(Node::Value value) {
    CheckIfBuilt();
    if (root_.IsNull()) {
        root_ = std::move(value);
        is_built_ = true;
        return *this;
    }

    if (nodes_stack_.back()->IsDict() && current_key_) {
        std::get<Dict>(nodes_stack_.back()->GetValue()).insert({*current_key_, value});
        current_key_.reset();
    } else if (nodes_stack_.back()->IsArray()) {
        std::get<Array>(nodes_stack_.back()->GetValue()).push_back(value);
    } else {
        throw std::logic_error("Invalid value usage."s);
    }

    return *this;
}

DictItemContext Builder::StartDict() {
    StartContainer<Dict>();
    return DictItemContext{*this};
}

ArrayItemContext Builder::StartArray() {
    StartContainer<Array>();
    return ArrayItemContext{*this};
}

BaseContext Builder::EndDict() {
    CheckIfBuilt();
    if (nodes_stack_.back()->IsDict()) {
        nodes_stack_.pop_back();
    } else {
        throw std::logic_error("EndDict() is called on non-Dict."s);
    }
    return BaseContext{*this};
}

BaseContext Builder::EndArray() {
    CheckIfBuilt();
    if (nodes_stack_.back()->IsArray()) {
        nodes_stack_.pop_back();
    } else {
        throw std::logic_error("EndArray() is called on non-Array."s);
    }
    return BaseContext{*this};
}

Node Builder::Build() {
    if (root_.IsNull() || (!nodes_stack_.empty() && (nodes_stack_.back()->IsArray() || nodes_stack_.back()->IsDict()))) {
        throw std::logic_error("Object is not finished.");
    }
    is_built_ = true;
    return root_;
}

void Builder::CheckIfBuilt() {
    if (is_built_) {
        throw std::logic_error("Object is already built."s);
    }
}

}