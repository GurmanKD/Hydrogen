#pragma once
#include <string>
#include <sstream>
#include <unordered_map>
#include <variant>
#include <iostream>

#include "parser.hpp"

namespace hy {

class Generator {
public:
  explicit Generator(const node::Program& program) : m_program(program) {}

  // Render whole program to NASM
  std::string generate() {
    m_output.clear();
    m_stack_size = 0;
    m_vars.clear();

    out("global _start\n");
    out("_start:\n");

    // Emit all statements
    for (const auto& st : m_program.statements) {
      genStatement(st);
    }

    // Default exit(0) if user didn't call exit(...)
    out("    mov     rax, 60\n");
    out("    mov     rdi, 0\n");
    out("    syscall\n");

    return m_output;
  }

private:
  // ---------- state ----------
  struct Var { size_t stack_location; }; // slot index (0 = bottom-most pushed)
  const node::Program& m_program;
  std::string m_output;
  size_t m_stack_size{0}; // number of 64-bit slots currently on stack
  std::unordered_map<std::string, Var> m_vars;

  // ---------- helpers ----------
  void out(const std::string& s) { m_output += s; }

  void push(const std::string& reg) {
    out("    push    " + reg + "\n");
    ++m_stack_size;
  }

  void pop(const std::string& reg) {
    out("    pop     " + reg + "\n");
    if (m_stack_size == 0) {
      std::cerr << "Internal error: pop on empty stack\n";
      std::exit(EXIT_FAILURE);
    }
    --m_stack_size;
  }

  // ---------- codegen ----------
  void genStatement(const node::Statement& st) {
    struct Visitor {
      Generator* g;
      void operator()(const node::StatementExit& s) const {
        g->genExpression(s.expr); // pushes value
        g->out("    mov     rax, 60\n"); // sys_exit
        g->pop("rdi");                   // exit code from expr
        g->out("    syscall\n");
      }
      void operator()(const node::StatementLet& s) const {
        // Evaluate init expr -> pushes value on stack
        g->genExpression(s.expr);

        // Identifier text
        const std::string name = *s.ident.str_value;

        // Check duplicate
        if (g->m_vars.find(name) != g->m_vars.end()) {
          std::cerr << "Identifier already used: " << name << "\n";
          std::exit(EXIT_FAILURE);
        }

        // The just-pushed value is at top => slot index = m_stack_size - 1
        g->m_vars.emplace(name, Var{ g->m_stack_size - 1 });
      }
    };
    std::visit(Visitor{this}, st);
  }

  void genExpression(const node::Expression& e) {
    struct Visitor {
      Generator* g;
      void operator()(const node::ExprIntLiteral& e) const {
        long long v = *e.literal.int_value;
        g->out("    mov     rax, " + std::to_string(v) + "\n");
        g->push("rax");
      }
      void operator()(const node::ExprIdent& e) const {
        const std::string name = *e.ident.str_value;

        auto it = g->m_vars.find(name);
        if (it == g->m_vars.end()) {
          std::cerr << "Undeclared identifier: " << name << "\n";
          std::exit(EXIT_FAILURE);
        }

        const size_t var_slot = it->second.stack_location;
        // Top is m_stack_size-1; compute slots from top
        if (g->m_stack_size == 0 || var_slot > g->m_stack_size - 1) {
          std::cerr << "Internal error: bad slot for " << name << "\n";
          std::exit(EXIT_FAILURE);
        }
        const size_t slots_from_top = (g->m_stack_size - 1) - var_slot;
        const size_t offset_bytes   = slots_from_top * 8;

        // Push a copy of qword at [rsp + offset]
        g->out("    push    qword [rsp + " + std::to_string(offset_bytes) + "]\n");
        ++g->m_stack_size;
      }
    };
    std::visit(Visitor{this}, e);
  }
};

} // namespace hy
