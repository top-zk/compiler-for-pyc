/* implements a semantic analyzer, according to the interface analyzer.h
 */

#ifndef _S_ANALYZER_H_
#define _S_ANALYZER_H_

// #include "symbol_table.h"
#include "analyzer.h"

// the symbol table on the top level.
// extern SymbTab * symbolTable;

/* Function buildSymtab constructs the symbol
 * table by preorder traversal of the syntax tree
 */
// void build_symb_tab(TreeNode *);

/* Procedure typeCheck performs type checking
 * by a postorder syntax tree traversal
 */
// void type_check(TreeNode *);

extern Bool A_debugAnalyzer;

/*  build_symbol_table()
    [Computation]:
    - constructs the symbol table by preorder traversal of the parse-tree that is known by the analyzer.
    - pre_proc is applied to each tree node.
    */
// void build_symbol_table(AnalyzerInfo *info);
/*st stands for symbol table*/

Analyzer *new_s_analyzer(TreeNode *parseTree);

// static void top_symbtb_initialize(AnalyzerInfo *info);
void semantic_analysis(Analyzer *analyzer);
static void pre_traverse(TreeNode *t, SymbolTable *st, Bool *errorFound, SymbolTable *(*pre_proc)(TreeNode *, SymbolTable *, Bool *));
static void post_traverse(TreeNode *t, Bool *errorFound, void (*post_proc)(TreeNode *, Bool *));
static SymbolTable *pre_proc(TreeNode *nd, SymbolTable *st, Bool *errorFound);
static void post_proc(TreeNode *nd, Bool *errorFound);
static void semantic_error(const TreeNode *nd, int errorNum, Bool *errorFound);
static Bool is_keyword(const char *name);
typedef struct analyzer Analyzer;

void destroyAnalyzer(Analyzer *self);
#endif
