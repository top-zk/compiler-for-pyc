#include "util.h"
#include "scanner.h"
#include "parse.h"
#include "s_analyzer.h"
#include "symbol_table.h"
typedef struct analyzerInfo
{
    SymbolTable *symbolTable; /* the symbol table on the top level. */
    Bool analyzerError;       /* When TRUE, some error is found the analyzer */
    TreeNode *parseTree;      /* the parse tree that the analyzer is working on*/
} AnalyzerInfo;

Bool A_debugAnalyzer = FALSE; /* by default as false, do not print debug information of running the analyzer*/
TreeNode *new_param_node(NodeKind kind, int lineno)
{
    TreeNode *node = (TreeNode *)malloc(sizeof(TreeNode));
    if (node == NULL)
    {
        fprintf(stderr, "Out of memory error at line %d\n", lineno);
        exit(EXIT_FAILURE);
    }
    node->nodeKind = kind;
    node->lineNum = lineno;
    // 初始化其他字段
    for (int i = 0; i < MAX_CHILDREN; i++)
        node->child[i] = NULL;
    node->lSibling = NULL;
    node->rSibling = NULL;
    node->attr.dclAttr.name = NULL;
    return node;
}
TreeNode *newNode(NodeKind kind)
{
    TreeNode *node = (TreeNode *)malloc(sizeof(TreeNode));
    if (node == NULL)
    {
        fprintf(stderr, "Out of memory error\n");
        exit(EXIT_FAILURE);
    }
    node->nodeKind = kind;
    node->lineNum = 0;
    // 初始化其他字段
    for (int i = 0; i < MAX_CHILDREN; i++)
        node->child[i] = NULL;
    node->lSibling = NULL;
    node->rSibling = NULL;
    node->attr.dclAttr.name = NULL;
    return node;
}
// SymbolTable* symbolTable;

/*  top_symbtb_initialize(void)
    [Computation]:
    - initialize the top-level symbol table and save it in the info of the analyzer, which is a parameter
    - Add the two dummy functions input and output
      int read(void)
      void write(int x)
      The TreeNode of these to functions are not on the syntax tree. And the body of these two functions are missing.
      We want all built-in functions can be found by the lookup function at the global symbol table.
   [Note]:
   This function must be called before any other symbol table is initialized, and it should only be called once during the whole program.
 */

static void top_symbtb_initialize(AnalyzerInfo *info)
{
    int j;
    /* add the read(), write(), and print() functions. Assume they appear at line 0.*/
    TreeNode *readNd = newNode(FUN_DCL);
    TreeNode *writeNd = newNode(FUN_DCL);
    TreeNode *printNd = newNode(FUN_DCL);
    TreeNode *inputNd = newNode(FUN_DCL);
    TreeNode *outputNd = newNode(FUN_DCL);
    TreeNode *p1, *p2, *p3;

    if (A_debugAnalyzer)
        printf("%20s \n", __FUNCTION__);

    info->symbolTable = st_initialize(TRUE); /* create an empty symbol table with id 0 */
    /*  The new_dcl_node will assign these fields to be NULL.
    inputNd->parent = outputNd->parent = inputNd->lSibling = inputNd->rSibling = outputNd->lSibling = outputNd->rSibling = NULL;
     */
    readNd->attr.dclAttr.type = INT_TYPE;
    writeNd->attr.dclAttr.type = VOID_TYPE;
    readNd->attr.dclAttr.name = "read";
    writeNd->attr.dclAttr.name = "write";
    p1 = new_param_node(VOID_PARAM, 0);
    p1->attr.dclAttr.type = VOID_TYPE;
    p1->attr.dclAttr.name = "void"; /*this is not required */
    /* new_param_node() has already assigned the lineNum with 0.
     * p1->lineNum = 0;
     */
    p2 = new_param_node(VAR_PARAM, 0);
    p2->attr.dclAttr.type = NUM_TYPE;
    p2->attr.dclAttr.name = "x";
    /* p2->lineNum = 0; */

    p3 = new_param_node(VAR_PARAM, 0);
    p3->attr.dclAttr.type = STR_TYPE;
    p2->attr.dclAttr.name = "y";

    readNd->child[0] = p1;
    writeNd->child[0] = p2;
    printNd->child[0] = p2;
    st_insert_dcl(inputNd, info->symbolTable);
    st_insert_dcl(outputNd, info->symbolTable);
    st_insert_dcl(printNd, info->symbolTable);
}

/*  pre_traverse()
    [Computation]:
    - It is a generic recursive syntax tree traversal routine: it applies pre_proc in preorder to the tree pointed to by t.
    [Preconditions]:
    - st is not NULL.
 */
static void pre_traverse(TreeNode *t, SymbolTable *st, Bool *errorFound, SymbolTable *(*pre_proc)(TreeNode *, SymbolTable *, Bool *))
{
    if (A_debugAnalyzer)
        printf("%20s \n", __FUNCTION__);
    if (t != NULL)
    {
        SymbolTable *newSt = pre_proc(t, st, errorFound);
        int i;
        for (i = 0; i < MAX_CHILDREN; i++)
            pre_traverse(t->child[i], newSt, errorFound, pre_proc);
        pre_traverse(t->rSibling, st, errorFound, pre_proc);
        /* siblings of the node t share the same scope with nd, since they are in the same same block. */
    }
}

/*  post_traverse()
    [Computation]:
    - It is a generic recursive syntax tree traversal routine: it applies post_proc in post-order to the tree pointed to by t.
 */
static void post_traverse(TreeNode *t, Bool *errorFound, void (*post_proc)(TreeNode *, Bool *))
{
    if (A_debugAnalyzer)
        printf("%20s \n", __FUNCTION__);
    if (t != NULL)
    {
        int i;
        for (i = 0; i < MAX_CHILDREN; i++)
            post_traverse(t->child[i], errorFound, post_proc);
        post_proc(t, errorFound);
        post_traverse(t->rSibling, errorFound, post_proc);
    }
}

static Bool is_keyword(const char *name)
{
    if (A_debugAnalyzer)
        printf("%20s \n", __FUNCTION__);
    if ((strcmp(name, "if") == 0) ||
        (strcmp(name, "else") == 0) ||
        (strcmp(name, "while") == 0) ||
        (strcmp(name, "num") == 0) ||
        (strcmp(name, "void") == 0) ||
        (strcmp(name, "return") == 0))
        return TRUE;
    else
        return FALSE;
}

/* pre_proc()
[Parameters]:
- nd is node in the syntax tree.
- st is the symbol table that corresponds to the block where nd appears. st is not NULL (it is initialized).
[Computation]:
   Detailed description like c-minus.pdf
   [Updating the syntax tree]:
   - Attach a new symbol table when a new block is reached (it node is a compound statement, or a function definition).
   - if the node is a declaration node, insert a bucket list record for this declaration into the symbol table st.
   - if the node is reference of a name, look up in the symbol table st to find the bucket-list-record of the declaration of the name, and insert a line list record into the bucket list record.
   [Errors that should be detected]
   -  For a declaration node, the name to be declared is already declared in st.
   -  For a reference of a name, the name cannot be found (lookup) in the symbol table (st, and the upper ones of st), or the name can be found but is not proper (like the name of an array is found to be a function )
   - If some error is found, set the parameter * errorFound to be TRUE.
[Return]
   - If a new symbol table is attached, return it. Otherwise, return the parameter st.
 */
static SymbolTable *pre_proc(TreeNode *nd, SymbolTable *st, Bool *errorFound)
{
    if (A_debugAnalyzer)
        printf("%20s \n", __FUNCTION__);

    if (nd == NULL)
        return st;

    switch (nd->nodeKind)
    {
    case STMT_ND:
        switch (nd->kind.stmt)
        {
        case CMPD_STMT:                   // Compound statement
            st = st_initialize(FALSE);    // Create a new symbol table for the new block
            nd->attr.dclAttr.symbol = st; // Attach the new symbol table to the node
            break;
        default:
            break;
        }
        break;
    case DCL_ND:
        switch (nd->kind.dcl)
        {
        case VAR_DCL:
            if (st_lookup(st, nd->attr.dclAttr.name) != NULL)
            {
                // 如果常量名称已存在于符号表中，报错
                fprintf(stderr, "Error: '%s' already declared in this scope (Line %d)\n",
                        nd->attr.dclAttr.name, nd->lineNum);
                *errorFound = TRUE;
            }
            else
            {
                // 如果提供了初始值，可能需要进行类型检查

                if (nd != NULL)
                {
                    // 假设 `checkType` 是一个类型检查函数
                    if (!checkType(nd->attr.dclAttr.initValue, nd->attr.dclAttr.type))
                    {
                        fprintf(stderr, "Error: Initial value for '%s' has incompatible type (Line %d)\n",
                                nd->attr.dclAttr.name, nd->lineNum);
                        *errorFound = TRUE;
                        break;
                    }
                }
            // Variable declaration
            case ARRAY_DCL: // Array declaration
                if (st_lookup(st, nd->attr.dclAttr.name) != NULL)
                {
                    fprintf(stderr, "Error: '%s' already declared in this scope\n", nd->attr.dclAttr.name);
                    *errorFound = TRUE;
                }
                else
                {
                    // 数组声明需要检查数组大小是否为常量
                    if (nd->attr.dclAttr.size == 0)
                    {
                        fprintf(stderr, "Error: Array size must be a constant (Line %d)\n", nd->lineNum);
                        *errorFound = TRUE;
                    }
                    st_insert_dcl(st, nd->attr.dclAttr.name, nd->lineNum, nd);
                }
                break;
            case FUN_DCL: // Function declaration
                if (st_lookup(st, nd->attr.dclAttr.name) != NULL)
                {
                    fprintf(stderr, "Error: '%s' already declared in this scope\n", nd->attr.dclAttr.name);
                    *errorFound = TRUE;
                }
                break;
            default:
                break;
            }
            break;
        case EXPR_ND:
            switch (nd->kind.expr)
            {
            case OP_EXPR: // 运算符表达式
            {
                // 运算符表达式可能涉及两个或更多操作数，需要递归处理子节点
                SymbolTable *stLeft = handleExpr(st, nd->child[0]);  // 左操作数
                SymbolTable *stRight = handleExpr(st, nd->child[1]); // 右操作数

                // 在处理完子节点后，这里可以对运算符进行类型检查
                if (nd->attr.exprAttr.op == PLUS || nd->attr.exprAttr.op == MINUS)
                {
                    if (nd->child[0]->type != INT || nd->child[1]->type != INT)
                    {
                        fprintf(stderr, "Error: Arithmetic operators require integer operands (Line %d)\n", nd->lineNum);
                        *errorFound = TRUE;
                    }
                }
                // 运算符逻辑处理结束，返回当前符号表
                return st;
            }

            case CONST_EXPR: // 常量表达式
            {
                // 常量表达式通常不涉及符号表处理，只需记录类型
                nd->type = INT; // 假设所有常量是整数类型
                return st;
            }

            case ID_EXPR: // 标识符表达式
            {
                // 查询符号表，确认标识符是否已声明
                SymbolTable *entry = st_lookup(st, nd->attr.exprAttr.name);
                if (entry == NULL)
                {
                    fprintf(stderr, "Error: Identifier '%s' not declared (Line %d)\n", nd->attr.exprAttr.name, nd->lineNum);
                    *errorFound = TRUE;
                }
                else
                {
                    // 如果找到符号表项，记录符号的类型
                    nd->type = entry->type;
                }
                return st;
            }

            case ARRAY_EXPR: // 数组表达式
            {
                // 数组表达式通常有两个部分：数组名和索引
                SymbolTable *entry = st_lookup(st, nd->attr.exprAttr.name);
                if (entry == NULL)
                {
                    fprintf(stderr, "Error: Array '%s' not declared (Line %d)\n", nd->attr.exprAttr.name, nd->lineNum);
                    *errorFound = TRUE;
                }
                else
                {
                    // 检查索引是否为整数类型
                    if (nd->child[0]->type != INT)
                    {
                        fprintf(stderr, "Error: Array index must be an integer (Line %d)\n", nd->lineNum);
                        *errorFound = TRUE;
                    }
                    // 如果符号表中找到数组，将数组的元素类型赋给表达式
                    nd->type = entry->type; // 假设符号表存储了数组元素的类型
                }
                return st;
            }

            case ASN_EXPR: // 赋值表达式
            {
                // 赋值表达式左边是目标，右边是值
                SymbolTable *stLeft = handleExpr(st, nd->child[0]);  // 左操作数
                SymbolTable *stRight = handleExpr(st, nd->child[1]); // 右操作数

                // 检查左值是否是可赋值的变量
                if (nd->child[0]->kind.expr != ID_EXPR && nd->child[0]->kind.expr != ARRAY_EXPR)
                {
                    fprintf(stderr, "Error: Left-hand side of assignment must be a variable (Line %d)\n", nd->lineNum);
                    *errorFound = TRUE;
                }
                else if (nd->child[0]->type != nd->child[1]->type)
                {
                    // 检查左右类型是否匹配
                    fprintf(stderr, "Error: Type mismatch in assignment (Line %d)\n", nd->lineNum);
                    *errorFound = TRUE;
                }
                return st;
            }

            case CALL_EXPR: // Function call
                if (st_lookup(st, nd->attr.exprAttr.name) == NULL)
                {
                    fprintf(stderr, "Error: '%s' not declared in this scope\n", nd->attr.exprAttr.name);
                    *errorFound = TRUE;
                }
                else
                {
                    st_insert(st, nd->attr.exprAttr.name, nd->lineNum, nd);
                }
                break;
            default:
                break;
            }
            break;
        }

        // Process children nodes
        for (int i = 0; i < MAX_CHILDREN; i++)
        {
            st = pre_proc(nd->child[i], st, errorFound);
        }

        // Process sibling nodes
        st = pre_proc(nd->lSibling, st, errorFound);
        st = pre_proc(nd->rSibling, st, errorFound);

        return st;
    }
}
/* post_proc()
[Parameters]:
- nd is node in the syntax tree.
- errorFound is a pointer to a Boolean value. It will be set with TRUE when error is found
[Computation]:
   Detailed description in c-minus.pdf
   [Updating the syntax tree]:
   -  Update the type field of an expression node.
   [Errors that should be detected]
   -  improper type. For example， for an assignment like x = e, x is num variable, the type of the type of RHS expression e is not num.
 */
/*static void post_proc(TreeNode *nd, Bool *errorFound)
{
    if (nd == NULL)
        return;

    // Traverse the syntax tree using child array
    for (int i = 0; i < MAX_CHILDREN; i++)
    {
        post_proc(nd->child[i], errorFound);
    }

    // Traverse the syntax tree using siblings
    post_proc(nd->lSibling, errorFound);
    post_proc(nd->rSibling, errorFound);

    // Check and update the type field of an expression node
    if (nd->nodeKind == EXPR_ND)
    {
        switch (nd->kind.expr)
        {
        case OP_EXPR:
            if (nd->child[0]->type != nd->child[1]->type)
            {
                *errorFound = TRUE;
            }
            nd->type = nd->child[0]->type;
            break;
        case CONST_EXPR:
            nd->type = INT;
            break;
        case ID_EXPR:
            nd->type = nd->attr.exprAttr.name->type;
            break;
        default:
            break;
        }
    }

    // Detect improper type for assignment
    if (nd->nodeKind == STMT_ND && nd->kind.stmt == ASSIGN)
    {
        if (nd->child[0]->type != nd->child[1]->type)
        {
            *errorFound = TRUE;
        }
    }
    // Check and update the type field of an expression node
    if (nd->nodeKind == DCL_ND && nd->kind.dcl == VAR_DCL)
    {
        if (nd->attr.dclAttr.initValue != NULL)
        {
            if (nd->attr.dclAttr.initValue->type != nd->attr.dclAttr.type)
            {
                *errorFound = TRUE;
            }
        }
    }
}
*/
void post_proc(TreeNode *nd, Bool *errorFound)
{
    if (nd == NULL)
        return;

    // 遍历子节点
    for (int i = 0; i < MAX_CHILDREN; i++)
    {
        post_proc(nd->child[i], errorFound);
    }

    // 根据节点类型进行处理
    switch (nd->nodeKind)
    {
    case EXPR_ND:
        switch (nd->kind.expr)
        {
        case ASN_EXPR:
            // 检查赋值表达式的左右类型是否匹配
            if (!checkExprType(nd->child[0], nd->child[1]->type, errorFound))
            {
                printf("Error: Type mismatch in assignment at line %d\n", nd->lineNum);
            }
            break;
        case OP_EXPR:
            // 根据操作符更新表达式类型
            if (nd->attr.exprAttr.op == PLUS || nd->attr.exprAttr.op == MINUS)
            {
                nd->type = INT_TYPE;
            }
            else if (nd->attr.exprAttr.op == LT || nd->attr.exprAttr.op == GT ||
                     nd->attr.exprAttr.op == EQ || nd->attr.exprAttr.op == UNEQ)
            {
                nd->type = INT_TYPE;
            }
            break;
        default:
            break;
        }
        break;
    case STMT_ND:
        switch (nd->kind.stmt)
        {
        case RTN_STMT:
            // 检查返回语句的类型是否正确
            if (!checkExprType(nd->child[0], nd->type, errorFound))
            {
                printf("Error: Type mismatch in return statement at line %d\n", nd->lineNum);
            }
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    // 处理兄弟节点
    post_proc(nd->rSibling, errorFound);
}
/*  build_symbol_table()
    [Computation]:
    - constructs the symbol table by preorder traversal of the parse-tree that is known by the analyzer.
    - pre_proc is applied to each tree node.
 */
void build_symbol_table(AnalyzerInfo *info)
{
    if (A_debugAnalyzer)
        printf("%20s \n", __FUNCTION__);
    /*initialize the symbol_table */
    top_symbtb_initialize(info);
    pre_traverse(info->parseTree, info->symbolTable, &(info->analyzerError), pre_proc);
}

/* type_check()
   [Computation]:
   - type_check performs type checking by a post-order traversal on the parse-tree of an analyzer.
   - post_proc is applied to each tree node.
 */
void type_check(AnalyzerInfo *info)
{
    if (A_debugAnalyzer)
        printf("%20s \n", __FUNCTION__);
    post_traverse(info->parseTree, &(info->analyzerError), post_proc);
}