#pragma once
#include <sstream>
#include <string>
#include "parser.hpp"

class Generator {
public:
  explicit Generator(const node::Exit& root) : m_root(root) {}

  [[nodiscard]] std::string generate() const {
    std::ostringstream asmout;
    asmout << "global _start\n"
              "_start:\n"
              "    mov     rax, 60\n"
              "    mov     rdi, " << m_root.expr.int_literal.value.value() << "\n"
              "    syscall\n";
    return asmout.str();
  }

private:
  const node::Exit& m_root;
};
