/*	QtExt - Qt-based utility classes and functions (extends Qt library)

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Heiko Fechner

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
*/

#include "QtExt_Settings.h"

#include <QSettings>
#include <QProcess> // for open text editor
#include <QCheckBox>

#include <IBK_ArgParser.h>

//#include "DelDirectories.h"
//#include "DelConversion.h"

namespace QtExt {


Settings::Settings(const QString & organization, const QString & appName) :
	m_organization(organization),
	m_appName(appName),
	m_maxRecentProjects(10),
	m_maxNumUNDOSteps(50)
{
	for(unsigned int i=0; i<NumCmdLineFlags; ++i)
		m_flags[i] = false;
}


void Settings::setDefaults() {
	// compose default values

	// installation directory
	m_installDir = qApp->applicationDirPath();

	// clear last project file (if any, it is read in the read() function)
	m_lastProjectFile.clear();

	// set default log level
	m_userLogLevelConsole = IBK::VL_STANDARD;
	m_userLogLevelLogfile = IBK::VL_INFO;

	m_maxRecentProjects = 10;

	m_maxNumUNDOSteps = 10000;

	// determine text executable
	m_textEditorExecutable.clear();
#ifdef Q_OS_UNIX
	m_textEditorExecutable = "gedit";
#elif defined(Q_OS_WIN)
	m_textEditorExecutable = "C:\\Program Files (x86)\\Notepad++\\notepad++.exe";
#else
	// OS x editor?
#endif

	/// \todo Implement default text editor detection

	m_flags[NoSplashScreen] = false;

}


void Settings::updateArgParser(IBK::ArgParser & argParser) {
	argParser.addFlag(0, "no-splash", "Disables splash screen.");
	argParser.addOption(0, "lang", "Specify the program language using a 2 character language ID.", "en;de;it;...", "en");
}


void Settings::applyCommandLineArgs(const IBK::ArgParser & argParser) {
	if (argParser.flagEnabled("no-splash"))
		m_flags[NoSplashScreen] = true;
	if (argParser.hasOption("lang"))
		m_langId = QString::fromStdString(argParser.option("lang"));
	// first positional argument is project file
	if (argParser.args().size() > 1){
		std::string str = argParser.args()[1];
#ifdef Q_OS_WIN
		// On Windows, use codepage encoding instead of UTF8
		m_initialProjectFile = QString::fromLatin1(str.c_str() );
#else
		m_initialProjectFile = QString::fromUtf8( str.c_str() );
#endif
	}
}


void Settings::read() {
	//const char * const FUNC_ID = "[Settings::read]";

	QSettings settings( m_organization, m_appName );

	m_lastProjectFile = settings.value("LastProjectFile", QString()).toString();
	m_recentProjects = settings.value("RecentProjects", QStringList()).toStringList();

	m_maxRecentProjects = settings.value("MaxRecentProjects", m_maxRecentProjects).toUInt();
	m_maxNumUNDOSteps = settings.value("MaxNumUndoSteps", m_maxNumUNDOSteps).toUInt();

	QString tmpTextEditorExecutable = settings.value("TextEditorExecutable", m_textEditorExecutable ).toString();
	if (!tmpTextEditorExecutable.isEmpty())
		m_textEditorExecutable = tmpTextEditorExecutable;
	m_langId = settings.value("LangID", QString() ).toString();

	m_userLogLevelConsole = (IBK::verbosity_levels_t)settings.value("UserLogLevelConsole", m_userLogLevelConsole ).toInt();
	m_userLogLevelLogfile = (IBK::verbosity_levels_t)settings.value("UserLogLevelLogfile", m_userLogLevelLogfile ).toInt();

	int count = settings.beginReadArray("DoNotShowAgainDialogs");
	for (int i=0; i<count; ++i) {
		settings.setArrayIndex(i);
		QString s = settings.value("DialogID").toString();
		bool checked = settings.value("Checked", false).toBool();
		if (!s.isEmpty())
			m_doNotShowAgainDialogs[s] = checked;
	}
	settings.endArray();

	count = settings.beginReadArray("DoNotShowAgainDialogsAnswer");
	for (int i=0; i<count; ++i) {
		settings.setArrayIndex(i);
		QString s = settings.value("DialogID").toString();
		QMessageBox::StandardButton answer = static_cast<QMessageBox::StandardButton>(settings.value("Answer", QMessageBox::NoButton).toInt());
		if (!s.isEmpty())
			m_doNotShowAgainDialogsAnswer[s] = answer;
	}
	settings.endArray();


}


void Settings::readMainWindowSettings(QByteArray &geometry, QByteArray &state) {
	QSettings settings( m_organization, m_appName );
	geometry = settings.value("MainWindowGeometry", QByteArray()).toByteArray();
	state = settings.value("MainWindowState", QByteArray()).toByteArray();
}


void Settings::write(QByteArray geometry, QByteArray state) {

	QSettings settings( m_organization, m_appName );
	settings.setValue("LastProjectFile", m_lastProjectFile);
	settings.setValue("RecentProjects", m_recentProjects);

	settings.setValue("MaxRecentProjects", m_maxRecentProjects );
	settings.setValue("UndoSize",m_maxNumUNDOSteps);

	settings.setValue("TextEditorExecutable", m_textEditorExecutable );
	settings.setValue("LangID", m_langId );

	settings.setValue("UserLogLevelConsole", m_userLogLevelConsole);
	settings.setValue("UserLogLevelLogfile", m_userLogLevelLogfile);

	settings.setValue("MainWindowGeometry", geometry);
	settings.setValue("MainWindowState", state);

	settings.beginWriteArray("DoNotShowAgainDialogs");
	int i=0;
	for (QMap<QString, bool>::const_iterator it = m_doNotShowAgainDialogs.constBegin();
		 it != m_doNotShowAgainDialogs.constEnd(); ++it, ++i)
	{
		settings.setArrayIndex(i);
		settings.setValue("DialogID", it.key());
		settings.setValue("Checked", it.value());
	}
	settings.endArray();

	settings.beginWriteArray("DoNotShowAgainDialogsAnswer");
	i=0;
	for (QMap<QString, QMessageBox::StandardButton>::const_iterator it = m_doNotShowAgainDialogsAnswer.constBegin();
		 it != m_doNotShowAgainDialogsAnswer.constEnd(); ++it, ++i)
	{
		settings.setArrayIndex(i);
		settings.setValue("DialogID", it.key());
		settings.setValue("Answer", (int)it.value());
	}
	settings.endArray();

}


bool Settings::openFileInTextEditor(QWidget * parent, const QString & filepath) const {
	// check if editor has been set in preferences
	if (m_textEditorExecutable.isEmpty()) {
		QMessageBox::critical(parent, tr("Missing user preferences"), tr("Please open the preferences dialog and specify "
																	   "a text editor first!"));
		return false;
	}

	bool res = QProcess::startDetached( m_textEditorExecutable, QStringList() << filepath );
	if (!res) {
		QMessageBox::critical(parent, tr("Error starting external application"), tr("Text editor '%1' could not be started.")
							  .arg(m_textEditorExecutable));
	}
	return res;
}


void Settings::showDoNotShowAgainMessage(QWidget * parent, const QString & doNotShowAgainDialogID,
									  const QString & title, const QString & msg)
{
	if (!m_doNotShowAgainDialogs.contains(doNotShowAgainDialogID) || !m_doNotShowAgainDialogs[doNotShowAgainDialogID]) {
		QMessageBox msgBox(QMessageBox::Information, title, msg, QMessageBox::Ok, parent);
		msgBox.setCheckBox( new QCheckBox(tr("Do not show this dialog again.")) );
		msgBox.exec();
		if (msgBox.checkBox()->isChecked())
			m_doNotShowAgainDialogs[doNotShowAgainDialogID] = true;
	}
}

QMessageBox::StandardButton Settings::showDoNotShowAgainQuestion(QWidget * parent, const QString & doNotShowAgainDialogID,
																 const QString & title, const QString & msg,
																 QMessageBox::StandardButtons buttons)
{
	if (!m_doNotShowAgainDialogs.contains(doNotShowAgainDialogID) || !m_doNotShowAgainDialogs[doNotShowAgainDialogID]) {
		QMessageBox msgBox(QMessageBox::Question, title, msg, buttons, parent);
		msgBox.setCheckBox( new QCheckBox(tr("Do not show this dialog again.")) );
		QMessageBox::StandardButton res = static_cast<QMessageBox::StandardButton>(msgBox.exec());
		if (msgBox.checkBox()->isChecked()) {
			m_doNotShowAgainDialogs[doNotShowAgainDialogID] = true;
			m_doNotShowAgainDialogsAnswer[doNotShowAgainDialogID] = res;
		}
		return res;
	}
	else {
		return m_doNotShowAgainDialogsAnswer[doNotShowAgainDialogID];
	}
}

} // namespace QtExt

