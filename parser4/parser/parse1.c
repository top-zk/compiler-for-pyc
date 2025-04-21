#include "libs.h"
#include "scanner.h"
#include "parse.h"
#include "util.h"

TreeNode *parse(Parser *p)
{
    if (!p->info)
    {
        fprintf(stderr, "parse: p->info is NULL\n");
        exit(1);
    }
    ParserInfo *info = (ParserInfo *)p->info;
    info->errorCount = 0;
    info->currentTokenNode = info->tokenList.head;

    TreeNode *tree = parse_program(p); // start form program
    if (info->errorCount > 0)
    {
        fprintf(stderr, "There are %d syntax errors in the program\n", info->errorCount);
    }
    return tree;
}

void set_token_list(Parser *p, List tokenList)
{
    if (!p->info)
    {
        p->info = malloc(sizeof(ParserInfo));
        memset(p->info, 0, sizeof(ParserInfo));
    }
    ParserInfo *info = (ParserInfo *)p->info;
    info->tokenList = tokenList;
    info->currentTokenNode = tokenList.head;
}

void free_tree(Parser *p, TreeNode *tree)
{
    if (!tree)
        return;
    for (int i = 0; i < MAX_CHILDREN; i++)
    {
        free_tree(p, tree->child[i]);
    }
    free_tree(p, tree->rSibling);
    free(tree);
}

Parser *createParser()
{
    Parser *p = (Parser *)malloc(sizeof(Parser));
    p->parse = parse;
    p->set_token_list = set_token_list;
    p->free_tree = free_tree;
    p->info = NULL;
    // p->print_tree = print_tree;
    return p;
}
void destroyParser(Parser *p)
{
    if (p)
    {
        if (p->info)
        {
            free(p->info);
        }
        free(p);
    }
}
/****************************
 * 下面是具体的解析函数框架 *
 ****************************/

/* 获取当前token类型和lexeme的帮助函数 */
Token *currentToken(ParserInfo *info)
{
    if (info->currentTokenNode)
    {
        return (info->currentTokenNode->t);
    }
    return NULL; // 获得info中当下指向的Node结构体中的token
} // 获得当前
Bool checkType(Token *token, TokenType type)
{
    if (token)
    {
        return token->type == type;
    }
    return FALSE;
} // 检查当下info指向的token的与预期的token类型是否匹配

Bool moveTokenNext(ParserInfo *info)
{
    if (info->currentTokenNode && info->currentTokenNode->next)
    {
        info->currentTokenNode = info->currentTokenNode->next;
        return TRUE;
    }
    return FALSE;
} // 移动token链表的指针，指向下一个节点

Bool checkMove(ParserInfo *info, TokenType type)
{
    Token *t = info->currentTokenNode->t;
    if (t && t->type == type)
    {
        if (info->currentTokenNode && info->currentTokenNode->next)
        {
            info->currentTokenNode = info->currentTokenNode->next;
            return TRUE;
        }
        else
        {
            info->currentTokenNode = NULL;
            return TRUE;
        } // last token
    }
    else
    { // not match
        fprintf(stderr, "Syntax error: expect %d, but get %d\n", type, t->type);
        info->errorCount++;
        return FALSE;
    }
} // checkType+moveTokenNext，token类型匹配之后移动指针到下一个
TreeNode *newNode(NodeKind nodeKind)
{
    TreeNode *node = (TreeNode *)malloc(sizeof(TreeNode));
    if (!node)
    {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    node->nodeKind = nodeKind;
    for (int i = 0; i < MAX_CHILDREN; i++)
    {
        node->child[i] = NULL;
    }
    node->lSibling = NULL;
    node->rSibling = NULL;
    node->parent = NULL;
    node->lineNum = 0;
    // node->type = ;
    switch (nodeKind)
    {
    case DCL_ND:
        node->kind.dcl = 0;
        node->attr.dclAttr.type = VOID;
        node->attr.dclAttr.name = NULL;
        node->attr.dclAttr.size = 0;
        break;
    case STMT_ND:
        node->kind.stmt = 0;
        break;
    case EXPR_ND:
        node->kind.expr = 0;
        node->attr.exprAttr.op = 0;
        node->attr.exprAttr.val = 0;
        node->attr.exprAttr.name = NULL;
        break;
    case PARAM_ND:
        node->kind.param = 0;
        node->attr.dclAttr.type = VOID;
        node->attr.dclAttr.name = NULL;
        node->attr.dclAttr.size = 0;
        break;
    case ROOT:
        break;
    default:
        fprintf(stderr, "Error: unknown node kind\n");
        exit(1);
    }
    node->type = VOID;
    node->something = NULL;
    return node;
} // 创建一个新节点，对于TreeNode的一些初始化（初始化可能有问题，比如lineNum。
Bool canStartDeclaration(TokenType t)
{
    return (t == INT || t == FRAC || t == VOID || t == DEF);
}
Bool looksLikeFunDeclaration(ParserInfo *f)
{
    if (checkType(f->currentTokenNode->t, INT) || checkType(f->currentTokenNode->t, FRAC) || checkType(f->currentTokenNode->t, VOID) || checkType(f->currentTokenNode->t, DEF))
    {
        if (checkType(f->currentTokenNode->next->t, ID))
        {
            if (checkType(f->currentTokenNode->next->next->t, LPAR))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}
/****************************
 * 文法解析函数 *
 ***************************/
/*program --> declaration-list*/

/*declaration_list -> declaration_list declaration | declaration*/
TreeNode *parse_program(Parser *p)
{
    TreeNode *root = newNode(ROOT);
    Bool s;
    if ((root->child[0] = declaration_list(p->info, &s)), s == TRUE)
    {
        return root;
    }
    else
    {
        printf("Error:Failed to parse declaration_list in program. \n");
        removeNode(root);
        return NULL;
    }
    return root;
}
TreeNode *declaration_list(ParserInfo *f, Bool *status)
{
    TreeNode *result = newNode(DCL_ND);
    Bool s;
    TreeNode *firstDecl = declaration(f, &s);
    if (s == FALSE)
    {
        printf("Error: expected at least one declaration.\n");
        removeNode(result);
        *status = FALSE;
        return NULL;
    }
    result->child[0] = firstDecl;

    TreeNode *currentNode = firstDecl;
    while (canStartDeclaration(currentToken(f)->type))
    {
        TreeNode *nextDecl = declaration(f, &s);
        if (s == FALSE)
        {
            break;
        }
        result->child[1] = nextDecl; // 有问题
        currentNode->rSibling = nextDecl;
        nextDecl->lSibling = currentNode;
        currentNode = nextDecl;
    }
    *status = TRUE;
    return result;
}
/*declaration -> var_declaration | fun_declaration*/
TreeNode *declaration(ParserInfo *f, Bool *status)
{

    TreeNode *node = NULL;
    Bool s;
    if (looksLikeFunDeclaration(f))
    {
        node = fun_declaration(f, &s);
    }
    else
    {
        node = var_declaration(f, &s);
    }
    *status = s;
    return node;
}
/*var_declaration -> type_specifier ID SEMI | type_specifier ID LPAR params RPAR compound_stmt*/
TreeNode *var_declaration(ParserInfo *f, Bool *status)
{
    TreeNode *node = newNode(DCL_ND);
    Bool s;
    if (checkMove(f, INT) || checkMove(f, FRAC) || checkMove(f, VOID))
    {
        if (checkMove(f, ID))
        {
            if (checkMove(f, SEMI))
            {
                *status = TRUE;
                return node;
            }
        }
    }
    printf("something wrong");
    *status = FALSE;
    removeNode(node);
    return NULL;
}
/*fun-declaration --> type-specifier ID ( param-list ) compound-stmt | def ID (param-list): compound-stmt*/
TreeNode *fun_declaration(ParserInfo *f, Bool *status)
{
    TreeNode *node = newNode(DCL_ND);
    Bool s;
    if (checkMove(f, INT) || checkMove(f, FRAC) || checkMove(f, VOID))
    {
        if (checkMove(f, ID))
        {
            if (checkMove(f, LPAR))
            {
                if ((node->child[0] = param_list(f, &s)), s == TRUE)
                {
                    if (checkMove(f, RPAR))
                    {
                        if ((node->child[1] = compound_stmt(f, &s)), s == TRUE)
                        {
                            *status = TRUE;
                            return node;
                        }
                    }
                }
            }
        }
    }
    else if (checkMove(f, DEF))
    {
        if (checkMove(f, ID))
        {
            if (checkMove(f, LPAR))
            {
                if ((node->child[0] = param_list(f, &s)), s == TRUE)
                {
                    if (checkMove(f, RPAR))
                    {
                        if (checkMove(f, COLON))
                        {
                            if ((node->child[1] = compound_stmt(f, &s)), s == TRUE)
                            {
                                *status = TRUE;
                                return node;
                            }
                        }
                    }
                }
            }
        }
    }
    printf("something wrong");
    *status = FALSE;
    removeNode(node);
    return NULL;
}
/*param-list --> param-list, param | param | empty*/
TreeNode *param_list(ParserInfo *f, Bool *status)
{
    TreeNode *result = newNode(PARAM_ND);
    Bool s;
    TreeNode *firstParam = param(f, &s);
    if (s == FALSE)
    {
        printf("Error: expected at least one parameter.\n");
        removeNode(result);
        *status = FALSE;
        return NULL;
    }
    result->child[0] = firstParam;

    TreeNode *currentNode = firstParam;
    while (checkMove(f, COMMA))
    {
        TreeNode *nextParam = param(f, &s);
        if (s == FALSE)
        {
            break;
        }
        result->child[1] = nextParam; // 有问题
        currentNode->rSibling = nextParam;
        nextParam->lSibling = currentNode;
        currentNode = nextParam;
    }
    *status = TRUE;
    return result;
}
/*param --> type-specifier ID | type-specifier ID[] | ID*/
TreeNode *param(ParserInfo *f, Bool *status)
{
    TreeNode *node = newNode(PARAM_ND);
    Bool s;
    if (checkMove(f, INT) || checkMove(f, FRAC) || checkMove(f, VOID))
    {
        if (checkMove(f, ID))
        {
            if (checkMove(f, LBRA))
            {
                if (checkMove(f, RBRA))
                {
                    *status = TRUE;
                    return node;
                }
            }
            *status = TRUE;
            return node;
        }
    }
    else if (checkMove(f, ID))
    {
        *status = TRUE;
        return node;
    }
    printf("something wrong");
    *status = FALSE;
    removeNode(node);
    return NULL;
}
/*compound-stmt --> { local-declarations statement-list }*/
TreeNode *compound_stmt(ParserInfo *f, Bool *status)
{
    TreeNode *root = NULL;
    Bool s;
    if (!checkMove(f, LCUR))
    {
        printf("Error: expected { in compound statement.\n");
        exit(1);
    }
    root = newNode(STMT_ND);
    if ((root->child[0] = local_declarations(f, &s)), s == TRUE)
    {
        if ((root->child[1] = statement_list(f, &s)), s == TRUE)
        {
            if (!checkMove(f, RCUR))
            {
                printf("missing } in compound statement");
                exit(2);
            }
            *status = TRUE;
            return root;
        }
    }
    printf("something wrong");
    *status = FALSE;
    removeNode(root);
    return NULL;
}

/*local-declarations --> local-declarations var-declaration | empty*/
TreeNode *local_declarations(ParserInfo *f, Bool *status)
{
    TreeNode *head = NULL;
    TreeNode *tail = NULL;

    while (canStartDeclaration(currentToken(f)->type))
    {
        Bool s;
        TreeNode *newNode = var_declaration(f, &s);
        if (s == FALSE)
        {
            printf("Error: expected at least one declaration.\n");
            *status = FALSE;
            return head;
        }
        if (head == NULL)
        {
            head = newNode;
            tail = newNode;
        }
        else
        {
            tail->rSibling = newNode;
            newNode->lSibling = tail;
            tail = newNode;
        }
    }
    *status = TRUE;
    return head;
}

/*statement-list --> statement-list statement | empty*/
TreeNode *statement_list(ParserInfo *f, Bool *status)
{
    TreeNode *head = NULL;
    TreeNode *tail = NULL;
    while (TRUE)
    {
        Bool s;
        TreeNode *newNode = statement(f, &s);
        if (s == FALSE)
        {
            printf("Error: expected at least one statement.\n");
            *status = FALSE;
            return head;
        }
        if (head == NULL)
        {
            head = newNode;
            tail = newNode;
        }
        else
        {
            tail->rSibling = newNode;
            newNode->lSibling = tail;
            tail = newNode;
        }
    }
    *status = TRUE;
    return head;
}
/*statement --> expression-stmt | compound-stmt | selection-stmt | iteration-stmt | return-stmt*/
TreeNode *statement(ParserInfo *f, Bool *status)
{
    TreeNode *node = NULL;
    Bool s;
    Token *t = currentToken(f);
    if (!t)
    {
        *status = FALSE;
        return NULL;
    }
    if (checkType(t, LCUR))
        node = compound_stmt(f, &s);
    else if (checkType(t, IF))
    {
        node = selection_stmt(f, &s);
    }
    else if (checkType(t, WHILE) || checkType(t, DO) || checkType(t, FOR))
    {
        node = iteration_stmt(f, &s);
    }
    else if (checkType(t, RETURN))
    {
        node = return_stmt(f, &s);
    }
    else
    {
        node = expression_stmt(f, &s);
    }
    *status = s;
    return node;
}
/*expression-stmt --> expression ; | ;*/
TreeNode *expression_stmt(ParserInfo *f, Bool *status)
{
    TreeNode *node = newNode(STMT_ND);
    Bool s;
    if (checkType(currentToken(f), SEMI))
    {
        moveTokenNext(f);
        *status = TRUE;
        return node;
    }
    else
    {
        if ((node->child[0] = expression(f, &s)), s == TRUE)
        {
            if (checkMove(f, SEMI))
            {
                *status = TRUE;
                return node;
            }
        }
    }
    printf("something wrong");
    *status = FALSE;
    removeNode(node);
    return NULL;
}
/*selection-stmt --> if ( expression ) statement | if ( expression ) statement else statement | if expression : statement | if expression : statement else : statement*/
TreeNode *selection_stmt(ParserInfo *f, Bool *status)
{
    TreeNode *node = newNode(STMT_ND);
    Bool s;
    if (!checkMove(f, IF))
    {
        printf("missing if in selection statement");
        *status = FALSE;
        removeNode(node);
        return NULL;
    }

    Token *t = currentToken(f);
    if (!t)
    {
        printf("unexpected end of tokens after 'if'");
        removeNode(node);
        *status = FALSE;
        return NULL;
    }

    if (checkType(t, LPAR))
    {
        moveTokenNext(f);
        if (node->child[0] = expression(f, &s), s == TRUE)
        {
            if (checkMove(f, RPAR))
            {
                if (node->child[1] = statement(f, &s), s == TRUE)
                {
                    if (checkType(currentToken(f), ELSE))
                    {
                        moveTokenNext(f);
                        if (node->child[2] = statement(f, &s), s == TRUE)
                        {
                            *status = TRUE;
                            return node;
                        }
                    }
                    else
                    {
                        *status = TRUE;
                        return node;
                    }
                }
            }
        }
    }
    else
    {
        if (node->child[0] = expression(f, &s), s == TRUE)
        {
            if (checkMove(f, COLON))
            {
                if (node->child[1] = statement(f, &s), s == TRUE)
                {
                    if (checkType(currentToken(f), ELSE))
                    {
                        moveTokenNext(f);
                        if (checkMove(f, COLON))
                        {
                            if (node->child[2] = statement(f, &s), s == TRUE)
                            {
                                *status = TRUE;
                                return node;
                            }
                        }
                    }
                    else
                    {
                        *status = TRUE;
                        return node;
                    }
                }
            }
        }
    }
    printf("something wrong");
    *status = FALSE;
    removeNode(node);
    return NULL;
}
/*iteration-stmt --> while expression : statement | while ( expression ) statement | do statement while (expression) | do: statement while expression | for（expression；expression；expression）statement ｜ for ID in expression : statement*/
TreeNode *iteration_stmt(ParserInfo *f, Bool *status)
{
    TreeNode *node = newNode(STMT_ND);
    Bool s;
    if (checkType(currentToken(f), WHILE))
    {
        moveTokenNext(f);
        if (checkType(currentToken(f), LPAR))
        {
            moveTokenNext(f);
            if (node->child[0] = expression(f, &s), s == TRUE)
            {
                if (checkMove(f, RPAR))
                {
                    if (node->child[1] = statement(f, &s), s == TRUE)
                    {
                        *status = TRUE;
                        return node;
                    }
                }
            }
        }
        else if (node->child[0] = expression(f, &s), s == TRUE)
        {
            if (checkMove(f, COLON))
            {
                if (node->child[1] = statement(f, &s), s == TRUE)
                {
                    *status = TRUE;
                    return node;
                }
            }
        }
    }
    else if (checkType(currentToken(f), DO))
    {
        moveTokenNext(f);
        if (node->child[0] = statement(f, &s), s == TRUE)
        {
            if (checkMove(f, WHILE))
            {
                if (checkMove(f, LPAR))
                {
                    if (node->child[1] = expression(f, &s), s == TRUE)
                    {
                        if (checkMove(f, RPAR))
                        {
                            *status = TRUE;
                            return node;
                        }
                    }
                }
            }
        }
        else if (checkType(currentToken(f), COLON))
        {
            moveTokenNext(f);
            if (node->child[0] = statement(f, &s), s == TRUE)
            {
                if (checkMove(f, WHILE))
                {
                    if (node->child[1] = expression(f, &s), s == TRUE)
                    {
                        *status = TRUE;
                        return node;
                    }
                }
            }
        }
    }
    else if (checkType(currentToken(f), FOR))
    {
    }
    else
    {
        printf("something wrong");
        *status = FALSE;
        removeNode(node);
        return NULL;
    }
}
/*return-stmt --> return expression ; | return ;*/
TreeNode *return_stmt(ParserInfo *f, Bool *status)
{
    TreeNode *node = newNode(STMT_ND);
    node->kind.stmt = 0; // 初始化 STMT_ND 类型的节点
    Bool s;
    if (checkMove(f, RETURN))
    {
        if (checkType(currentToken(f), SEMI))
        {
            moveTokenNext(f);
            *status = TRUE;
            return node;
        }
        else
        {
            if (node->child[0] = expression(f, &s), s == TRUE)
            {
                if (checkMove(f, SEMI))
                {
                    *status = TRUE;
                    return node;
                }
            }
        }
    }
    printf("something wrong");
    *status = FALSE;
    removeNode(node);
    return NULL;
}
/*expression --> var = expression | simple-expression*/
TreeNode *expression(ParserInfo *f, Bool *status)
{
    TreeNode *node = newNode(EXPR_ND);
    node->kind.expr = 0; // 初始化 EXPR_ND 类型的节点
    node->attr.exprAttr.op = 0;
    node->attr.exprAttr.val = 0;
    node->attr.exprAttr.name = NULL;

    Bool s;
    if (node->child[0] = var(f, &s), s == TRUE)
    {
        if (checkMove(f, ASSIGN))
        {
            if (node->child[1] = expression(f, &s), s == TRUE)
            {
                *status = TRUE;
                return node;
            }
        }
    }
    else
    {
        if (node->child[0] = simple_expression(f, &s), s == TRUE)
        {
            *status = TRUE;
            return node;
        }
    }
    printf("something wrong");
    *status = FALSE;
    removeNode(node);
    return NULL;
}
/*var --> ID | ID [ expression ]*/
TreeNode *var(ParserInfo *f, Bool *status)
{
    TreeNode *node = newNode(EXPR_ND); //
    node->kind.expr = 0;               // 初始化 EXPR_ND 类型的节点
    node->attr.exprAttr.op = 0;
    node->attr.exprAttr.val = 0;
    node->attr.exprAttr.name = NULL;
    Bool s;
    if (checkMove(f, ID))
    {
        if (checkType(currentToken(f), LBRA))
        {
            moveTokenNext(f);
            if (node->child[0] = expression(f, &s), s == TRUE)
            {
                if (checkMove(f, RBRA))
                {
                    *status = TRUE;
                    return node;
                }
            }
        }
        *status = TRUE;
        return node;
    }
}
/*simple-expression --> additive-expression relop additive-expression | additive-expression*/
TreeNode *simple_expression(ParserInfo *f, Bool *status)
{
    TreeNode *node = newNode(EXPR_ND);
    node->kind.expr = 0; // 初始化 EXPR_ND 类型的节点
    node->attr.exprAttr.op = 0;
    node->attr.exprAttr.val = 0;
    node->attr.exprAttr.name = NULL;
    Bool s;
    if (node->child[0] = additive_expression(f, &s), s == TRUE)
    {
        if (node->child[1] = relop(f, &s), s == TRUE)
        {
            if (node->child[2] = additive_expression(f, &s), s == TRUE)
            {
                *status = TRUE;
                return node;
            }
        }
        *status = TRUE;
        return node;
    }
    printf("something wrong");
    *status = FALSE;
    removeNode(node);
    return NULL;
}
/*relop --> < | <= | > | >= | == | !=*/
TreeNode *relop(ParserInfo *f, Bool *status)
{
    TreeNode *node = newNode(EXPR_ND);
    node->kind.expr = 0; // 初始化 EXPR_ND 类型的节点
    node->attr.exprAttr.op = 0;
    node->attr.exprAttr.val = 0;
    node->attr.exprAttr.name = NULL;
    Bool s;
    if (checkMove(f, LT) || checkMove(f, LTE) || checkMove(f, GT) || checkMove(f, GTE) || checkMove(f, EQ) || checkMove(f, UNEQ))
    {
        *status = TRUE;
        return node;
    }
    printf("something wrong");
    *status = FALSE;
    removeNode(node);
    return NULL;
}
/*additive-expression --> additive-expression addop term | term*/
TreeNode *additive_expression(ParserInfo *f, Bool *status)
{
    TreeNode *node = newNode(EXPR_ND);
    node->kind.expr = 0; // 初始化 EXPR_ND 类型的节点
    node->attr.exprAttr.op = 0;
    node->attr.exprAttr.val = 0;
    node->attr.exprAttr.name = NULL;

    Bool s;
    if (node->child[0] = additive_expression(f, &s), s == TRUE)
    {
        if (node->child[1] = addop(f, &s), s == TRUE)
        {
            if (node->child[2] = term(f, &s), s == TRUE)
            {
                *status = TRUE;
                return node;
            }
        }
    }
    else
    {
        if (node->child[0] = term(f, &s), s == TRUE)
        {
            *status = TRUE;
            return node;
        }
    }

    printf("something wrong");
    *status = FALSE;
    removeNode(node);
    return NULL;
}
/*addop --> + | -*/
TreeNode *addop(ParserInfo *f, Bool *status)
{
    TreeNode *node = newNode(EXPR_ND);
    node->kind.expr = 0; // 初始化 EXPR_ND 类型的节点
    node->attr.exprAttr.op = 0;
    node->attr.exprAttr.val = 0;
    node->attr.exprAttr.name = NULL;
    Bool s;
    if (checkMove(f, PLUS) || checkMove(f, MINUS))
    {
        *status = TRUE;
        return node;
    }
    printf("expect + or -");
    *status = FALSE;
    removeNode(node);
    return NULL;
}
/*term --> term mulop factor | factor*/
TreeNode *term(ParserInfo *f, Bool *status)
{
    TreeNode *node = newNode(EXPR_ND);
    node->kind.expr = 0; // 初始化 EXPR_ND 类型的节点
    node->attr.exprAttr.op = 0;
    node->attr.exprAttr.val = 0;
    node->attr.exprAttr.name = NULL;
    Bool s;
    if (node->child[0] = term(f, &s), s == TRUE)
    {
        if (node->child[1] = mulop(f, &s), s == TRUE)
        {
            if (node->child[2] = factor(f, &s), s == TRUE)
            {
                *status = TRUE;
                return node;
            }
        }
    }
    else
    {
        if (node->child[0] = factor(f, &s), s == TRUE)
        {
            *status = TRUE;
            return node;
        }
    }
    printf("something wrong");
    *status = FALSE;
    removeNode(node);
    return NULL;
}
/*mulop --> * | / */
TreeNode *mulop(ParserInfo *f, Bool *status)
{
    TreeNode *node = newNode(EXPR_ND);
    node->kind.expr = 0; // 初始化 EXPR_ND 类型的节点
    node->attr.exprAttr.op = 0;
    node->attr.exprAttr.val = 0;
    node->attr.exprAttr.name = NULL;

    Bool s;
    if (checkMove(f, MUL) || checkMove(f, DIV))
    {
        *status = TRUE;
        return node;
    }
    printf("expect * or /");
    *status = FALSE;
    removeNode(node);
    return NULL;
}
/*factor --> ( expression ) | var | call | NUM*/
TreeNode *factor(ParserInfo *f, Bool *status)
{
    TreeNode *node = newNode(EXPR_ND);
    node->kind.expr = 0; // 初始化 EXPR_ND 类型的节点
    node->attr.exprAttr.op = 0;
    node->attr.exprAttr.val = 0;
    node->attr.exprAttr.name = NULL;
    Bool s;
    if (checkType(currentToken(f), LPAR))
    {
        moveTokenNext(f);
        if (node->child[0] = expression(f, &s), s == TRUE)
        {
            if (checkMove(f, RPAR))
            {
                *status = TRUE;
                return node;
            }
        }
    }
    else if (node->child[0] = var(f, &s), s == TRUE)
    {
        *status = TRUE;
        return node;
    }
    else if (node->child[0] = call(f, &s), s == TRUE)
    {
        *status = TRUE;
        return node;
    }
    else if (node->child[0] = num(f, &s), s == TRUE)
    {
        *status = TRUE;
        return node;
    }
    else if (node->child[0] = Str(f, &s), s == TRUE)
    {
        *status = TRUE;
        return node;
    }
}
/*num --> INTL | FRACL*/
TreeNode *num(ParserInfo *f, Bool *status)
{
    TreeNode *node = newNode(EXPR_ND);
    node->kind.expr = 0; // 初始化 EXPR_ND 类型的节点
    node->attr.exprAttr.op = 0;
    node->attr.exprAttr.val = 0;
    node->attr.exprAttr.name = NULL;
    Bool s;
    if (checkMove(f, INTL) || checkMove(f, FRACL))
    {
        *status = TRUE;
        return node;
    }
    printf("expect INTL or FRACL");
    *status = FALSE;
    removeNode(node);
    return NULL;
}
/*Str --> STRL*/
TreeNode *Str(ParserInfo *f, Bool *status)
{
    TreeNode *node = newNode(EXPR_ND);
    Bool s;
    if (checkMove(f, STRL))
    {
        *status = TRUE;
        return node;
    }
    printf("expect STRL");
    *status = FALSE;
    removeNode(node);
    return NULL;
}
/*call --> ID ( args )*/
TreeNode *call(ParserInfo *f, Bool *status)
{
    TreeNode *node = newNode(EXPR_ND);
    node->kind.expr = 0; // 初始化 EXPR_ND 类型的节点
    node->attr.exprAttr.op = 0;
    node->attr.exprAttr.val = 0;
    node->attr.exprAttr.name = NULL;
    Bool s;
    if (checkType(currentToken(f), ID))
        moveTokenNext(f);
    {
        if (checkMove(f, LPAR))
        {
            if (node->child[0] = args(f, &s), s == TRUE)
            {
                if (checkMove(f, RPAR))
                {
                    *status = TRUE;
                    return node;
                }
            }
        }
    }
    printf("something wrong");
    *status = FALSE;
    removeNode(node);
    return NULL;
}
/*args --> arg-list | empty*/
TreeNode *args(ParserInfo *f, Bool *status)
{
    TreeNode *node = newNode(PARAM_ND);
    node->kind.param = 0; // 初始化 PARAM_ND 类型的节点
    node->attr.dclAttr.type = VOID;
    node->attr.dclAttr.name = NULL;
    node->attr.dclAttr.size = 0;
    Bool s;
    if (node->child[0] = arg_list(f, &s), s == TRUE)
    {
        *status = TRUE;
        return node;
    }
    *status = TRUE;
    return node;
}
/*arg-list --> arg-list , expression | expression*/
TreeNode *arg_list(ParserInfo *f, Bool *status)
{
    TreeNode *head = NULL;
    TreeNode *tail = NULL;
    Bool s;
    TreeNode *expr = expression(f, &s);
    if (s == FALSE)
    {
        printf("failed to parse argument expression");
        *status = FALSE;
        return NULL;
    }

    head = expr;
    tail = expr;

    // 解析后续参数（逗号分隔的多个表达式）
    while (checkType(currentToken(f), COMMA))
    {
        moveTokenNext(f);

        // 解析下一个表达式
        expr = expression(f, &s);
        if (s == FALSE)
        {
            printf("failed to parse argument expression after ','");
            *status = FALSE;
            removeNode(head); // 释放已解析节点
            return NULL;
        }

        // 将新表达式挂在兄弟节点上
        tail->rSibling = expr;
        expr->lSibling = tail;
        tail = expr; // 更新尾部指针
    }

    *status = TRUE;
    return head; // 返回头节点
}