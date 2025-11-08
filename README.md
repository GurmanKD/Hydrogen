# Hydrogen (hy) — a tiny self-hosting-curious language  
**Milestone 2: Tokenizer · Parser · AST · Generator**

> 🧠 **Goal:** compile a minimal language statement  
> `exit <int>;` → NASM x86-64 assembly → ELF executable → returns that exit code.

---

### 📘 Notion Notes
**Part 1:** [https://l1nq.com/HydrogenPart1NotionNotes](https://l1nq.com/HydrogenPart1NotionNotes)

---

## ✨ Features (so far)

#### 🪄 Milestone 1
- Lexer for: `return`, integer literals, `;`
- Direct `tokens → NASM` (no AST)
- Assembles with `nasm` + `ld`
- Demo program returns the chosen exit code (0–255)

#### ⚙️ Milestone 2
- Replaced keyword `return` → `exit`
- Introduced a **Tokenizer class** with `peek()` + `consume()`
- Added a **Parser + AST**  
  Grammar:  

ExitNode → 'exit' Expression ';'
Expression → IntLiteral
- Added a **Generator** that emits NASM from the AST
- Full pipeline: **tokenize → parse → generate → assemble → link → run**

---

## 🧩 Requirements
- Linux / WSL (for `nasm` + `ld`)
- `cmake`, `g++` or `clang++`

Install on Ubuntu:
```bash
sudo apt update
sudo apt install -y nasm build-essential cmake
```
🏗️ Build & Run
Using helper scripts:
```bash
./scripts/build.sh
./scripts/run.sh
# prints "exit code: 21"
```
Or manually:
```bash
cmake -S . -B build
cmake --build build -j
./build/hydro examples/exit_ok.hy
./out
echo $?
```
🧠 How it works
Tokenize
```bash
exit 21;
```
→ [EXIT, INT_LITERAL(21), SEMICOLON]
Parse → AST
Exit {
  Expression {
    IntLiteral(21)
  }
}
Generate NASM
global _start
_start:
    mov     rax, 60      ; sys_exit
    mov     rdi, 21      ; exit code
    syscall
Assemble + Link
nasm -felf64 out.asm
ld -o out out.o
Run
./out
echo $?   # → 21
🧱 Project Structure
```bash
Hydrogen/
├─ src/
│  ├─ main.cpp
│  ├─ tokenization.hpp
│  ├─ parser.hpp
│  └─ generation.hpp
├─ examples/
│  └─ exit_ok.hy
├─ scripts/
│  ├─ build.sh
│  └─ run.sh
├─ docs/
│  └─ grammar.md
├─ CMakeLists.txt
└─ README.md
```
🗓️ Milestone History
Version	Stage	Highlights
v0.1-part1	Minimal compiler	tokens → NASM → ELF
v0.2-part2	Tokenizer + Parser + AST + Generator	full pipeline implemented
🧩 Hydrogen is an educational experiment in building a self-hosting compiler from scratch — one step at a time.
