#include "json.h"
#include <optional>

namespace json {
using namespace std::literals;

class Builder;
class BaseContext;
class DictItemContext;
class DictKeyContext;
class ArrayItemContext;

class BaseContext {
public:
    BaseContext(Builder& b);
    DictKeyContext Key(std::string key);
    BaseContext Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    BaseContext EndDict();
    BaseContext EndArray();
    Node Build();

protected:
    Builder& GetBuilder();

private:
    Builder& b_;
};

class DictItemContext
    : public BaseContext {
public:
    BaseContext Value(Node::Value value) = delete;
    DictItemContext StartDict() = delete;
    ArrayItemContext StartArray() = delete;
    BaseContext EndArray() = delete;
    Node Build() = delete;
};

class DictKeyContext
    : public BaseContext {
public:
    DictItemContext Value(Node::Value value);
    DictKeyContext Key(std::string key) = delete;
    BaseContext EndDict() = delete;
    BaseContext EndArray() = delete;
    Node Build() = delete;
};

class ArrayItemContext
    : public BaseContext {
public:
    ArrayItemContext Value(Node::Value value);
    DictKeyContext Key(std::string key) = delete;
    BaseContext EndDict() = delete;
    Node Build() = delete;
};

class Builder {
public:
    DictKeyContext Key(std::string key);
    Builder& Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    BaseContext EndDict();
    BaseContext EndArray();
    Node Build();

private:
    Node root_;
    std::vector<Node*> nodes_stack_;
    std::optional<std::string> current_key_;
    bool is_built_ = false;

    void CheckIfBuilt();

    template <typename T>
    void StartInDict();

    template <typename T>
    void StartInArray();

    template <typename T>
    void StartContainer();
};

template <typename T>
void Builder::StartInDict() {
    auto& current_dict = std::get<Dict>(nodes_stack_.back()->GetValue());
    current_dict.insert({*current_key_, T()});
    nodes_stack_.push_back(&(current_dict.at(*current_key_)));
    current_key_.reset();
}

template <typename T>
void Builder::StartInArray() {
    auto& current_array = std::get<Array>(nodes_stack_.back()->GetValue());
    current_array.push_back(T());
    nodes_stack_.push_back(&current_array.back());
}

template <typename T>
void Builder::StartContainer() {
    CheckIfBuilt();
    if (root_.IsNull()) {
        root_ = T();
        nodes_stack_.push_back(&root_);
        return;
    }

    if (nodes_stack_.back()->IsDict() && current_key_) {
        StartInDict<T>();
    } else if (nodes_stack_.back()->IsArray()) {
        StartInArray<T>();
    } else {
        throw std::logic_error("Container can't be started."s);
    }
}

}