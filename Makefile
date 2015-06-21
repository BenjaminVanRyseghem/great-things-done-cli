CC=gcc
LIBS=
LIBS_DIR=

NAME=gtd

ifeq ($(OS),Windows_NT)
#     ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
#         CCFLAGS += -D AMD64
#     endif
# I have no clue what to do on Windows
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
    endif
    ifeq ($(UNAME_S),Darwin)
        LIBS += /usr/local/opt/argp-standalone/lib/libargp.a
        LIBS_DIR += -L/usr/local/opt/argp-standalone/lib
    endif
endif

all: $(NAME)

$(NAME): $(NAME).o
	$(CC) $(NAME).o -o $(NAME) $(LIBS_DIR) $(LIBS)

$(NAME).o: $(NAME).c
	$(CC) -c $(NAME).c

clean:
	rm *.o $(NAME)
	