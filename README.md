# Kiwi Grammar

Maps Korean morphemes emitted from [Kiwi](https://github.com/bab2min/Kiwi) to grammar points from [Kimchi Grammar](https://github.com/Alaanor/kimchi-grammar).

Generates tokens and plain text output for each sentence.
Tokens have a root morpheme and a list of grammar points with a corresponding range that were used to form the token.

## Example Input and Output

```
>> 담배하고 술을 끊었어요.
TK : 담배하고 : 담배:NNG
 + : noun_하다 (2-3)
 + : verb_고 (3-4)
PT : ␣
TK : 술을 : 술:NNG
 + : noun_을_를 (1-2)
PT : ␣
TK : 끊었어요 : 끊:VV
 + : verb_았_었_했 (1-2)
 + : verb_아_어_여 (2-3)
 + : verb_요 (3-4)
PT : .
```

Currently, grammar patterns are defined in this fork of Kimchi Grammar: [RicBent/kimchi-grammar](https://github.com/RicBent/kimchi-grammar).

## Building

Install the CMake build system if you haven't already. For the WebAssembly build, you will also need to install the Emscripten SDK.

### CLI Tool:

```bash
./build.sh
```

### WebAssembly Module:

```bash
./build_wasm.sh
```

## Usage (CLI Tool)

```
Usage: ./kiwi-grammar [-m MODEL_DIR] [-r RULES_PATH] [-s]
  -m MODEL_DIR: Kiwi model directory (default: model)
  -r RULES_PATH: Path to the rules file or directory (default: detect-rules)
  -s: Strip HTML tags
```

`RULES_PATH` can either be the path the `point` directory in the Kimchi Grammar repository, or a file exported from the `detect-export.py` script.

## License

This project is licensed under the GNU General Public License v3.0 (GPL-3.0). See the [LICENSE](LICENSE) file for details.
