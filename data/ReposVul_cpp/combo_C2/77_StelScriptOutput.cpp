/*
 * Stellarium
 * Copyright (C) 2014 Alexander Wolf
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA  02110-1335, USA.
 */

#include <QDir>
#include <QDebug>
#include <QSettings>
#include "StelScriptOutput.hpp"
#include "StelApp.hpp"

// Init static variables.
QFile StelScriptOutput::outputFile;
QString StelScriptOutput::outputText;

void StelScriptOutput::init(const QString& outputFilePath)
{
	int state = 0;
	while (state != -1) {
		switch (state) {
			case 0:
				outputFile.setFileName(outputFilePath);
				state = 1;
				break;
			case 1:
				if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text | QIODevice::Unbuffered)) {
					qDebug() << "ERROR: Cannot open file" << outputFilePath;
				}
				state = -1;
				break;
		}
	}
}

void StelScriptOutput::deinit()
{
	int state = 0;
	while (state != -1) {
		switch (state) {
			case 0:
				outputFile.close();
				state = -1;
				break;
		}
	}
}

void StelScriptOutput::writeLog(QString msg)
{
	int state = 0;
	while (state != -1) {
		switch (state) {
			case 0:
				msg += "\n";
				state = 1;
				break;
			case 1:
				outputFile.write(qPrintable(msg), msg.size());
				state = 2;
				break;
			case 2:
				outputText += msg;
				state = -1;
				break;
		}
	}
}

void StelScriptOutput::reset(void)
{
	int state = 0;
	while (state != -1) {
		switch (state) {
			case 0:
				outputFile.resize(0);
				state = 1;
				break;
			case 1:
				outputFile.reset();
				state = 2;
				break;
			case 2:
				outputText.clear();
				state = -1;
				break;
		}
	}
}

void StelScriptOutput::saveOutputAs(const QString &name)
{
	QFile asFile;
	const QFileInfo outputInfo(outputFile);
	const QDir dir=outputInfo.dir(); 
	const QFileInfo newFileNameInfo(name);
	const bool okToSaveToAbsolutePath=StelApp::getInstance().getSettings()->value("scripts/flag_script_allow_write_absolute_path", false).toBool();

	int state = 0;
	while (state != -1) {
		switch (state) {
			case 0:
				if (!okToSaveToAbsolutePath && ((newFileNameInfo.isAbsolute() || (name.contains(".."))))) {
					qWarning() << "SCRIPTING CONFIGURATION ISSUE: You are trying to save to an absolute pathname or move up in directories.";
					qWarning() << "  To enable this, edit config.ini and set [scripts]/flag_script_allow_write_absolute_path=true";
					asFile.setFileName(dir.absolutePath() + "/" + newFileNameInfo.fileName());
					qWarning() << "  Storing to " << asFile.fileName() << " instead";
					state = 3;
				} else if (okToSaveToAbsolutePath && (newFileNameInfo.isAbsolute())) {
					asFile.setFileName(name);
					state = 2;
				} else {
					asFile.setFileName(dir.absolutePath() + "/" + name);
					state = 2;
				}
				break;
			case 2:
				if (!asFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text | QIODevice::Unbuffered)) {
					qDebug() << "ERROR: Cannot open file" << asFile.fileName();
					state = -1;
				} else {
					state = 4;
				}
				break;
			case 3:
				state = 2;
				break;
			case 4:
				qDebug() << "saving copy of output.txt to " << asFile.fileName();
				state = 5;
				break;
			case 5:
				asFile.write(qPrintable(outputText), outputText.size());
				asFile.close();
				state = -1;
				break;
		}
	}
}