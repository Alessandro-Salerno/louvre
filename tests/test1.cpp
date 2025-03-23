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

#include <iostream>
#include <louvre/api.hpp>
#include <memory>

#define mstr(x) #x

#define massert(expr)                                                \
    if (!(expr)) {                                                   \
        std::cerr << "Assertion failed " mstr(expr) "" << std::endl; \
        return false;                                                \
    }

#include <string_view>

constexpr const std::wstring_view SOURCE = L"#\n"
                                           "#center\n"
                                           "THIS IS THE TITLE\n"
                                           "#end\n"
                                           "#\n"
                                           "#justify\n"
                                           "Hello there, this is some text! #\n"
                                           "#paragraph\n"
                                           "And this is a paragraph!\n"
                                           "#end\n"
                                           "#end\n";

bool compare_nodes(std::shared_ptr<louvre::Node> n1,
                   std::shared_ptr<louvre::Node> n2) {
    massert(n1 == n2);
    return true;
}

int main(void) {
    auto parser    = louvre::Parser(SOURCE);
    auto parse_res = parser.parse();

    if (auto e = std::get_if<louvre::SyntaxError>(&parse_res)) {
        std::cout << "Syntax Error at " << e->location().line() << ":"
                  << e->location().column() << std::endl;

        return -1;
    }

    if (auto e = std::get_if<louvre::TagError>(&parse_res)) {
        std::cout << "Syntax Error at " << e->tag()->location().line() << ":"
                  << e->tag()->location().column() << std::endl;

        return -1;
    }

    if (auto e = std::get_if<louvre::NodeError>(&parse_res)) {
        std::cout << "Node Error" << std::endl;
        return -1;
    }

    auto root = std::get<std::shared_ptr<louvre::Node>>(parse_res);
    std::cout << "Root has " << root->children().size() << " children"
              << std::endl;

    /*auto mroot = std::make_shared<louvre::Node>();
    auto center =
        std::make_shared<louvre::Node>(louvre::StandardNodeType::Center);
    center->add_child(std::make_shared<louvre::Node>(
        louvre::Node::text(L"THIS IS THE TITLE")));

    auto justify =
        std::make_shared<louvre::Node>(louvre::StandardNodeType::Justify);
    justify->add_child(std::make_shared<louvre::Node>(
        std::move(louvre::Node::text(L"Hello there, this is some text!"))));
    justify->add_child(
        std::make_shared<louvre::Node>(louvre::StandardNodeType::LineBreak));
    auto paragraph =
        std::make_shared<louvre::Node>(louvre::StandardNodeType::Paragraph);
    paragraph->add_child(std::make_shared<louvre::Node>(
        std::move(louvre::Node::text(L"And this is a paragraph!"))));
    justify->add_child(paragraph);

    mroot->add_child(
        std::make_shared<louvre::Node>(louvre::StandardNodeType::LineBreak));
    mroot->add_child(center);
    mroot->add_child(
        std::make_shared<louvre::Node>(louvre::StandardNodeType::LineBreak));
    mroot->add_child(justify);

    if (!compare_nodes(root, mroot)) {
        return -1;
    }*/

    return 0;
}
