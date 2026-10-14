#ifndef CRIPTO_RSA_H
#define CRIPTO_RSA_H

#include <gmp.h>

#ifdef __cplusplus
extern "C" {
#endif

int eh_primo(mpz_t a);

void mdc(mpz_t resultado, const mpz_t a, const mpz_t b);

void inverso_modular(mpz_t d, const mpz_t e, const mpz_t phi);

void mod_exp(mpz_t result, const mpz_t base, const mpz_t exp, const mpz_t mod);

#ifdef __cplusplus
}
#endif

#endif
