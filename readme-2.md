# LinEd – a tiny EDLIN‑style line editor (Turbo C & ANSI C89)

LinEd is a compact, console line editor modeled after classic DOS tools (like **EDLIN**). It’s written in portable C89 so it builds on **Turbo C** (for retro/DOS use) and modern toolchains (GCC/Clang) on macOS and Linux.

This version adds **numbered input prompts** (e.g., `00012:`) so you always see the line number you’re inserting or editing.

---

## Table of Contents
- [Features](#features)
- [Quick Start](#quick-start)
- [Build & Install](#build--install)
  - [Turbo C (DOS / DOSBox)](#turbo-c-dos--dosbox)
  - [macOS (clang) / Linux (gcc)](#macos-clang--linux-gcc)
  - [Windows (MinGW-W64)](#windows-mingw-w64)
  - [Run from anywhere (PATH)](#run-from-anywhere-path)
- [Usage](#usage)
  - [Invocation](#invocation)
  - [Editing session examples](#editing-session-examples)
- [Command Reference](#command-reference)
  - [Line ranges](#line-ranges)
  - [`L` – list](#l--list)
  - [`I` – insert](#i--insert)
  - [`E` – edit one line](#e--edit-one-line)
  - [`D` – delete](#d--delete)
  - [`S` – search](#s--search)
  - [`R` – replace](#r--replace)
  - [`O` – open (load)](#o--open-load)
  - [`W` – write (save)](#w--write-save)
  - [`P` – print status](#p--print-status)
  - [`H`/`?` – help](#h--help)
  - [`Q` – quit](#q--quit)
- [Behavior & Notes](#behavior--notes)
- [Limits & Configuration](#limits--configuration)
- [Portability Notes](#portability-notes)
- [Troubleshooting](#troubleshooting)
- [Roadmap / Ideas](#roadmap--ideas)
- [Credits & License](#credits--license)

---

## Features
- **Minimal, fast**: starts instantly, great for quick line‑oriented edits.
- **C89 portable**: compiles with Turbo C and modern compilers.
- **Numbered prompts**: insert/edit prompts prefix each input line with `00000:`‑style line numbers.
- **Range‑aware commands**: list, delete, search, and replace over `a,b` line ranges.
- **Search**: case‑insensitive find over a range.
- **Replace**: `/old/new/` with optional `g` flag for global per‑line replacement.
- **Status line**: shows total lines and current file after each command.
- **Small memory footprint**: fixed maximum lines/line length (tunable).

---

## Quick Start
```bash
# macOS / Linux
clang -std=c89 -Wall -Wextra -O2 -o lined lined3.c   # or: gcc -std=c89 ...
./lined myfile.txt
```
Inside LinEd, type commands like `L`, `I`, `E n`, `D a,b`, `S /text/`, `R a,b /old/new/g`, `W`, `Q`.

---

## Build & Install

### Turbo C (DOS / DOSBox)
**Option A — Turbo C IDE**
1. Copy `lined3.c` into your Turbo C project directory (e.g., `C:\TC\PROJECTS`).
2. Start Turbo C, **File → Open** `lined3.c`.
3. **Compile → Make** (or **Alt+F9**), then **Run** (or **Ctrl+F9**).

**Option B — Command line (tcc)**
```bat
REM From TC\BIN
SET INCLUDE=C:\TC\INCLUDE
SET LIB=C:\TC\LIB
TCC -ml -O lined3.c
```
This should produce `LINED.EXE` in the current directory.

> 💡 Using **DOSBox**? Mount your TC folder (e.g., `mount c c:\tc`), then follow the steps above inside DOSBox.

### macOS (clang) / Linux (gcc)
```bash
# Build
clang -std=c89 -Wall -Wextra -O2 -o lined lined3.c   # or: gcc -std=c89 -Wall -Wextra -O2 -o lined lined3.c

# Run
./lined [file]
```

### Windows (MinGW-W64)
```cmd
gcc -std=c89 -Wall -Wextra -O2 -o lined.exe lined3.c
lined.exe [file]
```

### Run from anywhere (PATH)
- **macOS/Linux (user only)**
  ```bash
  mkdir -p ~/bin
  mv lined ~/bin/
  echo 'export PATH="$HOME/bin:$PATH"' >> ~/.zshrc  # or ~/.bashrc
  source ~/.zshrc
  ```
- **macOS/Linux (system‑wide)**
  ```bash
  sudo cp lined /usr/local/bin/
  ```
- **DOS**: Put `LINED.EXE` in a directory listed in `PATH` (e.g., `C:\DOS`), then update `AUTOEXEC.BAT` if needed.

---

## Usage

### Invocation
```text
lined           # start with an empty buffer
lined file.txt  # load file.txt into the buffer at startup
```

When LinEd starts, it prints a banner and a status line:
```
========================
LinEd   Line Editor Ver. 1.0
Mickey Lawless (c) 2025, 2026
Editing: FILE.TXT
========================
Lines: 42  File: file.txt
*
```
Type commands at the `*` prompt.

### Editing session examples
**Insert a few lines at the end:**
```
* I
-- Insert mode at line 00041 (end with a single '.') --
00042: first new line
00043: another line
00044: .
Lines: 44  File: file.txt
*
```

**Edit line 12:**
```
* E 12
00011: old text on line 11  ← shows current content
00012: new text for line 12 ← you type this
```

**Search (case‑insensitive) and list matches with numbers:**
```
* S /error/
00007: Error: missing semicolon
00019: critical ERROR here
-- 2 match(es)
```

**Replace in a range (global per‑line):**
```
* R 1,100 /foo/bar/g
Replaced 8 occurrence(s).
```

---

## Command Reference

### Line ranges
A **range** is written `a,b` and is **inclusive**. If only one number `n` is given, it means `n,n`. If no range is given, most range‑taking commands default to the entire buffer.

- `a` and `b` are 1‑based line numbers.
- Out‑of‑range values are clamped to `[1, line_count]`.
- If `a > b`, they’re swapped.

### `L` – list
```
L            # list all lines
L a,b        # list the inclusive range
```
Lists lines in the form `00012: text...`.

### `I` – insert
```
I            # insert at end
I n          # insert before line n
```
Enters **insert mode**. Each input prompt shows the next line number like `00037:`. Type a single dot (`.`) on a line by itself to finish.

### `E` – edit one line
```
E n
```
Shows the current content of line `n`, then prompts with `000n:` for your replacement text.

### `D` – delete
```
D a,b        # delete lines a..b
```
Deletes a contiguous block and closes the gap.

### `S` – search
```
S /text/     # search entire buffer (case-insensitive)
S a,b /text/ # search within a range
```
Prints matching lines with their numbers. Search is **case‑insensitive**.

### `R` – replace
```
R a,b /old/new/     # replace first occurrence of 'old' per line in a..b
R a,b /old/new/g    # replace all occurrences per line (global)
```
Replacement is **case‑sensitive** (by design). If you need case‑insensitive replacement, search first to identify lines, then adjust case as needed.

### `O` – open (load)
```
O filename
```
Replaces the current buffer with the contents of `filename`. Sets the current file.

### `W` – write (save)
```
W             # write to current file name (if set)
W filename    # write to a new file and make it current
```
Writes the buffer to disk. Each line is written with a trailing `\n`.

### `P` – print status
```
P
```
Prints `Lines: N  File: <name-or-(none)>`.

### `H` / `?` – help
```
H   or   ?
```
Shows a concise command summary.

### `Q` – quit
```
Q
```
Exits immediately.

---

## Behavior & Notes
- **Prompts with numbers**: Insert and edit prompts display a zero‑padded 5‑digit line number (e.g., `00005:`).
- **Status after commands**: LinEd prints a status line after every command.
- **Text mode I/O**: Files are opened in text mode (`"rt"`/`"wt"`). On Unixy systems the `t` is ignored; on DOS it normalizes CRLF.
- **Memory & limits**: Lines are stored in memory as heap‑allocated strings. See limits below.

---

## Limits & Configuration
These constants are defined at the top of the source:
```c
#define MAX_LINES 1200   // maximum number of lines
#define LINE_LEN  256    // maximum length per line (input & storage)
```
Increase them to handle larger files (subject to memory). Note that extremely long lines are not wrapped; they must fit in `LINE_LEN - 1`.

---

## Portability Notes
- **C89**: The code avoids C99 constructs (e.g., variable declarations are at block scope starts), so it builds with Turbo C.
- **Warnings**: On modern compilers with `-Wall -Wextra`, you should get few or no warnings. On very old Turbo C, you may see benign “suspicious pointer conversion” warnings if prototypes differ; these don’t prevent compilation.
- **Line endings**: Reading handles `\n` and `\r\n`. Writing uses `\n`.

---

## Troubleshooting
- **It won’t run from anywhere**: Put the binary in a directory that’s on your PATH (e.g., `/usr/local/bin` on macOS/Linux, `C:\DOS` on DOS) or add your directory to PATH.
- **Turbo C compile errors**: Ensure you didn’t accidentally nest a second `switch(cmd)` in `main`. The valid `switch` should include all cases (`L I D E R S O W P H ? Q`). Missing braces or a truncated file will cause “Unexpected end of file” errors.
- **Saved file has extra line endings**: LinEd writes `\n`. On DOS viewers expecting `\r\n`, you may see a single line; open with editors that understand Unix line endings or convert if needed.

---

## Roadmap / Ideas
- Optional case‑insensitive `R` (replace) mode (`rg`).
- Write with original line endings per file.
- Undo of last edit block.
- Read‑only mode.
- Scripting/batch commands from a file.

---

## Credits & License
- Original author banner (retained in program output):
  - **Mickey Lawless (c) 2025, 2026**
- Portability & numbered‑prompt patch: this repo.

**License**: Choose a license (e.g., MIT) and add it as `LICENSE` in the repo. If unspecified, the code is “all rights reserved” by default.

