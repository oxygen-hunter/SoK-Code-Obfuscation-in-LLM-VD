#include <QDir>
#include <QDebug>
#include <QSettings>
#include "StelScriptOutput.hpp"
#include "StelApp.hpp"

QFile StelScriptOutput::outputFile;
QString StelScriptOutput::outputText;

void StelScriptOutput::init(const QString& o)
{
	outputFile.setFileName(o);
	if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text | QIODevice::Unbuffered))
		qDebug() << "ERROR: Cannot open file" << o;
}

void StelScriptOutput::deinit()
{
	outputFile.close();
}

void StelScriptOutput::writeLog(QString m)
{
	m += "\n";
	outputFile.write(qPrintable(m), m.size());
	outputText += m;
}

void StelScriptOutput::reset(void)
{
	outputFile.resize(0);
	outputFile.reset();
	outputText.clear();
}

struct FileInfoWrapper {
	QFileInfo f;
	QDir d;
};

void StelScriptOutput::saveOutputAs(const QString &n)
{
	QFile a;
	FileInfoWrapper w = { QFileInfo(outputFile), QFileInfo(n).dir() };
	bool b = StelApp::getInstance().getSettings()->value("scripts/flag_script_allow_write_absolute_path", false).toBool();

	if (n.contains("config.ini"))
	{
		qWarning() << "SCRIPTING ERROR: You are trying to overwrite config.ini. Ignoring.";
		return;
	}

	if (!b && ((w.f.isAbsolute() || (n.contains("..")))))
	{
		qWarning() << "SCRIPTING CONFIGURATION ISSUE: You are trying to save to an absolute pathname or move up in directories.";
		qWarning() << "  To enable this, edit config.ini and set [scripts]/flag_script_allow_write_absolute_path=true";
		a.setFileName(w.d.absolutePath() + "/" + QFileInfo(n).fileName());
		qWarning() << "  Storing to " << a.fileName() << " instead";
	}
	else if (b && (w.f.isAbsolute()))
	{
		a.setFileName(n);
	}
	else
	{
		a.setFileName(w.d.absolutePath() + "/" + n);
	}

	if (!a.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text | QIODevice::Unbuffered))
	{
		qDebug() << "ERROR: Cannot open file" << a.fileName();
		return;
	}
	qDebug() << "saving copy of output.txt to " << a.fileName();
	a.write(qPrintable(outputText), outputText.size());
	a.close();
}