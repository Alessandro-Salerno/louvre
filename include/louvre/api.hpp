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

#include <string_view>
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
    Group,
    Custom,
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
    const std::wstring_view        mName;
    const SourceLocation           mLocation;
    std::vector<std::wstring_view> mArguments;

    public:
    Tag(std::wstring_view name, SourceLocation location)
        : mName(name), mLocation(location) {};

    inline const std::wstring_view name() const {
        return this->mName;
    }

    inline const SourceLocation location() const {
        return this->mLocation;
    }

    inline const std::vector<std::wstring_view> arguments() const {
        return this->mArguments;
    }

    inline void add_argument(std::wstring_view argument) {
        this->mArguments.push_back(argument);
    }
};

class Node : std::enable_shared_from_this<Node> {
    private:
    const std::variant<StandardNodeType, std::wstring_view> mType;
    std::optional<std::wstring_view>                        mText;
    std::optional<std::shared_ptr<Tag>>                     mTag;
    std::optional<std::shared_ptr<Node>>                    mParent;
    std::vector<std::shared_ptr<Node>>                      mChildren;

    public:
    Node() : Node(StandardNodeType::Root) {};
    Node(StandardNodeType type) : mType(type) {};
    Node(std::wstring_view type) : mType(type) {};
    Node(Node &&other) noexcept;

    static inline Node text(std::wstring_view text) {
        Node n(StandardNodeType::Text);
        n.mText = text;
        return n; // ret val optimization helps here
    }

    inline const std::variant<StandardNodeType, std::wstring_view>
    type() const {
        return this->mType;
    }

    inline const std::optional<std::wstring_view> text() const {
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

    inline void add_child(std::shared_ptr<Node> child) {
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
    const std::wstring_view mMessage;
    const SourceLocation    mLocation;

    public:
    SyntaxError(std::wstring_view message, SourceLocation location)
        : mMessage(message), mLocation(location) {};

    inline const std::wstring_view message() const {
        return this->mMessage;
    }

    inline const SourceLocation location() const {
        return this->mLocation;
    }
};

class TagError {
    private:
    const std::wstring_view    mMessage;
    const std::shared_ptr<Tag> mTag;

    public:
    TagError(std::wstring_view message, std::shared_ptr<Tag> tag)
        : mMessage(message), mTag(tag) {};

    inline const std::wstring_view message() const {
        return this->mMessage;
    }

    inline const std::shared_ptr<Tag> tag() const {
        return this->mTag;
    }
};

class NodeError {
    private:
    const std::wstring_view     mMessage;
    const std::shared_ptr<Node> mNode;

    public:
    NodeError(std::wstring_view message, std::shared_ptr<Node> node)
        : mMessage(message), mNode(node) {};

    inline const std::wstring_view message() const {
        return this->mMessage;
    }

    inline const std::shared_ptr<Node> node() const {
        return this->mNode;
    }
};

class Parser {
    private:
    const std::wstring_view mSource;
    std::unordered_map<
        std::wstring_view,
        std::function<std::pair<ParserAction, Node>(std::shared_ptr<Tag>)>>
                mTagBindings;
    std::size_t mOffset;
    std::size_t mLine;
    std::size_t mColumn;

    public:
    Parser(std::wstring_view source);

    inline void add_tag_binding(
        std::wstring_view tag,
        std::function<std::pair<ParserAction, Node>(std::shared_ptr<Tag>)>
            binding) {
        this->mTagBindings[tag] = binding;
    }

    std::variant<std::shared_ptr<Node>, SyntaxError, TagError, NodeError>
    parse();

    private:
    static inline bool          is_tag_char(wchar_t c);
    static inline std::wstring  trim(std::wstring &s);
    inline const SourceLocation location() const;
    inline bool                 can_advance(std::size_t amount = 1) const;
    inline void                 advance(std::size_t amount = 1);
    inline void                 backtracK(std::size_t amount = 1);
    inline const std::optional<wchar_t> peek(std::size_t ahead = 0) const;
    inline wchar_t                      quick_peek(std::size_t ahead = 0) const;
    inline void                         advance_line();
    inline wchar_t                      consume();
    void                                skip_whitespace();
    const std::variant<wchar_t, SyntaxError>
                 consume_if(const std::wstring_view &allowed);
    std::wstring collect_sequence();
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
