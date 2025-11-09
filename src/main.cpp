#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "tokenization.hpp"
#include "parser.hpp"
#include "generation.hpp"

static std::string read_all(const std::string& path) {
  std::ifstream in(path);
  if (!in) {
    std::cerr << "Failed to open: " << path << "\n";
    std::exit(EXIT_FAILURE);
  }
  std::ostringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

static void write_all(const std::string& path, const std::string& data) {
  std::ofstream out(path);
  if (!out) {
    std::cerr << "Failed to write: " << path << "\n";
    std::exit(EXIT_FAILURE);
  }
  out << data;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "usage: hydro <source.hy>\n";
    return 1;
  }

  const std::string src_path = argv[1];
  const std::string src = read_all(src_path);

  // 1) tokenize
  hy::Tokenizer tz(src);
  auto tokens = tz.tokenize();

  // 2) parse
  hy::Parser parser(std::move(tokens));
  auto program = parser.parseProgram();
  if (!program) {
    std::cerr << "Parse failed.\n";
    return 1;
  }

  // 3) generate asm
  hy::Generator gen(*program);
  const std::string asm_text = gen.generate();
  write_all("out.asm", asm_text);

  // 4) assemble + link (Linux/WSL)
  //    nasm -> ELF64 obj, then ld -> executable "out"
  int rc = std::system("nasm -felf64 out.asm -o out.o");
  if (rc != 0) { std::cerr << "nasm failed\n"; return 1; }
  rc = std::system("ld -o out out.o");
  if (rc != 0) { std::cerr << "ld failed\n"; return 1; }

  std::cout << "Built ./out\n";
  return 0;
}
