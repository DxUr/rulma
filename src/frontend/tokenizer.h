#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "literal.h"

typedef enum {
    TK_EMPTY,
    // Basic
    TK_ANNOTATION,
    TK_IDENTIFIER,
    TK_LITERAL,
    // Comparison
    TK_LESS,
    TK_LESS_EQUAL,
    TK_GREATER,
    TK_GREATER_EQUAL,
    TK_EQUAL_EQUAL,
    TK_BANG_EQUAL,
    // Logical
    TK_AND,
    TK_OR,
    TK_NOT,
    TK_AMPERSAND_AMPERSAND,
    TK_PIPE_PIPE,
    TK_BANG,
    // Bitwise
    TK_AMPERSAND,
    TK_PIPE,
    TK_TILDE,
    TK_CARET,
    TK_LESS_LESS,
    TK_GREATER_GREATER,
    // Math
    TK_PLUS,
    TK_MINUS,
    TK_STAR,
    TK_STAR_STAR,
    TK_SLASH,
    TK_PERCENT,
    // Assignment
    TK_EQUAL,
    TK_PLUS_EQUAL,
    TK_MINUS_EQUAL,
    TK_STAR_EQUAL,
    TK_STAR_STAR_EQUAL,
    TK_SLASH_EQUAL,
    TK_PERCENT_EQUAL,
    TK_LESS_LESS_EQUAL,
    TK_GREATER_GREATER_EQUAL,
    TK_AMPERSAND_EQUAL,
    TK_PIPE_EQUAL,
    TK_CARET_EQUAL,
    // Control flow
    TK_IF,
    TK_ELIF,
    TK_ELSE,
    TK_FOR,
    TK_WHILE,
    TK_BREAK,
    TK_CONTINUE,
    TK_PASS,
    TK_RET,
    TK_MATCH,
    TK_WHEN,
    // Keywords
    TK_AS,
    TK_ASSERT,
    TK_AWAIT,
    TK_BREAKPOINT,
    TK_CLASS,
    TK_CLASS_NAME,
    TK_CONST,
    TK_ENUM,
    TK_EXTENDS,
    TK_FUNC,
    TK_IN,
    TK_IS,
    TK_LET,
    TK_PRELOAD,
    TK_SELF,
    TK_SIGNAL,
    TK_SPACE,
    TK_STATIC,
    TK_SUPER,
    TK_TRAIT,
    TK_TYPE,
    TK_VOID,
    TK_YIELD,
    // Punctuation
    TK_BRACKET_OPEN,
    TK_BRACKET_CLOSE,
    TK_BRACE_OPEN,
    TK_BRACE_CLOSE,
    TK_PARENTHESIS_OPEN,
    TK_PARENTHESIS_CLOSE,
    TK_COMMA,
    TK_SEMICOLON,
    TK_PERIOD,
    TK_PERIOD_PERIOD,
    TK_COLON,
    TK_DOLLAR,
    TK_FORWARD_ARROW,
    TK_UNDERSCORE,
    // Whitespace
    TK_NEWLINE,
    TK_INDENT,
    TK_DEDENT,
    // Constants
    TK_CONST_PI,
    TK_CONST_TAU,
    TK_CONST_INF,
    TK_CONST_NAN,
    // Error message improvement
    TK_VCS_CONFLICT_MARKER,
    TK_BACKTICK,
    TK_QUESTION_MARK,
    // Special
    TK_ERROR,
    TK_EOF
} TokenType;


typedef struct Tokenizer Tokenizer;

typedef char (*TkGetCharCallback)(void*);

typedef struct Token Token;


Tokenizer *tokenizerInit(TkGetCharCallback p_get_char, void* p_bind_ctx, const char *p_source);
Token *tokenizerAdvance(Tokenizer *p_tokenizer);
TokenType tokenizerAdvanceType(Tokenizer *p_tokenizer);
Token *tokenizerGetCurrent(Tokenizer *p_tokenizer);
TokenType tokenizerGetCurrentType(Tokenizer *p_tokenizer);
void tokenizerPush(Tokenizer *p_tokenizer);
Token *tokenizerPop(Tokenizer *p_tokenizer);

Literal *tokenizerTokenGetLiteral(const Token *p_token);
const char *tokenizerTokenGetErrorString(const Token *p_token);
TokenType tokenizerTokenGetType(const Token *p_token);
const char *tokenizerTokenGetTypeName(const Token *p_token);
const char *tokenizerTokenTypeName(const TokenType p_type);
int tokenizerTokenGetLine(const Token *p_token);
const char *tokenizerTokenGetSource(const Token *p_token);

void tokenizerTerminate(Tokenizer *p_tokenizer);

#endif // TOKENIZER_H
