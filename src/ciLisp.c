#include "ciLisp.h"

void yyerror(char *s) {
    fprintf(stderr, "\nERROR: %s\n", s);
    // note stderr that normally defaults to stdout, but can be redirected: ./src 2> src.log
    // CLion will display stderr in a different color from stdin and stdout
}

// Array of string values for operations.
// Must be in sync with funcs in the OPER_TYPE enum in order for resolveFunc to work.
char *funcNames[] = {
        "neg",
        "abs",
        "exp",
        "sqrt",
        "log",
        "exp2",
        "cbrt",

        "add",
        "sub",
        "mult",
        "div",
        "remainder",
        "pow",
        "max",
        "min",
        "hypot",
        "read",
        "rand",
        "print",
        "equal",
        "less",
        "greater",
        ""
};

OPER_TYPE resolveFunc(char *funcName)
{
    int i = 0;
    while (funcNames[i][0] != '\0')
    {
        if (strcmp(funcNames[i], funcName) == 0)
            return i;
        i++;
    }
    return CUSTOM_OPER;
}

// Called when an INT or DOUBLE token is encountered (see ciLisp.l and ciLisp.y).
// Creates an AST_NODE for the number.
// Sets the AST_NODE's type to number.
// Populates the value of the contained NUMBER_AST_NODE with the argument value.
// SEE: AST_NODE, NUM_AST_NODE, AST_NODE_TYPE.
AST_NODE *createNumberNode(double value, NUM_TYPE type)
{
    AST_NODE *node;
    size_t nodeSize;

    // allocate space for the fixed sie and the variable part (union)
    nodeSize = sizeof(AST_NODE);
    if ((node = calloc(nodeSize, 1)) == NULL)
        yyerror("Memory allocation failed!");

    node->type = NUM_NODE_TYPE;
    node->data.number.value = value;
    node->data.number.type = type;

    return node;
}

// Called when an f_expr is created (see ciLisp.y).
// Creates an AST_NODE for a function call.
// Sets the created AST_NODE's type to function.
// Populates the contained FUNC_AST_NODE with:
//      - An OPER_TYPE (the enum identifying the specific function being called)
//      - 2 AST_NODEs, the operands
// SEE: AST_NODE, FUNC_AST_NODE, AST_NODE_TYPE.
AST_NODE *createFunctionNode(char *funcName, AST_NODE *op1, AST_NODE *op2)
{
    AST_NODE *node;
    size_t nodeSize;

    // allocate space (or error)
    nodeSize = sizeof(AST_NODE);
    if ((node = calloc(nodeSize, 1)) == NULL)
        yyerror("Memory allocation failed!");

    // NOTE: you do not need to populate the "ident" field unless the function is type CUSTOM_OPER.
    // When you do have a CUSTOM_OPER, you do NOT need to allocate and strcpy here.
    // The funcName will be a string identifier for which space should be allocated in the tokenizer.
    // For CUSTOM_OPER functions, you should simply assign the "ident" pointer to the passed in funcName.
    // For functions other than CUSTOM_OPER, you should free the funcName after you're assigned the OPER_TYPE.

    node->type = FUNC_NODE_TYPE;

    node->data.function.oper = resolveFunc(funcName);

    if (node->data.function.oper == CUSTOM_OPER)
        node->data.function.ident = funcName;


    node->data.function.op1 = op1;
    node->data.function.op2 = op2;


    return node;
}

AST_NODE *createSymbolNode(char *ident){
    AST_NODE *node;
    size_t nodeSize = sizeof(AST_NODE);
    if ((node = calloc(nodeSize, 1)) == NULL)
        yyerror("Memory allocation failed!");


}

AST_NODE *createSymbolTable(SYMBOL_TABLE_NODE *symbol, AST_NODE *node){
    node->symbolTable = symbol;
}

SYMBOL_TABLE_NODE *createSymbolTableNode(char *ident, AST_NODE *valueNode){
    //TODO createSymbolNode
    SYMBOL_TABLE_NODE *node;
    size_t nodeSize = sizeof(SYMBOL_TABLE_NODE);
    if ((node = calloc(nodeSize, 1)) == NULL)
        yyerror("Memory allocation failed!");

    node->ident = ident;
    node->val = valueNode;
    node->next = NULL;

}

SYMBOL_TABLE_NODE *addToSymbolTable(SYMBOL_TABLE_NODE *parentNode, SYMBOL_TABLE_NODE *newNode){

    if(parentNode->ident == newNode->ident){
        yyerror("Conflicting Symbol Definition");
    }

    while(parentNode->next != NULL){
        parentNode = parentNode->next;
        if(parentNode->ident == newNode->ident){
            yyerror("Conflicting Symbol Definition");
        }
    }
    parentNode->next = newNode;

//    if(parentNode->next == NULL){
//        parentNode->next = &newNode;
//    }else{
//        addToSymbolTable(parentNode->next, newNode);
//    }
//    return parentNode;
}


// Called after execution is done on the base of the tree.
// (see the program production in ciLisp.y)
// Recursively frees the whole abstract syntax tree.
// You'll need to update and expand freeNode as the project develops.
void freeNode(AST_NODE *node)
{
    if (!node)
        return;

    if (node->type == FUNC_NODE_TYPE)
    {
        // Recursive calls to free child nodes
        freeNode(node->data.function.op1);
        freeNode(node->data.function.op2);

        // Free up identifier string if necessary
        if (node->data.function.oper == CUSTOM_OPER)
        {
            free(node->data.function.ident);
        }
    }

    free(node);
}

// Evaluates an AST_NODE.
// returns a RET_VAL storing the the resulting value and type.
// You'll need to update and expand eval (and the more specific eval functions below)
// as the project develops.
RET_VAL eval(AST_NODE *node)
{
    if (!node)
        return (RET_VAL){INT_TYPE, NAN};

    RET_VAL result = {INT_TYPE, NAN}; // see NUM_AST_NODE, because RET_VAL is just an alternative name for it.

    // Make calls to other eval functions based on node type.
    // Use the results of those calls to populate result.
    switch (node->type)
    {
        case NUM_NODE_TYPE:
            result = evalNumNode(node);
            break;
        case FUNC_NODE_TYPE:
            result = evalFuncNode(node);
            break;
        case SYMBOL_TABLE_NODE_TYPE:
            result = evalSymbolTableNode(node);
        default:
            yyerror("Invalid AST_NODE_TYPE, probably invalid writes somewhere!");
    }

    return result;
}  

// returns a pointer to the NUM_AST_NODE (aka RET_VAL) referenced by node.
// DOES NOT allocate space for a new RET_VAL.
RET_VAL evalNumNode(AST_NODE *node)
{
    if (!node)
        return (RET_VAL){INT_TYPE, NAN};

    RET_VAL result = {INT_TYPE, NAN};

    // SEE: AST_NODE, AST_NODE_TYPE, NUM_AST_NODE

    result.value = node->data.number.value;
    result.type = node->data.number.type;

    return result;
}


RET_VAL evalFuncNode(AST_NODE *node)
{
    if (!node)
        return (RET_VAL){INT_TYPE, NAN};

    RET_VAL result = {INT_TYPE, NAN};

    double op1;
    double op2;


    if(node->data.function.op1->type == FUNC_NODE_TYPE){
        RET_VAL temp = evalFuncNode(node->data.function.op1);
        node->data.function.op1->data.number.value = temp.value;
        node->data.function.op1->data.number.type = temp.type;
    }

    op1 = node->data.function.op1->data.number.value;

    if(node->data.function.oper >= ADD_OPER) {
        // If binary func
        if(node->data.function.op2->type == FUNC_NODE_TYPE){
            RET_VAL temp = evalFuncNode(node->data.function.op2);
            node->data.function.op2->data.number.value = temp.value;
            node->data.function.op2->data.number.type = temp.type;
        }

        op2 = node->data.function.op2->data.number.value;
        result.type = node->data.function.op1->data.number.type || node->data.function.op2->data.number.type;
    }else {
        //if unary func
        result.type = node->data.function.op1->data.number.type;
    }


    // TODO     evalFuncNode:             populate result with the result of running the function on its operands.
    // SEE: AST_NODE, AST_NODE_TYPE, FUNC_AST_NODE

    switch (node->data.function.oper){
        case NEG_OPER:
            result.value = -1 * op1;
            break;
        case ABS_OPER:
            result.value = fabs(op1);
            break;
        case EXP_OPER:
            result.value = exp(op1);
            break;
        case SQRT_OPER:
            result.value = sqrt(op1);
            break;
        case LOG_OPER:
            result.value = log(op1);
            break;
        case EXP2_OPER:
            result.value = exp2(op1);
            break;
        case CBRT_OPER:
            result.value = cbrt(op1);
            break;

        case ADD_OPER:
            result.value = op1 + op2;
            break;
        case SUB_OPER:
            result.value = op1 - op2;
            break;
        case MULT_OPER:
            result.value = op1 * op2;
            break;
        case DIV_OPER:
            result.value = op1 / op2;
            break;
        case REMAINDER_OPER:
            result.value = remainder(op1, op2);
            break;
        case POW_OPER:
            result.value = pow(op1, op2);
            break;
        case MAX_OPER:
            result.value = fmax(op1, op2);
            break;
        case MIN_OPER:
            result.value = fmin(op1, op2);
            break;
        case HYPOT_OPER:
            result.value = hypot(op1, op2);
            break;
//        case READ_OPER:
//            result.value =
//            break;
//        case RAND_OPER:
//            result.value =
//            break;
//        case PRINT_OPER:
//            result.value =
//            break;
//        case EQUAL_OPER:
//            result.value =
//            break;
//        case LESS_OPER:
//            result.value =
//            break;
//        case GREATER_OPER:
//            result.value =
//            break;
//        case CUSTOM_OPER:
//            result.value =
//            break;

    }



    if(result.type == INT_TYPE)
        result.value = round(result.value);

    return result;
}

RET_VAL evalSymbolNode(AST_NODE *node){
    //TODO evalSymbolNode

    if(!node)
        return (RET_VAL){INT_TYPE, NAN};

    RET_VAL result = {INT_TYPE, NAN};

    char *temp = node->parent->symbolTable->ident;

    while(node->data.symbol.ident != temp){

    }
}

// prints the type and value of a RET_VAL
void printRetVal(RET_VAL val)
{
    switch (val.type){
        case INT_TYPE:
            printf("INT_TYPE: %ld", (long)floor(val.value) );
            break;
        case DOUBLE_TYPE:
            printf("DOUBLE_TYPE: %f", val.value);
            break;
        default:
            yyerror("Invalid Type Error in printRetVal");
            break;
    }
}
