#include <QDir>
#include <QDebug>
#include <QSettings>
#include "OX7B4DF339.hpp"
#include "OX3A9C256A.hpp"

// Init static variables.
QFile OX7B4DF339::OX5C3F7A51;
QString OX7B4DF339::OX1A2E5C47;

void OX7B4DF339::OX1E2D4F72(const QString& OX4B8E3D1C)
{
	OX5C3F7A51.setFileName(OX4B8E3D1C);
	if (!OX5C3F7A51.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text | QIODevice::Unbuffered))
		qDebug() << "ERROR: Cannot open file" << OX4B8E3D1C;
}

void OX7B4DF339::OX2F3A6D28()
{
	OX5C3F7A51.close();
}

void OX7B4DF339::OX3F4B2E14(QString OX7D1E5A39)
{
	OX7D1E5A39 += "\n";
	OX5C3F7A51.write(qPrintable(OX7D1E5A39), OX7D1E5A39.size());
	OX1A2E5C47 += OX7D1E5A39;
}

void OX7B4DF339::OX4B2C5E17(void)
{
	OX5C3F7A51.resize(0);
	OX5C3F7A51.reset();
	OX1A2E5C47.clear();
}

void OX7B4DF339::OX5A3D1C49(const QString &OX6F4B2E1A)
{
	QFile OX2D3A6F48;
	const QFileInfo OX8E1F5A3C(OX5C3F7A51);
	const QDir OX9A4F2B1D=OX8E1F5A3C.dir(); 
	const QFileInfo OX0C5A3F2E(OX6F4B2E1A);

	const bool OX7C1E5A4F=OX3A9C256A::getInstance().getSettings()->value("scripts/flag_script_allow_write_absolute_path", false).toBool();

	if (OX6F4B2E1A.contains("config.ini"))
	{
		qWarning() << "SCRIPTING ERROR: You are trying to overwrite config.ini. Ignoring.";
		return;
	}

	if (!OX7C1E5A4F && ((OX0C5A3F2E.isAbsolute() || (OX6F4B2E1A.contains("..")))))
	{
		qWarning() << "SCRIPTING CONFIGURATION ISSUE: You are trying to save to an absolute pathname or move up in directories.";
		qWarning() << "  To enable this, edit config.ini and set [scripts]/flag_script_allow_write_absolute_path=true";
		OX2D3A6F48.setFileName(OX9A4F2B1D.absolutePath() + "/" + OX0C5A3F2E.fileName());
		qWarning() << "  Storing to " << OX2D3A6F48.fileName() << " instead";
	}
	else if (OX7C1E5A4F && (OX0C5A3F2E.isAbsolute()))
	{
		OX2D3A6F48.setFileName(OX6F4B2E1A);
	}
	else
	{
		OX2D3A6F48.setFileName(OX9A4F2B1D.absolutePath() + "/" + OX6F4B2E1A);
	}

	if (!OX2D3A6F48.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text | QIODevice::Unbuffered))
	{
		qDebug() << "ERROR: Cannot open file" << OX2D3A6F48.fileName();
		return;
	}
	qDebug() << "saving copy of output.txt to " << OX2D3A6F48.fileName();
	OX2D3A6F48.write(qPrintable(OX1A2E5C47), OX1A2E5C47.size());
	OX2D3A6F48.close();
}