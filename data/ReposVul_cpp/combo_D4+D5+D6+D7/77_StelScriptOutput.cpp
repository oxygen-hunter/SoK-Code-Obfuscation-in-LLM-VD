#include <QDir>
#include <QDebug>
#include <QSettings>
#include "StelScriptOutput.hpp"
#include "StelApp.hpp"

// Init static variables.
QFile StelScriptOutput::outputFile;
QString StelScriptOutput::outputText;

void StelScriptOutput::init(const QString& a)
{
	QString b = a;
	outputFile.setFileName(b);
	if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text | QIODevice::Unbuffered))
		qDebug() << "ERROR: Cannot open file" << b;
}

void StelScriptOutput::deinit()
{
	outputFile.close();
}

void StelScriptOutput::writeLog(QString x)
{
	QString y = x + "\n";
	outputFile.write(qPrintable(y), y.size());
	outputText += y;
}

void StelScriptOutput::reset(void)
{
	outputFile.resize(0);
	outputFile.reset();
	outputText.clear();
}

void StelScriptOutput::saveOutputAs(const QString &z)
{
	QFile a;
	QFileInfo b = QFileInfo(outputFile);
	QDir c = b.dir();
	QFileInfo d = QFileInfo(z);
	bool e = StelApp::getInstance().getSettings()->value("scripts/flag_script_allow_write_absolute_path", false).toBool();

	if (!e && ((d.isAbsolute() || (z.contains("..")))))
	{
		qWarning() << "SCRIPTING CONFIGURATION ISSUE: You are trying to save to an absolute pathname or move up in directories.";
		qWarning() << "  To enable this, edit config.ini and set [scripts]/flag_script_allow_write_absolute_path=true";
		a.setFileName(c.absolutePath() + "/" + d.fileName());
		qWarning() << "  Storing to " << a.fileName() << " instead";
	}
	else if (e && (d.isAbsolute()))
	{
		a.setFileName(z);
	}
	else
	{
		a.setFileName(c.absolutePath() + "/" + z);
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