#ifndef SCANNER_H
#define SCANNER_H

typedef enum tokentype
{

  PLUS,
  MINUS,
  MUL,
  DIV,
  MOD,
  PPLUS,
  MMINUS,
  POWER,
  LT,
  GT,
  LTE,
  GTE,
  EQ,
  UNEQ,
  AAND,
  OR,
  NOT,
  ASSIGN,
  PLUS_ASSIGN,
  MINUS_ASSIGN,
  MUL_ASSIGN,
  DIV_ASSIGN,
  MOD_ASSIGN,
  POWER_ASSIGN,
  AND,
  LEFT_SHIFT,
  RIGHT_SHIFT,
  TERNARY,
  SIZE, /*operators + - * / % ++ -- ** < > <= >= == != && || ! = += -= *= /= %= **= & << >> ,? : ,sizeof */
  INT,
  FRAC,
  STR,
  ID,
  INTL,
  FRACL,
  STRL, // literal
  LCUR,
  RCUR,
  LPAR,
  RPAR,
  LBRA,
  RBRA, // { } ( ) [ ]
  NEWLINE,
  COLON,
  SEMI,
  COMMA,   // \n : ; ,
  INDENT,  // 缩进
  COMMENT, // 注释
  DO,
  WHILE,
  FOR,
  DEF,
  RETURN,
  END_OF_FILE // end of file
} TokenType;

typedef struct token
{
  TokenType type;
  char *info;
} Token;

typedef struct node
{
  Token t;
  struct node *next;
  struct node *prev;
} Node;
typedef struct list
{
  Node *head;  // pointer to the first node
  Node *tail; // pointer to the last node
  int size;    // number of elements in the list
} List;

void initList(List *list);                                   // 初始化链表
void addToken(List *list, TokenType type, const char *info); // 添加token
void tokenizeFile(List *list, FILE *file);                   // 识别文件中的token，并将其添加到链表中
TokenType identifyTokenType(const char *token);              //// 识别Token类型（重点写的部分）
void printTokens(const List *list);                          // 打印token
void freeList(List *list);                                   // 释放链表

#endif // SCANNER_H
