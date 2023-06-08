#include "json.h"
#include <exception>

using namespace std::literals;

namespace json {

Node::Node(Value value) : variant(std::move(value)) {}

bool Node::IsInt() const {
    return std::holds_alternative<int>(*this);
}

bool Node::IsDouble() const {
    return IsInt() || IsPureDouble();
}

bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(*this);
}

bool Node::IsBool() const {
    return std::holds_alternative<bool>(*this);
}

bool Node::IsString() const {
    return std::holds_alternative<std::string>(*this);
}

bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(*this);
}

bool Node::IsArray() const {
    return std::holds_alternative<Array>(*this);
}

bool Node::IsDict() const {
    return std::holds_alternative<Dict>(*this);
}

int Node::AsInt() const {
    if (!IsInt()) {
        throw InvalidTypeError("Value is not an int.");
    }
    return std::get<int>(*this);
}

bool Node::AsBool() const {
    if (!IsBool()) {
        throw InvalidTypeError("Value is not a bool.");
    }
    return std::get<bool>(*this);
}

double Node::AsDouble() const {
    if (IsPureDouble()) {
        return std::get<double>(*this);
    } else if (IsInt()) {
        return static_cast<double>(AsInt());
    }

    throw InvalidTypeError("Value is not a number.");
}

const std::string& Node::AsString() const {
    if (!IsString()) {
        throw InvalidTypeError("Value is not a string.");
    }
    return std::get<std::string>(*this);
}

const Array& Node::AsArray() const {
    if (!IsArray()) {
        throw InvalidTypeError("Value is not an array.");
    }
    return std::get<Array>(*this);
}

const Dict& Node::AsDict() const {
    if (!IsDict()) {
        throw InvalidTypeError("Value is not a map.");
    }
    return std::get<Dict>(*this);
}

const Node::Value& Node::GetValue() const {
    return *this;
}

Node::Value& Node::GetValue() {
    return *this;
}

bool Node::operator==(const Node& rhs) const {
    return GetValue() == rhs.GetValue();
}

bool Node::operator!=(const Node& rhs) const {
    return !(*this == rhs);
}

Node LoadNode(std::istream& input);
Node LoadString(std::istream& input);

Node LoadArray(std::istream& input) {
    Array result;

    while(true) {
        char c;
        if (!(input >> c)) {
            throw ParsingError("Array parsing error.");
        } else if (c == ']') {
            break;
        } else if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    return Node(std::move(result));
}

Node LoadDict(std::istream& input) {
    Dict result;

    while(true) {
        char c;
        if (!(input >> c)) {
            throw ParsingError("Dict parsing error.");
        } else if (c == '}') {
            break;
        } else if (c == ',') {
            input >> c;
        }

        std::string key = LoadString(input).AsString();
        input >> c;
        result.insert({std::move(key), LoadNode(input)});
    }

    return Node(std::move(result));
}

Node LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return Node(std::stoi(parsed_num));
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return Node(std::stod(parsed_num));
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
Node LoadString(std::istream& input) {
    using namespace std::literals;

    auto it = std::istreambuf_iterator<char>(input);
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
        } else if (ch == '\\') {
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
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return Node(s);
}

Node LoadNull(std::istream& input) {
    char c;
    input >> c;

    std::string s;
    s += c;

    if (c == 'n') {
        for (size_t i = 0; i < 3; ++i) {
            if (input >> c) {
                s += c;
            } else {
                throw ParsingError("Null parsing error."s);
            }
        }

        if (s != "null") {
            throw ParsingError("Unexpected non-null type."s);
        } else {
            return Node(nullptr);
        }

    } else {
        throw ParsingError("Unexpected non-null type."s);
    }
}

Node LoadBool(std::istream& input) {
    char c;
    input >> c;

    std::string s;
    s += c;

    if (c == 't') {

        for (size_t i = 0; i < 3; ++i) {
            if (input >> c) {
                s += c;
            } else {
                throw ParsingError("Bool parsing error."s);
            }
        }

        if (s != "true") {
            throw ParsingError("Unexpected non-boolean type."s);
        } else {
            return Node(true);
        }

    } else if (c == 'f') {

        for (size_t i = 0; i < 4; ++i) {
            if (input >> c) {
                s += c;
            } else {
                throw ParsingError("Null parsing error."s);
            }
        }

        if (s != "false") {
            throw ParsingError("Unexpected non-boolean type."s);
        } else {
            return Node(false);
        }

    } else {
        throw ParsingError("Unexpected non-boolean type."s);
    }
}

Node LoadNode(std::istream& input) {
    char c;
    input >> c;

    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else if (c == 't' || c == 'f') {
        input.putback(c);
        return LoadBool(input);
    } else if (c == 'n') {
        input.putback(c);
        return LoadNull(input);
    } else {
        input.putback(c);
        return LoadNumber(input);
    }
}

Document::Document(Node root)
    : root_(std::move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

bool operator==(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}

bool operator!=(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() != rhs.GetRoot();
}

Document Load(std::istream& input) {
    return Document{LoadNode(input)};
}

void PrintNode(const Node& node, std::ostream& out);

struct ValuePrinter {
    std::ostream& out;

    ValuePrinter(std::ostream& output)
        : out(output)
    {
    }

    template <typename Value>
    void operator()(const Value& value) {
        out << value;
    }

    void operator()(const std::string& value) {
        out << "\"";

        for (char c : value) {
            switch (c) {
                case '"':
                    out << "\\\"";
                    break;
                case '\\':
                    out << "\\\\";
                    break;
                case '\n':
                    out << "\\n";
                    break;
                case '\t':
                    out << "\t";
                    break;
                case '\r':
                    out << "\\r";
                    break;
                default:
                    out << c;
                    break;
            }
        }

        out << "\"";
    }

    void operator()(std::nullptr_t) {
        out << "null"sv;
    }

    void operator()(const Array& arr) {
        out << "["sv;
        bool is_first = true;
        for (const auto& node : arr) {
            if (is_first) {
                is_first = false;
            } else {
                out << ", ";
            }
            PrintNode(node, out);
        }
        out << "]"sv;
    }

    void operator()(const Dict& dict) {
        out << "{"sv;
        bool is_first = true;
        for (const auto& [key, node] : dict) {
            if (is_first) {
                is_first = false;
            } else {
                out << ", ";
            }
            out << "\""sv << key << "\": "sv;
            PrintNode(node, out);
        }
        out << "}"sv;
    }

    void operator()(bool value) {
        if (value) {
            out << "true"sv;
        } else {
            out << "false"sv;
        }
    }
};

void PrintNode(const Node& node, std::ostream& out) {
    std::visit(ValuePrinter(out), node.GetValue());
}

void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(), output);
}

}  // namespace json