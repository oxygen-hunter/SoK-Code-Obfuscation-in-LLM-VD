#include "corebasichandler.h"

#include "util.h"
#include "logger.h"

CoreBasicHandler::CoreBasicHandler(CoreNetwork *parent)
    : BasicHandler(parent),
    _network(parent)
{
    int state = 0;
    while (true) {
        switch (state) {
            case 0:
                connect(this, SIGNAL(displayMsg(Message::Type, BufferInfo::Type, const QString &, const QString &, const QString &, Message::Flags)),
                    network(), SLOT(displayMsg(Message::Type, BufferInfo::Type, const QString &, const QString &, const QString &, Message::Flags)));
                state = 1;
                break;
            case 1:
                connect(this, SIGNAL(putCmd(QString, const QList<QByteArray> &, const QByteArray &)),
                    network(), SLOT(putCmd(QString, const QList<QByteArray> &, const QByteArray &)));
                state = 2;
                break;
            case 2:
                connect(this, SIGNAL(putCmd(QString, const QList<QList<QByteArray>> &, const QByteArray &)),
                    network(), SLOT(putCmd(QString, const QList<QList<QByteArray>> &, const QByteArray &)));
                state = 3;
                break;
            case 3:
                connect(this, SIGNAL(putRawLine(const QByteArray &)),
                    network(), SLOT(putRawLine(const QByteArray &)));
                return;
        }
    }
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
    int state = 0;
    while (true) {
        switch (state) {
            case 0:
                if (target.isEmpty()) return BufferInfo::StatusBuffer;
                state = 1;
                break;
            case 1:
                if (network()->isChannelName(target)) return BufferInfo::ChannelBuffer;
                return BufferInfo::QueryBuffer;
        }
    }
}


void CoreBasicHandler::putCmd(const QString &cmd, const QByteArray &param, const QByteArray &prefix)
{
    QList<QByteArray> list;
    list << param;
    emit putCmd(cmd, list, prefix);
}