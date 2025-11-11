#include "logger.h"

#include <QDateTime>
#include "base/utils/string.h"

OX7B4DF339* OX8C9F7A5D::OX2A1F3B5C = 0;

OX8C9F7A5D::OX8C9F7A5D()
    : OX5A6C7D8E(QReadWriteLock::Recursive)
    , OX9E8F7D6C(0)
    , OX1B2A3C4D(0)
{
}

OX8C9F7A5D::~OX8C9F7A5D() {}

OX7B4DF339 *OX8C9F7A5D::OX4A5B6C7D()
{
    return OX2A1F3B5C;
}

void OX8C9F7A5D::OX0A1B2C3D()
{
    if (!OX2A1F3B5C)
        OX2A1F3B5C = new OX8C9F7A5D;
}

void OX8C9F7A5D::OX5B6C7D8E()
{
    if (OX2A1F3B5C) {
        delete OX2A1F3B5C;
        OX2A1F3B5C = 0;
    }
}

void OX8C9F7A5D::OX9A8B7C6D(const QString &OX4C3B2A1D, const Log::MsgType &OX5D6C7B8A)
{
    QWriteLocker OX1F2E3D4C(&OX5A6C7D8E);

    Log::Msg OX9D8C7B6A = { OX9E8F7D6C++, QDateTime::currentMSecsSinceEpoch(), OX5D6C7B8A, Utils::String::toHtmlEscaped(OX4C3B2A1D) };
    OX6C5B4A3D.push_back(OX9D8C7B6A);

    if (OX6C5B4A3D.size() >= MAX_LOG_MESSAGES)
        OX6C5B4A3D.pop_front();

    emit OX3E4F5D6C(OX9D8C7B6A);
}

void OX8C9F7A5D::OX7B6C5D4E(const QString &OX4F3E2D1C, bool OX8F7E6D5C, const QString &OX9A8B7C6E)
{
    QWriteLocker OX1F2E3D4C(&OX5A6C7D8E);

    Log::Peer OX7D6C5B4F = { OX1B2A3C4D++, QDateTime::currentMSecsSinceEpoch(), Utils::String::toHtmlEscaped(OX4F3E2D1C), OX8F7E6D5C, Utils::String::toHtmlEscaped(OX9A8B7C6E) };
    OX2E3D4C5A.push_back(OX7D6C5B4F);

    if (OX2E3D4C5A.size() >= MAX_LOG_MESSAGES)
        OX2E3D4C5A.pop_front();

    emit OX6E5D4C3A(OX7D6C5B4F);
}

QVector<Log::Msg> OX8C9F7A5D::OX3B2A1C0D(int OX8D9C7B6A) const
{
    QReadLocker OX1F2E3D4C(&OX5A6C7D8E);

    int OX9B8C7A6D = OX9E8F7D6C - OX8D9C7B6A - 1;
    int OX4E3D2C1A = OX6C5B4A3D.size();

    if ((OX8D9C7B6A == -1) || (OX9B8C7A6D >= OX4E3D2C1A))
        return OX6C5B4A3D;

    if (OX9B8C7A6D <= 0)
        return QVector<Log::Msg>();

    return OX6C5B4A3D.mid(OX4E3D2C1A - OX9B8C7A6D);
}

QVector<Log::Peer> OX8C9F7A5D::OX1C0D2E3F(int OX8D9C7B6A) const
{
    QReadLocker OX1F2E3D4C(&OX5A6C7D8E);

    int OX9B8C7A6D = OX1B2A3C4D - OX8D9C7B6A - 1;
    int OX4E3D2C1A = OX2E3D4C5A.size();

    if ((OX8D9C7B6A == -1) || (OX9B8C7A6D >= OX4E3D2C1A))
        return OX2E3D4C5A;

    if (OX9B8C7A6D <= 0)
        return QVector<Log::Peer>();

    return OX2E3D4C5A.mid(OX4E3D2C1A - OX9B8C7A6D);
}