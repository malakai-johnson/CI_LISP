%{
    #include "ciLisp.h"
%}

%union {
    double dval;
    char *sval;
    struct ast_node *astNode;
    struct table_node *tableNode;
};

%token <sval> FUNC SYMBOL TYPE
%token <dval> INT DOUBLE
%token LPAREN RPAREN LET COND LAMBDA EOL QUIT

%type <astNode> s_expr s_expr_list f_expr number type
%type <tableNode> let_list let_section let_elem arg_list

%%

program:
    s_expr EOL {
        fprintf(stderr, "yacc: program ::= s_expr EOL\n");
        if ($1) {
            printRetVal(eval($1));
            freeNode($1);
        }
    };

s_expr:
    number {
        fprintf(stderr, "yacc: s_expr ::= number\n");
        $$ = $1;
    }
    | f_expr {
        $$ = $1;
    }
    | LPAREN let_section s_expr RPAREN{
    	fprintf(stderr, "yacc: s_expr ::= LPAREN let_section s_expr RPAREN\n");
    	$$ = addSymbolTable($2, $3);
    }
    | LPAREN COND s_expr s_expr s_expr RPAREN{
    	$$ = createCondNode($3, $4, $5);
    }
    | SYMBOL {
    	fprintf(stderr, "yacc: s_expr ::= symbol\n");
    	$$ = createSymbolNode($1);
    }
    | QUIT {
        fprintf(stderr, "yacc: s_expr ::= QUIT\n");
        exit(EXIT_SUCCESS);
    }
    | error {
        fprintf(stderr, "yacc: s_expr ::= error\n");
        yyerror("unexpected token");
        $$ = NULL;
    };

s_expr_list:
	s_expr s_expr_list {
		fprintf(stderr, "yacc: s_expr_list ::= s_expr s_expr_list\n");
		$$ = addAstNode($1, $2);
	}
	| s_expr{
		fprintf(stderr, "yacc: s_expr_list ::= s_expr\n");
		$$ = $1;
	}

number:
    INT {
        fprintf(stderr, "yacc: number ::= INT\n");
        $$ = createNumberNode($1, INT_TYPE);
    }
    | DOUBLE {
        fprintf(stderr, "yacc: number ::= DOUBLE\n");
        $$ = createNumberNode($1, DOUBLE_TYPE);
    };

f_expr:
    LPAREN FUNC s_expr_list RPAREN {
        fprintf(stderr, "yacc: s_expr ::= LPAREN FUNC expr RPAREN\n");
        $$ = createFunctionNode($2, $3);
    }
    | LPAREN FUNC RPAREN {
        fprintf(stderr, "yacc: s_expr ::= LPAREN FUNC expr RPAREN\n");
        $$ = createFunctionNode($2, NULL);
    }
    | LPAREN SYMBOL s_expr_list RPAREN {
        fprintf(stderr, "yacc: s_expr ::= LPAREN FUNC expr RPAREN\n");
//        AST_NODE *temp = createFunctionNode($2, $3);
//        TABLE_NODE *tempSymbolTableNode = createSymbolTableNode($2, NULL,
//        $$ = createFunctionNode(createSymbolNode($2), $3);
	$$ = createFunctionNode($2, $3);
    }
//
//    | LPAREN FUNC s_expr s_expr RPAREN {
//        fprintf(stderr, "yacc: s_expr ::= LPAREN FUNC expr expr RPAREN\n");
//        $$ = createFunctionNode($2, $3, $4);
//    };

let_section:
	LPAREN let_list RPAREN {
        	fprintf(stderr, "yacc: let_section ::= LPAREN let_list RPAREN\n");
		$$ = $2;
	};
let_list:
	LET let_elem {
        	fprintf(stderr, "yacc: let_list ::= let let_elem\n");
		$$ = $2
	}
	| let_list let_elem {
        	fprintf(stderr, "yacc: let_list ::= let_list let_elem\n");
        	addToTable($1, $2);
		$$ = $1;
	};
let_elem:
	LPAREN SYMBOL s_expr RPAREN {
		fprintf(stderr, "yacc: let_elem ::= LPAREN SYMBOL s_expr RPAREN\n");
		$$ = createSymbolTableNode($2, $3, NO_TYPE);
	}
	| LPAREN type SYMBOL s_expr RPAREN {
		fprintf(stderr, "yacc: let_elem ::= LPAREN type SYMBOL s_expr RPAREN\n");
		$$ = createSymbolTableNode($3, $4, $2);
	};
	| LPAREN type SYMBOL LAMBDA LPAREN arg_list RPAREN s_expr RPAREN{
		fprintf(stderr, "yacc: let_elem ::= LPAREN type SYMBOL LAMBDA LPAREN arg_list RPAREN s_expr RPAREN\n");
		$$ = createFuncTableNode($3, $8, $2, $6);
	};
	| LPAREN SYMBOL LAMBDA LPAREN arg_list RPAREN s_expr RPAREN{
		fprintf(stderr, "yacc: let_elem ::= LPAREN SYMBOL LAMBDA LPAREN arg_list RPAREN s_expr RPAREN\n");
		$$ = createFuncTableNode($2, $7, NO_TYPE, $5);
	};

arg_list:
	SYMBOL arg_list{
		$$ = createArgNode($1, $2);
	}
	| SYMBOL {
		$$ = createArgNode($1, NULL);
	}

type:
	TYPE {
		$$ = resolveType($1);
	}
%%

