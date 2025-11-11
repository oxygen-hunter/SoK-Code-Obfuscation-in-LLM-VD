#include <QDir>
#include <QDebug>
#include <QSettings>
#include "OX7B4DF339.hpp"
#include "OX1A2C3E4F.hpp"

QFile OX7B4DF339::OX9C8D7E6F;
QString OX7B4DF339::OX5A7B8C9D;

void OX7B4DF339::OX1D2E3F4A(const QString& OX6B7C8D9E)
{
	OX9C8D7E6F.setFileName(OX6B7C8D9E);
	if (!OX9C8D7E6F.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text | QIODevice::Unbuffered))
		qDebug() << "ERROR: Cannot open file" << OX6B7C8D9E;
}

void OX7B4DF339::OX2E3F4A5B()
{
	OX9C8D7E6F.close();
}

void OX7B4DF339::OX3F4A5B6C(QString OX5C6D7E8F)
{
	OX5C6D7E8F += "\n";
	OX9C8D7E6F.write(qPrintable(OX5C6D7E8F), OX5C6D7E8F.size());
	OX5A7B8C9D += OX5C6D7E8F;
}

void OX7B4DF339::OX4A5B6C7D(void)
{
	OX9C8D7E6F.resize(0);
	OX9C8D7E6F.reset();
	OX5A7B8C9D.clear();
}

void OX7B4DF339::OX5B6C7D8E(const QString &OX7E8F9A0B)
{
	QFile OX8F9A0B1C;
	const QFileInfo OX9A0B1C2D(OX9C8D7E6F);
	const QDir OX0B1C2D3E=OX9A0B1C2D.dir();
	const QFileInfo OX1C2D3E4F(OX7E8F9A0B);

	const bool OX2D3E4F5A=OX1A2C3E4F::getInstance().getSettings()->value("scripts/flag_script_allow_write_absolute_path", false).toBool();

	if (!OX2D3E4F5A && ((OX1C2D3E4F.isAbsolute() || (OX7E8F9A0B.contains("..")))))
	{
		qWarning() << "SCRIPTING CONFIGURATION ISSUE: You are trying to save to an absolute pathname or move up in directories.";
		qWarning() << "  To enable this, edit config.ini and set [scripts]/flag_script_allow_write_absolute_path=true";
		OX8F9A0B1C.setFileName(OX0B1C2D3E.absolutePath() + "/" + OX1C2D3E4F.fileName());
		qWarning() << "  Storing to " << OX8F9A0B1C.fileName() << " instead";
	}
	else if (OX2D3E4F5A && (OX1C2D3E4F.isAbsolute()))
	{
		OX8F9A0B1C.setFileName(OX7E8F9A0B);
	}
	else
	{
		OX8F9A0B1C.setFileName(OX0B1C2D3E.absolutePath() + "/" + OX7E8F9A0B);
	}

	if (!OX8F9A0B1C.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text | QIODevice::Unbuffered))
	{
		qDebug() << "ERROR: Cannot open file" << OX8F9A0B1C.fileName();
		return;
	}
	qDebug() << "saving copy of output.txt to " << OX8F9A0B1C.fileName();
	OX8F9A0B1C.write(qPrintable(OX5A7B8C9D), OX5A7B8C9D.size());
	OX8F9A0B1C.close();
}