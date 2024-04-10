#include "tokenizer.h"


#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


#define MAX_KEYWORD_LENGTH 10
#define MIN_KEYWORD_LENGTH 2
#define MAX_IDENTIFIER_LENGTH 64
#define MAX_NUMBER_LENGTH 128


struct Token {
    TokenType type;
    void* data;
	int line;
	const char *source;
};


struct Tokenizer {
    TkGetCharCallback get_char_callback;
    void* callback_bind_ctx;
    Token *current_tk;
    bool tk_should_be_free;
    char last_char;
    bool is_char_consumed;
	int line;
	const char *source;
};


/* ERRORS: */
enum {
	UNREACHABLE,
	IDENTIFIER_TOO_LONG,
	INVALID_NUMBER,
	UNTERMINATED_STRING,
};
static const char *errors[] = {
	"Unreachable",
	"Identifier too long",
	"Invalid number",
	"Unterminated string",
};
#define ERR(E) (errors[E])


static const char *token_names[] = {
	"Empty", // EMPTY,
	// Basic
	"Annotation", // ANNOTATION
	"Identifier", // IDENTIFIER,
	"Literal", // LITERAL,
	// Comparison
	"<", // LESS,
	"<=", // LESS_EQUAL,
	">", // GREATER,
	">=", // GREATER_EQUAL,
	"==", // EQUAL_EQUAL,
	"!=", // BANG_EQUAL,
	// Logical
	"and", // AND,
	"or", // OR,
	"not", // NOT,
	"&&", // AMPERSAND_AMPERSAND,
	"||", // PIPE_PIPE,
	"!", // BANG,
	// Bitwise
	"&", // AMPERSAND,
	"|", // PIPE,
	"~", // TILDE,
	"^", // CARET,
	"<<", // LESS_LESS,
	">>", // GREATER_GREATER,
	// Math
	"+", // PLUS,
	"-", // MINUS,
	"*", // STAR,
	"**", // STAR_STAR,
	"/", // SLASH,
	"%", // PERCENT,
	// Assignment
	"=", // EQUAL,
	"+=", // PLUS_EQUAL,
	"-=", // MINUS_EQUAL,
	"*=", // STAR_EQUAL,
	"**=", // STAR_STAR_EQUAL,
	"/=", // SLASH_EQUAL,
	"%=", // PERCENT_EQUAL,
	"<<=", // LESS_LESS_EQUAL,
	">>=", // GREATER_GREATER_EQUAL,
	"&=", // AMPERSAND_EQUAL,
	"|=", // PIPE_EQUAL,
	"^=", // CARET_EQUAL,
	// Control flow
	"if", // IF,
	"elif", // ELIF,
	"else", // ELSE,
	"for", // FOR,
	"while", // WHILE,
	"break", // BREAK,
	"continue", // CONTINUE,
	"pass", // PASS,
	"ret", // RETURN,
	"match", // MATCH,
	"when", // WHEN,
	// Keywords
	"as", // AS,
	"assert", // ASSERT,
	"await", // AWAIT,
	"breakpoint", // BREAKPOINT,
	"class", // CLASS,
	"class_name", // CLASS_NAME,
	"const", // CONST,
	"enum", // ENUM,
	"extends", // EXTENDS,
	"func", // FUNC,
	"in", // IN,
	"is", // IS,
	"let", // LET,
	"preload", // PRELOAD,
	"self", // SELF,
	"signal", // SIGNAL,
	"space", // NAMESPACE
	"static", // STATIC,
	"super", // SUPER,
	"trait", // TRAIT,
	"type", // TYPE,
	"void", // VOID,
	"yield", // YIELD,
	// Punctuation
	"[", // BRACKET_OPEN,
	"]", // BRACKET_CLOSE,
	"{", // BRACE_OPEN,
	"}", // BRACE_CLOSE,
	"(", // PARENTHESIS_OPEN,
	")", // PARENTHESIS_CLOSE,
	",", // COMMA,
	";", // SEMICOLON,
	".", // PERIOD,
	"..", // PERIOD_PERIOD,
	":", // COLON,
	"$", // DOLLAR,
	"->", // FORWARD_ARROW,
	"_", // UNDERSCORE,
	// Whitespace
	"Newline", // NEWLINE,
	"Indent", // INDENT,
	"Dedent", // DEDENT,
	// Constants
	"PI", // CONST_PI,
	"TAU", // CONST_TAU,
	"INF", // CONST_INF,
	"NaN", // CONST_NAN,
	// Error message improvement
	"VCS conflict marker", // VCS_CONFLICT_MARKER,
	"`", // BACKTICK,
	"?", // QUESTION_MARK,
	// Special
	"Error", // ERROR,
	"End of file", // EOF,
};


/*
 * Utility functions
*/


static void _advance(Tokenizer *p_tokenizer) {
    p_tokenizer->last_char = p_tokenizer->get_char_callback(p_tokenizer->callback_bind_ctx);
	p_tokenizer->is_char_consumed = false;
}


static void _consume(Tokenizer *p_tokenizer) {
    p_tokenizer->is_char_consumed = true;
}


static char _get_current_char(Tokenizer *p_tokenizer) {
    if (p_tokenizer->is_char_consumed)
        _advance(p_tokenizer);
    return p_tokenizer->last_char;
}


static bool _is_digit(char p_char) {
    return p_char >= '0' && p_char <= '9';
}


static inline bool _is_underscore(char p_char) {
	return p_char == '_';
}


static bool _is_alpha(char p_char) {
    return  (p_char >= 'A' && p_char <= 'Z') ||
            (p_char >= 'a' && p_char <= 'z');
}


static inline bool _is_alphanum(char p_char) {
	return _is_digit(p_char) || _is_alpha(p_char);
}


/*
 * Tokenizer functions
*/


static void _free_token(Token *p_token) {
	switch (p_token->type) {
		case TK_LITERAL:
		case TK_IDENTIFIER:
			if(p_token->data)
				literalFree((Literal*)p_token->data);
		default:
			break;
	}
    free(p_token);
}


static Token *_create_token(Tokenizer *p_tokenizer, TokenType p_type, void *p_data) {
    Token *tk = (Token*)malloc(sizeof(Token));
    if (!tk)
        return NULL;
    tk->type = p_type;
    tk->data = p_data;
	tk->line = p_tokenizer->line;
	tk->source = p_tokenizer->source;
    if (p_tokenizer->current_tk && p_tokenizer->tk_should_be_free)
        _free_token(p_tokenizer->current_tk);
    p_tokenizer->current_tk = tk;
    return tk;
}


static TokenType _which_identifier(const char *p_str, size_t p_len) {
	if (p_len > MAX_KEYWORD_LENGTH || p_len < MIN_KEYWORD_LENGTH)
		return TK_IDENTIFIER;
	
	#define KEYWORD_GROUP(CHAR) \
			break;\
		case CHAR:
	#define KEYWORD(N, T) \
		if (p_len == sizeof(N) - 1)\
			if (!strncmp(token_names[T], p_str, p_len))\
				return T;
	
	switch (*p_str) {
		case '\0':
		KEYWORD_GROUP('a')
			KEYWORD("as", TK_AS)
			KEYWORD("and", TK_AND)
			KEYWORD("assert", TK_ASSERT)
			KEYWORD("await", TK_AWAIT)
		KEYWORD_GROUP('b')
			KEYWORD("break", TK_BREAK)
			KEYWORD("breakpoint", TK_BREAKPOINT)
		KEYWORD_GROUP('c')
			KEYWORD("class", TK_CLASS)
			KEYWORD("class_name", TK_CLASS_NAME)
			KEYWORD("const", TK_CONST)
			KEYWORD("continue", TK_CONTINUE)
		KEYWORD_GROUP('e')
			KEYWORD("elif", TK_ELIF)
			KEYWORD("else", TK_ELSE)
			KEYWORD("enum", TK_ENUM)
			KEYWORD("extends", TK_EXTENDS)
		KEYWORD_GROUP('f')
			KEYWORD("for", TK_FOR)
			KEYWORD("func", TK_FUNC)
		KEYWORD_GROUP('i')
			KEYWORD("if", TK_IF)
			KEYWORD("in", TK_IN)
			KEYWORD("is", TK_IS)
		KEYWORD_GROUP('l')
			KEYWORD("let", TK_LET)
		KEYWORD_GROUP('m')
			KEYWORD("match", TK_MATCH)
		KEYWORD_GROUP('n')
			KEYWORD("not", TK_NOT)
		KEYWORD_GROUP('o')
			KEYWORD("or", TK_OR)
		KEYWORD_GROUP('p')
			KEYWORD("pass", TK_PASS)
			KEYWORD("preload", TK_PRELOAD)
		KEYWORD_GROUP('r')
			KEYWORD("ret", TK_RET)
		KEYWORD_GROUP('s')
			KEYWORD("self", TK_SELF)
			KEYWORD("signal", TK_SIGNAL)
			KEYWORD("space", TK_SPACE)
			KEYWORD("static", TK_STATIC)
			KEYWORD("super", TK_SUPER)
		KEYWORD_GROUP('t')
			KEYWORD("trait", TK_TRAIT)
			KEYWORD("type", TK_TYPE)
		KEYWORD_GROUP('v')
			KEYWORD("void", TK_VOID)
		KEYWORD_GROUP('w')
			KEYWORD("while", TK_WHILE)
			KEYWORD("when", TK_WHEN)
		KEYWORD_GROUP('y')
			KEYWORD("yield", TK_YIELD)
		KEYWORD_GROUP('I')
			KEYWORD("inf", TK_CONST_INF)
		KEYWORD_GROUP('N')
			KEYWORD("nan", TK_CONST_NAN)
		KEYWORD_GROUP('P')
			KEYWORD("pi", TK_CONST_PI)
		KEYWORD_GROUP('T')
			KEYWORD("tau", TK_CONST_TAU)
	}
	return TK_IDENTIFIER;

	#undef KEYWORD_GROUP
	#undef KEYWORD
}


static Token *_parse_identifier(Tokenizer *p_tokenizer) {
	char name[MAX_IDENTIFIER_LENGTH];
	name[MAX_IDENTIFIER_LENGTH - 1] = '\0';
	size_t len = 0;
	bool has_digit = false;
	for (int i = 0; i < MAX_IDENTIFIER_LENGTH - 1; i++) {
		char c = _get_current_char(p_tokenizer);
		if (_is_alpha(c) || _is_underscore(c))
			goto set_char;
		if (_is_digit(c)) {
			has_digit = true;
			goto set_char;
		}
		name[i] = '\0';
		break;
		set_char:
			len++;
			name[i] = c;
			_consume(p_tokenizer);
	}

	if (len == 1 && _is_underscore(*name))
		return _create_token(p_tokenizer, TK_UNDERSCORE, NULL);

	if (len == MAX_IDENTIFIER_LENGTH - 1 && _is_alphanum(_get_current_char(p_tokenizer)))
		return _create_token(p_tokenizer, TK_ERROR, (void*)ERR(IDENTIFIER_TOO_LONG));

	TokenType tk_type = has_digit ? TK_IDENTIFIER : _which_identifier(name, len);
	Literal *lt = NULL;
	if (tk_type == TK_IDENTIFIER)
		lt = literalCreate(LT_STRING, (void*)name);
	return _create_token(p_tokenizer, tk_type, (void*)lt);
}


static Token *_parse_number(Tokenizer *p_tokenizer) {
	char buffer[MAX_NUMBER_LENGTH];
	buffer[MAX_NUMBER_LENGTH - 1] = '\0';
	size_t len = 0;
	memset(buffer, 0, MAX_NUMBER_LENGTH);
	enum {
		INT,
		FLOAT
	} type = INT;

	for (int i = 0; i < MAX_NUMBER_LENGTH - 1; i++) {
		char c = _get_current_char(p_tokenizer);
		if (_is_digit(c))
			goto set_char;
		if (c == '.') {
			if (type == FLOAT)
				goto invalid;
			type = FLOAT;
			goto set_char;
		}
		buffer[i] = '\0';
		break;

		set_char:
			len++;
			buffer[i] = c;
			_consume(p_tokenizer);
	}
	if (len == MAX_NUMBER_LENGTH - 1 && _is_digit(_get_current_char(p_tokenizer)))
		goto invalid;
	
	// FIXME: GET NUMBER SUFFIX
	
	Literal *lt;
	switch (type) {
		case INT:;
			int integer = strtol(buffer, NULL, 10);
			lt = literalCreate(LT_INT, &integer);
			break;
		case FLOAT:;
			float floating_point = strtof(buffer, NULL);
			lt = literalCreate(LT_FLOAT, &floating_point);
			break;
	}

	return _create_token(p_tokenizer, TK_LITERAL, (void*)lt);
	invalid:
		return _create_token(p_tokenizer, TK_ERROR, (void*)ERR(INVALID_NUMBER));
}


static Token *_parse_string(Tokenizer *p_tokenizer) {
	_consume(p_tokenizer);
	char c;
	while (c = _get_current_char(p_tokenizer), c != '"') {
		if (c == -1)
			return _create_token(p_tokenizer, TK_ERROR, (void*)ERR(UNTERMINATED_STRING));
		_consume(p_tokenizer);
	}
	_consume(p_tokenizer);
	Literal *lt = literalCreate(LT_STRING, "Good Job");
	return _create_token(p_tokenizer, TK_LITERAL, (void*)lt);
}


static void _parse_comment(Tokenizer *p_tokenizer) {
	char c;
	while (c = _get_current_char(p_tokenizer), c != '\n' && c != -1)
		_consume(p_tokenizer);
}


Tokenizer *tokenizerInit(TkGetCharCallback p_get_char, void* p_bind_ctx, const char *p_source) {
    Tokenizer* tk = (Tokenizer*)malloc(sizeof(Tokenizer));
    if (!tk)
        return NULL;
    tk->get_char_callback = p_get_char;
    tk->callback_bind_ctx = p_bind_ctx;
    tk->current_tk = NULL;
    tk->tk_should_be_free = true;
    tk->last_char = -1;
    tk->is_char_consumed = true;
	tk->line = 1;
	tk->source = p_source;
    return tk;
}


Token *tokenizerAdvance(Tokenizer *p_tokenizer) {
    start:;
    char c = _get_current_char(p_tokenizer);
    switch (c) {
        case -1:
            return _create_token(p_tokenizer, TK_EOF, NULL);
        case '\n':
			p_tokenizer->line++;
		case '\t':
        case ' ':
            _consume(p_tokenizer);
            goto start;
		case '@':
			_consume(p_tokenizer);
			return _create_token(p_tokenizer, TK_ANNOTATION, NULL);
		case '<':
			_consume(p_tokenizer);
			switch (_get_current_char(p_tokenizer)) {
				case '=':
					_consume(p_tokenizer);
					return _create_token(p_tokenizer, TK_LESS_EQUAL, NULL);
				case '<':
					_consume(p_tokenizer);
					return _create_token(p_tokenizer, TK_LESS_LESS, NULL);
			}
			return _create_token(p_tokenizer, TK_LESS, NULL);
		case '>':
			_consume(p_tokenizer);
			switch (_get_current_char(p_tokenizer)) {
				case '=':
					_consume(p_tokenizer);
					return _create_token(p_tokenizer, TK_GREATER_EQUAL, NULL);
				case '<':
					_consume(p_tokenizer);
					return _create_token(p_tokenizer, TK_GREATER_GREATER, NULL);
			}
			return _create_token(p_tokenizer, TK_GREATER, NULL);
		case '=':
			_consume(p_tokenizer);
			if (_get_current_char(p_tokenizer) == '=') {
				_consume(p_tokenizer);
				return _create_token(p_tokenizer, TK_EQUAL_EQUAL, NULL);
			}
			return _create_token(p_tokenizer, TK_EQUAL, NULL);
		case '!':
			_consume(p_tokenizer);
			if (_get_current_char(p_tokenizer) == '=') {
				_consume(p_tokenizer);
				return _create_token(p_tokenizer, TK_BANG_EQUAL, NULL);
			}
			return _create_token(p_tokenizer, TK_BANG, NULL);
		case '&':
			_consume(p_tokenizer);
			switch (_get_current_char(p_tokenizer)) {
				case '&':
					_consume(p_tokenizer);
					return _create_token(p_tokenizer, TK_AMPERSAND_AMPERSAND, NULL);
				case '=':
					_consume(p_tokenizer);
					return _create_token(p_tokenizer, TK_AMPERSAND_EQUAL, NULL);
			}
			return _create_token(p_tokenizer, TK_AMPERSAND, NULL);
		case '|':
			_consume(p_tokenizer);
			switch (_get_current_char(p_tokenizer)) {
				case '|':
					_consume(p_tokenizer);
					return _create_token(p_tokenizer, TK_PIPE_PIPE, NULL);
				case '=':
					_consume(p_tokenizer);
					return _create_token(p_tokenizer, TK_PIPE_EQUAL, NULL);
			}
			return _create_token(p_tokenizer, TK_PIPE, NULL);
		case '~':
			_consume(p_tokenizer);
			return _create_token(p_tokenizer, TK_TILDE, NULL);
		case '^':
			_consume(p_tokenizer);
			if (_get_current_char(p_tokenizer) == '=') {
				_consume(p_tokenizer);
				return _create_token(p_tokenizer, TK_CARET_EQUAL, NULL);
			}
			return _create_token(p_tokenizer, TK_CARET, NULL);
		case '+':
			_consume(p_tokenizer);
			if (_get_current_char(p_tokenizer) == '=') {
				_consume(p_tokenizer);
				return _create_token(p_tokenizer, TK_PLUS_EQUAL, NULL);
			}
			return _create_token(p_tokenizer, TK_PLUS, NULL);
		case '-':
			_consume(p_tokenizer);
			switch (_get_current_char(p_tokenizer)) {
				case '=':
					_consume(p_tokenizer);
					return _create_token(p_tokenizer, TK_MINUS_EQUAL, NULL);
				case '>':
					_consume(p_tokenizer);
					return _create_token(p_tokenizer, TK_FORWARD_ARROW, NULL);
			}
			return _create_token(p_tokenizer, TK_MINUS, NULL);
		case '*':
			_consume(p_tokenizer);
			if (_get_current_char(p_tokenizer) == '*') {
				_consume(p_tokenizer);
				if (_get_current_char(p_tokenizer) == '=') {
					_consume(p_tokenizer);
					return _create_token(p_tokenizer, TK_STAR_STAR_EQUAL, NULL);
				}
				return _create_token(p_tokenizer, TK_STAR_STAR, NULL);
			}
			return _create_token(p_tokenizer, TK_STAR, NULL);
		case '/':
			_consume(p_tokenizer);
			if (_get_current_char(p_tokenizer) == '=') {
				_consume(p_tokenizer);
				return _create_token(p_tokenizer, TK_SLASH_EQUAL, NULL);
			}
			return _create_token(p_tokenizer, TK_SLASH, NULL);
		case '%':
			_consume(p_tokenizer);
			if (_get_current_char(p_tokenizer) == '=') {
				_consume(p_tokenizer);
				return _create_token(p_tokenizer, TK_PERCENT_EQUAL, NULL);
			}
			return _create_token(p_tokenizer, TK_PERCENT, NULL);
		case '[':
			_consume(p_tokenizer);
			return _create_token(p_tokenizer, TK_BRACKET_OPEN, NULL);
		case ']':
			_consume(p_tokenizer);
			return _create_token(p_tokenizer, TK_BRACKET_CLOSE, NULL);
		case '{':
			_consume(p_tokenizer);
			return _create_token(p_tokenizer, TK_BRACE_OPEN, NULL);
		case '}':
			_consume(p_tokenizer);
			return _create_token(p_tokenizer, TK_BRACE_CLOSE, NULL);
		case '(':
			_consume(p_tokenizer);
			return _create_token(p_tokenizer, TK_PARENTHESIS_OPEN, NULL);
		case ')':
			_consume(p_tokenizer);
			return _create_token(p_tokenizer, TK_PARENTHESIS_CLOSE, NULL);
		case ',':
			_consume(p_tokenizer);
			return _create_token(p_tokenizer, TK_COMMA, NULL);
		case ';':
			_consume(p_tokenizer);
			return _create_token(p_tokenizer, TK_SEMICOLON, NULL);
		case '.':
			_consume(p_tokenizer);
			if (_get_current_char(p_tokenizer) == '.') {
				_consume(p_tokenizer);
				return _create_token(p_tokenizer, TK_PERIOD_PERIOD, NULL);
			}
			return _create_token(p_tokenizer, TK_PERIOD, NULL);
		case ':':
			_consume(p_tokenizer);
			return _create_token(p_tokenizer, TK_COLON, NULL);
		case '$':
			_consume(p_tokenizer);
			return _create_token(p_tokenizer, TK_DOLLAR, NULL);
		case '"':
			return _parse_string(p_tokenizer);
		case '#':
			_consume(p_tokenizer);
            _parse_comment(p_tokenizer);
			goto start;
        default:
            if (_is_digit(c))
                return _parse_number(p_tokenizer);
            else if (_is_alpha(c) || _is_underscore(c))
                return _parse_identifier(p_tokenizer);
    }

    return _create_token(p_tokenizer, TK_ERROR, (void*)ERR(UNREACHABLE));
}


TokenType tokenizerAdvanceType(Tokenizer *p_tokenizer) {
	return tokenizerAdvance(p_tokenizer)->type;
}


Token *tokenizerGetCurrent(Tokenizer *p_tokenizer) {
	if (!p_tokenizer->current_tk)
		tokenizerAdvance(p_tokenizer);
    return p_tokenizer->current_tk;
}


TokenType tokenizerGetCurrentType(Tokenizer *p_tokenizer) {
	if (!p_tokenizer->current_tk)
		tokenizerAdvance(p_tokenizer);
	return p_tokenizer->current_tk->type;
}


void tokenizerPush(Tokenizer *p_tokenizer) {
	// FIXME: IMPLEMENT THIS
}


Token *tokenizerPop(Tokenizer *p_tokenizer) {
	// FIXME: IMPLEMENT THIS
    return NULL;
}


Literal *tokenizerTokenGetLiteral(const Token *p_token) {
	switch (p_token->type) {
		case TK_IDENTIFIER:
		case TK_LITERAL:
			return (Literal*)p_token->data;
		default:
			break;
	}
	return NULL;
}


const char *tokenizerTokenGetErrorString(const Token *p_token) {
	if (p_token->type == TK_ERROR)
		return (char*)p_token->data;
	return NULL;
}


TokenType tokenizerTokenGetType(const Token *p_token) {
	return p_token->type;
}


const char *tokenizerTokenGetTypeName(const Token *p_token) {
    return token_names[p_token->type];
}


const char *tokenizerTokenTypeName(const TokenType p_type) {
	return token_names[p_type];
}


int tokenizerTokenGetLine(const Token *p_token) {
	return p_token->line;
}


const char *tokenizerTokenGetSource(const Token *p_token) {
	return p_token->source;
}


void tokenizerTerminate(Tokenizer *p_tokenizer)
{
    free(p_tokenizer);
}
