#include <stdio.h>
#include <stdlib.h>
#include "scanner.h"
TokenType identifyTokenType(const char *token) {
    if (strcmp(token, "+") == 0) return PLUS;
    if (strcmp(token, "-") == 0) return MINUS;
    if (strcmp(token, "*") == 0) return MUL;
    if (strcmp(token, "/") == 0) return DIV;
    if (strcmp(token, "%") == 0) return MOD;
    if (strcmp(token, "++") == 0) return PPLUS;
    if (strcmp(token, "--") == 0) return MMINUS;
    if (strcmp(token, "**") == 0) return POWER;
    if (strcmp(token, "<") == 0) return LT;
    if (strcmp(token, ">") == 0) return GT;
    if (strcmp(token, "<=") == 0) return LTE;
    if (strcmp(token, ">=") == 0) return GTE;
    if (strcmp(token, "==") == 0) return EQ;
    if (strcmp(token, "!=") == 0) return UNEQ;
    if (strcmp(token, "&&") == 0) return AAND;
    if (strcmp(token, "||") == 0) return OR;
    if (strcmp(token, "!") == 0) return NOT;
    if (strcmp(token, "=") == 0) return ASSIGN;
    if (strcmp(token, "+=") == 0) return PLUS_ASSIGN;
    if (strcmp(token, "-=") == 0) return MINUS_ASSIGN;
    if (strcmp(token, "*=") == 0) return MUL_ASSIGN;
    if (strcmp(token, "/=") == 0) return DIV_ASSIGN;
    if (strcmp(token, "%=") == 0) return MOD_ASSIGN;
    if (strcmp(token, "**=") == 0) return POWER_ASSIGN;
    if (strcmp(token, "&") == 0) return AND;
    if (strcmp(token, "<<") == 0) return LEFT_SHIFT;
    if (strcmp(token, ">>") == 0) return RIGHT_SHIFT;
    if (strcmp(token, "?") == 0) return TERNARY;
    if (strcmp(token, ":") == 0) return COLON;
    if (strcmp(token, ";") == 0) return SEMI;
    if (strcmp(token, ",") == 0) return COMMA;
    if (strcmp(token, "{") == 0) return LCUR;
    if (strcmp(token, "}") == 0) return RCUR;
    if (strcmp(token, "(") == 0) return LPAR;
    if (strcmp(token, ")") == 0) return RPAR;
    if (strcmp(token, "[") == 0) return LBRA;
    if (strcmp(token, "]") == 0) return RBRA;
    if (strcmp(token, "\n") == 0) return NEWLINE;
    if (strcmp(token, ":") == 0) return COLON;
    if (strcmp(token, ";") == 0) return SEMI;
    if (strcmp(token, ",") == 0) return COMMA;

    // 如果token不是上述任何一种，则返回UNKNOWN
    return UNKNOWN;
}
void initList(List *list)
{
  list->head = NULL;
  list->tail = NULL;
  int size = 0;
}

void addToken(List *list, TokenType type, const char *info)
{
  Node *newNode = (Node *)malloc(sizeof(Node));
  if (newNode == NULL)
  {
    fprintf(stderr, "Memory allocation failed!\n");
    return;
  }
                            
  newNode->t.type = type;
  newNode->t.info = strdup(info); // 复制字符串以确保其独立存储

  // 初始化新节点的指针
  newNode->next = NULL; 
  newNode->prev = NULL;
  if (list->head == NULL)
  {
    list->tail = newNode;
    list->head = newNode;
  } // 如果链表为空，则新节点是链表的第一个节点
  else
  { // 否则将其添加到链表末尾
    list->tail->next = newNode;
    newNode->prev = list->tail;
    list->tail = newNode;
  }
  list->size++;
}

void tokenizeFile(List *list, FILE *file)
{
  char ch;
  
}
TokenType identifyTokenType(const char *token)
{
   
}
void printTokens(const List *list)
{
  Node *current = list->head;
  while (current)
  {
    printf("%s", current->t.type);
    if (current->t.info != NULL)
    {
      printf(":%s\n", current->t.info);
    }
    current = current->next;
  }
}
void freeList(List *list)
{

  if (list == NULL || list->tail == NULL)
  {
    return;
  }
  while (list->tail != NULL)
  {
    Node *temp = list->tail;
    list->tail = list->tail->prev;
    free(temp->t.info);
    free(temp);
    list->size--;
  }
  list->head = NULL;
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    printf("Usage: %s <input file>\n", argv[0]);
    return 1;
  }
  FILE *file = fopen(argv[1], "r");
  if (file == NULL)
  {
    printf("Cannot open file %s\n", argv[1]);
    return 1;
  }
  List tokenList;
  initList(&tokenList);

  tokenizeFile(&tokenList, file);

  printTokens(&tokenList);

  fclose(file);
  freeList(&tokenList);

  return 0;
}
