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


QString CoreBasicHandler::serverDecode(const QByteArray &h)
{
    return network()->serverDecode(h);
}


QStringList CoreBasicHandler::serverDecode(const QList<QByteArray> &j)
{
    QStringList z;
    foreach(QByteArray e, j) z << network()->serverDecode(e);
    return z;
}


QString CoreBasicHandler::channelDecode(const QString &n, const QByteArray &h)
{
    return network()->channelDecode(n, h);
}


QStringList CoreBasicHandler::channelDecode(const QString &n, const QList<QByteArray> &j)
{
    QStringList z;
    foreach(QByteArray e, j) z << network()->channelDecode(n, e);
    return z;
}


QString CoreBasicHandler::userDecode(const QString &k, const QByteArray &h)
{
    return network()->userDecode(k, h);
}


QStringList CoreBasicHandler::userDecode(const QString &k, const QList<QByteArray> &j)
{
    QStringList z;
    foreach(QByteArray e, j) z << network()->userDecode(k, e);
    return z;
}


/*** ***/

QByteArray CoreBasicHandler::serverEncode(const QString &n)
{
    return network()->serverEncode(n);
}


QList<QByteArray> CoreBasicHandler::serverEncode(const QStringList &z)
{
    QList<QByteArray> j;
    foreach(QString e, z) j << network()->serverEncode(e);
    return j;
}


QByteArray CoreBasicHandler::channelEncode(const QString &n, const QString &m)
{
    return network()->channelEncode(n, m);
}


QList<QByteArray> CoreBasicHandler::channelEncode(const QString &n, const QStringList &z)
{
    QList<QByteArray> j;
    foreach(QString e, z) j << network()->channelEncode(n, e);
    return j;
}


QByteArray CoreBasicHandler::userEncode(const QString &k, const QString &m)
{
    return network()->userEncode(k, m);
}


QList<QByteArray> CoreBasicHandler::userEncode(const QString &k, const QStringList &z)
{
    QList<QByteArray> j;
    foreach(QString e, z) j << network()->userEncode(k, e);
    return j;
}


// ====================
//  protected:
// ====================
BufferInfo::Type CoreBasicHandler::typeByTarget(const QString &r) const
{
    if (r.isEmpty())
        return BufferInfo::StatusBuffer;

    if (network()->isChannelName(r))
        return BufferInfo::ChannelBuffer;

    return BufferInfo::QueryBuffer;
}


void CoreBasicHandler::putCmd(const QString &cmd, const QByteArray &param, const QByteArray &prefix)
{
    QList<QByteArray> list;
    list << param;
    emit putCmd(cmd, list, prefix);
}