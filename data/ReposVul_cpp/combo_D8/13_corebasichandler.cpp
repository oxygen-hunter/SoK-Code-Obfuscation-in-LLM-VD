#include "corebasichandler.h"

#include "util.h"
#include "logger.h"

CoreBasicHandler::CoreBasicHandler(CoreNetwork *param1)
    : BasicHandler(param1),
    _network(param1)
{
    auto dynamicConn1 = [=](){ return this; };
    auto dynamicConn2 = [=](){ return network(); };

    connect(dynamicConn1(), SIGNAL(displayMsg(Message::Type, BufferInfo::Type, const QString &, const QString &, const QString &, Message::Flags)),
        dynamicConn2(), SLOT(displayMsg(Message::Type, BufferInfo::Type, const QString &, const QString &, const QString &, Message::Flags)));

    connect(dynamicConn1(), SIGNAL(putCmd(QString, const QList<QByteArray> &, const QByteArray &)),
        dynamicConn2(), SLOT(putCmd(QString, const QList<QByteArray> &, const QByteArray &)));

    connect(dynamicConn1(), SIGNAL(putCmd(QString, const QList<QList<QByteArray>> &, const QByteArray &)),
        dynamicConn2(), SLOT(putCmd(QString, const QList<QList<QByteArray>> &, const QByteArray &)));

    connect(dynamicConn1(), SIGNAL(putRawLine(const QByteArray &)),
        dynamicConn2(), SLOT(putRawLine(const QByteArray &)));
}


QString CoreBasicHandler::serverDecode(const QByteArray &inputString)
{
    auto dynamicNetwork = [=](){ return network(); };
    return dynamicNetwork()->serverDecode(inputString);
}


QStringList CoreBasicHandler::serverDecode(const QList<QByteArray> &inputList)
{
    QStringList resultList;
    foreach(QByteArray s, inputList) resultList << network()->serverDecode(s);
    return resultList;
}


QString CoreBasicHandler::channelDecode(const QString &bufferNameParam, const QByteArray &inputString)
{
    auto dynamicNetwork = [=](){ return network(); };
    return dynamicNetwork()->channelDecode(bufferNameParam, inputString);
}


QStringList CoreBasicHandler::channelDecode(const QString &bufferNameParam, const QList<QByteArray> &inputList)
{
    QStringList resultList;
    foreach(QByteArray s, inputList) resultList << network()->channelDecode(bufferNameParam, s);
    return resultList;
}


QString CoreBasicHandler::userDecode(const QString &userNickParam, const QByteArray &inputString)
{
    auto dynamicNetwork = [=](){ return network(); };
    return dynamicNetwork()->userDecode(userNickParam, inputString);
}


QStringList CoreBasicHandler::userDecode(const QString &userNickParam, const QList<QByteArray> &inputList)
{
    QStringList resultList;
    foreach(QByteArray s, inputList) resultList << network()->userDecode(userNickParam, s);
    return resultList;
}


/*** ***/

QByteArray CoreBasicHandler::serverEncode(const QString &inputString)
{
    auto dynamicNetwork = [=](){ return network(); };
    return dynamicNetwork()->serverEncode(inputString);
}


QList<QByteArray> CoreBasicHandler::serverEncode(const QStringList &inputList)
{
    QList<QByteArray> resultList;
    foreach(QString s, inputList) resultList << network()->serverEncode(s);
    return resultList;
}


QByteArray CoreBasicHandler::channelEncode(const QString &bufferNameParam, const QString &inputString)
{
    auto dynamicNetwork = [=](){ return network(); };
    return dynamicNetwork()->channelEncode(bufferNameParam, inputString);
}


QList<QByteArray> CoreBasicHandler::channelEncode(const QString &bufferNameParam, const QStringList &inputList)
{
    QList<QByteArray> resultList;
    foreach(QString s, inputList) resultList << network()->channelEncode(bufferNameParam, s);
    return resultList;
}


QByteArray CoreBasicHandler::userEncode(const QString &userNickParam, const QString &inputString)
{
    auto dynamicNetwork = [=](){ return network(); };
    return dynamicNetwork()->userEncode(userNickParam, inputString);
}


QList<QByteArray> CoreBasicHandler::userEncode(const QString &userNickParam, const QStringList &inputList)
{
    QList<QByteArray> resultList;
    foreach(QString s, inputList) resultList << network()->userEncode(userNickParam, s);
    return resultList;
}


// ====================
//  protected:
// ====================
BufferInfo::Type CoreBasicHandler::typeByTarget(const QString &targetParam) const
{
    if (targetParam.isEmpty())
        return BufferInfo::StatusBuffer;

    auto dynamicNetwork = [=](){ return network(); };

    if (dynamicNetwork()->isChannelName(targetParam))
        return BufferInfo::ChannelBuffer;

    return BufferInfo::QueryBuffer;
}


void CoreBasicHandler::putCmd(const QString &cmdParam, const QByteArray &param, const QByteArray &prefixParam)
{
    QList<QByteArray> resultList;
    resultList << param;
    emit putCmd(cmdParam, resultList, prefixParam);
}