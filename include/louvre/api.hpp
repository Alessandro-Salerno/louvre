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

    public:
    SourceLocation(std::size_t line, std::size_t column)
        : mLine(line), mColumn(column) {};

    inline const std::size_t line() const {
        return this->mLine;
    }

    inline const std::size_t column() const {
        return this->mColumn;
    }
};

class Tag {
    private:
    const std::u8string        mName;
    const SourceLocation       mLocation;
    std::vector<std::u8string> mArguments;

    public:
    Tag(std::u8string name, SourceLocation location)
        : mName(name), mLocation(location) {};

    inline const std::u8string name() const {
        return this->mName;
    }

    inline const SourceLocation location() const {
        return this->mLocation;
    }

    inline const std::vector<std::u8string> arguments() const {
        return this->mArguments;
    }

    inline void add_argument(std::u8string argument) {
        this->mArguments.push_back(argument);
    }
};

class Node : public std::enable_shared_from_this<Node> {
    private:
    const std::variant<StandardNodeType, std::u8string> mType;
    std::optional<std::u8string>                        mText;
    std::optional<std::shared_ptr<Tag>>                 mTag;
    std::optional<std::shared_ptr<Node>>                mParent;
    std::vector<std::shared_ptr<Node>>                  mChildren;
    std::size_t                                         mNum;

    public:
    Node() : Node(StandardNodeType::Root) {};
    Node(StandardNodeType type) : mType(type) {};
    Node(std::u8string type) : mType(type), mNum(0) {};
    Node(Node &&other) noexcept = default;

    static inline Node text(std::u8string text) {
        Node n(StandardNodeType::Text);
        n.mText = text;
        return n; // ret val optimization helps here
    }

    inline const std::variant<StandardNodeType, std::u8string> type() const {
        return this->mType;
    }

    inline const std::optional<std::u8string> text() const {
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
    const std::u8string  mMessage;
    const SourceLocation mLocation;

    public:
    SyntaxError(std::u8string message, SourceLocation location)
        : mMessage(message), mLocation(location) {};

    inline const std::u8string message() const {
        return this->mMessage;
    }

    inline const SourceLocation location() const {
        return this->mLocation;
    }
};

class TagError {
    private:
    const std::u8string        mMessage;
    const std::shared_ptr<Tag> mTag;

    public:
    TagError(std::u8string message, std::shared_ptr<Tag> tag)
        : mMessage(message), mTag(tag) {};

    inline const std::u8string message() const {
        return this->mMessage;
    }

    inline const std::shared_ptr<Tag> tag() const {
        return this->mTag;
    }
};

class NodeError {
    private:
    const std::u8string         mMessage;
    const std::shared_ptr<Node> mNode;

    public:
    NodeError(std::u8string message, std::shared_ptr<Node> node)
        : mMessage(message), mNode(node) {};

    inline const std::u8string message() const {
        return this->mMessage;
    }

    inline const std::shared_ptr<Node> node() const {
        return this->mNode;
    }
};

class Parser {
    private:
    const std::u8string mSource;
    std::unordered_map<
        std::u8string,
        std::function<std::pair<ParserAction, Node>(std::shared_ptr<Tag>)>>
                mTagBindings;
    std::size_t mOffset;
    std::size_t mLine;
    std::size_t mColumn;

    public:
    Parser(std::u8string source);

    inline void add_tag_binding(
        std::u8string tag,
        std::function<std::pair<ParserAction, Node>(std::shared_ptr<Tag>)>
            binding) {
        this->mTagBindings[tag] = binding;
    }

    std::variant<std::shared_ptr<Node>, SyntaxError, TagError, NodeError>
    parse();

    private:
    static inline bool          is_tag_char(char32_t c);
    static inline std::u8string trim(std::u8string &s);
    inline const SourceLocation location() const;
    inline bool                 can_advance(std::size_t amount = 1) const;
    inline void                 advance(std::size_t amount = 1);
    inline void                 backtracK(std::size_t amount = 1);
    inline const std::optional<char32_t> peek(std::size_t ahead = 0) const;
    inline char32_t quick_peek(std::size_t ahead = 0) const;
    inline void     advance_line();
    inline char32_t consume();
    void            skip_whitespace();
    const std::variant<char32_t, SyntaxError>
                  consume_if(const std::u8string &allowed);
    std::u8string collect_sequence();
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
