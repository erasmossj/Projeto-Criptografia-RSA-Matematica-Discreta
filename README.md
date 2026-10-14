<p align = "center">
<img src="https://ufal.br/ufal/resolveuid/458d5fb68aa7459bb9ec4ac2ce59c6b4/@@images/image/mini" alt="Logo da UFAL" width="150">
</p>

<h1 align = "center">
<b>Criptografia RSA</b>
</h1>

Programa de criptografia baseado no algoritmo RSA, desenvolvido pelos autores para a disciplina de Matemática Discreta, ministrada pelo professor Bruno Pimentel no Instituto de Computação da UFAL (IC/UFAL). Ele gera as chaves a partir de dois primos P e Q e criptografa/descriptografa mensagens de texto.

## Download

Para usar o programa no Windows não é preciso compilar nada:

1. Baixe o pacote **[cripto_rsa-windows-x64.zip](https://github.com/erasmossj/Projeto-Criptografia-RSA-Matematica-Discreta/releases/download/v.1.0.2/cripto_rsa-windows-x64.zip)** (release [v.1.0.2](https://github.com/erasmossj/Projeto-Criptografia-RSA-Matematica-Discreta/releases/tag/v.1.0.2)).
2. Extraia o zip. Todos os arquivos precisam ficar juntos na pasta `cripto_rsa`.
3. Execute `cripto_rsa.exe`.

Os arquivos `chaves.txt`, `mensagem_criptografada.txt` e `mensagem_descriptografada.txt` são gravados na pasta em que o programa foi aberto.

Em caso de dúvidas, você pode seguir este manual de instruções: [Manual de Instruções](https://docs.google.com/document/d/1v1mLUwItjrb9E2OQbRLOgDU-GP0jsxHTi8m_-9Un1uA/)

> O código-fonte não guarda mais executáveis nem DLLs: o pacote do Windows fica na página de releases.

## Arquitetura

O projeto é dividido em duas camadas, e a dependência vai num sentido só: a interface conhece o núcleo, o núcleo não conhece a interface.

```
┌──────────────────────────────────────────────┐
│ Interface — C++ / Qt 6 Widgets  (src/gui)    │
│  MainWindow: lê os campos, chama o núcleo e  │
│  mostra o resultado ou o erro                │
│  arquivos: grava chaves.txt e mensagens .txt │
└──────────────────────┬───────────────────────┘
                       │ include/rsa.h  (API em C, extern "C")
┌──────────────────────▼───────────────────────┐
│ Núcleo — C / GMP  (src/core)                 │
│  rsa.c: gerar/validar chaves, cifrar,        │
│         decifrar; erros como rsa_status      │
│  aritmetica.c: primo, MDC, inverso modular,  │
│                exponenciação modular         │
└──────────────────────────────────────────────┘
```

O núcleo não mostra mensagens, não abre janelas e não grava arquivos. Por isso ele é testado em C puro, sem Qt.

```
.
├── include/
│   └── rsa.h                 API pública do núcleo
├── src/
│   ├── core/
│   │   ├── aritmetica.c      eh_primo, mdc, inverso_modular, mod_exp
│   │   └── rsa.c             chaves (P, Q, N, Phi, E, D), cifrar e decifrar
│   └── gui/
│       ├── main.cpp
│       ├── mainwindow.h
│       ├── mainwindow.cpp
│       ├── mainwindow.ui     tela (Qt Designer)
│       ├── arquivos.h
│       └── arquivos.cpp      gravação dos .txt
├── tests/
│   └── test_rsa.c            testes do núcleo
├── assets/
│   ├── cripto_rsa.ico
│   └── recursos.qrc          ícone embutido no executável
├── Makefile                  núcleo + testes
├── CMakeLists.txt            núcleo + testes + interface Qt
└── .github/workflows/ci.yml
```

> **Sobre a tela:** os arquivos `mainwindow.h` e `mainwindow.ui` da versão original nunca foram versionados. Eles foram recriados a partir do `mainwindow.cpp`, com os mesmos widgets, nomes e comportamento. O visual pode ser diferente do executável v1.0.2.

## Como compilar

### Dependências

| Parte | Precisa de |
|-------|-----------|
| Núcleo e testes | compilador C (gcc ou clang), `make`, GMP |
| Interface | compilador C++17, CMake ≥ 3.16, Qt 6 (Widgets), GMP |

- **Debian/Ubuntu:** `sudo apt install build-essential cmake libgmp-dev qt6-base-dev libgl-dev`
- **Windows (MSYS2, ambiente UCRT64):** `pacman -S make mingw-w64-ucrt-x86_64-{gcc,cmake,ninja,qt6-base,gmp}`

### Núcleo e testes (Makefile)

```
make          # gera build/nucleo/libcripto.a
make test     # compila e roda tests/test_rsa.c
make clean
```

### Interface (CMake + Qt 6)

```
cmake -B build
cmake --build build
ctest --test-dir build     # mesmos testes do núcleo
```

O executável fica em `build/cripto_rsa` (ou `build/cripto_rsa.exe` no Windows). Para distribuir no Windows, copie as DLLs do Qt com `windeployqt build/cripto_rsa.exe`.

O CI (GitHub Actions) roda os testes do núcleo e compila a interface no Ubuntu e no Windows (MSYS2) a cada push e pull request.

## Autores:
| Nome                                   | Número de Matrícula | E-mail                  | Curso               |
|----------------------------------------|---------------------|-------------------------|---------------------|
| Erasmo da Silva Sá Junior              | 202407358           | essj@ic.ufal.br        | Ciência da Computação |
| Artur Ferreira Marques da Silva        | 202437820           | afms@ic.ufal.br        | Ciência da Computação |
| Caio Henrique Gavazza Patriota         | 202407231           | chgp@ic.ufal.br        | Ciência da Computação |
| José Riquelme Teixeira da Silva        | 202407400           | jrts@ic.ufal.br        | Ciência da Computação |
| Daniel de Araujo Albuquerque           | 202407311           | daa@ic.ufal.br         | Ciência da Computação |
| Victor André Lopes Brasileiro          | 202407269           | valb1@ic.ufal.br       | Ciência da Computação |
| Pablo Levy Fernandes Alcântara         | 202407429           | plfa1@ic.ufal.br       | Ciência da Computação |

## Contribuições Especiais

Gostaríamos de agradecer especialmente aos colaboradores que contribuíram para o desenvolvimento do design visual do ícone do nosso projeto!

- 🎨 **José Avelino Freire Júnior (FDA/UFAL)** – Contribuiu com o design do ícone da aplicação, trazendo uma identidade visual moderna e intuitiva.

Essas contribuições foram fundamentais para tornar nosso projeto mais familiar ao usuário final!

## Licença

Este projeto é licenciado pelos autores sob a **Licença MIT** (veja [LICENSE](LICENSE)).

### Bibliotecas e Dependências

- **Qt Community Edition (Open Source)**: licenciado sob a [LGPL](https://www.qt.io/licensing/). Consulte o link para mais detalhes.
- **GMP (GNU MP Library)**: licenciada sob a [LGPL v3](https://gmplib.org/). O executável da release v.1.0.2 usa uma versão **estática** da GMP.

### Substituindo a GMP (LGPL)

A LGPL garante a quem usa o programa o direito de trocar a GMP por outra versão. Como o código-fonte completo está neste repositório, basta recompilar apontando para a GMP desejada:

1. Obtenha o código-fonte da GMP em [https://gmplib.org/](https://gmplib.org/) e compile/instale-a (por exemplo, em `/opt/gmp`).
2. Recompile o projeto apontando para ela:
   - Núcleo e testes: `make test GMP_PREFIX=/opt/gmp`
   - Interface: `cmake -B build -DGMP_ROOT=/opt/gmp` (ou `-DGMP_LIBRARY=/caminho/libgmp.a -DGMP_INCLUDE_DIR=/caminho/include`) e `cmake --build build`.

Se você redistribuir o programa com uma GMP modificada, inclua o código-fonte dessa GMP ou informe claramente onde obtê-lo, junto com esta nota sobre a LGPL. O arquivo de licença deve acompanhar qualquer redistribuição do código-fonte.

Se tiver dúvidas sobre os termos da licença ou sobre a substituição das dependências, consulte os links da seção "Bibliotecas e Dependências".
