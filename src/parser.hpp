#pragma once
#include <optional>
#include <vector>
#include "tokenization.hpp"

namespace node {
  struct Expression {
    Token int_literal; // must be TokenType::IntLiteral
  };
  struct Exit {
    Expression expr;
  };
}

class Parser {
public:
  explicit Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {}

  std::optional<node::Exit> parse() {
    // Expect: Exit Expression Semicolon
    if (!peek().has_value() || peek()->type != TokenType::Exit) return std::nullopt;
    consume(); // 'exit'

    auto expr = parseExpression();
    if (!expr.has_value()) return std::nullopt;

    if (!peek().has_value() || peek()->type != TokenType::Semicolon)
      return std::nullopt;
    consume(); // ';'

    return node::Exit{*expr};
  }

private:
  std::optional<node::Expression> parseExpression() {
    if (peek().has_value() && peek()->type == TokenType::IntLiteral) {
      Token t = consume();
      return node::Expression{t};
    }
    return std::nullopt;
  }

  [[nodiscard]] std::optional<Token> peek(int ahead = 1) const {
    if (m_index + static_cast<size_t>(ahead) > m_tokens.size()) return std::nullopt;
    return m_tokens.at(m_index + ahead - 1);
  }
  Token consume() { return m_tokens.at(m_index++); }

  std::vector<Token> m_tokens;
  std::size_t m_index = 0;
};
