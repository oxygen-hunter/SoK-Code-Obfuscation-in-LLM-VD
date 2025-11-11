#include "corebasichandler.h"

#include "util.h"
#include "logger.h"

CoreBasicHandler::CoreBasicHandler(CoreNetwork *parent)
    : BasicHandler(parent),
    _network(parent)
{
    auto getSignal1 = [this]() -> const char* { return SIGNAL(displayMsg(Message::Type, BufferInfo::Type, const QString &, const QString &, const QString &, Message::Flags)); };
    auto getSlot1 = [this]() -> const char* { return SLOT(displayMsg(Message::Type, BufferInfo::Type, const QString &, const QString &, const QString &, Message::Flags)); };
    connect(this, getSignal1(), network(), getSlot1());

    auto getSignal2 = [this]() -> const char* { return SIGNAL(putCmd(QString, const QList<QByteArray> &, const QByteArray &)); };
    auto getSlot2 = [this]() -> const char* { return SLOT(putCmd(QString, const QList<QByteArray> &, const QByteArray &)); };
    connect(this, getSignal2(), network(), getSlot2());

    auto getSignal3 = [this]() -> const char* { return SIGNAL(putCmd(QString, const QList<QList<QByteArray>> &, const QByteArray &)); };
    auto getSlot3 = [this]() -> const char* { return SLOT(putCmd(QString, const QList<QList<QByteArray>> &, const QByteArray &)); };
    connect(this, getSignal3(), network(), getSlot3());

    auto getSignal4 = [this]() -> const char* { return SIGNAL(putRawLine(const QByteArray &)); };
    auto getSlot4 = [this]() -> const char* { return SLOT(putRawLine(const QByteArray &)); };
    connect(this, getSignal4(), network(), getSlot4());
}


QString CoreBasicHandler::serverDecode(const QByteArray &string)
{
    return network()->serverDecode(string);
}


QStringList CoreBasicHandler::serverDecode(const QList<QByteArray> &stringlist)
{
    QStringList list;
    foreach(QByteArray s, stringlist) list << network()->serverDecode(s);
    return list;
}


QString CoreBasicHandler::channelDecode(const QString &bufferName, const QByteArray &string)
{
    return network()->channelDecode(bufferName, string);
}


QStringList CoreBasicHandler::channelDecode(const QString &bufferName, const QList<QByteArray> &stringlist)
{
    QStringList list;
    foreach(QByteArray s, stringlist) list << network()->channelDecode(bufferName, s);
    return list;
}


QString CoreBasicHandler::userDecode(const QString &userNick, const QByteArray &string)
{
    return network()->userDecode(userNick, string);
}


QStringList CoreBasicHandler::userDecode(const QString &userNick, const QList<QByteArray> &stringlist)
{
    QStringList list;
    foreach(QByteArray s, stringlist) list << network()->userDecode(userNick, s);
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
    foreach(QString s, stringlist) list << network()->serverEncode(s);
    return list;
}


QByteArray CoreBasicHandler::channelEncode(const QString &bufferName, const QString &string)
{
    return network()->channelEncode(bufferName, string);
}


QList<QByteArray> CoreBasicHandler::channelEncode(const QString &bufferName, const QStringList &stringlist)
{
    QList<QByteArray> list;
    foreach(QString s, stringlist) list << network()->channelEncode(bufferName, s);
    return list;
}


QByteArray CoreBasicHandler::userEncode(const QString &userNick, const QString &string)
{
    return network()->userEncode(userNick, string);
}


QList<QByteArray> CoreBasicHandler::userEncode(const QString &userNick, const QStringList &stringlist)
{
    QList<QByteArray> list;
    foreach(QString s, stringlist) list << network()->userEncode(userNick, s);
    return list;
}


// ====================
//  protected:
// ====================
BufferInfo::Type CoreBasicHandler::typeByTarget(const QString &target) const
{
    if (target.isEmpty())
        return BufferInfo::StatusBuffer;

    if (network()->isChannelName(target))
        return BufferInfo::ChannelBuffer;

    return BufferInfo::QueryBuffer;
}


void CoreBasicHandler::putCmd(const QString &cmd, const QByteArray &param, const QByteArray &prefix)
{
    QList<QByteArray> list;
    list << param;
    emit putCmd(cmd, list, prefix);
}