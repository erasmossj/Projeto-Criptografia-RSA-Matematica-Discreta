#include "arquivos.h"

#include <QFile>

namespace {

// QIODevice::Text reproduz o modo texto do fopen original ("w"/"a"):
// no Windows, '\n' é gravado como "\r\n".
bool gravar(const char *nome, const QByteArray &conteudo, QIODevice::OpenMode modo) {
    QFile arquivo(QString::fromUtf8(nome));
    if (!arquivo.open(modo | QIODevice::Text)) {
        return false;
    }
    return arquivo.write(conteudo) == conteudo.size();
}

} // namespace

namespace arquivos {

bool salvarMensagemCriptografada(const QByteArray &cifrado) {
    return gravar(kMensagemCriptografada, cifrado, QIODevice::WriteOnly | QIODevice::Truncate);
}

bool salvarMensagemDescriptografada(const QByteArray &mensagem) {
    return gravar(kMensagemDescriptografada, mensagem, QIODevice::WriteOnly | QIODevice::Truncate);
}

bool iniciarChaves(const QByteArray &p, const QByteArray &q, const QByteArray &n) {
    QByteArray conteudo = "P: " + p + "\n" + "Q: " + q + "\n";
    if (!n.isEmpty()) {
        conteudo += "N: " + n + "\n";
    }
    return gravar(kChaves, conteudo, QIODevice::WriteOnly | QIODevice::Truncate);
}

bool anexarChavesED(const QByteArray &e, const QByteArray &d) {
    return gravar(kChaves, "E: " + e + "\n" + "D: " + d, QIODevice::WriteOnly | QIODevice::Append);
}

} // namespace arquivos
