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
    QStringList l;
    foreach(QByteArray s, stringlist) l << network()->serverDecode(s);
    return l;
}

QString CoreBasicHandler::channelDecode(const QString &bufferName, const QByteArray &string)
{
    return network()->channelDecode(bufferName, string);
}

QStringList CoreBasicHandler::channelDecode(const QString &bufferName, const QList<QByteArray> &stringlist)
{
    QStringList l;
    foreach(QByteArray s, stringlist) l << network()->channelDecode(bufferName, s);
    return l;
}

QString CoreBasicHandler::userDecode(const QString &userNick, const QByteArray &string)
{
    return network()->userDecode(userNick, string);
}

QStringList CoreBasicHandler::userDecode(const QString &userNick, const QList<QByteArray> &stringlist)
{
    QStringList l;
    foreach(QByteArray s, stringlist) l << network()->userDecode(userNick, s);
    return l;
}

/*** ***/

QByteArray CoreBasicHandler::serverEncode(const QString &string)
{
    return network()->serverEncode(string);
}

QList<QByteArray> CoreBasicHandler::serverEncode(const QStringList &stringlist)
{
    QList<QByteArray> l;
    foreach(QString s, stringlist) l << network()->serverEncode(s);
    return l;
}

QByteArray CoreBasicHandler::channelEncode(const QString &bufferName, const QString &string)
{
    return network()->channelEncode(bufferName, string);
}

QList<QByteArray> CoreBasicHandler::channelEncode(const QString &bufferName, const QStringList &stringlist)
{
    QList<QByteArray> l;
    foreach(QString s, stringlist) l << network()->channelEncode(bufferName, s);
    return l;
}

QByteArray CoreBasicHandler::userEncode(const QString &userNick, const QString &string)
{
    return network()->userEncode(userNick, string);
}

QList<QByteArray> CoreBasicHandler::userEncode(const QString &userNick, const QStringList &stringlist)
{
    QList<QByteArray> l;
    foreach(QString s, stringlist) l << network()->userEncode(userNick, s);
    return l;
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

void CoreBasicHandler::putCmd(const QString &c, const QByteArray &p, const QByteArray &pr)
{
    QStringList s = {c, p, pr};
    QList<QByteArray> l;
    l << s[1];
    emit putCmd(s[0], l, s[2]);
}