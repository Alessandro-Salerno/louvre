# `louvre`
[contributors-shield]: https://img.shields.io/github/contributors/Alessandro-Salerno/louvre.svg?style=flat-square
[contributors-url]: https://github.com/Alessandro-Salerno/louvre/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/Alessandro-Salerno/louvre.svg?style=flat-square
[forks-url]: https://github.com/Alessandro-Salerno/louvre/network/members
[stars-shield]: https://img.shields.io/github/stars/Alessandro-Salerno/louvre.svg?style=flat-square
[stars-url]: https://github.com/Alessandro-Salerno/louvre/stargazers
[issues-shield]: https://img.shields.io/github/issues/Alessandro-Salerno/louvre.svg?style=flat-square
[issues-url]: https://github.com/Alessandro-Salerno/louvre/issues
[license-shield]: https://img.shields.io/github/license/Alessandro-Salerno/louvre.svg?style=flat-square
[license-url]: https://github.com/Alessandro-Salerno/louvre/blob/master/LICENSE.txt

[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![MIT License][license-shield]][license-url]
![](https://tokei.rs/b1/github/Alessandro-Salerno/louvre)
![shield](https://img.shields.io/static/v1?label=version&message=0.1.0&color=blue) 


`louvre` is a simple and extensible markup language meant to provide a single source frontend for multiple output formats.

```
#
#center
	WELCOME TO LOUVRE #
	SIMPLE AND EXTENSIBLE MARKUP LANGUAGE
#end
#
#justify
	Introduction #
	#paragraph
		This is a demo of louvre!
	#end
	# Credits #
	#paragraph
		This was made by Alessandro Salerno (alevm1710)!
	#end
#end
```

## Syntax
`louvre` is meant to be fast to type and easy to learn. It uses tags starting with `#` to instruct the parser and the emitter on how to interpret a block. `louvre` ignores new line and tab characters in order to allow for more flexibility in both the user code and parser implementation.

Anything that is not a tag generates a text node which is then interpreted by the emitter.

### Standard tags
Emitters are required to support the following tags with the signatures provided. 
| Tag | Description |
| - | - |
| `#` | Forces a new line |
| `##` | Outputs regular `#` |
| `#left` | Forcefs alignment to the left |
| `#center` | Forces alignment to the center |
| `#right` | Forces alignment to the right |
| `#justify` | Forces line justification |
| `#paragrah` | Creates an indented block |
| `#bullets` | Creates a list of bullet points |
| `#numbers` | Creates a numbered list |
| `#item` | Creates a block that can easily be identified by the emitter when traversing `#bullets` or `#numbers` |
| `#end` | Closes a block and tells the parser to walk back to the parent node before continuing |

### Custom tags
Emitters may introduce custom tags to speed-up writing of specific types of documents. An emitter meant for legal purposes, for example, may create an `#article` tag that expands to a block containing a title with an auto-incremental article number and a justified body.

Note that tags are not required to create a block. Tags may also be used to set properties or perform other functions.

### Tag arguments
Tags can also have arguments. Arguments may be passed to a tag using the syntax:
```
#tag(arg1, arg2, ...)
```
Emitters may implement their own tags or extend standard tags with arguments. For example, a `txt` emitter may give its users to change the character used for bullet points:
```
#bullets(*)
```

## Installing `liblouvre`
If you're using CMake as build system, your `CMakeLists.txt`file should look something like this:
```cmake
cmake_minimum_required(VERSION 3.16) # Change this as you please, as ong as it works
project(myproject) # Change this to the name of your project

include(ExternalProject)

set(LOUVRE louvre_project)
ExternalProject_Add(
    louvre_project
    GIT_REPOSITORY https://github.com/Alessandro-Salerno/louvre
    GIT_TAG <commit>
    PREFIX ${PROJECT_SOURCE_DIR}/external/${LOUVRE}
    CONFIGURE_COMMAND cmake ../${LOUVRE}
    BUILD_COMMAND cmake --build .
    INSTALL_COMMAND cmake --install . --prefix ${PROJECT_SOURCE_DIR}
    UPDATE_COMMAND ""
)

add_library(louvre STATIC IMPORTED)
set_property(TARGET louvre PROPERTY IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/lib/liblouvre.a)
add_dependencies(louvre louvre_project)
```
Remember to replace `<commit>` with the commit tag of the version you're interested in.

## Using `liblouvre`
Using `liblouvre` is as simple as including the `louvre/api.hpp` file, linking to the static library and implementing a program that makes use of the API:
```cpp
#include <fstream>
#include <iostream>
#include <louvre/api.hpp>
#include <variant>

int main(int argc, const char *const argv[]) {
    if (argc != 2) {
        std::cerr << "ERROR: Need exactly one argument: file path" << std::endl;
        return -1;
    }

    // Open wchar input stream
    std::wifstream input(argv[1]);

    // Read whole file
    std::u8string source((std::istreambuf_iterator<char32_t>(input)),
                        std::istreambuf_iterator<char32_t>());

    // Create the louvre parser instance using the contents of the file
    auto parser = louvre::Parser(source);

    // Parse the file
    std::variant<std::shared_ptr<louvre::Node>,
                 louvre::SyntaxError,
                 louvre::TagError,
                 louvre::NodeError>
        parse_result = parser.parse();

    // EXAMPLE: output number of children of the root node
    if (auto rootp = std::get_if<std::shared_ptr<louvre::Node>>(&parse_result)) {
        auto root = *rootp;
        std::cout << "Root has " << root->children().size() << " children"
                  << std::endl;
    }
}

```

## Building `liblouvre`
To build `liblouvre`, you'll need a C++ compiler compatible with C++ 20 and [Cmake](https://cmake.org/). Once the necessary software is installed, just type the following command:
```bash
git clone https://github.com/Alessandro-Salerno/louvre && \
cd louvre && \
mkdir -p build && \
cd build && \
cmake .. && \
make && \
ctest
```
The resulting `liblouvre.a` file will be in the `build` directory.

## License
Distributed under the Apache License 2.0. See [LICENSE](LICENSE) for details.

