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

QStringList CoreBasicHandler::serverDecodeHelper(const QList<QByteArray> &stringlist, QStringList &list, int i)
{
    if (i < stringlist.size()) {
        list << network()->serverDecode(stringlist[i]);
        return serverDecodeHelper(stringlist, list, i + 1);
    }
    return list;
}

QStringList CoreBasicHandler::serverDecode(const QList<QByteArray> &stringlist)
{
    QStringList list;
    return serverDecodeHelper(stringlist, list, 0);
}


QString CoreBasicHandler::channelDecode(const QString &bufferName, const QByteArray &string)
{
    return network()->channelDecode(bufferName, string);
}

QStringList CoreBasicHandler::channelDecodeHelper(const QString &bufferName, const QList<QByteArray> &stringlist, QStringList &list, int i)
{
    if (i < stringlist.size()) {
        list << network()->channelDecode(bufferName, stringlist[i]);
        return channelDecodeHelper(bufferName, stringlist, list, i + 1);
    }
    return list;
}

QStringList CoreBasicHandler::channelDecode(const QString &bufferName, const QList<QByteArray> &stringlist)
{
    QStringList list;
    return channelDecodeHelper(bufferName, stringlist, list, 0);
}


QString CoreBasicHandler::userDecode(const QString &userNick, const QByteArray &string)
{
    return network()->userDecode(userNick, string);
}

QStringList CoreBasicHandler::userDecodeHelper(const QString &userNick, const QList<QByteArray> &stringlist, QStringList &list, int i)
{
    if (i < stringlist.size()) {
        list << network()->userDecode(userNick, stringlist[i]);
        return userDecodeHelper(userNick, stringlist, list, i + 1);
    }
    return list;
}

QStringList CoreBasicHandler::userDecode(const QString &userNick, const QList<QByteArray> &stringlist)
{
    QStringList list;
    return userDecodeHelper(userNick, stringlist, list, 0);
}


/*** ***/

QByteArray CoreBasicHandler::serverEncode(const QString &string)
{
    return network()->serverEncode(string);
}

QList<QByteArray> CoreBasicHandler::serverEncodeHelper(const QStringList &stringlist, QList<QByteArray> &list, int i)
{
    if (i < stringlist.size()) {
        list << network()->serverEncode(stringlist[i]);
        return serverEncodeHelper(stringlist, list, i + 1);
    }
    return list;
}

QList<QByteArray> CoreBasicHandler::serverEncode(const QStringList &stringlist)
{
    QList<QByteArray> list;
    return serverEncodeHelper(stringlist, list, 0);
}


QByteArray CoreBasicHandler::channelEncode(const QString &bufferName, const QString &string)
{
    return network()->channelEncode(bufferName, string);
}

QList<QByteArray> CoreBasicHandler::channelEncodeHelper(const QString &bufferName, const QStringList &stringlist, QList<QByteArray> &list, int i)
{
    if (i < stringlist.size()) {
        list << network()->channelEncode(bufferName, stringlist[i]);
        return channelEncodeHelper(bufferName, stringlist, list, i + 1);
    }
    return list;
}

QList<QByteArray> CoreBasicHandler::channelEncode(const QString &bufferName, const QStringList &stringlist)
{
    QList<QByteArray> list;
    return channelEncodeHelper(bufferName, stringlist, list, 0);
}


QByteArray CoreBasicHandler::userEncode(const QString &userNick, const QString &string)
{
    return network()->userEncode(userNick, string);
}

QList<QByteArray> CoreBasicHandler::userEncodeHelper(const QString &userNick, const QStringList &stringlist, QList<QByteArray> &list, int i)
{
    if (i < stringlist.size()) {
        list << network()->userEncode(userNick, stringlist[i]);
        return userEncodeHelper(userNick, stringlist, list, i + 1);
    }
    return list;
}

QList<QByteArray> CoreBasicHandler::userEncode(const QString &userNick, const QStringList &stringlist)
{
    QList<QByteArray> list;
    return userEncodeHelper(userNick, stringlist, list, 0);
}


// ====================
//  protected:
// ====================
BufferInfo::Type CoreBasicHandler::typeByTarget(const QString &target) const
{
    BufferInfo::Type result;
    switch (target.isEmpty() ? 0 : network()->isChannelName(target) ? 1 : 2) {
        case 0:
            result = BufferInfo::StatusBuffer;
            break;
        case 1:
            result = BufferInfo::ChannelBuffer;
            break;
        default:
            result = BufferInfo::QueryBuffer;
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