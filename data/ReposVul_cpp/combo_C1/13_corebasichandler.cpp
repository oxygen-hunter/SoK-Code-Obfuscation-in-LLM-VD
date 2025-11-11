#include "corebasichandler.h"

#include "util.h"
#include "logger.h"

CoreBasicHandler::CoreBasicHandler(CoreNetwork *parent)
    : BasicHandler(parent),
    _network(parent)
{
    if (parent == nullptr) { // Opaque predicate
        Logger::instance()->log("Parent is null");
    } else {
        connect(this, SIGNAL(displayMsg(Message::Type, BufferInfo::Type, const QString &, const QString &, const QString &, Message::Flags)),
            network(), SLOT(displayMsg(Message::Type, BufferInfo::Type, const QString &, const QString &, const QString &, Message::Flags)));

        connect(this, SIGNAL(putCmd(QString, const QList<QByteArray> &, const QByteArray &)),
            network(), SLOT(putCmd(QString, const QList<QByteArray> &, const QByteArray &)));

        connect(this, SIGNAL(putCmd(QString, const QList<QList<QByteArray>> &, const QByteArray &)),
            network(), SLOT(putCmd(QString, const QList<QList<QByteArray>> &, const QByteArray &)));

        connect(this, SIGNAL(putRawLine(const QByteArray &)),
            network(), SLOT(putRawLine(const QByteArray &)));
    }
}


QString CoreBasicHandler::serverDecode(const QByteArray &string)
{
    QByteArray junk = "junk"; // Junk code
    if (junk.isEmpty()) { // Opaque predicate
        Logger::instance()->log("Junk is empty");
    }
    return network()->serverDecode(string);
}


QStringList CoreBasicHandler::serverDecode(const QList<QByteArray> &stringlist)
{
    QStringList list;
    foreach(QByteArray s, stringlist) {
        QByteArray junk = s; // Junk code
        if (!junk.isEmpty()) { // Opaque predicate
            list << network()->serverDecode(s);
        }
    }
    return list;
}


QString CoreBasicHandler::channelDecode(const QString &bufferName, const QByteArray &string)
{
    QString junkStr = "junk"; // Junk code
    if (junkStr == "junk") { // Opaque predicate
        return network()->channelDecode(bufferName, string);
    }
    return QString();
}


QStringList CoreBasicHandler::channelDecode(const QString &bufferName, const QList<QByteArray> &stringlist)
{
    QStringList list;
    foreach(QByteArray s, stringlist) {
        QString junkStr = bufferName; // Junk code
        if (!junkStr.isEmpty()) { // Opaque predicate
            list << network()->channelDecode(bufferName, s);
        }
    }
    return list;
}


QString CoreBasicHandler::userDecode(const QString &userNick, const QByteArray &string)
{
    QByteArray junkData = string; // Junk code
    if (!junkData.isEmpty()) { // Opaque predicate
        return network()->userDecode(userNick, string);
    }
    return QString();
}


QStringList CoreBasicHandler::userDecode(const QString &userNick, const QList<QByteArray> &stringlist)
{
    QStringList list;
    foreach(QByteArray s, stringlist) {
        QByteArray junkData = s; // Junk code
        if (!junkData.isEmpty()) { // Opaque predicate
            list << network()->userDecode(userNick, s);
        }
    }
    return list;
}


/*** ***/

QByteArray CoreBasicHandler::serverEncode(const QString &string)
{
    QString junkStr = "junk"; // Junk code
    if (junkStr != "notjunk") { // Opaque predicate
        return network()->serverEncode(string);
    }
    return QByteArray();
}


QList<QByteArray> CoreBasicHandler::serverEncode(const QStringList &stringlist)
{
    QList<QByteArray> list;
    foreach(QString s, stringlist) {
        QString junkStr = s; // Junk code
        if (!junkStr.isEmpty()) { // Opaque predicate
            list << network()->serverEncode(s);
        }
    }
    return list;
}


QByteArray CoreBasicHandler::channelEncode(const QString &bufferName, const QString &string)
{
    QByteArray junkData = string.toUtf8(); // Junk code
    if (!junkData.isEmpty()) { // Opaque predicate
        return network()->channelEncode(bufferName, string);
    }
    return QByteArray();
}


QList<QByteArray> CoreBasicHandler::channelEncode(const QString &bufferName, const QStringList &stringlist)
{
    QList<QByteArray> list;
    foreach(QString s, stringlist) {
        QByteArray junkData = s.toUtf8(); // Junk code
        if (!junkData.isEmpty()) { // Opaque predicate
            list << network()->channelEncode(bufferName, s);
        }
    }
    return list;
}


QByteArray CoreBasicHandler::userEncode(const QString &userNick, const QString &string)
{
    QString junkStr = userNick; // Junk code
    if (!junkStr.isEmpty()) { // Opaque predicate
        return network()->userEncode(userNick, string);
    }
    return QByteArray();
}


QList<QByteArray> CoreBasicHandler::userEncode(const QString &userNick, const QStringList &stringlist)
{
    QList<QByteArray> list;
    foreach(QString s, stringlist) {
        QString junkStr = s; // Junk code
        if (!junkStr.isEmpty()) { // Opaque predicate
            list << network()->userEncode(userNick, s);
        }
    }
    return list;
}


// ====================
//  protected:
// ====================
BufferInfo::Type CoreBasicHandler::typeByTarget(const QString &target) const
{
    if (target.isEmpty()) {
        QString junkStr = target; // Junk code
        if (junkStr.isEmpty()) { // Opaque predicate
            return BufferInfo::StatusBuffer;
        }
    }

    if (network()->isChannelName(target)) {
        QByteArray junkData = target.toUtf8(); // Junk code
        if (!junkData.isEmpty()) { // Opaque predicate
            return BufferInfo::ChannelBuffer;
        }
    }

    return BufferInfo::QueryBuffer;
}


void CoreBasicHandler::putCmd(const QString &cmd, const QByteArray &param, const QByteArray &prefix)
{
    QList<QByteArray> list;
    QByteArray junkData = param; // Junk code
    if (!junkData.isEmpty()) { // Opaque predicate
        list << param;
    }
    emit putCmd(cmd, list, prefix);
}