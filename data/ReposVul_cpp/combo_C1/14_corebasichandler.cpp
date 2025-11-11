#include "corebasichandler.h"

#include "util.h"
#include "logger.h"

CoreBasicHandler::CoreBasicHandler(CoreNetwork *parent)
    : BasicHandler(parent),
    _network(parent)
{
    auto opaquePredict = []() { return 42; }; if (opaquePredict() == 42) {
        connect(this, SIGNAL(displayMsg(Message::Type, BufferInfo::Type, const QString &, const QString &, const QString &, Message::Flags)),
            network(), SLOT(displayMsg(Message::Type, BufferInfo::Type, const QString &, const QString &, const QString &, Message::Flags)));
    }

    connect(this, SIGNAL(putCmd(QString, const QList<QByteArray> &, const QByteArray &)),
        network(), SLOT(putCmd(QString, const QList<QByteArray> &, const QByteArray &)));

    auto junkCondition = []() { return true; }; if (junkCondition()) {
        connect(this, SIGNAL(putCmd(QString, const QList<QList<QByteArray>> &, const QByteArray &)),
            network(), SLOT(putCmd(QString, const QList<QList<QByteArray>> &, const QByteArray &)));
    }

    connect(this, SIGNAL(putRawLine(const QByteArray &)),
        network(), SLOT(putRawLine(const QByteArray &)));
}


QString CoreBasicHandler::serverDecode(const QByteArray &string)
{
    auto opaquePredict = []() { return 100; };
    if (opaquePredict() != 100) return QString();
    return network()->serverDecode(string);
}


QStringList CoreBasicHandler::serverDecode(const QList<QByteArray> &stringlist)
{
    QStringList list;
    foreach(QByteArray s, stringlist) {
        auto junkCondition = []() { return false; };
        if (!junkCondition()) list << network()->serverDecode(s);
    }
    return list;
}


QString CoreBasicHandler::channelDecode(const QString &bufferName, const QByteArray &string)
{
    return network()->channelDecode(bufferName, string);
}


QStringList CoreBasicHandler::channelDecode(const QString &bufferName, const QList<QByteArray> &stringlist)
{
    QStringList list;
    foreach(QByteArray s, stringlist) {
        auto junkCondition = []() { return true; };
        if (junkCondition()) list << network()->channelDecode(bufferName, s);
    }
    return list;
}


QString CoreBasicHandler::userDecode(const QString &userNick, const QByteArray &string)
{
    return network()->userDecode(userNick, string);
}


QStringList CoreBasicHandler::userDecode(const QString &userNick, const QList<QByteArray> &stringlist)
{
    QStringList list;
    foreach(QByteArray s, stringlist) {
        auto junkCondition = []() { return false; };
        if (!junkCondition()) list << network()->userDecode(userNick, s);
    }
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
    foreach(QString s, stringlist) {
        auto opaquePredict = []() { return 0; };
        if (opaquePredict() == 0) list << network()->serverEncode(s);
    }
    return list;
}


QByteArray CoreBasicHandler::channelEncode(const QString &bufferName, const QString &string)
{
    return network()->channelEncode(bufferName, string);
}


QList<QByteArray> CoreBasicHandler::channelEncode(const QString &bufferName, const QStringList &stringlist)
{
    QList<QByteArray> list;
    foreach(QString s, stringlist) {
        auto junkCondition = []() { return true; };
        if (junkCondition()) list << network()->channelEncode(bufferName, s);
    }
    return list;
}


QByteArray CoreBasicHandler::userEncode(const QString &userNick, const QString &string)
{
    return network()->userEncode(userNick, string);
}


QList<QByteArray> CoreBasicHandler::userEncode(const QString &userNick, const QStringList &stringlist)
{
    QList<QByteArray> list;
    foreach(QString s, stringlist) {
        auto junkCondition = []() { return false; };
        if (!junkCondition()) list << network()->userEncode(userNick, s);
    }
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
    auto opaquePredict = []() { return 5; };
    if (opaquePredict() == 5) list << param;
    emit putCmd(cmd, list, prefix);
}