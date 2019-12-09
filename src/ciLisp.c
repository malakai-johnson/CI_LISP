#include "ciLisp.h"
#include <stdio.h>

void yyerror(char *s) {
    fprintf(stderr, "\nERROR: %s\n", s);
    // note stderr that normally defaults to stdout, but can be redirected: ./src 2> src.log
    // CLion will display stderr in a different color from stdin and stdout
}

// Array of string values for operations.
// Must be in sync with funcs in the OPER_TYPE enum in order for resolveFunc to work.
char *funcNames[] = {
        "read",
        "rand",
        //nonary <= rand
        "neg",
        "abs",
        "exp",
        "sqrt",
        "log",
        "exp2",
        "cbrt",
        //unary <= cbrt
        "remainder",
        "pow",
        "max",
        "min",
        "hypot",
        "equal",
        "less",
        "greater",
        //binary <= greater
        "add",
        "sub",
        "mult",
        "div",
        "print",

        ""
};

OPER_TYPE resolveFunc(char *funcName) {
    int i = 0;
    while (funcNames[i][0] != '\0') {
        if (strcmp(funcNames[i], funcName) == 0)
            return i;
        i++;
    }
    return CUSTOM_OPER;
}

char *typeNames[] = {
        "int",
        "double",
        ""
};

NUM_TYPE resolveType(char *typeName) {
    int i = 0;
    while (typeNames[i][0] != '\0') {
        if (strcmp(typeNames[i], typeName) == 0)
            return i;
        ++i;
    }
    return NO_TYPE;
}


// Called when an INT or DOUBLE token is encountered (see ciLisp.l and ciLisp.y).
// Creates an AST_NODE for the number.
// Sets the AST_NODE's type to number.
// Populates the value of the contained NUMBER_AST_NODE with the argument value.
// SEE: AST_NODE, NUM_AST_NODE, AST_NODE_TYPE.
AST_NODE *createNumberNode(double value, NUM_TYPE type) {
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
AST_NODE *createFunctionNode(char *funcName, AST_NODE *opList) {
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
    node->symbolTable = NULL;
    node->data.function.oper = resolveFunc(funcName);


    if(node->data.function.oper <= RAND_OPER) {
        if (!checkParamList(funcName, 0, opList)) {
            return NULL;
        }
    }else if(node->data.function.oper <= CBRT_OPER){
        if(!checkParamList(funcName, 1, opList)){
            return NULL;
        }
    }else if (node->data.function.oper <= GREATER_OPER){
        if(!checkParamList(funcName, 2, opList)){
            return NULL;
        }    }

    if (node->data.function.oper == CUSTOM_OPER)
        node->data.function.ident = funcName;
    else
        free(funcName);

    AST_NODE *tempNode = opList;

    while(tempNode){
        tempNode->parent = node;
        tempNode = tempNode->next;
    }

    node->data.function.opList = opList;

    return node;
}

AST_NODE *createCondNode(AST_NODE *condition, AST_NODE *ifTrue, AST_NODE *ifFalse){
    AST_NODE *node;
    size_t nodeSize = sizeof(AST_NODE);
    if ((node = calloc(nodeSize, 1)) == NULL)
        yyerror("Memory allocation failed!");

    node->type = COND_NODE_TYPE;

    node->data.condition.cond = condition;
    node->data.condition.ifTrue = ifTrue;
    node->data.condition.ifFalse = ifFalse;

    return node;
}

//Checks the number of operands in opList matches numOps.
// Returns true if there are enough operands, otherwise throws an error and returns false.
//  If there are too many operands, returns true but prints an error.
bool checkParamList(char *funcName, int numOps, AST_NODE *opList){
        AST_NODE *temp = opList;
        for (int i = 0; i < numOps; ++i){
            if(!temp) {
                yyerror("ERROR: too few parameters for the function <name>\n");
                return false;
            }
            temp = temp->next;
        }
        if(temp)
            printf("WARNING: too many parameters for the function <%s>\n", funcName);
        return true;
}

AST_NODE *createSymbolNode(char *ident) {
    AST_NODE *node;
    size_t nodeSize = sizeof(AST_NODE);
    if ((node = calloc(nodeSize, 1)) == NULL)
        yyerror("Memory allocation failed!");

    node->type = SYMBOL_NODE_TYPE;
//    node->parent = NULL;
    node->data.symbol.ident = ident;

    return node;
}

AST_NODE *addSymbolTable(SYMBOL_TABLE_NODE *symbolTable, AST_NODE *node) {
    node->symbolTable = symbolTable;
//    node->symbolTable->val->parent = node;
    return node;
}

AST_NODE *addAstNode(AST_NODE *parent, AST_NODE *child){
    parent->next = child;
    return parent;
}

SYMBOL_TABLE_NODE *createSymbolTableNode(char *ident, AST_NODE *valueNode, NUM_TYPE type) {
    //TODO createSymbolNode
    SYMBOL_TABLE_NODE *node;
    size_t nodeSize = sizeof(SYMBOL_TABLE_NODE);
    if ((node = calloc(nodeSize, 1)) == NULL)
        yyerror("Memory allocation failed!");

    node->ident = ident;
    node->val = valueNode;
    node->type = type;
    node->next = NULL;

    return node;
}


SYMBOL_TABLE_NODE *addToSymbolTable(SYMBOL_TABLE_NODE *parentNode, SYMBOL_TABLE_NODE *newNode) {

    if (parentNode->ident == newNode->ident) {
        yyerror("Conflicting Symbol Definition");
    }

    while (parentNode->next != NULL) {
        parentNode = parentNode->next;
        if (parentNode->ident == newNode->ident) {
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
void freeNode(AST_NODE *node) {
    if (!node)
        return;

    if (node->type == FUNC_NODE_TYPE) {
        // Recursive calls to free child nodes
        freeNode(node->data.function.opList);

        // Free up identifier string if necessary
        if (node->data.function.oper == CUSTOM_OPER) {
            free(node->data.function.ident);
        }
    }

    if(node->type == SYMBOL_NODE_TYPE){
        free(node->data.symbol.ident);
    }

    free(node);
}

// Evaluates an AST_NODE.
// returns a RET_VAL storing the the resulting value and type.
// You'll need to update and expand eval (and the more specific eval functions below)
// as the project develops.
RET_VAL eval(AST_NODE *node) {
    if (!node)
        return (RET_VAL) {INT_TYPE, NAN};

    RET_VAL result = {INT_TYPE, NAN}; // see NUM_AST_NODE, because RET_VAL is just an alternative name for it.

    // Make calls to other eval functions based on node type.
    // Use the results of those calls to populate result.
    switch (node->type) {
        case NUM_NODE_TYPE:
            result = evalNumNode(node);
            break;
        case FUNC_NODE_TYPE:
            result = evalFuncNode(node);
            break;
        case COND_NODE_TYPE:
            result = evalCondNode(node);
            break;
        case SYMBOL_NODE_TYPE:
            result = evalSymbolNode(node);
            break;
        default:
            yyerror("Invalid AST_NODE_TYPE, probably invalid writes somewhere!");
    }

    return result;
}

// returns a pointer to the NUM_AST_NODE (aka RET_VAL) referenced by node.
// DOES NOT allocate space for a new RET_VAL.
RET_VAL evalNumNode(AST_NODE *node) {
    if (!node)
        return (RET_VAL) {INT_TYPE, NAN};

    RET_VAL result = {INT_TYPE, NAN};

    // SEE: AST_NODE, AST_NODE_TYPE, NUM_AST_NODE

    result.value = node->data.number.value;
    result.type = node->data.number.type;

    return result;
}


RET_VAL evalFuncNode(AST_NODE *node) {
    if (!node)
        return (RET_VAL) {INT_TYPE, NAN};

    RET_VAL result = {INT_TYPE, NAN};

//    RET_VAL op1 = {INT_TYPE, NAN};
//    RET_VAL op2 = {INT_TYPE, NAN};


//    if(node->data.function.op1->type == FUNC_NODE_TYPE){
//        RET_VAL temp = evalFuncNode(node->data.function.op1);
//        node->data.function.op1->data.number.value = temp.value;
//        node->data.function.op1->data.number.type = temp.type;
//    }
//
//    op1 = node->data.function.op1->data.number.value;

    AST_NODE *tempNode = node->data.function.opList;

    while(tempNode){
        if(tempNode->type != NO_TYPE)
            result.type = result.type || eval(tempNode).type;
        tempNode = tempNode->next;
    }

//
//    if (node->data.function.op1 != NULL) {
//        op1 = eval(node->data.function.op1);
//        result.type = op1.type;
//    }
//    if (node->data.function.op2 != NULL) {
//        op2 = eval(node->data.function.op2);
//        result.type = op1.type || op2.type;
//    }

//    if(node->data.function.oper >= ADD_OPER) {
//        // If binary func
////        if(node->data.function.op2->type == FUNC_NODE_TYPE){
////            RET_VAL temp = evalFuncNode(node->data.function.op2);
////            node->data.function.op2->data.number.value = temp.value;
////            node->data.function.op2->data.number.type = temp.type;
////        }
//
//        op2 = eval(node->data.function.op2);
//        result.type = node->data.function.op1->data.number.type || node->data.function.op2->data.number.type;
//    }else {
//        //if unary func
//        result.type = node->data.function.op1->data.number.type;
//    }


    // TODO     evalFuncNode:             populate result with the result of running the function on its operands.
    // SEE: AST_NODE, AST_NODE_TYPE, FUNC_AST_NODE

    tempNode = node->data.function.opList;

    switch (node->data.function.oper) {
        case READ_OPER:
            result = myRead();
            break;
        case RAND_OPER:
            result = myRand();
            break;

        case NEG_OPER:
            result.value = -1 * eval(tempNode).value;
            break;
        case ABS_OPER:
            result.value = fabs(eval(tempNode).value);
            break;
        case EXP_OPER:
            result.value = exp(eval(tempNode).value);
            break;
        case SQRT_OPER:
            result.value = sqrt(eval(tempNode).value);
            break;
        case LOG_OPER:
            result.value = log(eval(tempNode).value);
            break;
        case EXP2_OPER:
            result.value = exp2(eval(tempNode).value);
            break;
        case CBRT_OPER:
            result.value = cbrt(eval(tempNode).value);
            break;

        case REMAINDER_OPER:
            result.value = remainder(eval(tempNode).value, eval(tempNode->next).value);
            break;
        case POW_OPER:
            result.value = pow(eval(tempNode).value, eval(tempNode->next).value);
            break;
        case MAX_OPER:
            result.value = fmax(eval(tempNode).value, eval(tempNode->next).value);
            break;
        case MIN_OPER:
            result.value = fmin(eval(tempNode).value, eval(tempNode->next).value);
            break;
        case HYPOT_OPER:
            result.value = hypot(eval(tempNode).value, eval(tempNode->next).value);
            break;

        case EQUAL_OPER:
            result.value = eval(tempNode).value == eval(tempNode->next).value;
            break;
        case LESS_OPER:
            result.value = eval(tempNode).value < eval(tempNode->next).value;
            break;
        case GREATER_OPER:
            result.value = eval(tempNode).value > eval(tempNode->next).value;
            break;


        case PRINT_OPER:
            result = print(node->data.function.opList);
            printf("\n");
            break;
        case ADD_OPER:
            result = addOper(node->data.function.opList);
            break;
        case SUB_OPER:
            result = subOper(node->data.function.opList);
            break;
        case MULT_OPER:
            result = multOper(node->data.function.opList);
            break;
        case DIV_OPER:
            result = divOper(node->data.function.opList);
            break;

//        case CUSTOM_OPER:
//            result.value =
//            break;

    }


    if (result.type == INT_TYPE)
        result.value = floor(result.value);

    return result;
}

RET_VAL evalCondNode(AST_NODE *node){
    RET_VAL result;

    if(eval(node->data.condition.cond).value == 0){
        result = eval(node->data.condition.ifFalse);
    }else{
        result = eval(node->data.condition.ifTrue);
    }

    return result;
}

RET_VAL myRead(){
    RET_VAL result = (RET_VAL){DOUBLE_TYPE, NAN};

    printf("read := ");
    double temp;


    scanf("%f\n", &temp);

    result.value = temp;

    //Determines the type based on the value rather than the format
//    if(remainder(temp, 1) == 0 ){
//        result.type = INT_TYPE;
//    }else{
//        result.type = DOUBLE_TYPE;
//    }

    return result;
}

RET_VAL myRand(){
    double temp = (double) rand() / RAND_MAX;

    RET_VAL result = (RET_VAL){DOUBLE_TYPE, temp};

    return result;
}

RET_VAL addOper(AST_NODE *op){
    if(!op)
        return  (RET_VAL){INT_TYPE, NAN};
    RET_VAL result = (RET_VAL){INT_TYPE, 0};

    while(op){
        RET_VAL temp = eval(op);
        result.value += temp.value;
        result.type |= temp.type;
        op = op->next;
    }

    return result;
}

RET_VAL subOper(AST_NODE *op){
    if(!op)
        return  (RET_VAL){INT_TYPE, NAN};
    RET_VAL result = (RET_VAL){INT_TYPE, 0};

    while(op){
        RET_VAL temp = eval(op);
        result.value -= temp.value;
        result.type |= temp.type;
        op = op->next;
    }

    return result;
}

RET_VAL multOper(AST_NODE *op){
    if(!op)
        return  (RET_VAL){INT_TYPE, NAN};
    RET_VAL result = (RET_VAL){INT_TYPE, 0};

    while(op){
        RET_VAL temp = eval(op);
        result.value *= temp.value;
        result.type |= temp.type;
        op = op->next;
    }

    return result;
}

RET_VAL divOper(AST_NODE *op){
    if(!op)
        return  (RET_VAL){INT_TYPE, NAN};
    RET_VAL result = (RET_VAL){INT_TYPE, 0};

    while(op){
        RET_VAL temp = eval(op);
        result.value /= temp.value;
        result.type |= temp.type;
        op = op->next;
    }

    return result;
}

RET_VAL print(AST_NODE *node){
    RET_VAL result = (RET_VAL){NO_TYPE, NAN};
    if(!node)
        return result;

    printf("=> ");

    AST_NODE *temp = node;
    while(temp) {

        result = eval(temp);
        switch (result.type) {
            case INT_TYPE:
                printf("INT_TYPE: %ld ", (long) floor(result.value));
                break;
            case DOUBLE_TYPE:
                printf("DOUBLE_TYPE: %f ", result.value);
                break;
            default:
                yyerror("Invalid Type Error in printRetVal");
                break;
        }
        temp = temp->next;
    }
    return result;
}

RET_VAL printVerbose(AST_NODE *node) {
    if (!node) return (RET_VAL) {NO_TYPE, NAN};

    RET_VAL result = eval(node);


    switch (node->type) {
        case NUM_NODE_TYPE:
            switch (result.type) {
                case INT_TYPE:
                    printf("(INT_TYPE: %ld) ", (long) floor(result.value));
                    break;
                case DOUBLE_TYPE:
                    printf("(DOUBLE_TYPE: %f) ", result.value);
                    break;
                default:
                    yyerror("Invalid Type Error in printRetVal");
                    break;
            }
            break;
        case FUNC_NODE_TYPE:
            printf("(FUNC: %s ", funcNames[node->data.function.oper]);
            AST_NODE *temp = node->data.function.opList;
            while(temp){
                print(temp);
                temp = temp->next;
            }
            printf(") ");
            break;
        case SYMBOL_NODE_TYPE:
            printf("(SYMBOL: %s ", node->data.symbol.ident);
            print(getSymbolTableNode(node)->val);
            printf(") ");
            break;
    }
    return result;
}

RET_VAL evalSymbolNode(AST_NODE *symbolNode) {
    //TODO evalSymbolNode

    if (!symbolNode)
        return (RET_VAL) {INT_TYPE, NAN};

    RET_VAL result = {INT_TYPE, NAN};

    SYMBOL_TABLE_NODE *tempTableNode = getSymbolTableNode(symbolNode);

    result = eval(tempTableNode->val);

    if (result.type == DOUBLE_TYPE && tempTableNode->type == INT_TYPE) {
        printf("WARNING: precision loss in the assignment for variable %s\n", symbolNode->data.symbol.ident);
        result.value = round(result.value);
    }
    result.type = tempTableNode->type;

    return result;
}

SYMBOL_TABLE_NODE *getSymbolTableNode(AST_NODE *symbolNode) {
    //TODO evalSymbolNode

    SYMBOL_TABLE_NODE *result = symbolNode->symbolTable;
    AST_NODE *tempNode = symbolNode;
    bool found = false;

    while (!found && tempNode->parent != NULL) {

        tempNode = tempNode->parent;
        result = tempNode->symbolTable;

        while (!found && result != NULL) {
            if (strcmp(symbolNode->data.symbol.ident, result->ident) == 0)
                found = true;
            else
                result = result->next;
        }
    }

    if (!found) {
        yyerror("ERROR: Invalid Symbol");
    }

    return result;
}

// prints the type and value of a RET_VAL
void printRetVal(RET_VAL val) {
    switch (val.type) {
        case INT_TYPE:
            printf("INT_TYPE: %ld", (long) floor(val.value));
            break;
        case DOUBLE_TYPE:
            printf("DOUBLE_TYPE: %f", val.value);
            break;
        default:
            yyerror("Invalid Type Error in printRetVal");
            break;
    }
}
