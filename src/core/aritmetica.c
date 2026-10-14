#include <gmp.h>

// FUNÇÕES DE GERAÇÃO DE CHAVES

int eh_primo(mpz_t a) {
    mpf_t raiz, a_float; // a_float é necessário por causa do mpf_sqrt
    mpf_init(raiz);
    mpf_init(a_float);

    mpf_set_z(a_float, a); // passando o valor de a para a_float
    mpf_sqrt(raiz, a_float); // calcula a raiz quadrada
    mpf_clear(a_float);

    if (mpz_cmp_ui(a, 1) <= 0) { //mpz_cmp é um metodo para comparar, nesse caso, avalia se a é menor que 1
        mpf_clear(raiz);
        return 0;
    }

    mpz_t b;
    mpz_init_set_ui(b, 2); // começa a partir de 2

    // O loop irá verificar divisibilidade até a raiz
    while (mpf_cmp_ui(raiz, mpz_get_ui(b)) >= 0) {
        if (mpz_divisible_p(a, b)) {
            mpf_clear(raiz);
            mpz_clear(b);
            return 0; // a é divisível por b, então não é primo
        }
        mpz_add_ui(b, b, 1); // b++;
    }

    mpf_clear(raiz);
    mpz_clear(b);
    
    return 1; // Se não encontrou divisores, a é primo
}

// função para calcular o MDC - basicamente, algoritmo de Euclides com gmp
void mdc(mpz_t resultado, const mpz_t a, const mpz_t b) {
    mpz_init(resultado);
    if (mpz_cmp_ui(b, 0) == 0) { // verifica se b é zero.
        mpz_set(resultado, a); // se sim, então a é o resultado do mdc.
        return;
    }

    mpz_t novo_b;
    mpz_init(novo_b);
    mpz_mod(novo_b, a, b); // novo_b = a % b;
    mdc(resultado, b, novo_b);
    mpz_clear(novo_b);
}

// função para calcular o inverso modular
void inverso_modular(mpz_t d, const mpz_t e, const mpz_t phi) {
    mpz_t m0, t, q, x0, x1, a;
    mpz_init(m0);
    mpz_init(t);
    mpz_init(q);
    mpz_init(x0);
    mpz_init(x1);
    mpz_init(a);

    mpz_set(m0, phi); // armazena phi original, tem que armazenar pq é um ponteiro
    mpz_set_ui(x0, 0);
    mpz_set_ui(x1, 1);
    mpz_set(a, e);

    // loop para encontrar o inverso modular
    while (mpz_cmp_ui(a, 1) > 0) {
        // q = a / m
        mpz_fdiv_q(q, a, m0);

        // t = m
        mpz_set(t, m0);

        // m = a % m
        mpz_mod(m0, a, m0);

        // a = t
        mpz_set(a, t);

        // t = x0
        mpz_set(t, x0);

        // x0 = x1 - q * x0
        mpz_mul(x0, q, x0);
        mpz_sub(x0, x1, x0);

        // x1 = t
        mpz_set(x1, t);
    }

    if (mpz_cmp_ui(x1, 0) < 0) { // compara se x1 é positivo
        mpz_add(x1, x1, phi); // modifica x1 para garantir que seja positivo, senão ele gera uma chave negativa
    }

    // passando o resultado para d
    mpz_set(d, x1);

    mpz_clear(m0);
    mpz_clear(t);
    mpz_clear(q);
    mpz_clear(x0);
    mpz_clear(x1);
    mpz_clear(a);
}

// FUNÇÃO DE CRIPTOGRAFIA E DESCRIPTOGRAFIA
void mod_exp(mpz_t result, const mpz_t base, const mpz_t exp, const mpz_t mod) {
    mpz_set_ui(result, 1); // result = 1

    mpz_t base_mod, exp_copy; // criando copia do expoente para alterar nossas variaveis, pois a lib trata variaveis como ponteiros.
    mpz_init(base_mod);
    mpz_init(exp_copy);
    mpz_set(base_mod, base);
    mpz_mod(base_mod, base_mod, mod); // base_mod = base % mod
    mpz_set(exp_copy, exp); // copia do expoente

    while (mpz_cmp_ui(exp_copy, 0) > 0) { // se exp > 0
        if (mpz_odd_p(exp_copy)) { // se exp_copy é ímpar
            mpz_mul(result, result, base_mod);
            mpz_mod(result, result, mod); // result = (result * base_mod) % mod
        }
        mpz_fdiv_q_2exp(exp_copy, exp_copy, 1); // exp_copy >>= 1
        mpz_mul(base_mod, base_mod, base_mod);
        mpz_mod(base_mod, base_mod, mod); // base_mod = (base_mod * base_mod) % mod
    }

    mpz_clear(base_mod);
    mpz_clear(exp_copy);
}
