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

#include <source_location>
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
    Custom
};

class Tag {
    private:
    const std::wstring_view        mName;
    const std::source_location     mLocation;
    std::vector<std::wstring_view> mArguments;

    protected:
    Tag(std::wstring_view name, std::source_location location);

    public:
    std::wstring_view                    name() const;
    std::source_location                 location() const;
    const std::vector<std::wstring_view> arguments() const;
};

class Node {
    private:
    const std::variant<StandardNodeType, std::wstring_view> mType;
    const std::optional<std::wstring_view>                  mText;
    std::optional<std::shared_ptr<Tag>>                     mTag;
    std::optional<std::shared_ptr<Node>>                    mParent;
    std::vector<std::shared_ptr<Node>>                      mChildren;

    public:
    Node(StandardNodeType type);
    Node(std::wstring_view type);
    Node(Node &&other) noexcept;
    static Node text(std::wstring_view text);

    const std::variant<StandardNodeType, std::wstring_view> type() const;
    const std::optional<std::string_view>                   text() const;
    const std::optional<std::shared_ptr<Tag>>               tag() const;
    const std::optional<std::shared_ptr<Node>>              parent() const;
    const std::vector<std::shared_ptr<Node>>                children() const;

    void add_child(Node &child);

    protected:
    void add_dangling_child(Node &child);
    void set_parent(Node &parent);
    void set_tag(Tag &tag);
};

class SyntaxError {
    private:
    const std::wstring_view    mMessage;
    const std::source_location mLocation;

    protected:
    SyntaxError() = default;
    SyntaxError(std::wstring_view message);

    public:
    const std::wstring_view    message() const;
    const std::source_location location() const;
};

class TagError {
    private:
    const std::wstring_view    mMessage;
    const std::shared_ptr<Tag> mTag;

    protected:
    TagError() = default;
    TagError(std::wstring_view message);

    public:
    const std::wstring_view    message() const;
    const std::shared_ptr<Tag> tag() const;
};

class NodeError {
    private:
    const std::wstring_view     mMessage;
    const std::shared_ptr<Node> mNode;

    protected:
    NodeError() = default;
    NodeError(std::wstring_view message);

    public:
    const std::wstring_view     message() const;
    const std::shared_ptr<Node> node() const;
};

// TODO: review bindings
class Parser {
    private:
    const std::wstring_view mSource;
    std::unordered_map<std::string_view,
                       std::function<std::pair<ParserAction, Node>()>>
                mTagBindings;
    std::size_t mOffset;
    std::size_t mLine;
    std::size_t mColumn;

    public:
    std::variant<Node, SyntaxError, TagError, NodeError> parse();
    void
    add_tag_binding(std::wstring_view                              tag,
                    std::function<std::pair<ParserAction, Node>()> binding);

    private:
    static bool                  is_tag_char(wchar_t c);
    const std::source_location   location() const;
    bool                         can_advance(std::size_t amount = 1) const;
    void                         advance(std::size_t amount = 1);
    void                         backtracK(std::size_t amount = 1);
    const std::optional<wchar_t> peek(std::size_t ahead = 0) const;
    wchar_t                      quick_peek(std::size_t ahead = 0) const;
    void                         advance_line();
    wchar_t                      consume();
    void                         skip_whitespace();
    const std::variant<wchar_t, SyntaxError>
                                         consume_if(std::wstring_view &allowed);
    std::string                          collect_sequence();
    const std::variant<Tag, SyntaxError> collect_tag();
    const std::variant<std::pair<ParserAction, Node>, TagError> tag_to_node();
    const std::variant<std::pair<ParserAction, Node>, SyntaxError, TagError>
    collect_block();
};

} // namespace louvre
