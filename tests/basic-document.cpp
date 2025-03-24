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
#include <string>

#define mstr(x) #x

#define massert(expr)                                                \
    if (!(expr)) {                                                   \
        std::cerr << "Assertion failed " mstr(expr) "" << std::endl; \
        return false;                                                \
    }

const std::wstring SOURCE = L"#\n"
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

/*bool compare_tag(std::optional<std::shared_ptr<louvre::Tag>> o1,
                 std::optional<std::shared_ptr<louvre::Tag>> o2) {
    massert(o1.has_value() == o2.has_value());

    if (!o1.has_value()) {
        return true;
    }

    auto t1 = o1->get();
    auto t2 = o2->get();

    massert(t1->name() == t2->name());
    massert(t1->location().line() == t2->location().line());
    massert(t1->location().column() == t2->location().column());
    return true;
}*/

bool compare_nodes(std::shared_ptr<louvre::Node> n1,
                   std::shared_ptr<louvre::Node> n2,
                   std::size_t                   nest = 0) {
    std::cout << "Testing nest level " << nest << std::endl;
    massert(n1->type() == n2->type());
    massert(n1->text() == n2->text());
    massert(n1->children().size() == n2->children().size());

    for (int i = 0; i < n1->children().size(); i++) {
        massert(compare_nodes(
            n1->children().at(i), n2->children().at(i), nest + 1));
    }

    return true;
}

int main(void) {
    auto parser    = louvre::Parser(SOURCE);
    auto parse_res = parser.parse();

    if (auto e = std::get_if<louvre::SyntaxError>(&parse_res)) {
        std::wcout << "Syntax Error " << e->message() << " at "
                   << e->location().line() << ":" << e->location().column()
                   << std::endl;

        return -1;
    }

    if (auto e = std::get_if<louvre::TagError>(&parse_res)) {
        std::wcout << "Tag Error " << e->message() << "(" << e->tag()->name()
                   << ") at " << e->tag()->location().line() << ":"
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

    auto mroot = std::make_shared<louvre::Node>();
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
    }

    return 0;
}
