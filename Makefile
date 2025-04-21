# 编译器
CC = gcc

# 编译选项
CFLAGS = -Wall -Wextra -std=c11

# 目标文件
TARGET = syntax_tree

# 源文件
SRCS = syntax_tree.c parse.c scanner.c

# 生成的对象文件
OBJS = $(SRCS:.c=.o)

# 默认目标
all: $(TARGET)

# 链接目标文件生成可执行文件
$(TARGET): $(OBJS)
    $(CC) $(CFLAGS) -o $@ $^

# 编译源文件生成对象文件
%.o: %.c
    $(CC) $(CFLAGS) -c $< -o $@

# 清理生成的文件
clean:
    rm -f $(OBJS) $(TARGET)

# 伪目标，不生成文件
.PHONY: all clean