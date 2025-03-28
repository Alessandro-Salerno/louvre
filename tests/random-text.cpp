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

#include <clocale>
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

bool compare_nodes(std::shared_ptr<louvre::Node> n1,
                   std::shared_ptr<louvre::Node> n2,
                   std::size_t                   nest = 0) {
    std::cout << "Testing nest level " << nest << std::endl;
    massert(n1->type() == n2->type());
    if (n1->text()) {
        std::cout << "N1(" << (*n1->text()).length() << "): " << *n1->text();
        if (n2->text()) {
            std::cout << " N2(" << (*n2->text()).length()
                      << "): " << *n2->text();
        }
        std::cout << std::endl;
    }
    massert(n1->text() == n2->text());
    massert(n1->children().size() == n2->children().size());

    for (int i = 0; i < n1->children().size(); i++) {
        massert(compare_nodes(
            n1->children().at(i), n2->children().at(i), nest + 1));
    }

    return true;
}

std::string random_str(int length = 512) {
    auto characters = u8"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQ"
                      "RSTUVWXYZ0123456789àèìòùáéíóú ";
    std::string randomString = "";
    for (int i = 0; i < length; i++) {
        randomString += characters[rand() % 73];
    }
    return randomString;
}

int main(void) {
    std::string code      = random_str();
    auto        parser    = louvre::Parser(code);
    auto        parse_res = parser.parse();

    if (auto e = std::get_if<louvre::SyntaxError>(&parse_res)) {
        std::cout << "Syntax Error " << e->message() << " at "
                  << e->location().line() << ":" << e->location().column()
                  << std::endl;

        return -1;
    }

    if (auto e = std::get_if<louvre::TagError>(&parse_res)) {
        std::cout << "Tag Error " << e->message() << "(" << e->tag()->name()
                  << ") at " << e->tag()->location().line() << ":"
                  << e->tag()->location().column() << std::endl;

        return -1;
    }

    if (auto e = std::get_if<louvre::NodeError>(&parse_res)) {
        std::cout << "Node Error" << std::endl;
        return -1;
    }

    auto root     = std::get<std::shared_ptr<louvre::Node>>(parse_res);
    auto expected = std::make_shared<louvre::Node>();
    expected->add_child(
        std::make_shared<louvre::Node>(louvre::Node::text(code)));

    if (!compare_nodes(root, expected)) {
        return -1;
    }

    return 0;
}
