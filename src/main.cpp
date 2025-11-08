#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "tokenization.hpp"
#include "parser.hpp"
#include "generation.hpp"

static std::string read_file(const std::string& path) {
  std::ifstream in(path, std::ios::in);
  if (!in) throw std::runtime_error("Failed to open: " + path);
  std::ostringstream ss; ss << in.rdbuf(); return ss.str();
}

static void write_file(const std::string& path, const std::string& data) {
  std::ofstream out(path, std::ios::out | std::ios::trunc);
  if (!out) throw std::runtime_error("Failed to write: " + path);
  out << data;
}

static int sys(const std::string& cmd) {
  int rc = std::system(cmd.c_str());
  if (rc != 0) std::cerr << "[cmd failed] " << cmd << " (rc=" << rc << ")\n";
  return rc;
}

int main(int argc, char** argv) {
  try {
    if (argc != 2) { std::cerr << "Usage: hydro <input.hy>\n"; return EXIT_FAILURE; }
    const std::string in = argv[1];

    // 1) tokenize
    Tokenizer tz(read_file(in));
    auto tokens = tz.tokenize();

    // 2) parse
    Parser ps(std::move(tokens));
    auto tree = ps.parse();
    if (!tree.has_value()) { std::cerr << "Parse error: expected `exit <int>;`\n"; return EXIT_FAILURE; }

    // 3) codegen
    Generator gen(*tree);
    std::string asm_text = gen.generate();
    write_file("out.asm", asm_text);

    // assemble + link
    if (sys("nasm -felf64 out.asm") != 0) return EXIT_FAILURE;
    if (sys("ld -o out out.o") != 0) return EXIT_FAILURE;

    std::cout << "[ok] built out\n";
    return EXIT_SUCCESS;
  } catch (const std::exception& e) {
    std::cerr << "error: " << e.what() << "\n";
    return EXIT_FAILURE;
  }
}
