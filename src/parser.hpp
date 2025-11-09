#pragma once
#include <optional>
#include <string>
#include <variant>
#include <vector>
#include <iostream>

#include "tokenization.hpp"

namespace hy {
namespace node {

struct ExprIntLiteral { Token literal; };
struct ExprIdent      { Token ident;   };

using Expression = std::variant<ExprIntLiteral, ExprIdent>;

struct StatementExit { Expression expr; };
struct StatementLet  { Token ident; Expression expr; };

using Statement = std::variant<StatementExit, StatementLet>;

struct Program { std::vector<Statement> statements; };

} // namespace node

class Parser {
public:
  explicit Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {}

  std::optional<node::Program> parseProgram() {
    node::Program prog;
    m_index = 0;

    while (auto t = peek(0)) {
      auto st = parseStatement();
      if (!st) {
        std::cerr << "Invalid statement.\n";
        return std::nullopt;
      }
      prog.statements.push_back(*st);
    }

    return prog;
  }

private:
  // ---- statments ----
  std::optional<node::Statement> parseStatement() {
    auto t = peek(0);
    if (!t) return std::nullopt;

    // exit '(' Expression ')' ';'
    if (t->type == TokenType::Exit) {
      consume(); // exit
      if (!peekIs(TokenType::OpenParen))  return err("expected '(' after exit");
      consume();
      auto expr = parseExpression();
      if (!expr) return err("invalid expression inside exit(...)");
      if (!peekIs(TokenType::CloseParen)) return err("expected ')' after expression");
      consume();
      if (!peekIs(TokenType::Semi))       return err("expected ';' after exit(...)");
      consume();
      return node::Statement{ node::StatementExit{ *expr } };
    }

    // let IDENT '=' Expression ';'
    if (t->type == TokenType::Let) {
      consume(); // let
      if (!peekIs(TokenType::Ident)) return err("expected identifier after 'let'");
      Token ident = consume();
      if (!peekIs(TokenType::Equals)) return err("expected '=' after identifier");
      consume();
      auto expr = parseExpression();
      if (!expr) return err("invalid expression in variable initializer");
      if (!peekIs(TokenType::Semi)) return err("expected ';' after variable declaration");
      consume();
      return node::Statement{ node::StatementLet{ ident, *expr } };
    }

    return std::nullopt;
  }

  // ---- expressions ----
  std::optional<node::Expression> parseExpression() {
    auto t = peek(0);
    if (!t) return std::nullopt;

    if (t->type == TokenType::IntLiteral) {
      Token lit = consume();
      return node::Expression{ node::ExprIntLiteral{ lit } };
    }
    if (t->type == TokenType::Ident) {
      Token id = consume();
      return node::Expression{ node::ExprIdent{ id } };
    }
    return std::nullopt;
  }

  // ---- helpers ----
  std::optional<node::Statement> err(const char* msg) {
    std::cerr << msg << "\n";
    return std::nullopt;
  }

  bool peekIs(TokenType tt, size_t offset = 0) const {
    auto t = peek(offset);
    return t && t->type == tt;
  }

  std::optional<Token> peek(size_t offset = 0) const {
    const size_t i = m_index + offset;
    if (i < m_tokens.size()) return m_tokens[i];
    return std::nullopt;
  }

  Token consume() {
    return m_tokens[m_index++];
  }

  std::vector<Token> m_tokens;
  size_t m_index{0};
};

} // namespace hy
