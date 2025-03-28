/* Copyright 2025 Alessandro Salerno
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <unordered_map>

namespace louvre {
enum class ParserAction { End, AddChild, AddChildAndBranch, Ignore };

enum class StandardNodeType {
    Root,
    Left,
    Center,
    Right,
    Justify,
    Paragraph,
    Numebrs,
    Bullets,
    Item,
    Text,
    LineBreak,
    Null,
    Group
};

class SourceLocation {
    private:
    const std::size_t mLine;
    const std::size_t mColumn;
    const std::size_t mGlobalOffset;
    const std::size_t mLineOffset;

    public:
    SourceLocation(std::size_t line,
                   std::size_t column,
                   std::size_t global_offset,
                   std::size_t line_offset)
        : mLine(line), mColumn(column), mGlobalOffset(global_offset),
          mLineOffset(line_offset) {};

    inline const std::size_t line() const {
        return this->mLine;
    }

    inline const std::size_t column() const {
        return this->mColumn;
    }

    inline const std::size_t global_offset() const {
        return this->mGlobalOffset;
    }

    inline const std::size_t line_offset() const {
        return this->mLineOffset;
    }
};

class Tag {
    private:
    const std::string        mName;
    const SourceLocation     mLocation;
    std::vector<std::string> mArguments;

    public:
    Tag(std::string name, SourceLocation location)
        : mName(name), mLocation(location) {};

    inline const std::string name() const {
        return this->mName;
    }

    inline const SourceLocation location() const {
        return this->mLocation;
    }

    inline const std::vector<std::string> arguments() const {
        return this->mArguments;
    }

    inline void add_argument(std::string argument) {
        this->mArguments.push_back(argument);
    }
};

class Node : public std::enable_shared_from_this<Node> {
    private:
    const std::variant<StandardNodeType, std::string> mType;
    std::optional<std::string>                        mText;
    std::optional<std::shared_ptr<Tag>>               mTag;
    std::optional<std::shared_ptr<Node>>              mParent;
    std::vector<std::shared_ptr<Node>>                mChildren;
    std::size_t                                       mNum;

    public:
    Node() : Node(StandardNodeType::Root) {};
    Node(StandardNodeType type) : mType(type) {};
    Node(std::string type) : mType(type), mNum(0) {};
    Node(Node &&other) noexcept = default;

    static inline Node text(std::string text) {
        Node n(StandardNodeType::Text);
        n.mText = text;
        return n; // ret val optimization helps here
    }

    inline const std::variant<StandardNodeType, std::string> type() const {
        return this->mType;
    }

    inline const std::optional<std::string> text() const {
        return this->mText;
    }

    inline const std::optional<std::shared_ptr<Tag>> tag() const {
        return this->mTag;
    }

    inline const std::optional<std::shared_ptr<Node>> parent() const {
        return this->mParent;
    }

    inline const std::vector<std::shared_ptr<Node>> children() const {
        return this->mChildren;
    }

    inline const std::size_t number() const {
        return this->mNum;
    }

    inline void add_child(std::shared_ptr<Node> child) {
        child->mNum = this->children().size();
        this->add_dangling_child(child);
        child->set_parent(this->shared_from_this());
    }

    inline void add_dangling_child(std::shared_ptr<Node> child) {
        this->mChildren.push_back(child);
    }

    inline void set_parent(std::shared_ptr<Node> parent) {
        this->mParent = parent;
    }

    inline void set_tag(std::shared_ptr<Tag> tag) {
        this->mTag = tag;
    }
};

class SyntaxError {
    private:
    const std::string    mMessage;
    const SourceLocation mLocation;

    public:
    SyntaxError(std::string message, SourceLocation location)
        : mMessage(message), mLocation(location) {};

    inline const std::string message() const {
        return this->mMessage;
    }

    inline const SourceLocation location() const {
        return this->mLocation;
    }
};

class TagError {
    private:
    const std::string          mMessage;
    const std::shared_ptr<Tag> mTag;

    public:
    TagError(std::string message, std::shared_ptr<Tag> tag)
        : mMessage(message), mTag(tag) {};

    inline const std::string message() const {
        return this->mMessage;
    }

    inline const std::shared_ptr<Tag> tag() const {
        return this->mTag;
    }
};

class NodeError {
    private:
    const std::string           mMessage;
    const std::shared_ptr<Node> mNode;

    public:
    NodeError(std::string message, std::shared_ptr<Node> node)
        : mMessage(message), mNode(node) {};

    inline const std::string message() const {
        return this->mMessage;
    }

    inline const std::shared_ptr<Node> node() const {
        return this->mNode;
    }
};

class Parser {
    private:
    const std::string mSource;
    std::unordered_map<
        std::string,
        std::function<std::pair<ParserAction, Node>(std::shared_ptr<Tag>)>>
                mTagBindings;
    std::size_t mGlobalOffset;
    std::size_t mLineOffset;
    std::size_t mLine;
    std::size_t mColumn;

    public:
    Parser(std::string source);

    inline void add_tag_binding(
        std::string tag,
        std::function<std::pair<ParserAction, Node>(std::shared_ptr<Tag>)>
            binding) {
        this->mTagBindings[tag] = binding;
    }

    std::variant<std::shared_ptr<Node>, SyntaxError, TagError, NodeError>
    parse();

    private:
    static inline bool               is_tag_char(char c);
    static inline std::string        trim(std::string &s);
    inline const SourceLocation      location() const;
    inline bool                      can_advance(std::size_t amount = 0) const;
    inline void                      advance(std::size_t amount = 1);
    inline const std::optional<char> peek(std::size_t ahead = 0) const;
    inline char                      quick_peek(std::size_t ahead = 0) const;
    inline void                      advance_line();
    inline char                      consume();
    void                             skip_whitespace();
    const std::variant<char, SyntaxError>
                consume_if(const std::string &allowed);
    std::string collect_sequence();
    const std::variant<std::shared_ptr<Tag>, SyntaxError> collect_tag();
    const std::variant<std::pair<ParserAction, std::shared_ptr<Node>>, TagError>
    tag_to_node(std::shared_ptr<Tag> tag);
    const std::optional<
        std::variant<std::pair<ParserAction, std::shared_ptr<Node>>,
                     SyntaxError,
                     TagError>>
    collect_block();
};

} // namespace louvre
