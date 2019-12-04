#ifndef __cilisp_h_
#define __cilisp_h_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "ciLispParser.h"

int yyparse(void);

int yylex(void);

void yyerror(char *);

// Enum of all operators.
// must be in sync with funcs in resolveFunc()
typedef enum oper {
    NEG_OPER, // 0
    ABS_OPER,
    EXP_OPER,
    SQRT_OPER,
    LOG_OPER,
    EXP2_OPER,
    CBRT_OPER,
//              single op < ADD_OPER <= double op
    ADD_OPER,
    SUB_OPER,
    MULT_OPER,
    DIV_OPER,
    REMAINDER_OPER,
    POW_OPER,
    MAX_OPER,
    MIN_OPER,
    HYPOT_OPER,
    READ_OPER,
    RAND_OPER,
    PRINT_OPER,
    EQUAL_OPER,
    LESS_OPER,
    GREATER_OPER,
    CUSTOM_OPER =255
} OPER_TYPE;

OPER_TYPE resolveFunc(char *);

// Types of Abstract Syntax Tree nodes.
// Initially, there are only numbers and functions.
// You will expand this enum as you build the project.
typedef enum {
    NUM_NODE_TYPE,
    FUNC_NODE_TYPE,
    SYMBOL_NODE_TYPE
} AST_NODE_TYPE;

// Types of numeric values
typedef enum {
    INT_TYPE = 0,
    DOUBLE_TYPE
} NUM_TYPE;

// Node to store a number.
typedef struct {
    NUM_TYPE type;
    double value;
} NUM_AST_NODE;

typedef struct symbol_ast_node {
    char *ident;
} SYMBOL_AST_NODE;

// Values returned by eval function will be numbers with a type.
// They have the same structure as a NUM_AST_NODE.
// The line below allows us to give this struct another name for readability.
typedef NUM_AST_NODE RET_VAL;

// Node to store a function call with its inputs
typedef struct {
    OPER_TYPE oper;
    char* ident; // only needed for custom functions
    struct ast_node *op1;
    struct ast_node *op2;
} FUNC_AST_NODE;

typedef struct symbol_table_node {
    char *ident;
    struct ast_node *val;
    struct symbol_table_node *next;
} SYMBOL_TABLE_NODE;

// Generic Abstract Syntax Tree node. Stores the type of node,
// and reference to the corresponding specific node (initially a number or function call).
typedef struct ast_node {
    AST_NODE_TYPE type;
    SYMBOL_TABLE_NODE *symbolTable;
    struct ast_node *parent;
    union {
        NUM_AST_NODE number;
        FUNC_AST_NODE function;
        SYMBOL_AST_NODE symbol;
    } data;
} AST_NODE;

AST_NODE *createNumberNode(double value, NUM_TYPE type);
AST_NODE *createSymbolNode(char *ident);
AST_NODE *createFunctionNode(char *funcName, AST_NODE *op1, AST_NODE *op2);

AST_NODE *addSymbolTable(SYMBOL_TABLE_NODE *symbolTable, AST_NODE *node);
SYMBOL_TABLE_NODE *createSymbolTableNode(char *ident, AST_NODE *valueNode);
SYMBOL_TABLE_NODE *addToSymbolTable(SYMBOL_TABLE_NODE *headNode, SYMBOL_TABLE_NODE *newNode);

void freeNode(AST_NODE *node);

RET_VAL eval(AST_NODE *node);
RET_VAL evalNumNode(AST_NODE *node);
RET_VAL evalFuncNode(AST_NODE *node);
RET_VAL evalSymbolNode(AST_NODE *node);


OPER_TYPE getOperType(char *funcName);

void printRetVal(RET_VAL val);

#endif
