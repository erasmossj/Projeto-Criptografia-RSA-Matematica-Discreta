# Criptografia RSA — núcleo C/GMP + interface Qt 6 Widgets.
#
#   cmake -B build
#   cmake --build build
#
# Opções:
#   -DGMP_ROOT=<prefixo>          procura a GMP em <prefixo>/include e <prefixo>/lib
#   -DGMP_LIBRARY=<arquivo>       usa exatamente esta biblioteca GMP (.a, .so, .dll.a)
#   -DRSA_INTERFACE=OFF           compila só o núcleo e os testes (sem Qt)
#   -DRSA_TESTES=OFF              não compila os testes do núcleo
cmake_minimum_required(VERSION 3.16)

project(CriptografiaRSA VERSION 1.1.0 LANGUAGES C CXX)

option(RSA_INTERFACE "Compila a interface Qt 6" ON)
option(RSA_TESTES "Compila os testes do núcleo (ctest)" ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- GMP ---------------------------------------------------------------------
find_path(GMP_INCLUDE_DIR gmp.h HINTS "${GMP_ROOT}/include")
find_library(GMP_LIBRARY NAMES gmp libgmp HINTS "${GMP_ROOT}/lib")
if(NOT GMP_INCLUDE_DIR OR NOT GMP_LIBRARY)
    message(FATAL_ERROR
        "GMP não encontrada. Instale libgmp-dev (Debian/Ubuntu) ou "
        "mingw-w64-ucrt-x86_64-gmp (MSYS2), ou informe -DGMP_ROOT=<prefixo>.")
endif()
message(STATUS "GMP: ${GMP_LIBRARY}")

# --- Núcleo (C + GMP) ---------------------------------------------------------
add_library(cripto STATIC
    src/core/aritmetica.c
    src/core/rsa.c
)
target_include_directories(cripto PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${GMP_INCLUDE_DIR}"
)
target_link_libraries(cripto PUBLIC "${GMP_LIBRARY}")
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(cripto PRIVATE -Wall -Wextra)
endif()

# --- Testes do núcleo ----------------------------------------------------------
if(RSA_TESTES)
    enable_testing()
    add_executable(test_rsa tests/test_rsa.c)
    target_link_libraries(test_rsa PRIVATE cripto)
    add_test(NAME nucleo COMMAND test_rsa)
endif()

# --- Interface (Qt 6 Widgets) ------------------------------------------------
if(RSA_INTERFACE)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)

    set(FONTES_INTERFACE
        src/gui/main.cpp
        src/gui/mainwindow.h
        src/gui/mainwindow.cpp
        src/gui/mainwindow.ui
        src/gui/arquivos.h
        src/gui/arquivos.cpp
        assets/recursos.qrc
    )

    if(WIN32)
        # ícone do executável no Explorer do Windows
        set(ICONE_RC "${CMAKE_CURRENT_BINARY_DIR}/cripto_rsa.rc")
        file(WRITE "${ICONE_RC}"
            "IDI_ICON1 ICON \"${CMAKE_CURRENT_SOURCE_DIR}/assets/cripto_rsa.ico\"\n")
        list(APPEND FONTES_INTERFACE "${ICONE_RC}")
    endif()

    add_executable(cripto_rsa ${FONTES_INTERFACE})
    set_target_properties(cripto_rsa PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        WIN32_EXECUTABLE ON
    )
    target_link_libraries(cripto_rsa PRIVATE cripto Qt6::Widgets)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(cripto_rsa PRIVATE -Wall -Wextra)
    endif()
endif()
