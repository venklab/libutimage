
CC = gcc
CFLAGS = -std=c11 -fpic -Wall -Wextra -O3
LDFLAGS = -lm -shared
RM = rm -f

CLEANEXTS   = o so
TARGET_LIB = libutimage.so
SRCS = grow_opened.c colorslice.c mask.c intensity.c
OBJS = $(SRCS:.c=.o)

.PHONY: all
all: ${TARGET_LIB}


$(TARGET_LIB): $(OBJS)
	$(CC) ${LDFLAGS} -o $@ $^

$(SRCS:.c=.d):%.d:%.c
	$(CC) $(CFLAGS) -MM $< >$@

include $(SRCS:.c=.d)

.PHONY: clean
clean:
	-${RM} ${TARGET_LIB} ${OBJS} $(SRCS:.c=.d)

