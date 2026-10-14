/*
 * rsa.c — operações RSA de alto nível do núcleo.
 *
 * Estas rotinas estavam misturadas com a interface em mainwindow.cpp
 * (slots Primos_p_q, Coprimos_E_Phi, criptografar e descriptografar).
 * Aqui elas recebem e devolvem texto em base 10, sem QMessageBox, sem
 * printf e sem arquivos: os erros voltam como rsa_status.
 */
#include "rsa.h"

#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------------ */
/* Auxiliares                                                               */
/* ------------------------------------------------------------------------ */

/* Lê um inteiro em base 10 (mesma regra do original: mpz_set_str ignora espaços). */
static int ler_inteiro(mpz_t destino, const char *texto) {
    if (texto == NULL) {
        return -1;
    }
    return mpz_set_str(destino, texto, 10);
}

/* Converte para texto em base 10 num buffer alocado com malloc. */
static char *para_texto(const mpz_t x) {
    size_t tamanho = mpz_sizeinbase(x, 10) + 2; /* sinal e '\0' */
    char *texto = malloc(tamanho);
    if (texto != NULL) {
        mpz_get_str(texto, 10, x);
    }
    return texto;
}

typedef struct {
    char *dados;
    size_t usado;
    size_t capacidade;
} texto_dinamico;

static int texto_acrescentar(texto_dinamico *t, const char *trecho, size_t n) {
    if (t->usado + n + 1 > t->capacidade) {
        size_t nova = t->capacidade ? t->capacidade : 64;
        while (nova < t->usado + n + 1) {
            nova *= 2;
        }
        char *novo = realloc(t->dados, nova);
        if (novo == NULL) {
            return -1;
        }
        t->dados = novo;
        t->capacidade = nova;
    }
    memcpy(t->dados + t->usado, trecho, n);
    t->usado += n;
    t->dados[t->usado] = '\0';
    return 0;
}

/* ------------------------------------------------------------------------ */
/* API pública                                                              */
/* ------------------------------------------------------------------------ */

const char *rsa_descricao_status(rsa_status status) {
    switch (status) {
    case RSA_OK:                     return "Operação concluída.";
    case RSA_ERRO_NUMERO_INVALIDO:   return "Valor informado não é um número inteiro.";
    case RSA_ERRO_NAO_PRIMOS:        return "P e Q precisam ser primos.";
    case RSA_ERRO_NAO_COPRIMOS:      return "E e Phi precisam ser coprimos.";
    case RSA_ERRO_N_INVALIDO:        return "Chave inválida (N).";
    case RSA_ERRO_EXPOENTE_INVALIDO: return "Chave inválida (expoente).";
    case RSA_ERRO_MEMORIA:           return "Memória insuficiente.";
    }
    return "Erro desconhecido.";
}

void rsa_liberar(void *ptr) {
    free(ptr);
}

void rsa_modulo_liberar(rsa_modulo *modulo) {
    if (modulo == NULL) {
        return;
    }
    free(modulo->p);
    free(modulo->q);
    free(modulo->n);
    free(modulo->phi);
    modulo->p = modulo->q = modulo->n = modulo->phi = NULL;
}

rsa_status rsa_calcular_modulo(const char *p_texto, const char *q_texto, rsa_modulo *saida) {
    rsa_status status = RSA_OK;
    mpz_t p, q, n, phi;

    memset(saida, 0, sizeof *saida);
    mpz_inits(p, q, n, phi, NULL);

    if (ler_inteiro(p, p_texto) != 0 || ler_inteiro(q, q_texto) != 0) {
        status = RSA_ERRO_NUMERO_INVALIDO;
        goto fim;
    }

    saida->p = para_texto(p);
    saida->q = para_texto(q);
    if (saida->p == NULL || saida->q == NULL) {
        status = RSA_ERRO_MEMORIA;
        goto fim;
    }

    if (!(eh_primo(p) && eh_primo(q))) { // se não forem primos
        status = RSA_ERRO_NAO_PRIMOS;
        goto fim;
    }

    mpz_mul(n, p, q);     // n = p * q
    mpz_sub_ui(p, p, 1);  // p = p - 1
    mpz_sub_ui(q, q, 1);  // q = q - 1
    mpz_mul(phi, p, q);   // phi = (p - 1) * (q - 1)

    saida->n = para_texto(n);
    saida->phi = para_texto(phi);
    if (saida->n == NULL || saida->phi == NULL) {
        status = RSA_ERRO_MEMORIA;
    }

fim:
    mpz_clears(p, q, n, phi, NULL);
    if (status == RSA_ERRO_MEMORIA) {
        rsa_modulo_liberar(saida);
    }
    return status;
}

rsa_status rsa_calcular_d(const char *e_texto, const char *phi_texto, char **e_saida, char **d_saida) {
    rsa_status status = RSA_OK;
    mpz_t e, phi, d, resultado_mdc;

    *e_saida = NULL;
    *d_saida = NULL;
    mpz_inits(e, phi, d, resultado_mdc, NULL);

    if (ler_inteiro(e, e_texto) != 0 || ler_inteiro(phi, phi_texto) != 0) {
        status = RSA_ERRO_NUMERO_INVALIDO;
        goto fim;
    }

    mdc(resultado_mdc, e, phi);
    if (mpz_cmp_ui(resultado_mdc, 1) != 0) {
        status = RSA_ERRO_NAO_COPRIMOS;
        goto fim;
    }

    inverso_modular(d, e, phi);

    *e_saida = para_texto(e);
    *d_saida = para_texto(d);
    if (*e_saida == NULL || *d_saida == NULL) {
        free(*e_saida);
        free(*d_saida);
        *e_saida = NULL;
        *d_saida = NULL;
        status = RSA_ERRO_MEMORIA;
    }

fim:
    mpz_clears(e, phi, d, resultado_mdc, NULL);
    return status;
}

rsa_status rsa_cifrar(const unsigned char *mensagem, size_t tamanho,
                      const char *n_texto, const char *e_texto, char **cifrado) {
    rsa_status status = RSA_OK;
    texto_dinamico saida = { NULL, 0, 0 };
    mpz_t n, e, m, c;

    *cifrado = NULL;
    mpz_inits(n, e, m, c, NULL);

    // N = 0 provocaria divisão por zero em mod_exp
    if (ler_inteiro(n, n_texto) != 0 || mpz_sgn(n) == 0) {
        status = RSA_ERRO_N_INVALIDO;
        goto fim;
    }
    if (ler_inteiro(e, e_texto) != 0) {
        status = RSA_ERRO_EXPOENTE_INVALIDO;
        goto fim;
    }

    if (texto_acrescentar(&saida, "", 0) != 0) { // garante texto vazio para mensagem vazia
        status = RSA_ERRO_MEMORIA;
        goto fim;
    }

    // cada byte da mensagem é cifrado separadamente: c = m^e mod n
    for (size_t i = 0; i < tamanho; i++) {
        mpz_set_ui(m, mensagem[i]);
        mod_exp(c, m, e, n);

        char *c_str = para_texto(c);
        if (c_str == NULL
            || texto_acrescentar(&saida, c_str, strlen(c_str)) != 0
            || texto_acrescentar(&saida, " ", 1) != 0) {
            free(c_str);
            status = RSA_ERRO_MEMORIA;
            goto fim;
        }
        free(c_str);
    }

    *cifrado = saida.dados;
    saida.dados = NULL;

fim:
    free(saida.dados);
    mpz_clears(n, e, m, c, NULL);
    return status;
}

rsa_status rsa_decifrar(const char *cifrado, const char *n_texto, const char *d_texto,
                        unsigned char **mensagem, size_t *tamanho, size_t *invalidos) {
    rsa_status status = RSA_OK;
    char *copia = NULL;
    unsigned char *saida = NULL;
    size_t usados = 0;
    mpz_t n, d, c, m;

    *mensagem = NULL;
    *tamanho = 0;
    *invalidos = 0;
    mpz_inits(n, d, c, m, NULL);

    if (ler_inteiro(n, n_texto) != 0 || mpz_sgn(n) == 0) {
        status = RSA_ERRO_N_INVALIDO;
        goto fim;
    }
    if (ler_inteiro(d, d_texto) != 0) {
        status = RSA_ERRO_EXPOENTE_INVALIDO;
        goto fim;
    }

    if (cifrado == NULL) {
        cifrado = "";
    }
    size_t comprimento = strlen(cifrado);
    copia = malloc(comprimento + 1);
    saida = malloc(comprimento + 1); // nunca há mais números do que caracteres
    if (copia == NULL || saida == NULL) {
        status = RSA_ERRO_MEMORIA;
        goto fim;
    }
    memcpy(copia, cifrado, comprimento + 1);

    // os números vêm separados por espaço (como o split(" ") da versão original)
    size_t i = 0;
    while (i < comprimento) {
        while (i < comprimento && copia[i] == ' ') {
            i++;
        }
        if (i >= comprimento) {
            break;
        }
        size_t inicio = i;
        while (i < comprimento && copia[i] != ' ') {
            i++;
        }
        copia[i] = '\0';
        i++;

        if (mpz_set_str(c, copia + inicio, 10) != 0) {
            (*invalidos)++; // valor inválido: pula e segue para o próximo
            continue;
        }

        mod_exp(m, c, d, n); // m = c^d mod n

        if (mpz_cmp_ui(m, 0) > 0) { // zero não vira caractere
            saida[usados++] = (unsigned char) mpz_get_ui(m);
        }
    }

    saida[usados] = '\0';
    *mensagem = saida;
    *tamanho = usados;
    saida = NULL;

fim:
    free(copia);
    free(saida);
    mpz_clears(n, d, c, m, NULL);
    return status;
}
