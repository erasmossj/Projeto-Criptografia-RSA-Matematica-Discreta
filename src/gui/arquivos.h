// arquivos.h — gravação dos arquivos .txt gerados pelo programa.
//
// Fica na camada de interface (e não no núcleo) porque o núcleo só faz
// contas; e fica fora da MainWindow para que ela apenas orquestre:
// lê os campos, chama o núcleo, grava o resultado e mostra a resposta.
// Os nomes e o formato são os mesmos da versão original, sempre relativos
// à pasta em que o programa foi aberto.
#ifndef ARQUIVOS_H
#define ARQUIVOS_H

#include <QByteArray>

namespace arquivos {

inline constexpr const char *kMensagemCriptografada = "mensagem_criptografada.txt";
inline constexpr const char *kMensagemDescriptografada = "mensagem_descriptografada.txt";
inline constexpr const char *kChaves = "chaves.txt";

// Sobrescreve mensagem_criptografada.txt com os números cifrados ("c1 c2 ... ").
bool salvarMensagemCriptografada(const QByteArray &cifrado);

// Sobrescreve mensagem_descriptografada.txt com os bytes decifrados.
bool salvarMensagemDescriptografada(const QByteArray &mensagem);

// Recria chaves.txt com "P: ...", "Q: ..." e, se n não for vazio, "N: ...".
bool iniciarChaves(const QByteArray &p, const QByteArray &q, const QByteArray &n);

// Acrescenta "E: ..." e "D: ..." ao final de chaves.txt.
bool anexarChavesED(const QByteArray &e, const QByteArray &d);

} // namespace arquivos

#endif // ARQUIVOS_H
