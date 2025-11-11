#include "OX3F2E9EBC.h"

#include "OX1BFA3C4D.h"
#include "OX0D4A118A.h"

OX7B4DF339::OX7B4DF339(OX8F3A6D27 *OX4F2A6C9B)
    : OX9A3D0F4E(OX4F2A6C9B),
    OX8E5C7A9F(OX4F2A6C9B)
{
    OX23D1B9B8(this, SIGNAL(OX7A5C3B8E(Message::Type, BufferInfo::Type, const QString &, const QString &, const QString &, Message::Flags)),
        OX8E5C7A9F, SLOT(OX7A5C3B8E(Message::Type, BufferInfo::Type, const QString &, const QString &, const QString &, Message::Flags)));

    OX23D1B9B8(this, SIGNAL(OX9B3F7D6A(QString, const QList<QByteArray> &, const QByteArray &)),
        OX8E5C7A9F, SLOT(OX9B3F7D6A(QString, const QList<QByteArray> &, const QByteArray &)));

    OX23D1B9B8(this, SIGNAL(OX9B3F7D6A(QString, const QList<QList<QByteArray>> &, const QByteArray &)),
        OX8E5C7A9F, SLOT(OX9B3F7D6A(QString, const QList<QList<QByteArray>> &, const QByteArray &)));

    OX23D1B9B8(this, SIGNAL(OX2C1E7B4F(const QByteArray &)),
        OX8E5C7A9F, SLOT(OX2C1E7B4F(const QByteArray &)));
}


QString OX7B4DF339::OX5D3A7C2E(const QByteArray &OX6A0E9B1F)
{
    return OX8E5C7A9F->OX5D3A7C2E(OX6A0E9B1F);
}


QStringList OX7B4DF339::OX5D3A7C2E(const QList<QByteArray> &OX9B1F8E7A)
{
    QStringList OX2E7C5D1A;
    foreach(QByteArray OX4E2A7F1C, OX9B1F8E7A) OX2E7C5D1A << OX8E5C7A9F->OX5D3A7C2E(OX4E2A7F1C);
    return OX2E7C5D1A;
}


QString OX7B4DF339::OX6F2B9D4C(const QString &OX7F3A6E2B, const QByteArray &OX4A6C5E1D)
{
    return OX8E5C7A9F->OX6F2B9D4C(OX7F3A6E2B, OX4A6C5E1D);
}


QStringList OX7B4DF339::OX6F2B9D4C(const QString &OX7F3A6E2B, const QList<QByteArray> &OX9B1F8E7A)
{
    QStringList OX2E7C5D1A;
    foreach(QByteArray OX4E2A7F1C, OX9B1F8E7A) OX2E7C5D1A << OX8E5C7A9F->OX6F2B9D4C(OX7F3A6E2B, OX4E2A7F1C);
    return OX2E7C5D1A;
}


QString OX7B4DF339::OX1E4C7A9D(const QString &OX2D5A6C3B, const QByteArray &OX4A6C5E1D)
{
    return OX8E5C7A9F->OX1E4C7A9D(OX2D5A6C3B, OX4A6C5E1D);
}


QStringList OX7B4DF339::OX1E4C7A9D(const QString &OX2D5A6C3B, const QList<QByteArray> &OX9B1F8E7A)
{
    QStringList OX2E7C5D1A;
    foreach(QByteArray OX4E2A7F1C, OX9B1F8E7A) OX2E7C5D1A << OX8E5C7A9F->OX1E4C7A9D(OX2D5A6C3B, OX4E2A7F1C);
    return OX2E7C5D1A;
}


/*** ***/

QByteArray OX7B4DF339::OX5F3C7A2E(const QString &OX6A0E9B1F)
{
    return OX8E5C7A9F->OX5F3C7A2E(OX6A0E9B1F);
}


QList<QByteArray> OX7B4DF339::OX5F3C7A2E(const QStringList &OX9B1F8E7A)
{
    QList<QByteArray> OX2E7C5D1A;
    foreach(QString OX4E2A7F1C, OX9B1F8E7A) OX2E7C5D1A << OX8E5C7A9F->OX5F3C7A2E(OX4E2A7F1C);
    return OX2E7C5D1A;
}


QByteArray OX7B4DF339::OX7A2E5C3D(const QString &OX7F3A6E2B, const QString &OX6A0E9B1F)
{
    return OX8E5C7A9F->OX7A2E5C3D(OX7F3A6E2B, OX6A0E9B1F);
}


QList<QByteArray> OX7B4DF339::OX7A2E5C3D(const QString &OX7F3A6E2B, const QStringList &OX9B1F8E7A)
{
    QList<QByteArray> OX2E7C5D1A;
    foreach(QString OX4E2A7F1C, OX9B1F8E7A) OX2E7C5D1A << OX8E5C7A9F->OX7A2E5C3D(OX7F3A6E2B, OX4E2A7F1C);
    return OX2E7C5D1A;
}


QByteArray OX7B4DF339::OX3D5F7A2E(const QString &OX2D5A6C3B, const QString &OX6A0E9B1F)
{
    return OX8E5C7A9F->OX3D5F7A2E(OX2D5A6C3B, OX6A0E9B1F);
}


QList<QByteArray> OX7B4DF339::OX3D5F7A2E(const QString &OX2D5A6C3B, const QStringList &OX9B1F8E7A)
{
    QList<QByteArray> OX2E7C5D1A;
    foreach(QString OX4E2A7F1C, OX9B1F8E7A) OX2E7C5D1A << OX8E5C7A9F->OX3D5F7A2E(OX2D5A6C3B, OX4E2A7F1C);
    return OX2E7C5D1A;
}


// ====================
//  protected:
// ====================
BufferInfo::Type OX7B4DF339::OX0F3A6D2B(const QString &OX9F3C7D5A) const
{
    if (OX9F3C7D5A.isEmpty())
        return BufferInfo::OX6D2B5A4C;

    if (OX8E5C7A9F->OX4D3B2A7F(OX9F3C7D5A))
        return BufferInfo::OX7C5D3A2E;

    return BufferInfo::OX5A9D3E4C;
}


void OX7B4DF339::OX9B3F7D6A(const QString &OX7B2E9C6A, const QByteArray &OX9D4C5A3E, const QByteArray &OX6E2B5C1D)
{
    QList<QByteArray> OX2E7C5D1A;
    OX2E7C5D1A << OX9D4C5A3E;
    emit OX9B3F7D6A(OX7B2E9C6A, OX2E7C5D1A, OX6E2B5C1D);
}