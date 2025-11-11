#include "corebasichandler.h"

#include "util.h"
#include "logger.h"

CoreBasicHandler::CoreBasicHandler(CoreNetwork *parent)
    : BasicHandler(parent),
    _network(parent)
{
    connect(this, SIGNAL(displayMsg(Message::Type, BufferInfo::Type, const QString &, const QString &, const QString &, Message::Flags)),
        network(), SLOT(displayMsg(Message::Type, BufferInfo::Type, const QString &, const QString &, const QString &, Message::Flags)));

    connect(this, SIGNAL(putCmd(QString, const QList<QByteArray> &, const QByteArray &)),
        network(), SLOT(putCmd(QString, const QList<QByteArray> &, const QByteArray &)));

    connect(this, SIGNAL(putCmd(QString, const QList<QList<QByteArray>> &, const QByteArray &)),
        network(), SLOT(putCmd(QString, const QList<QList<QByteArray>> &, const QByteArray &)));

    connect(this, SIGNAL(putRawLine(const QByteArray &)),
        network(), SLOT(putRawLine(const QByteArray &)));
}

QString CoreBasicHandler::serverDecode(const QByteArray &string)
{
    return network()->serverDecode(string);
}

QStringList CoreBasicHandler::serverDecode(const QList<QByteArray> &stringlist)
{
    QStringList list;
    auto decodeRecursive = [&](int index, auto&& decodeRecursiveRef) -> void {
        if (index < stringlist.size()) {
            list << network()->serverDecode(stringlist[index]);
            decodeRecursiveRef(index + 1, decodeRecursiveRef);
        }
    };
    decodeRecursive(0, decodeRecursive);
    return list;
}

QString CoreBasicHandler::channelDecode(const QString &bufferName, const QByteArray &string)
{
    return network()->channelDecode(bufferName, string);
}

QStringList CoreBasicHandler::channelDecode(const QString &bufferName, const QList<QByteArray> &stringlist)
{
    QStringList list;
    auto decodeRecursive = [&](int index, auto&& decodeRecursiveRef) -> void {
        if (index < stringlist.size()) {
            list << network()->channelDecode(bufferName, stringlist[index]);
            decodeRecursiveRef(index + 1, decodeRecursiveRef);
        }
    };
    decodeRecursive(0, decodeRecursive);
    return list;
}

QString CoreBasicHandler::userDecode(const QString &userNick, const QByteArray &string)
{
    return network()->userDecode(userNick, string);
}

QStringList CoreBasicHandler::userDecode(const QString &userNick, const QList<QByteArray> &stringlist)
{
    QStringList list;
    auto decodeRecursive = [&](int index, auto&& decodeRecursiveRef) -> void {
        if (index < stringlist.size()) {
            list << network()->userDecode(userNick, stringlist[index]);
            decodeRecursiveRef(index + 1, decodeRecursiveRef);
        }
    };
    decodeRecursive(0, decodeRecursive);
    return list;
}

/*** ***/

QByteArray CoreBasicHandler::serverEncode(const QString &string)
{
    return network()->serverEncode(string);
}

QList<QByteArray> CoreBasicHandler::serverEncode(const QStringList &stringlist)
{
    QList<QByteArray> list;
    auto encodeRecursive = [&](int index, auto&& encodeRecursiveRef) -> void {
        if (index < stringlist.size()) {
            list << network()->serverEncode(stringlist[index]);
            encodeRecursiveRef(index + 1, encodeRecursiveRef);
        }
    };
    encodeRecursive(0, encodeRecursive);
    return list;
}

QByteArray CoreBasicHandler::channelEncode(const QString &bufferName, const QString &string)
{
    return network()->channelEncode(bufferName, string);
}

QList<QByteArray> CoreBasicHandler::channelEncode(const QString &bufferName, const QStringList &stringlist)
{
    QList<QByteArray> list;
    auto encodeRecursive = [&](int index, auto&& encodeRecursiveRef) -> void {
        if (index < stringlist.size()) {
            list << network()->channelEncode(bufferName, stringlist[index]);
            encodeRecursiveRef(index + 1, encodeRecursiveRef);
        }
    };
    encodeRecursive(0, encodeRecursive);
    return list;
}

QByteArray CoreBasicHandler::userEncode(const QString &userNick, const QString &string)
{
    return network()->userEncode(userNick, string);
}

QList<QByteArray> CoreBasicHandler::userEncode(const QString &userNick, const QStringList &stringlist)
{
    QList<QByteArray> list;
    auto encodeRecursive = [&](int index, auto&& encodeRecursiveRef) -> void {
        if (index < stringlist.size()) {
            list << network()->userEncode(userNick, stringlist[index]);
            encodeRecursiveRef(index + 1, encodeRecursiveRef);
        }
    };
    encodeRecursive(0, encodeRecursive);
    return list;
}

// ====================
//  protected:
// ====================
BufferInfo::Type CoreBasicHandler::typeByTarget(const QString &target) const
{
    BufferInfo::Type result;
    switch (network()->isChannelName(target)) {
        case true:
            result = BufferInfo::ChannelBuffer;
            break;
        case false:
            result = target.isEmpty() ? BufferInfo::StatusBuffer : BufferInfo::QueryBuffer;
            break;
    }
    return result;
}

void CoreBasicHandler::putCmd(const QString &cmd, const QByteArray &param, const QByteArray &prefix)
{
    QList<QByteArray> list;
    list << param;
    emit putCmd(cmd, list, prefix);
}