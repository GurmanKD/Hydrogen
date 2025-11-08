#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

enum class TokenType { Return, IntLiteral, Semicolon };

struct Token {
  TokenType type;
  std::optional<std::string> value; // used for IntLiteral
};

// --- file helpers ---
static std::string read_file(const std::string &path) {
  std::ifstream in(path, std::ios::in);
  if (!in) { throw std::runtime_error("Failed to open: " + path); }
  std::ostringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

static void write_file(const std::string &path, const std::string &data) {
  std::ofstream out(path, std::ios::out | std::ios::trunc);
  if (!out) { throw std::runtime_error("Failed to write: " + path); }
  out << data;
}

// --- lexer ---
static std::vector<Token> tokenize(const std::string &src) {
  std::vector<Token> tokens;
  std::string buf;
  for (std::size_t i = 0; i < src.size(); ++i) {
    char c = src.at(i);

    if (std::isspace(static_cast<unsigned char>(c))) {
      continue;
    }

    if (std::isalpha(static_cast<unsigned char>(c))) {
      buf.clear();
      buf.push_back(c);
      ++i;
      while (i < src.size() && std::isalnum(static_cast<unsigned char>(src.at(i)))) {
        buf.push_back(src.at(i));
        ++i;
      }
      --i; // step back one because for-loop will ++i
      if (buf == "return") {
        tokens.push_back(Token{TokenType::Return, std::nullopt});
      } else {
        throw std::runtime_error("Unknown identifier: " + buf);
      }
      continue;
    }

    if (std::isdigit(static_cast<unsigned char>(c))) {
      buf.clear();
      buf.push_back(c);
      ++i;
      while (i < src.size() && std::isdigit(static_cast<unsigned char>(src.at(i)))) {
        buf.push_back(src.at(i));
        ++i;
      }
      --i;
      tokens.push_back(Token{TokenType::IntLiteral, buf});
      continue;
    }

    if (c == ';') {
      tokens.push_back(Token{TokenType::Semicolon, std::nullopt});
      continue;
    }

    std::ostringstream err;
    err << "Unexpected character: '" << c << "'";
    throw std::runtime_error(err.str());
  }
  return tokens;
}

// --- codegen (tokens → NASM) ---
static std::string tokens_to_asm(const std::vector<Token> &toks) {
  // Expect: RETURN INT_LITERAL SEMICOLON
  if (toks.size() >= 3 &&
      toks[0].type == TokenType::Return &&
      toks[1].type == TokenType::IntLiteral &&
      toks[2].type == TokenType::Semicolon) {
    const std::string &val = toks[1].value.value(); // guaranteed present
    std::ostringstream out;
    out << "global _start\n"
           "_start:\n"
           "    mov     rax, 60\n"     // sys_exit
           "    mov     rdi, " << val << "\n"
           "    syscall\n";
    return out.str();
  }
  throw std::runtime_error("Syntax error: expected `return <int>;`");
}

static int sys(const std::string &cmd) {
  int rc = std::system(cmd.c_str());
  if (rc != 0) {
    std::cerr << "[cmd failed] " << cmd << " (rc=" << rc << ")\n";
  }
  return rc;
}

int main(int argc, char **argv) {
  try {
    if (argc != 2) {
      std::cerr << "Usage: hydro <input.hy>\n";
      return EXIT_FAILURE;
    }
    const std::string in_path = argv[1];
    const std::string asm_path = "out.asm";
    const std::string obj_path = "out.o";
    const std::string exe_path = "out";

    // 1) read
    const std::string source = read_file(in_path);

    // 2) lex
    const auto tokens = tokenize(source);

    // 3) codegen (direct, no AST for now)
    const std::string asm_text = tokens_to_asm(tokens);

    // 4) write asm
    write_file(asm_path, asm_text);

    // 5) assemble + link (Linux/x86-64)
    if (sys("nasm -felf64 " + asm_path) != 0) return EXIT_FAILURE;
    if (sys("ld -o " + exe_path + " " + obj_path) != 0) return EXIT_FAILURE;

    std::cout << "[ok] built " << exe_path << "\n";
    return EXIT_SUCCESS;
  } catch (const std::exception &e) {
    std::cerr << "error: " << e.what() << "\n";
    return EXIT_FAILURE;
  }
}
