# Núcleo RSA (C + GMP): biblioteca estática e testes.
#
#   make            compila build/nucleo/libcripto.a
#   make test       compila e roda tests/test_rsa.c
#   make clean      apaga build/nucleo
#
# Para usar outra GMP (por exemplo, uma versão que você mesmo compilou):
#   make test GMP_PREFIX=/caminho/da/gmp     (usa <prefixo>/include e <prefixo>/lib)
#
# A interface Qt é compilada pelo CMakeLists.txt (veja o README).

CC     ?= gcc
AR     ?= ar
CFLAGS ?= -O2 -g
# flags obrigatórias ficam fora de CFLAGS/CPPFLAGS: "make CFLAGS=..." não as remove
RSA_CFLAGS := -std=c11 -Wall -Wextra
RSA_CPPFLAGS := -Iinclude
LDLIBS += -lgmp

ifdef GMP_PREFIX
RSA_CPPFLAGS += -I$(GMP_PREFIX)/include
LDFLAGS  += -L$(GMP_PREFIX)/lib
endif

BUILD := build/nucleo
OBJS  := $(BUILD)/aritmetica.o $(BUILD)/rsa.o
LIB   := $(BUILD)/libcripto.a
TESTE := $(BUILD)/test_rsa

.PHONY: all test clean

all: $(LIB)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: src/core/%.c include/rsa.h | $(BUILD)
	$(CC) $(RSA_CPPFLAGS) $(CPPFLAGS) $(RSA_CFLAGS) $(CFLAGS) -c $< -o $@

$(LIB): $(OBJS)
	$(AR) rcs $@ $^

$(TESTE): tests/test_rsa.c include/rsa.h $(LIB) | $(BUILD)
	$(CC) $(RSA_CPPFLAGS) $(CPPFLAGS) $(RSA_CFLAGS) $(CFLAGS) $< $(LIB) $(LDFLAGS) $(LDLIBS) -o $@

test: $(TESTE)
	./$(TESTE)

clean:
	rm -rf $(BUILD)
