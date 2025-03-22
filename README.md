# `louvre`
`louvre` is a simple and extensible markup language meant to provide a single source frontend for multiple output formats.

```c
#
#center
	WELCOME TO LOUVRE #
	SIMPLE AND EXTENSIBLE MARKUP LANGUAGE
#end
#
#justify
	Introduction
	#
	#paragraph
		This is a demo of louvre!
	#end
	#
	Credits
	#
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

## License
Distributed under the Apache License 2.0. See [LICENSE](LICENSE) for details.

