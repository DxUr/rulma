#include "parser.h"
#include "error.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>


typedef enum {
	// PROC stands for procedure
	PROC_UNIT,
	PROC_DECLARATION,
	PROC_SPACE_DECLARATION,
	PROC_ENUM_DECLARATION,
	PROC_TYPE_DECLARATION,
	PROC_LET_DECLARATION,
	PROC_METHOD_DECLARATION,
	PROC_SCOPE,
	PROC_STATEMENT,
	PROC_EXPRESSION,
	PROC_EXP_BINARY,
	PROC_EXP_VALUE,
} PROC_TYPE;

typedef int PROC_STATE;

typedef struct ParseCtx {
	PROC_TYPE type;
	PROC_STATE state;
	struct ParseCtx *prev_ctx;
} ParseCtx;


struct Parser {
	Tokenizer *tokenizer;
	ParseCtx *stack_popped;
	ParseCtx *stack_top;
	int depth;
	int max_depth;
};


/***************
 *  MACROS	  
*/

#define STR(E) #E

#define ERR_EXPECTED_TERMINAL(TERMINAL) {\
	printf(__FILE__":%d\n", __LINE__);\
	errorExpectedToken(TERMINAL, tokenizerGetCurrent(p_parser->tokenizer));\
	_end_parsing(p_parser);\
	return -1;\
}

#define ERR_EXPECTED_NON_TERMINAL(NON_TERMINAL) {\
	printf(__FILE__":%d\n", __LINE__);\
	errorExpected(NON_TERMINAL, tokenizerGetCurrent(p_parser->tokenizer));\
	_end_parsing(p_parser);\
	return -1;\
}

#define ERR_UNREACHABLE() {\
	puts("\x1b[1;91mInternal Error:\x1b[1;97m unreachable.\x1b[0m");\
	_end_parsing(p_parser);\
	return -1;\
}

/*
 *  END MACROS
***************/


void _free_ctx(ParseCtx *p_ctx) {
	free(p_ctx);
}


ParseCtx *_create_ctx(PROC_TYPE p_type) {
	ParseCtx *ctx = (ParseCtx*)malloc(sizeof(ParseCtx));
	ctx->type = p_type;
	ctx->state = 0;
	ctx->prev_ctx = NULL;
	return ctx;
}


ParseCtx *_stack_push(Parser* p_parser, ParseCtx* p_ctx) {
	p_parser->depth++;
	if (p_parser->depth > p_parser->max_depth)
		p_parser->max_depth = p_parser->depth;
	p_ctx->prev_ctx = p_parser->stack_top;
	p_parser->stack_top = p_ctx;
	return p_ctx;
}


ParseCtx *_stack_pop(Parser* p_parser) {
	p_parser->depth--;
	_free_ctx(p_parser->stack_popped);
	p_parser->stack_popped = p_parser->stack_top;
	if (p_parser->stack_popped)
		p_parser->stack_top = p_parser->stack_top->prev_ctx;
	return p_parser->stack_popped;
}


void _stack_pop_then_free(Parser* p_parser) {
	_free_ctx(_stack_pop(p_parser));
	p_parser->stack_popped = NULL;
}


void _end_parsing(Parser *p_parser) {
	// FIXME: Free the syntax tree
}


Parser* parserInit(Tokenizer *p_tokenizer) {
	Parser *p = (Parser*)malloc(sizeof(Parser));
	p->tokenizer = p_tokenizer;
	p->stack_top = NULL;
	p->depth = 0;
	p->max_depth = 0;
	return p;
}


int parserParse(Parser *p_parser) {
	for (ParseCtx *ctx = _stack_push(p_parser, _create_ctx(PROC_UNIT)); ctx; ctx = p_parser->stack_top) {
		switch (ctx->type) {
			case PROC_UNIT:
				switch (ctx->state) {
					case 0:
						ctx->state++;
						unit_get_declaration:
						_stack_push(p_parser, _create_ctx(PROC_DECLARATION));
						continue;
					case 1:
						if (p_parser->stack_popped) {
							goto unit_get_declaration;
						}
						break;
				}
				if (tokenizerGetCurrentType(p_parser->tokenizer) != TK_EOF)
					ERR_EXPECTED_NON_TERMINAL("DECLARATION")
				_stack_pop(p_parser);
				continue;
			case PROC_DECLARATION:
				proc_declaration:
				switch (ctx->state) {
					case 0:
						_stack_push(p_parser, _create_ctx(PROC_LET_DECLARATION));
						ctx->state++;
						continue;
					case 2:
						_stack_push(p_parser, _create_ctx(PROC_METHOD_DECLARATION));
						ctx->state++;
						continue;
					case 4:
						break;
					default:
						if (p_parser->stack_popped) {
							_stack_pop(p_parser);
							continue;
						}
						ctx->state++;
						goto proc_declaration;
				}
				_stack_pop_then_free(p_parser);
				continue;
			case PROC_SPACE_DECLARATION:
				switch (ctx->state) {
					case 0:
						if (tokenizerGetCurrentType(p_parser->tokenizer) != TK_SPACE) {
							_stack_pop_then_free(p_parser);
							continue;
						}
						// if (tokenizerAdvanceType(p_parser->tokenizer) != TK_IDENTIFIER)
						// 	ERR_EXPECTED_TERMINAL(TK_IDENTIFIER);
						if (tokenizerAdvanceType(p_parser->tokenizer) != TK_BRACE_OPEN)
							ERR_EXPECTED_TERMINAL(TK_BRACE_OPEN)
						tokenizerAdvance(p_parser->tokenizer);
						ctx->state = 1;
					space_get_declaration:
						_stack_push(p_parser, _create_ctx(PROC_DECLARATION));
						continue;
					default:
						if (p_parser->stack_popped) {
							goto space_get_declaration;
						}
				}
				if (tokenizerGetCurrentType(p_parser->tokenizer) != TK_BRACE_CLOSE)
					ERR_EXPECTED_TERMINAL(TK_BRACE_CLOSE);
				tokenizerAdvance(p_parser->tokenizer);
				_stack_pop(p_parser);
				continue;
			case PROC_ENUM_DECLARATION:
				if (tokenizerGetCurrentType(p_parser->tokenizer) != TK_ENUM) {
					_stack_pop_then_free(p_parser);
					continue;
				}
				// if (tokenizerAdvanceType(p_parser->tokenizer) != TK_IDENTIFIER)
				//	ERR_EXPECTED_TERMINAL(TK_IDENTIFIER)
				if (tokenizerAdvanceType(p_parser->tokenizer) != TK_BRACE_OPEN)
					ERR_EXPECTED_TERMINAL(TK_BRACE_OPEN)
				while (tokenizerAdvanceType(p_parser->tokenizer) == TK_IDENTIFIER) {
					if (tokenizerAdvanceType(p_parser->tokenizer) == TK_PARENTHESIS_OPEN) {
						if (tokenizerAdvanceType(p_parser->tokenizer) == TK_IDENTIFIER)
						if (tokenizerAdvanceType(p_parser->tokenizer) == TK_PARENTHESIS_CLOSE)
							tokenizerAdvance(p_parser->tokenizer);
						else ERR_EXPECTED_TERMINAL(TK_PARENTHESIS_CLOSE)
						else ERR_EXPECTED_TERMINAL(TK_IDENTIFIER)
					}
					if (tokenizerGetCurrentType(p_parser->tokenizer) == TK_COMMA)
						continue;
					break;
				}
				if (tokenizerGetCurrentType(p_parser->tokenizer) != TK_BRACE_CLOSE)
					ERR_EXPECTED_TERMINAL(TK_BRACE_CLOSE)
				tokenizerAdvance(p_parser->tokenizer);
				_stack_pop(p_parser);
				continue;
			case PROC_TYPE_DECLARATION:
				switch (ctx->state) {
					case 0:
						if (tokenizerGetCurrentType(p_parser->tokenizer) != TK_TYPE) {
							_stack_pop_then_free(p_parser);
							continue;
						}
						// if (tokenizerAdvanceType(p_parser->tokenizer) != TK_IDENTIFIER)
						//	ERR_EXPECTED_TERMINAL(TK_IDENTIFIER);
						if (tokenizerAdvanceType(p_parser->tokenizer) != TK_BRACE_OPEN)
							ERR_EXPECTED_TERMINAL(TK_BRACE_OPEN)
						tokenizerAdvance(p_parser->tokenizer);
						ctx->state = 1;
					type_get_declaration:
						_stack_push(p_parser, _create_ctx(PROC_LET_DECLARATION));
						continue;
					default:
						if (p_parser->stack_popped) {
							goto type_get_declaration;
						}
				}
				if (tokenizerGetCurrentType(p_parser->tokenizer) != TK_BRACE_CLOSE)
					ERR_EXPECTED_TERMINAL(TK_BRACE_CLOSE);
				tokenizerAdvance(p_parser->tokenizer);
				_stack_pop(p_parser);
				continue;
			case PROC_LET_DECLARATION:
				switch (ctx->state) {
					case 0:
						if (tokenizerGetCurrentType(p_parser->tokenizer) != TK_LET) {
							_stack_pop_then_free(p_parser);
							continue;
						}
						if (tokenizerAdvanceType(p_parser->tokenizer) != TK_IDENTIFIER)
							ERR_EXPECTED_TERMINAL(TK_IDENTIFIER)
						if (tokenizerAdvanceType(p_parser->tokenizer) == TK_COLON) {
							if (tokenizerAdvanceType(p_parser->tokenizer) == TK_CONST ||
								tokenizerGetCurrentType(p_parser->tokenizer) == TK_STATIC) {
								if (tokenizerAdvanceType(p_parser->tokenizer) != TK_IDENTIFIER)
									ERR_EXPECTED_TERMINAL(TK_IDENTIFIER)
							}
							else if (tokenizerGetCurrentType(p_parser->tokenizer) != TK_IDENTIFIER)
								ERR_EXPECTED_TERMINAL(TK_IDENTIFIER)
							tokenizerAdvance(p_parser->tokenizer);
						}
						if (tokenizerGetCurrentType(p_parser->tokenizer) != TK_EQUAL) {
							break;
						}
						ctx->state = 1;
						if (tokenizerAdvanceType(p_parser->tokenizer) == TK_SPACE)
							_stack_push(p_parser, _create_ctx(PROC_SPACE_DECLARATION));
						else if (tokenizerGetCurrentType(p_parser->tokenizer) == TK_ENUM)
							_stack_push(p_parser, _create_ctx(PROC_ENUM_DECLARATION));
						else if (tokenizerGetCurrentType(p_parser->tokenizer) == TK_TYPE) {
							_stack_push(p_parser, _create_ctx(PROC_TYPE_DECLARATION));
						} else _stack_push(p_parser, _create_ctx(PROC_EXPRESSION));
						continue;
					default:
						if (p_parser->stack_popped) {
							break;
						}
						ERR_EXPECTED_NON_TERMINAL("EXPRESSION")
				}
				_stack_pop(p_parser);
				continue;
			case PROC_METHOD_DECLARATION:
				switch (ctx->state) {
					case 0:
						if (tokenizerGetCurrentType(p_parser->tokenizer) != TK_FN) {
							_stack_pop_then_free(p_parser);
							continue;
						}
						if (tokenizerAdvanceType(p_parser->tokenizer) != TK_IDENTIFIER)
							ERR_EXPECTED_TERMINAL(TK_IDENTIFIER)
						if (tokenizerAdvanceType(p_parser->tokenizer) != TK_PARENTHESIS_OPEN)
							ERR_EXPECTED_TERMINAL(TK_PARENTHESIS_OPEN);
						if (tokenizerAdvanceType(p_parser->tokenizer) != TK_PARENTHESIS_CLOSE)
							ERR_EXPECTED_TERMINAL(TK_PARENTHESIS_CLOSE)
						if (tokenizerAdvanceType(p_parser->tokenizer) == TK_IDENTIFIER) {
							// FIXME: implement fntion parameters
							tokenizerAdvance(p_parser->tokenizer);
						}
						ctx->state = 1;
						_stack_push(p_parser, _create_ctx(PROC_SCOPE));
						continue;
					case 1:
						if (!p_parser->stack_popped)
							ERR_EXPECTED_NON_TERMINAL("SCOPE")
				}
				_stack_pop(p_parser);
				continue;
			case PROC_SCOPE:
				switch (ctx->state) {
					case 0:
						if (tokenizerGetCurrentType(p_parser->tokenizer) != TK_BRACE_OPEN) {
							_stack_pop_then_free(p_parser);
							continue;
						}
						tokenizerAdvance(p_parser->tokenizer);
					scope_get_declaration:
						ctx->state = 1;
						_stack_push(p_parser, _create_ctx(PROC_DECLARATION));
						continue;
					case 2:
						_stack_push(p_parser, _create_ctx(PROC_STATEMENT));
						ctx->state++;
						continue;
					case 4:
						break;
					default:
						if (p_parser->stack_popped) {
							goto scope_get_declaration;
						}
						ctx->state++;
						continue;
				}
				if (tokenizerGetCurrentType(p_parser->tokenizer) != TK_BRACE_CLOSE)
					ERR_EXPECTED_TERMINAL(TK_BRACE_CLOSE)
				tokenizerAdvance(p_parser->tokenizer);
				_stack_pop(p_parser);
				continue;
			case PROC_STATEMENT:
				switch (ctx->state) {
					case 0:
						switch (tokenizerGetCurrentType(p_parser->tokenizer)) {
							case TK_IDENTIFIER:
								if (tokenizerAdvanceType(p_parser->tokenizer) != TK_EQUAL)
									ERR_EXPECTED_TERMINAL(TK_EQUAL)
								tokenizerAdvance(p_parser->tokenizer);
								_stack_push(p_parser, _create_ctx(PROC_EXPRESSION));
								ctx->state = 1;
								continue;
							case TK_IF:
								tokenizerAdvance(p_parser->tokenizer);
								_stack_push(p_parser, _create_ctx(PROC_EXPRESSION));
								ctx->state = 2;
								continue;
							case TK_MATCH:
								tokenizerAdvance(p_parser->tokenizer);
								_stack_push(p_parser, _create_ctx(PROC_EXPRESSION));
								ctx->state = 4;
								continue;
							case TK_WHILE:
								// FIXME: implement while statement
							case TK_FOR:
								// FIXME: implement for statement
							case TK_RET:
								// FIXME: implement return statement
							case TK_BREAK:
								// FIXME: implement break statement
							case TK_CONTINUE:
								// FIXME: implement continue statement
							default:
						}
						break;
					case 1:
						if (p_parser->stack_popped) {
							_stack_pop(p_parser);
							continue;
						}
						ERR_EXPECTED_NON_TERMINAL("EXPRESSION")
					case 2:
						if (p_parser->stack_popped) {
							_stack_push(p_parser, _create_ctx(PROC_SCOPE));
							ctx->state = 3;
							continue;
						}
						ERR_EXPECTED_NON_TERMINAL("EXPRESSION")
					case 3:
						if (p_parser->stack_popped) {
							_stack_pop(p_parser);
							continue;
						}
						ERR_EXPECTED_NON_TERMINAL("SCOPE")
					case 4:
						if (!p_parser->stack_popped)
							ERR_EXPECTED_NON_TERMINAL("EXPRESSION")
						if (tokenizerGetCurrentType(p_parser->tokenizer) != TK_BRACE_OPEN)
							ERR_EXPECTED_TERMINAL(TK_BRACE_OPEN)
						tokenizerAdvance(p_parser->tokenizer);
						if (tokenizerGetCurrentType(p_parser->tokenizer) == TK_BRACE_CLOSE) {
							tokenizerAdvance(p_parser->tokenizer);
							_stack_pop(p_parser);
							continue;
						}
					match_get_case:
						if (tokenizerGetCurrentType(p_parser->tokenizer) == TK_UNDERSCORE) {
							if (tokenizerAdvanceType(p_parser->tokenizer) != TK_FORWARD_ARROW)
								ERR_EXPECTED_TERMINAL(TK_FORWARD_ARROW)
							tokenizerAdvance(p_parser->tokenizer);
							_stack_push(p_parser, _create_ctx(PROC_SCOPE));
							ctx->state = 8;
							continue;
						}
						_stack_push(p_parser, _create_ctx(PROC_EXPRESSION));
						ctx->state = 5;
						continue;
					case 5:
						if (!p_parser->stack_popped)
							goto match_end;
						__attribute__((fallthrough));
					case 6:
						if (!p_parser->stack_popped)
							ERR_EXPECTED_NON_TERMINAL("EXPRESSION")
						if (tokenizerGetCurrentType(p_parser->tokenizer) == TK_COMMA) {
							tokenizerAdvance(p_parser->tokenizer);
							_stack_push(p_parser, _create_ctx(PROC_EXPRESSION));
							ctx->state = 6;
							continue;
						}
						if (tokenizerGetCurrentType(p_parser->tokenizer) != TK_FORWARD_ARROW)
							ERR_EXPECTED_TERMINAL(TK_FORWARD_ARROW)
						tokenizerAdvance(p_parser->tokenizer);
						_stack_push(p_parser, _create_ctx(PROC_SCOPE));
						ctx->state = 7;
						continue;
					case 7:
						if (!p_parser->stack_popped)
							ERR_EXPECTED_NON_TERMINAL("SCOPE")
						goto match_get_case;
					case 8:
						if (!p_parser->stack_popped)
							ERR_EXPECTED_NON_TERMINAL("SCOPE")
						__attribute__((fallthrough));
					case 9:
					match_end:
						if (tokenizerGetCurrentType(p_parser->tokenizer) != TK_BRACE_CLOSE)
							ERR_EXPECTED_TERMINAL(TK_BRACE_CLOSE)
						tokenizerAdvance(p_parser->tokenizer);
						_stack_pop(p_parser);
						continue;
				}
				_stack_pop_then_free(p_parser);
				continue;
			case PROC_EXPRESSION:
				// TODO: Use a regular Pratt parsing algorithem currently this is enough
				switch (ctx->state) {
					case 0:
						ctx->state++;
						expression_get_binary:
						_stack_push(p_parser, _create_ctx(PROC_EXP_BINARY));
						continue;
					case 1:
						if (p_parser->stack_popped) {
							switch (tokenizerGetCurrentType(p_parser->tokenizer)) {
								case TK_PLUS:
								case TK_MINUS:
									tokenizerAdvance(p_parser->tokenizer);
									goto expression_get_binary;
								default:
									break;
							}
							break;
						}
						_stack_pop_then_free(p_parser);
						continue;
					case 2:
						if (p_parser->stack_popped) {
							break;
						}
						ERR_EXPECTED_NON_TERMINAL("EXPRESSION")
				}
				_stack_pop(p_parser);
				continue;
			case PROC_EXP_BINARY:
				switch (ctx->state) {
					case 0:
						if (tokenizerGetCurrentType(p_parser->tokenizer) == TK_MINUS
						|| tokenizerGetCurrentType(p_parser->tokenizer) == TK_BANG) {
							tokenizerAdvance(p_parser->tokenizer);
						}
						_stack_push(p_parser, _create_ctx(PROC_EXP_VALUE));
						ctx->state++;
						continue;
					case 1:
						if (p_parser->stack_popped) {
							switch (tokenizerGetCurrentType(p_parser->tokenizer)) {
								case TK_STAR:
								case TK_SLASH:
									tokenizerAdvance(p_parser->tokenizer);
									_stack_push(p_parser, _create_ctx(PROC_EXP_BINARY));
									ctx->state++;
									continue;
								default:
									break;
							}
							break;
						}
						_stack_pop_then_free(p_parser);
						continue;
					case 2:
						if (p_parser->stack_popped) {
							break;
						}
						ERR_EXPECTED_NON_TERMINAL("EXPRESSION")
				}
				_stack_pop(p_parser);
				continue;
			case PROC_EXP_VALUE:
				switch (ctx->state) {
					case 0:
						if (tokenizerGetCurrentType(p_parser->tokenizer) == TK_LITERAL) {
							tokenizerAdvance(p_parser->tokenizer);
							break;
						}
						if (tokenizerGetCurrentType(p_parser->tokenizer) == TK_IDENTIFIER) {
							tokenizerAdvance(p_parser->tokenizer);
							break;
						}
						if (tokenizerGetCurrentType(p_parser->tokenizer) != TK_PARENTHESIS_OPEN) {
							_stack_pop_then_free(p_parser);
							continue;
						}
						tokenizerAdvance(p_parser->tokenizer);
						_stack_push(p_parser, _create_ctx(PROC_EXPRESSION));
						ctx->state++;
						continue;
					case 1:
						if (p_parser->stack_popped) {
							if (tokenizerGetCurrentType(p_parser->tokenizer) != TK_PARENTHESIS_CLOSE)
								ERR_EXPECTED_TERMINAL(TK_PARENTHESIS_CLOSE)
							tokenizerAdvance(p_parser->tokenizer);
							break;
						}
						ERR_EXPECTED_NON_TERMINAL("EXPRESSION")
				}
				_stack_pop(p_parser);
				continue;
			default:
				ERR_UNREACHABLE()
		}
	}
	return 0;
}


void parserTerminate(Parser *p_parser) {
	free(p_parser);
}



