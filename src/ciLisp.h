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
    READ_OPER, // 0
    RAND_OPER,
    //end of nonary

    NEG_OPER,
    ABS_OPER,
    EXP_OPER,
    SQRT_OPER,
    LOG_OPER,
    EXP2_OPER,
    CBRT_OPER,
    //end of unary

    REMAINDER_OPER,
    POW_OPER,
    MAX_OPER,
    MIN_OPER,
    HYPOT_OPER,
    EQUAL_OPER,
    LESS_OPER,
    GREATER_OPER,
    //end of binary

    ADD_OPER,
    SUB_OPER,
    MULT_OPER,
    DIV_OPER,
    PRINT_OPER,

    CUSTOM_OPER =255
} OPER_TYPE;

OPER_TYPE resolveFunc(char *);

// Types of Abstract Syntax Tree nodes.
// Initially, there are only numbers and functions.
// You will expand this enum as you build the project.
typedef enum {
    NUM_NODE_TYPE,
    FUNC_NODE_TYPE,
    SYMBOL_NODE_TYPE,
    COND_NODE_TYPE
} AST_NODE_TYPE;

// Types of numeric values
typedef enum {
    INT_TYPE = 0,
    DOUBLE_TYPE =1,
    NO_TYPE
} NUM_TYPE;

NUM_TYPE resolveType(char*);

// Node to store a number.
typedef struct {
    NUM_TYPE type;
    double value;
} NUM_AST_NODE;

typedef struct symbol_ast_node {
    char *ident;
} SYMBOL_AST_NODE;

typedef struct {
    struct ast_node *cond;
    struct ast_node *ifTrue;
    struct ast_node *ifFalse;
} COND_AST_NODE;

// Values returned by eval function will be numbers with a type.
// They have the same structure as a NUM_AST_NODE.
// The line below allows us to give this struct another name for readability.
typedef NUM_AST_NODE RET_VAL;

// Node to store a function call with its inputs
typedef struct {
    OPER_TYPE oper;
    char* ident; // only needed for custom functions
    struct ast_node *opList;
} FUNC_AST_NODE;

typedef enum {
    SYMBOL_TABLE_NODE_TYPE,
    FUNC_TABLE_NODE_TYPE
} TABLE_NODE_TYPE;

//typedef struct arg_node {
//    char *ident;
//    struct arg_node *next;
//} ARG_NODE;

typedef struct {
//    struct table_node *argList;
    struct ast_node *customOper;

} FUNC_TABLE_NODE;

typedef struct{
    struct ast_node *val;
} SYMBOL_TABLE_NODE;

typedef struct table_node {
    TABLE_NODE_TYPE nodeType;
    char *ident;
    NUM_TYPE type;

    union {
        SYMBOL_TABLE_NODE symbol;
        FUNC_TABLE_NODE function;
    } data;

    struct table_node *next;
} TABLE_NODE;

// Generic Abstract Syntax Tree node. Stores the type of node,
// and reference to the corresponding specific node (initially a number or function call).
typedef struct ast_node {
    AST_NODE_TYPE type;
    TABLE_NODE *symbolTable;
    struct ast_node *parent;
    union {
        NUM_AST_NODE number;
        FUNC_AST_NODE function;
        COND_AST_NODE condition;
        SYMBOL_AST_NODE symbol;
    } data;
    struct ast_node *next;
} AST_NODE;

AST_NODE *createNumberNode(double value, NUM_TYPE type);
AST_NODE *createSymbolNode(char *ident);
AST_NODE *createFunctionNode(char *funcName, AST_NODE *opList);
AST_NODE *createCondNode(AST_NODE *condition, AST_NODE *ifTrue, AST_NODE *ifFalse);
bool checkParamList(char *funcName, int numOps, AST_NODE *opList);
AST_NODE *addAstNode(AST_NODE *parent, AST_NODE *child);

AST_NODE *addSymbolTable(TABLE_NODE *symbolTable, AST_NODE *node);
TABLE_NODE *createSymbolTableNode(char *ident, AST_NODE *valueNode, NUM_TYPE type);
TABLE_NODE *createArgNode(char *ident, TABLE_NODE *next);
TABLE_NODE *createFuncTableNode(char *ident, AST_NODE *customOper, NUM_TYPE type, TABLE_NODE *argList);
TABLE_NODE *addToTable(TABLE_NODE *headNode, TABLE_NODE *newNode);

void freeNode(AST_NODE *node);

RET_VAL eval(AST_NODE *node);
RET_VAL evalNumNode(AST_NODE *node);
RET_VAL evalFuncNode(AST_NODE *node);
RET_VAL evalCustomFunc(AST_NODE *symboNode, AST_NODE *opList);
RET_VAL evalCondNode(AST_NODE *node);

TABLE_NODE *createArgOpList(TABLE_NODE *args, AST_NODE *opList);

RET_VAL myRead();
RET_VAL myRand();
RET_VAL addOper(AST_NODE *op);
RET_VAL subOper(AST_NODE *op);
RET_VAL multOper(AST_NODE *op);
RET_VAL divOper(AST_NODE *op);
RET_VAL print(AST_NODE *node);
RET_VAL evalSymbolNode(AST_NODE *node);
TABLE_NODE *getSymbolTableNode(AST_NODE *symbolNode);
void freeSymbolTable(TABLE_NODE *symbolTable);



OPER_TYPE getOperType(char *funcName);

void printRetVal(RET_VAL val);

#endif
