/*
 * test_rsa.c — testes do núcleo RSA em C puro (sem Qt).
 *
 * Mini-harness: cada VERIFICA conta uma verificação; ao final o programa
 * imprime o resumo e sai com código 1 se alguma falhou.
 * Exemplo clássico usado: p = 61, q = 53, n = 3233, phi = 3120, e = 17, d = 2753.
 */
#include "rsa.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int total = 0;
static int falhas = 0;

#define VERIFICA(cond)                                                         \
    do {                                                                       \
        total++;                                                               \
        if (!(cond)) {                                                         \
            falhas++;                                                          \
            fprintf(stderr, "  FALHOU %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        }                                                                      \
    } while (0)

#define TEXTO_IGUAL(a, b) VERIFICA((a) != NULL && strcmp((a), (b)) == 0)

static void secao(const char *nome) {
    printf("- %s\n", nome);
}

/* ------------------------------------------------------------------------ */
/* Aritmética                                                               */
/* ------------------------------------------------------------------------ */

static int primo_ui(long valor) {
    mpz_t a;
    mpz_init_set_si(a, valor);
    int r = eh_primo(a);
    mpz_clear(a);
    return r;
}

static void testa_eh_primo(void) {
    secao("eh_primo");
    VERIFICA(primo_ui(0) == 0);
    VERIFICA(primo_ui(1) == 0);
    VERIFICA(primo_ui(-7) == 0); /* antes abortava em mpf_sqrt de número negativo */
    VERIFICA(primo_ui(2) == 1);
    VERIFICA(primo_ui(3) == 1);
    VERIFICA(primo_ui(53) == 1);
    VERIFICA(primo_ui(61) == 1);
    VERIFICA(primo_ui(7919) == 1);
    VERIFICA(primo_ui(4) == 0);
    VERIFICA(primo_ui(9) == 0);
    VERIFICA(primo_ui(91) == 0);    /* 7 * 13 */
    VERIFICA(primo_ui(3233) == 0);  /* 61 * 53 */
    VERIFICA(primo_ui(7921) == 0);  /* 89 * 89 */
}

static unsigned long mdc_ui(unsigned long a, unsigned long b) {
    mpz_t r, x, y;
    mpz_init(r);
    mpz_init_set_ui(x, a);
    mpz_init_set_ui(y, b);
    mdc(r, x, y);
    unsigned long v = mpz_get_ui(r);
    mpz_clears(r, x, y, NULL);
    return v;
}

static void testa_mdc(void) {
    secao("mdc");
    VERIFICA(mdc_ui(17, 3120) == 1);
    VERIFICA(mdc_ui(48, 18) == 6);
    VERIFICA(mdc_ui(18, 48) == 6);
    VERIFICA(mdc_ui(10, 0) == 10);
    VERIFICA(mdc_ui(0, 7) == 7);
    VERIFICA(mdc_ui(12, 3120) == 12);
}

static unsigned long inverso_ui(unsigned long e, unsigned long phi) {
    mpz_t d, x, y;
    mpz_init(d);
    mpz_init_set_ui(x, e);
    mpz_init_set_ui(y, phi);
    inverso_modular(d, x, y);
    unsigned long v = mpz_get_ui(d);
    mpz_clears(d, x, y, NULL);
    return v;
}

static void testa_inverso_modular(void) {
    secao("inverso_modular");
    VERIFICA(inverso_ui(17, 3120) == 2753);
    VERIFICA(inverso_ui(3, 11) == 4);
    VERIFICA(inverso_ui(7, 40) == 23);
    /* propriedade: e * d = 1 (mod phi) para vários e coprimos de 3120 */
    for (unsigned long e = 3; e < 200; e += 2) {
        if (mdc_ui(e, 3120) == 1) {
            unsigned long d = inverso_ui(e, 3120);
            VERIFICA(d < 3120 && (e * d) % 3120 == 1);
        }
    }
}

static unsigned long exp_ui(unsigned long b, unsigned long e, unsigned long m) {
    mpz_t r, x, y, z;
    mpz_init(r);
    mpz_init_set_ui(x, b);
    mpz_init_set_ui(y, e);
    mpz_init_set_ui(z, m);
    mod_exp(r, x, y, z);
    unsigned long v = mpz_get_ui(r);
    mpz_clears(r, x, y, z, NULL);
    return v;
}

static void testa_mod_exp(void) {
    secao("mod_exp");
    VERIFICA(exp_ui(65, 17, 3233) == 2790);
    VERIFICA(exp_ui(2790, 2753, 3233) == 65);
    VERIFICA(exp_ui(4, 13, 497) == 445);
    VERIFICA(exp_ui(5, 0, 13) == 1);
    /* confere contra mpz_powm da própria GMP */
    for (unsigned long b = 0; b < 300; b += 7) {
        for (unsigned long e = 0; e < 60; e += 11) {
            mpz_t esperado, x, y, m;
            mpz_init(esperado);
            mpz_init_set_ui(x, b);
            mpz_init_set_ui(y, e);
            mpz_init_set_ui(m, 3233);
            mpz_powm(esperado, x, y, m);
            VERIFICA(exp_ui(b, e, 3233) == mpz_get_ui(esperado));
            mpz_clears(esperado, x, y, m, NULL);
        }
    }
}

/* ------------------------------------------------------------------------ */
/* Operações RSA                                                            */
/* ------------------------------------------------------------------------ */

static void testa_calcular_modulo(void) {
    rsa_modulo m;
    secao("rsa_calcular_modulo");

    VERIFICA(rsa_calcular_modulo("61", "53", &m) == RSA_OK);
    TEXTO_IGUAL(m.p, "61");
    TEXTO_IGUAL(m.q, "53");
    TEXTO_IGUAL(m.n, "3233");
    TEXTO_IGUAL(m.phi, "3120");
    rsa_modulo_liberar(&m);
    VERIFICA(m.p == NULL && m.n == NULL);

    VERIFICA(rsa_calcular_modulo("4", "53", &m) == RSA_ERRO_NAO_PRIMOS);
    TEXTO_IGUAL(m.p, "4"); /* P e Q continuam disponíveis para o chaves.txt */
    TEXTO_IGUAL(m.q, "53");
    VERIFICA(m.n == NULL && m.phi == NULL);
    rsa_modulo_liberar(&m);

    VERIFICA(rsa_calcular_modulo("abc", "53", &m) == RSA_ERRO_NUMERO_INVALIDO);
    VERIFICA(m.p == NULL && m.q == NULL);
    rsa_modulo_liberar(&m);

    VERIFICA(rsa_calcular_modulo("", "53", &m) == RSA_ERRO_NUMERO_INVALIDO);
    rsa_modulo_liberar(&m);

    VERIFICA(rsa_calcular_modulo("-61", "53", &m) == RSA_ERRO_NAO_PRIMOS);
    rsa_modulo_liberar(&m);
}

static void testa_calcular_d(void) {
    char *e = NULL;
    char *d = NULL;
    secao("rsa_calcular_d");

    VERIFICA(rsa_calcular_d("17", "3120", &e, &d) == RSA_OK);
    TEXTO_IGUAL(e, "17");
    TEXTO_IGUAL(d, "2753");
    rsa_liberar(e);
    rsa_liberar(d);

    VERIFICA(rsa_calcular_d("12", "3120", &e, &d) == RSA_ERRO_NAO_COPRIMOS);
    VERIFICA(e == NULL && d == NULL);

    VERIFICA(rsa_calcular_d("x", "3120", &e, &d) == RSA_ERRO_NUMERO_INVALIDO);
    VERIFICA(rsa_calcular_d("17", "", &e, &d) == RSA_ERRO_NUMERO_INVALIDO);
}

static void testa_cifrar(void) {
    char *c = NULL;
    secao("rsa_cifrar");

    /* formato do arquivo original: cada número seguido de um espaço */
    VERIFICA(rsa_cifrar((const unsigned char *) "A", 1, "3233", "17", &c) == RSA_OK);
    TEXTO_IGUAL(c, "2790 ");
    rsa_liberar(c);

    VERIFICA(rsa_cifrar((const unsigned char *) "", 0, "3233", "17", &c) == RSA_OK);
    TEXTO_IGUAL(c, "");
    rsa_liberar(c);

    VERIFICA(rsa_cifrar((const unsigned char *) "A", 1, "", "17", &c) == RSA_ERRO_N_INVALIDO);
    VERIFICA(c == NULL);
    VERIFICA(rsa_cifrar((const unsigned char *) "A", 1, "0", "17", &c) == RSA_ERRO_N_INVALIDO);
    VERIFICA(rsa_cifrar((const unsigned char *) "A", 1, "3233", "e", &c) == RSA_ERRO_EXPOENTE_INVALIDO);
}

static void testa_decifrar(void) {
    unsigned char *m = NULL;
    size_t tamanho = 0;
    size_t invalidos = 0;
    secao("rsa_decifrar");

    VERIFICA(rsa_decifrar("2790 2790 ", "3233", "2753", &m, &tamanho, &invalidos) == RSA_OK);
    VERIFICA(tamanho == 2 && invalidos == 0 && memcmp(m, "AA", 3) == 0);
    rsa_liberar(m);

    /* espaços repetidos são ignorados, como no split(" ", SkipEmptyParts) */
    VERIFICA(rsa_decifrar("  2790   2790", "3233", "2753", &m, &tamanho, &invalidos) == RSA_OK);
    VERIFICA(tamanho == 2);
    rsa_liberar(m);

    /* número inválido é pulado e contado */
    VERIFICA(rsa_decifrar("2790 xyz 2790", "3233", "2753", &m, &tamanho, &invalidos) == RSA_OK);
    VERIFICA(tamanho == 2 && invalidos == 1 && memcmp(m, "AA", 2) == 0);
    rsa_liberar(m);

    /* m = 0 não vira caractere */
    VERIFICA(rsa_decifrar("0 2790", "3233", "2753", &m, &tamanho, &invalidos) == RSA_OK);
    VERIFICA(tamanho == 1 && m[0] == 'A');
    rsa_liberar(m);

    VERIFICA(rsa_decifrar("", "3233", "2753", &m, &tamanho, &invalidos) == RSA_OK);
    VERIFICA(tamanho == 0 && m != NULL && m[0] == '\0');
    rsa_liberar(m);

    VERIFICA(rsa_decifrar("2790", "abc", "2753", &m, &tamanho, &invalidos) == RSA_ERRO_N_INVALIDO);
    VERIFICA(m == NULL);
    VERIFICA(rsa_decifrar("2790", "3233", "", &m, &tamanho, &invalidos) == RSA_ERRO_EXPOENTE_INVALIDO);
}

static void ida_e_volta(const unsigned char *texto, size_t tamanho,
                        const char *n, const char *e, const char *d) {
    char *cifrado = NULL;
    unsigned char *volta = NULL;
    size_t tamanho_volta = 0;
    size_t invalidos = 0;

    VERIFICA(rsa_cifrar(texto, tamanho, n, e, &cifrado) == RSA_OK);
    VERIFICA(rsa_decifrar(cifrado, n, d, &volta, &tamanho_volta, &invalidos) == RSA_OK);
    VERIFICA(invalidos == 0);
    VERIFICA(tamanho_volta == tamanho && memcmp(volta, texto, tamanho) == 0);
    rsa_liberar(cifrado);
    rsa_liberar(volta);
}

static void testa_ida_e_volta(void) {
    secao("ida e volta (cifrar e decifrar)");

    const char *ascii = "Matematica Discreta - IC/UFAL 2024!";
    ida_e_volta((const unsigned char *) ascii, strlen(ascii), "3233", "17", "2753");

    /* acentos: a interface cifra os bytes UTF-8 do texto */
    const char *acentos = "Olá, Matemática Discreta! Ação, coração, pão e maçã.";
    ida_e_volta((const unsigned char *) acentos, strlen(acentos), "3233", "17", "2753");

    /* ASCII estendido: todos os bytes de 1 a 255 (0 é descartado na volta, como no original) */
    unsigned char todos[255];
    for (int i = 0; i < 255; i++) {
        todos[i] = (unsigned char) (i + 1);
    }
    ida_e_volta(todos, sizeof todos, "3233", "17", "2753");

    /* chaves geradas pelo próprio núcleo, com primos maiores */
    rsa_modulo mod;
    char *e = NULL;
    char *d = NULL;
    VERIFICA(rsa_calcular_modulo("7919", "104729", &mod) == RSA_OK);
    VERIFICA(rsa_calcular_d("65537", mod.phi, &e, &d) == RSA_OK);
    ida_e_volta((const unsigned char *) acentos, strlen(acentos), mod.n, e, d);
    rsa_liberar(e);
    rsa_liberar(d);
    rsa_modulo_liberar(&mod);
}

int main(void) {
    printf("Testes do núcleo RSA\n");

    testa_eh_primo();
    testa_mdc();
    testa_inverso_modular();
    testa_mod_exp();
    testa_calcular_modulo();
    testa_calcular_d();
    testa_cifrar();
    testa_decifrar();
    testa_ida_e_volta();

    printf("%d verificações, %d falhas\n", total, falhas);
    return falhas == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
