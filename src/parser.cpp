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
 *   See the License for the specific la nguage governing permissions and
 *   limitations under the License.
 */

#include <cwctype>
#include <louvre/api.hpp>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>

namespace louvre {
Parser::Parser(std::string source) : mSource(source) {
    this->mOffset = 0;
    this->mLine   = 0;
    this->mColumn = 0;

    // #end
    this->add_tag_binding("end", [](std::shared_ptr<Tag> tag) {
        return std::make_pair(ParserAction::End, Node(StandardNodeType::Null));
    });

    // #left
    this->add_tag_binding("left", [](std::shared_ptr<Tag> tag) {
        return std::make_pair(ParserAction::AddChildAndBranch,
                              Node(StandardNodeType::Left));
    });

    // #center
    this->add_tag_binding("center", [](std::shared_ptr<Tag> tag) {
        return std::make_pair(ParserAction::AddChildAndBranch,
                              Node(StandardNodeType::Center));
    });

    // #right
    this->add_tag_binding("right", [](std::shared_ptr<Tag> tag) {
        return std::make_pair(ParserAction::AddChildAndBranch,
                              Node(StandardNodeType::Right));
    });

    // #justify
    this->add_tag_binding("justify", [](std::shared_ptr<Tag> tag) {
        return std::make_pair(ParserAction::AddChildAndBranch,
                              Node(StandardNodeType::Justify));
    });

    // #paragraph
    this->add_tag_binding("paragraph", [](std::shared_ptr<Tag> tag) {
        return std::make_pair(ParserAction::AddChildAndBranch,
                              Node(StandardNodeType::Paragraph));
    });

    // #numbers
    this->add_tag_binding("numbers", [](std::shared_ptr<Tag> tag) {
        return std::make_pair(ParserAction::AddChildAndBranch,
                              Node(StandardNodeType::Numebrs));
    });

    // #bullets
    this->add_tag_binding("bullets", [](std::shared_ptr<Tag> tag) {
        return std::make_pair(ParserAction::AddChildAndBranch,
                              Node(StandardNodeType::Bullets));
    });

    // #item
    this->add_tag_binding("item", [](std::shared_ptr<Tag> tag) {
        return std::make_pair(ParserAction::AddChildAndBranch,
                              Node(StandardNodeType::Item));
    });

    // # (new line)
    this->add_tag_binding("", [](std::shared_ptr<Tag> tag) {
        return std::make_pair(ParserAction::AddChild,
                              Node(StandardNodeType::LineBreak));
    });
}

std::variant<std::shared_ptr<Node>, SyntaxError, TagError, NodeError>
Parser::parse() {
    auto root = std::make_shared<Node>();

    while (this->can_advance()) {
        const std::optional<
            std::variant<std::pair<ParserAction, std::shared_ptr<Node>>,
                         SyntaxError,
                         TagError>>
            block_opt = this->collect_block();

        if (!block_opt) {
            break;
        }

        const auto block_res = block_opt.value();

        if (std::holds_alternative<SyntaxError>(block_res)) {
            return std::get<SyntaxError>(block_res);
        }

        if (std::holds_alternative<TagError>(block_res)) {
            return std::get<TagError>(block_res);
        }

        const auto [action, node] =
            std::get<std::pair<ParserAction, std::shared_ptr<Node>>>(block_res);

        switch (action) {
        case ParserAction::AddChild:
            root->add_child(node);
            break;

        case ParserAction::AddChildAndBranch:
            root->add_child(node);
            root = node;
            break;

        case ParserAction::End:
            if (!root->parent()) {
                return NodeError("Unexpected branch return at root leve", node);
            }

            root = root->parent().value();
            break;

        default:
            break;
        }
    }

    return root;
}

// TODO: properly support UTF8 whitespace chracters
inline std::string Parser::trim(std::string &s) {
    size_t start = 0;
    while (start < s.length() && std::isspace(s[start])) {
        start++;
    }

    size_t end = s.length();
    while (end > start && std::isspace(s[end - 1])) {
        end--;
    }

    s = s.substr(start, end - start);
    return s;
}

inline bool Parser::is_tag_char(char32_t c) {
    return std::iswalnum(c) || '_' == c;
}

inline const SourceLocation Parser::location() const {
    return SourceLocation(this->mLine, this->mColumn);
}

inline bool Parser::can_advance(std::size_t amount) const {
    return this->mOffset + amount < this->mSource.length();
}

inline void Parser::advance(std::size_t amount) {
    this->mOffset += amount;
    this->mColumn += amount;
}

inline void Parser::backtracK(std::size_t amount) {
    this->advance(-amount);
}

inline const std::optional<char32_t> Parser::peek(std::size_t ahead) const {
    if (this->can_advance(ahead)) {
        return this->mSource[this->mOffset + ahead];
    }

    return std::nullopt;
}

inline char32_t Parser::quick_peek(std::size_t ahead) const {
    return this->peek(ahead).value_or('\0');
}

inline void Parser::advance_line() {
    this->mLine++;
    this->mColumn = 0;
}

inline char32_t Parser::consume() {
    char32_t c = this->quick_peek();
    this->advance();
    return c;
}

void Parser::skip_whitespace() {
    while (this->can_advance() && std::iswspace(this->quick_peek())) {
        this->advance();
    }
}

const std::variant<char32_t, SyntaxError>
Parser::consume_if(const std::string &allowed) {
    if (!this->peek().has_value()) {
        return SyntaxError("Unexpected EOF", this->location());
    }

    if (std::string::npos == allowed.find(this->quick_peek())) {
        return SyntaxError("Unexpected token", this->location());
    }

    return this->consume();
}

std::string Parser::collect_sequence() {
    std::string buf;

    while (this->can_advance() && Parser::is_tag_char(this->quick_peek())) {
        buf.push_back(this->consume());
    }

    return buf;
}

const std::variant<std::shared_ptr<Tag>, SyntaxError> Parser::collect_tag() {
    this->advance();
    SourceLocation location = this->location();
    std::string    tag_name = this->collect_sequence();
    auto           tag = std::make_shared<Tag>(std::move(tag_name), location);

    if (std::holds_alternative<SyntaxError>(this->consume_if("("))) {
        return tag;
    }

    while (true) {
        this->skip_whitespace();
        std::string arg = this->collect_sequence();

        if (!arg.empty()) {
            tag->add_argument(arg);
        }

        std::variant<char32_t, SyntaxError> next = this->consume_if(",)");

        if (std::holds_alternative<SyntaxError>(next)) {
            return std::get<SyntaxError>(next);
        }

        if (')' == std::get<char32_t>(next)) {
            break;
        }
    }

    return tag;
}

const std::variant<std::pair<ParserAction, std::shared_ptr<Node>>, TagError>
Parser::tag_to_node(std::shared_ptr<Tag> tag) {
    if (this->mTagBindings.contains(tag->name())) {
        auto handler        = this->mTagBindings.at(tag->name());
        auto [action, node] = handler(tag);
        node.set_tag(tag);
        return std::make_pair(action, std::make_shared<Node>(std::move(node)));
    }

    return TagError("Unknown tag", tag);
}

const std::optional<std::variant<std::pair<ParserAction, std::shared_ptr<Node>>,
                                 SyntaxError,
                                 TagError>>
Parser::collect_block() {
    std::string buf;

    while (this->can_advance()) {
        const char32_t cur = this->quick_peek();

        if ('\t' == cur) {
            this->advance();
            continue;
        }

        if (' ' == cur) {
            if (!buf.ends_with(' ')) {
                buf.push_back(cur);
            }
            this->advance();
            continue;
        }

        const char32_t next = this->quick_peek(1);

        if (cur == next && '#' == cur) {
            buf.push_back(cur);
            this->advance(2);
            continue;
        }

        if ('\n' == cur || '\r' == cur) {
            if (!buf.ends_with(' ')) {
                buf.push_back(' ');
            }

            this->advance();
            this->advance_line();
            continue;
        }

        if ('#' != cur) {
            buf.push_back(cur);
            this->advance();
            continue;
        }

        // #<tag>
        if (!Parser::trim(buf).empty()) {
            return std::make_pair(
                ParserAction::AddChild,
                std::make_shared<Node>(std::move(Node::text(buf))));
        }

        const std::variant<std::shared_ptr<Tag>, SyntaxError> tag_res =
            this->collect_tag();

        if (std::holds_alternative<SyntaxError>(tag_res)) {
            return std::get<SyntaxError>(tag_res);
        }

        const auto tag = std::get<std::shared_ptr<Tag>>(tag_res);
        const std::variant<std::pair<ParserAction, std::shared_ptr<Node>>,
                           TagError>
            node_res = this->tag_to_node(tag);

        if (std::holds_alternative<TagError>(node_res)) {
            return std::get<TagError>(node_res);
        }

        return std::get<std::pair<ParserAction, std::shared_ptr<Node>>>(
            node_res);
    }

    if (!Parser::trim(buf).empty()) {
        return std::make_pair(
            ParserAction::AddChild,
            std::make_shared<Node>(std::move(Node::text(buf))));
    }

    return std::nullopt;
}

} // namespace louvre
