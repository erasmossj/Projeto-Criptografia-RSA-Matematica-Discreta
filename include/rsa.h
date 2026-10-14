/*
 * rsa.h — API pública do núcleo de criptografia RSA (C + GMP).
 *
 * O núcleo não conhece a interface: não mostra mensagens, não abre janelas
 * e não grava arquivos. Toda falha volta como um rsa_status explícito, e
 * quem chama (a interface Qt ou os testes) decide o que fazer com ela.
 *
 * Camadas:
 *   src/core/aritmetica.c  aritmética modular (primalidade, MDC, inverso, potência)
 *   src/core/rsa.c         operações RSA de alto nível sobre texto em base 10
 */
#ifndef RSA_H
#define RSA_H

#include <stddef.h>
#include <gmp.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------ */
/* Aritmética modular (src/core/aritmetica.c)                               */
/* ------------------------------------------------------------------------ */

/* Retorna 1 se a for primo e 0 caso contrário (divisão por tentativa até a raiz). */
int eh_primo(mpz_t a);

/* resultado = MDC(a, b) pelo algoritmo de Euclides. resultado já deve estar inicializado. */
void mdc(mpz_t resultado, const mpz_t a, const mpz_t b);

/* d = inverso de e módulo phi (Euclides estendido). Exige MDC(e, phi) = 1. */
void inverso_modular(mpz_t d, const mpz_t e, const mpz_t phi);

/* result = base^exp mod mod (exponenciação rápida). */
void mod_exp(mpz_t result, const mpz_t base, const mpz_t exp, const mpz_t mod);

/* ------------------------------------------------------------------------ */
/* Operações RSA (src/core/rsa.c)                                           */
/* ------------------------------------------------------------------------ */

typedef enum {
    RSA_OK = 0,
    RSA_ERRO_NUMERO_INVALIDO,   /* P, Q, E ou Phi não é um inteiro em base 10   */
    RSA_ERRO_NAO_PRIMOS,        /* P ou Q não é primo                          */
    RSA_ERRO_NAO_COPRIMOS,      /* MDC(E, Phi) diferente de 1                  */
    RSA_ERRO_N_INVALIDO,        /* N não é inteiro ou é zero                   */
    RSA_ERRO_EXPOENTE_INVALIDO, /* E (cifrar) ou D (decifrar) não é inteiro    */
    RSA_ERRO_MEMORIA            /* falha de alocação                           */
} rsa_status;

/* Descrição curta, em português, de um rsa_status. */
const char *rsa_descricao_status(rsa_status status);

/* Libera qualquer texto ou buffer devolvido pelo núcleo. Aceita NULL. */
void rsa_liberar(void *ptr);

/* Resultado da etapa "P e Q" da geração de chaves. Todos os campos são
 * textos em base 10 alocados pelo núcleo (ou NULL quando não se aplicam). */
typedef struct {
    char *p;    /* P normalizado; preenchido sempre que P e Q forem inteiros  */
    char *q;    /* Q normalizado; idem                                         */
    char *n;    /* N = P * Q; só quando P e Q são primos                       */
    char *phi;  /* Phi = (P - 1) * (Q - 1); só quando P e Q são primos         */
} rsa_modulo;

/* Valida P e Q e calcula N e Phi.
 * Retorna RSA_OK, RSA_ERRO_NUMERO_INVALIDO, RSA_ERRO_NAO_PRIMOS ou RSA_ERRO_MEMORIA.
 * Em RSA_ERRO_NAO_PRIMOS, saida->p e saida->q continuam preenchidos.
 * Libere com rsa_modulo_liberar em qualquer caso. */
rsa_status rsa_calcular_modulo(const char *p, const char *q, rsa_modulo *saida);
void rsa_modulo_liberar(rsa_modulo *modulo);

/* Verifica se E é coprimo de Phi e calcula D = E^-1 mod Phi.
 * Em RSA_OK, *e_saida recebe E normalizado e *d_saida recebe D (liberar com rsa_liberar).
 * Retorna RSA_OK, RSA_ERRO_NUMERO_INVALIDO, RSA_ERRO_NAO_COPRIMOS ou RSA_ERRO_MEMORIA. */
rsa_status rsa_calcular_d(const char *e, const char *phi, char **e_saida, char **d_saida);

/* Cifra a mensagem byte a byte: c = m^E mod N para cada byte m.
 * *cifrado recebe os números em base 10, cada um seguido de um espaço
 * (ex.: "2790 1307 "). Mensagem vazia gera texto vazio.
 * Retorna RSA_OK, RSA_ERRO_N_INVALIDO, RSA_ERRO_EXPOENTE_INVALIDO ou RSA_ERRO_MEMORIA. */
rsa_status rsa_cifrar(const unsigned char *mensagem, size_t tamanho,
                      const char *n, const char *e, char **cifrado);

/* Decifra números separados por espaço: m = c^D mod N para cada número c.
 * Cada m maior que zero vira um byte de *mensagem (terminada em '\0';
 * *tamanho não conta o terminador). Números inválidos são pulados e
 * contados em *invalidos.
 * Retorna RSA_OK, RSA_ERRO_N_INVALIDO, RSA_ERRO_EXPOENTE_INVALIDO ou RSA_ERRO_MEMORIA. */
rsa_status rsa_decifrar(const char *cifrado, const char *n, const char *d,
                        unsigned char **mensagem, size_t *tamanho, size_t *invalidos);

#ifdef __cplusplus
}
#endif

#endif /* RSA_H */
