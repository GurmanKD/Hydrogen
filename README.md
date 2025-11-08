# Hydrogen (hy) — a tiny self-hosting-curious language (Milestone 1)

**Goal:** from a single statement `return <int>;` produce x86-64 Linux assembly (NASM), assemble to ELF, and run.

## Features (so far)
- Lexer for: `return`, integer literals, `;`
- Direct tokens → NASM codegen (no AST yet)
- Assembles with `nasm` and links with `ld` from the compiler
- Demo program returns the chosen exit code (0–255)

## Requirements
- Linux (or WSL)
- `nasm`, `ld` (binutils), `cmake`, `g++/clang++`

Install on Ubuntu:
```bash
sudo apt-get update
sudo apt-get install -y nasm build-essential cmake
```
Build & Run
```bash
./scripts/build.sh
./scripts/run.sh
# shows "exit code: 21"
```
Or manually:
```bash
cmake -S . -B build
cmake --build build -j
./build/hydro examples/return_ok.hy
./out
echo $?
```
How it works
1. Tokenize: `return 21;` → `[RETURN, INT_LITERAL(21), SEMICOLON]`
2. Codegen: `emit NASM:`
```bash
global _start
_start:
    mov rax, 60
    mov rdi, 21
    syscall
```
3. Assemble/Link: `nasm -felf64 out.asm && ld -o out out.o`
4. Run: `./out; echo $?` → `21`