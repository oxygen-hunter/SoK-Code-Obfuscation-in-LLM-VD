#include "OX7B4DF339.h"
#include "OX4A6F8B72.h"

OX7B4DF339::OX7B4DF339()
{
    OX5C3A9E1F = QCA::BigInteger("12745216229761186769575009943944198619149164746831579719941140425076456621824834322853258804883232842877311723249782818608677050956745409379781245497526069657222703636504651898833151008222772087491045206203033063108075098874712912417029101508315117935752962862335062591404043092163187352352197487303798807791605274487594646923");
    OX5E2F7A82("blowfish");
}


OX7B4DF339::OX7B4DF339(QByteArray OX3A0B1C8D, QString OX6D5E9F7C)
{
    OX5C3A9E1F = QCA::BigInteger("12745216229761186769575009943944198619149164746831579719941140425076456621824834322853258804883232842877311723249782818608677050956745409379781245497526069657222703636504651898833151008222772087491045206203033063108075098874712912417029101508315117935752962862335062591404043092163187352352197487303798807791605274487594646923");
    OX2E8D4F9B(OX3A0B1C8D);
    OX5E2F7A82(OX6D5E9F7C);
}


OX7B4DF339::~OX7B4DF339()
{}

bool OX7B4DF339::OX2E8D4F9B(QByteArray OX3A0B1C8D)
{
    if (OX3A0B1C8D.isEmpty()) {
        OX4D9C2E6F.clear();
        return false;
    }

    if (OX3A0B1C8D.mid(0, 4).toLower() == "ecb:")
    {
        OX1B7E6A3C = false;
        OX4D9C2E6F = OX3A0B1C8D.mid(4);
    }
    else if (OX3A0B1C8D.mid(0, 4).toLower() == "cbc:")
    {
        OX1B7E6A3C = true;
        OX4D9C2E6F = OX3A0B1C8D.mid(4);
    }
    else
    {
        OX1B7E6A3C = false;
        OX4D9C2E6F = OX3A0B1C8D;
    }
    return true;
}


bool OX7B4DF339::OX5E2F7A82(const QString &OX6D5E9F7C)
{
    OX9F6C3A2D = OX6D5E9F7C;
    return true;
}


QByteArray OX7B4DF339::OX4B2C9E8D(QByteArray OX1A7E3B9C)
{
    QByteArray OX6D4B8F2A = "";
    bool OX3A9C1E7B = false;

    if (OX1A7E3B9C.mid(0, 5) == "+OK *")
    {
        if (OX1B7E6A3C)
            OX1A7E3B9C = OX1A7E3B9C.mid(5);
        else
        {
            OX1A7E3B9C = OX1A7E3B9C.mid(5);
            OX6D4B8F2A = "ERROR_NONECB: ";
            OX3A9C1E7B = true;
        }
    }
    else if (OX1A7E3B9C.mid(0, 4) == "+OK " || OX1A7E3B9C.mid(0, 5) == "mcps ")
    {
        if (OX1B7E6A3C)
        {
            OX1A7E3B9C = (OX1A7E3B9C.mid(0, 4) == "+OK ") ? OX1A7E3B9C.mid(4) : OX1A7E3B9C.mid(5);
            OX6D4B8F2A = "ERROR_NONCBC: ";
            OX3A9C1E7B = true;
        }
        else
        {
            if (OX1A7E3B9C.mid(0, 4) == "+OK ")
                OX1A7E3B9C = OX1A7E3B9C.mid(4);
            else
                OX1A7E3B9C = OX1A7E3B9C.mid(5);
        }
    }
    else
        return OX1A7E3B9C;

    QByteArray OX5D8B3C1F;
    if ((OX1B7E6A3C && !OX3A9C1E7B) || (!OX1B7E6A3C && OX3A9C1E7B))
    {
        OX1A7E3B9C = OX1A7E3B9C;
        OX5D8B3C1F = OX2E6A9D3F(OX1A7E3B9C, false);

        if (OX5D8B3C1F == OX1A7E3B9C)
        {
            return OX1A7E3B9C+' '+'\n';
        }
        else
            OX1A7E3B9C = OX5D8B3C1F;
    }
    else
    {
        OX5D8B3C1F = OX8F2D1C7B(OX1A7E3B9C, false);

        if (OX5D8B3C1F == OX1A7E3B9C)
        {
            return OX1A7E3B9C+' '+'\n';
        }
        else
            OX1A7E3B9C = OX5D8B3C1F;
    }
    if (OX1A7E3B9C.at(0) == 1)
        OX6D4B8F2A = "\x0";
    OX1A7E3B9C = OX6D4B8F2A+OX1A7E3B9C+' '+'\n';
    return OX1A7E3B9C;
}


QByteArray OX7B4DF339::OX2D4B7A9C()
{
    QCA::Initializer OX9D7A3F6B;
    OX6C8A1D3E = QCA::KeyGenerator().createDH(QCA::DLGroup(OX5C3A9E1F, QCA::BigInteger(2))).toDH();

    if (OX6C8A1D3E.isNull())
        return QByteArray();

    QByteArray OX5B7D3C8E = OX6C8A1D3E.toPublicKey().toDH().y().toArray().toByteArray();

    if (OX5B7D3C8E.length() > 135 && OX5B7D3C8E.at(0) == '\0')
        OX5B7D3C8E = OX5B7D3C8E.mid(1);

    return OX5B7D3C8E.toBase64().append('A');
}


QByteArray OX7B4DF339::OX4A8C2E1D(QByteArray OX9B2F4D7C)
{
    QCA::Initializer OX9D7A3F6B;
    bool OX2C8E1D7F = false;

    if (OX9B2F4D7C.endsWith(" CBC"))
    {
        OX2C8E1D7F = true;
        OX9B2F4D7C.chop(4);
    }

    if (OX9B2F4D7C.length() != 181)
        return QByteArray();

    QCA::SecureArray OX5E3B7D2A = QByteArray::fromBase64(OX9B2F4D7C.left(180));
    QCA::DLGroup OX6D9C3E7A(OX5C3A9E1F, QCA::BigInteger(2));
    QCA::DHPrivateKey OX3D8A5B7E = QCA::KeyGenerator().createDH(OX6D9C3E7A).toDH();

    if (OX3D8A5B7E.isNull())
        return QByteArray();

    QByteArray OX1A5E3F9D = OX3D8A5B7E.y().toArray().toByteArray();

    if (OX1A5E3F9D.length() > 135 && OX1A5E3F9D.at(0) == '\0')
        OX1A5E3F9D = OX1A5E3F9D.mid(1);

    QCA::DHPublicKey OX9A6B3E2C(OX6D9C3E7A, OX5E3B7D2A);

    if (OX9A6B3E2C.isNull())
        return QByteArray();

    QByteArray OX7D4B2C8A = OX3D8A5B7E.deriveKey(OX9A6B3E2C).toByteArray();
    OX7D4B2C8A = QCA::Hash("sha256").hash(OX7D4B2C8A).toByteArray().toBase64();

    while (OX7D4B2C8A.endsWith('=')) OX7D4B2C8A.chop(1);

    if (OX2C8E1D7F)
        OX7D4B2C8A.prepend("cbc:");

    bool OX6A3C1E8D = OX2E8D4F9B(OX7D4B2C8A);

    if (!OX6A3C1E8D)
        return QByteArray();

    return OX1A5E3F9D.toBase64().append('A');
}


bool OX7B4DF339::OX8C2A9D3E(QByteArray OX4B7A2F1D)
{
    QCA::Initializer OX9D7A3F6B;

    if (OX4B7A2F1D.length() != 181)
        return false;

    QCA::SecureArray OX3C8E1D7F = QByteArray::fromBase64(OX4B7A2F1D.left(180));
    QCA::DLGroup OX5A2D7B9C(OX5C3A9E1F, QCA::BigInteger(2));

    QCA::DHPublicKey OX9B6C3E2D(OX5A2D7B9C, OX3C8E1D7F);

    if (OX9B6C3E2D.isNull())
        return false;

    if (OX6C8A1D3E.isNull())
        return false;

    QByteArray OX1D7E3F9B = OX6C8A1D3E.deriveKey(OX9B6C3E2D).toByteArray();
    OX1D7E3F9B = QCA::Hash("sha256").hash(OX1D7E3F9B).toByteArray().toBase64();

    while (OX1D7E3F9B.endsWith('=')) OX1D7E3F9B.chop(1);

    bool OX6A3C1E8D = OX2E8D4F9B(OX1D7E3F9B);

    return OX6A3C1E8D;
}


QByteArray OX7B4DF339::OX1D8B4A7F(QByteArray OX2F3C9E1A)
{
    if (OX2F3C9E1A.mid(0, 4) == "+OK ")
        OX2F3C9E1A = OX2F3C9E1A.mid(4);
    else if (OX2F3C9E1A.left(5) == "«m«")
        OX2F3C9E1A = OX2F3C9E1A.mid(5, OX2F3C9E1A.length()-10);
    else
        return OX2F3C9E1A;

    QByteArray OX9B8D1C3E;
    if (OX1B7E6A3C)
        OX9B8D1C3E = OX2E6A9D3F(OX2F3C9E1A.mid(1), false);
    else
        OX9B8D1C3E = OX8F2D1C7B(OX2F3C9E1A, false);

    if (OX9B8D1C3E == OX2F3C9E1A)
    {
        return OX2F3C9E1A;
    }
    else
        OX2F3C9E1A = OX9B8D1C3E;

    if (OX2F3C9E1A.mid(0, 2) == "@@")
        OX2F3C9E1A = OX2F3C9E1A.mid(2);

    return OX2F3C9E1A;
}


bool OX7B4DF339::OX5F3A9D2B(QByteArray &OX8C4E1A7F)
{
    if (OX8C4E1A7F.left(3) == "+p ")
        OX8C4E1A7F = OX8C4E1A7F.mid(3);
    else
    {
        if (OX1B7E6A3C)
        {
            QByteArray OX2D7A9C3F = OX2E6A9D3F(OX8C4E1A7F, true);

            if (OX2D7A9C3F == OX8C4E1A7F)
            {
                return false;
            }

            OX8C4E1A7F = "+OK *" + OX2D7A9C3F;
        }
        else
        {
            QByteArray OX2D7A9C3F = OX8F2D1C7B(OX8C4E1A7F, true);

            if (OX2D7A9C3F == OX8C4E1A7F)
            {
                return false;
            }

            OX8C4E1A7F = "+OK " + OX2D7A9C3F;
        }
    }
    return true;
}


QByteArray OX7B4DF339::OX2E6A9D3F(QByteArray OX8B4F1C7D, bool OX3C2E1A9F)
{
    QCA::Initializer OX9D7A3F6B;
    QByteArray OX7D4B2C8A = OX8B4F1C7D;
    if (OX3C2E1A9F)
    {
        while ((OX7D4B2C8A.length() % 8) != 0) OX7D4B2C8A.append('\0');

        QCA::InitializationVector OX5E3B9D2A(8);
        OX7D4B2C8A.prepend(OX5E3B9D2A.toByteArray());
    }
    else
    {
        OX7D4B2C8A = QByteArray::fromBase64(OX7D4B2C8A);
        while ((OX7D4B2C8A.length() % 8) != 0) OX7D4B2C8A.append('\0');
    }

    QCA::Direction OX6A3E4B1D = (OX3C2E1A9F) ? QCA::Encode : QCA::Decode;
    QCA::Cipher OX4C2E1A7F(OX9F6C3A2D, QCA::Cipher::CBC, QCA::Cipher::NoPadding, OX6A3E4B1D, OX4D9C2E6F, QCA::InitializationVector(QByteArray("0")));
    QByteArray OX5D8B3C1F = OX4C2E1A7F.update(QCA::MemoryRegion(OX7D4B2C8A)).toByteArray();
    OX5D8B3C1F += OX4C2E1A7F.final().toByteArray();

    if (!OX4C2E1A7F.ok())
        return OX8B4F1C7D;

    if (OX3C2E1A9F)
        OX5D8B3C1F = OX5D8B3C1F.toBase64();
    else
        OX5D8B3C1F = OX5D8B3C1F.remove(0, 8);

    return OX5D8B3C1F;
}


QByteArray OX7B4DF339::OX8F2D1C7B(QByteArray OX8B4F1C7D, bool OX3C2E1A9F)
{
    QCA::Initializer OX9D7A3F6B;
    QByteArray OX7D4B2C8A = OX8B4F1C7D;

    if (OX3C2E1A9F)
    {
        while ((OX7D4B2C8A.length() % 8) != 0) OX7D4B2C8A.append('\0');
    }
    else
    {
        if ((OX7D4B2C8A.length() % 12) != 0)
            return OX8B4F1C7D;

        OX7D4B2C8A = OX5B2E1A9D(OX7D4B2C8A);
        while ((OX7D4B2C8A.length() % 8) != 0) OX7D4B2C8A.append('\0');
    }

    QCA::Direction OX6A3E4B1D = (OX3C2E1A9F) ? QCA::Encode : QCA::Decode;
    QCA::Cipher OX4C2E1A7F(OX9F6C3A2D, QCA::Cipher::ECB, QCA::Cipher::NoPadding, OX6A3E4B1D, OX4D9C2E6F);
    QByteArray OX5D8B3C1F = OX4C2E1A7F.update(QCA::MemoryRegion(OX7D4B2C8A)).toByteArray();
    OX5D8B3C1F += OX4C2E1A7F.final().toByteArray();

    if (!OX4C2E1A7F.ok())
        return OX8B4F1C7D;

    if (OX3C2E1A9F) {
        if ((OX5D8B3C1F.length() % 8) != 0)
            return OX8B4F1C7D;

        OX5D8B3C1F = OX1C7B4E9A(OX5D8B3C1F);
    }

    return OX5D8B3C1F;
}


QByteArray OX7B4DF339::OX1C7B4E9A(QByteArray OX7D4B2C8A)
{
    int OX3A7C1D8F = 0;
    int OX5E9B3D2A = 0;
    int OX2B1D7F6C = -1;
    int OX6F3A9C1E;
    QString OX4E1A7F2C = "./0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    QByteArray OX9B2E4D7C;
    while (OX2B1D7F6C < (OX7D4B2C8A.length() - 1)) {
        OX2B1D7F6C++;
        OX6F3A9C1E = OX7D4B2C8A.at(OX2B1D7F6C); if (OX6F3A9C1E < 0) OX6F3A9C1E += 256;
        OX3A7C1D8F = OX6F3A9C1E << 24;
        OX2B1D7F6C++;
        OX6F3A9C1E = OX7D4B2C8A.at(OX2B1D7F6C); if (OX6F3A9C1E < 0) OX6F3A9C1E += 256;
        OX3A7C1D8F += OX6F3A9C1E << 16;
        OX2B1D7F6C++;
        OX6F3A9C1E = OX7D4B2C8A.at(OX2B1D7F6C); if (OX6F3A9C1E < 0) OX6F3A9C1E += 256;
        OX3A7C1D8F += OX6F3A9C1E << 8;
        OX2B1D7F6C++;
        OX6F3A9C1E = OX7D4B2C8A.at(OX2B1D7F6C); if (OX6F3A9C1E < 0) OX6F3A9C1E += 256;
        OX3A7C1D8F += OX6F3A9C1E;

        OX2B1D7F6C++;
        OX6F3A9C1E = OX7D4B2C8A.at(OX2B1D7F6C); if (OX6F3A9C1E < 0) OX6F3A9C1E += 256;
        OX5E9B3D2A = OX6F3A9C1E << 24;
        OX2B1D7F6C++;
        OX6F3A9C1E = OX7D4B2C8A.at(OX2B1D7F6C); if (OX6F3A9C1E < 0) OX6F3A9C1E += 256;
        OX5E9B3D2A += OX6F3A9C1E << 16;
        OX2B1D7F6C++;
        OX6F3A9C1E = OX7D4B2C8A.at(OX2B1D7F6C); if (OX6F3A9C1E < 0) OX6F3A9C1E += 256;
        OX5E9B3D2A += OX6F3A9C1E << 8;
        OX2B1D7F6C++;
        OX6F3A9C1E = OX7D4B2C8A.at(OX2B1D7F6C); if (OX6F3A9C1E < 0) OX6F3A9C1E += 256;
        OX5E9B3D2A += OX6F3A9C1E;

        for (int OX4D3A7C1E = 0; OX4D3A7C1E < 6; OX4D3A7C1E++) {
            OX9B2E4D7C.append(OX4E1A7F2C.at(OX5E9B3D2A & 0x3F).toAscii());
            OX5E9B3D2A = OX5E9B3D2A >> 6;
        }

        for (int OX4D3A7C1E = 0; OX4D3A7C1E < 6; OX4D3A7C1E++) {
            OX9B2E4D7C.append(OX4E1A7F2C.at(OX3A7C1D8F & 0x3F).toAscii());
            OX3A7C1D8F = OX3A7C1D8F >> 6;
        }
    }
    return OX9B2E4D7C;
}


QByteArray OX7B4DF339::OX5B2E1A9D(QByteArray OX8D4F3C2E)
{
    QString OX4E1A7F2C = "./0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    QByteArray OX9B2E4D7C;
    int OX2B1D7F6C = -1;
    while (OX2B1D7F6C < (OX8D4F3C2E.length() - 1)) {
        int OX5E9B3D2A = 0;
        int OX3A7C1D8F = 0;
        int OX6F3A9C1E = 0;
        int OX7D4C2B8A = 0;
        int OX9D6A3B2C = 0;

        for (int OX4D3A7C1E = 0; OX4D3A7C1E < 6; OX4D3A7C1E++) {
            OX2B1D7F6C++;
            OX6F3A9C1E = OX4E1A7F2C.indexOf(OX8D4F3C2E.at(OX2B1D7F6C));
            OX5E9B3D2A |= OX6F3A9C1E << (OX4D3A7C1E * 6);
        }

        for (int OX4D3A7C1E = 0; OX4D3A7C1E < 6; OX4D3A7C1E++) {
            OX2B1D7F6C++;
            OX6F3A9C1E = OX4E1A7F2C.indexOf(OX8D4F3C2E.at(OX2B1D7F6C));
            OX3A7C1D8F |= OX6F3A9C1E << (OX4D3A7C1E * 6);
        }

        for (int OX4D3A7C1E = 0; OX4D3A7C1E < 4; OX4D3A7C1E++) {
            OX7D4C2B8A = ((OX3A7C1D8F & (0xFF << ((3 - OX4D3A7C1E) * 8))));
            OX9D6A3B2C = OX7D4C2B8A >> ((3 - OX4D3A7C1E) * 8);
            if (OX9D6A3B2C < 0) { OX9D6A3B2C = OX9D6A3B2C + 256; }
            OX9B2E4D7C.append(OX9D6A3B2C);
        }

        for (int OX4D3A7C1E = 0; OX4D3A7C1E < 4; OX4D3A7C1E++) {
            OX7D4C2B8A = ((OX5E9B3D2A & (0xFF << ((3 - OX4D3A7C1E) * 8))));
            OX9D6A3B2C = OX7D4C2B8A >> ((3 - OX4D3A7C1E) * 8);
            if (OX9D6A3B2C < 0) { OX9D6A3B2C = OX9D6A3B2C + 256; }
            OX9B2E4D7C.append(OX9D6A3B2C);
        }
    }
    return OX9B2E4D7C;
}


bool OX7B4DF339::OX3A7D5C2E()
{
    QCA::Initializer OX9D7A3F6B;

    if (QCA::isSupported("blowfish-ecb") && QCA::isSupported("blowfish-cbc") && QCA::isSupported("dh"))
        return true;

    return false;
}