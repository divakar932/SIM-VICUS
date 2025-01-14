#include "SVMainWindow.h"
#include <QApplication>
#include <QSplashScreen>
#include <QTimer>

#include <IBK_Exception.h>
#include <IBK_messages.h>
#include <IBK_ArgParser.h>
#include <IBK_BuildFlags.h>

#include <iostream>
#include <memory>

#include <QtExt_LanguageHandler.h>
#include <QtExt_Directories.h>
#include <QtExt_AutoUpdater.h>

#include "SVMessageHandler.h"
#include "SVSettings.h"
#include "SVConstants.h"
#include "SVDebugApplication.h"
#include "SVStyle.h"

/*! qDebug() message handler function, redirects debug messages to IBK::IBK_Message(). */
void qDebugMsgHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
	(void) type;
	std::string contextstr;
	if (context.file != nullptr && context.function != nullptr)
		contextstr = "[" + std::string(context.file) + "::" + std::string(context.function) + "]";
	IBK::IBK_Message(msg.toStdString(), IBK::MSG_DEBUG, contextstr.c_str(), IBK::VL_ALL);
}


int main(int argc, char *argv[]) {
	const char * const FUNC_ID = "[main]";

	QtExt::Directories::appname = "SIM-VICUS";
	QtExt::Directories::devdir = "SIM-VICUS";

	// create QApplication wrapped to catch rogue exceptions
	SVDebugApplication a(argc, argv);

	// install message handler to catch qDebug()
	qInstallMessageHandler(qDebugMsgHandler);

	// *** Locale setup for Unix/Linux ***
#if defined(Q_OS_UNIX)
	setlocale(LC_NUMERIC,"C");
#endif

	// Compose program name using the always use the major.minor version variant,
	// since this string is used to identify the registry/config file location.
	const QString ProgramVersionName = QString("SIM-VICUS %1").arg(VICUS::VERSION);

	qApp->setWindowIcon(QIcon(":/gfx/sim-vicus_icon_x64x64.png"));
	qApp->setApplicationName(ProgramVersionName);

	// disable ? button in windows
#if QT_VERSION >= 0x050A00
	QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
#elif QT_VERSION >= 0x050600
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

	// initialize resources in dependent libraries
	Q_INIT_RESOURCE(QtExt);

	// *** Create and initialize setting object of DSix Application ***

	SVSettings settings(ORG_NAME, ProgramVersionName);
	settings.setDefaults();
	settings.read();

	// *** Style Init ***

	SVStyle style; // constructor sets up most of the initialization

	// *** Initialize Command Line Argument Parser ***
	IBK::ArgParser argParser;
	settings.updateArgParser(argParser);
	argParser.setAppName(ProgramVersionName.toStdString());

	// *** Apply command line arguments ***
	argParser.parse(argc, argv);
	// handle default arguments (--help)
	if (argParser.flagEnabled("help")) {
		argParser.printHelp(std::cout);
		return EXIT_SUCCESS;
	}
	settings.applyCommandLineArgs(argParser);


	// *** Check for auto-update files ***
#if defined(Q_OS_WIN)
	QtExt::AutoUpdater autoUpdater;
	if (autoUpdater.installUpdateWhenAvailable(QtExt::Directories::updateFilePath()))
		return EXIT_SUCCESS;
#endif

	// *** Create log file directory and setup message handler ***
	QDir baseDir;
	baseDir.mkpath(QtExt::Directories::userDataDir());

	SVMessageHandler messageHandler;
	IBK::MessageHandlerRegistry::instance().setMessageHandler( &messageHandler );
	std::string errmsg;
	messageHandler.openLogFile(QtExt::Directories::globalLogFile().toUtf8().data(), false, errmsg);
	messageHandler.setConsoleVerbosityLevel( settings.m_userLogLevelConsole );
	messageHandler.setLogfileVerbosityLevel( settings.m_userLogLevelLogfile );


	// *** Install translator ***
	QtExt::LanguageHandler::instance().setup(SVSettings::instance().m_organization,
											 SVSettings::instance().m_appName,
											 QtExt::Directories::translationsDir(),
											 "SIM-VICUS" );
	if (argParser.hasOption("lang")) {
		std::string dummy = argParser.option("lang");
		QString langid = QString::fromStdString(dummy);
		if (langid != QtExt::LanguageHandler::instance().langId()) {
			IBK::IBK_Message( IBK::FormatString("Installing translator for language: '%1'.\n")
								.arg(langid.toUtf8().data()),
								IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			QtExt::LanguageHandler::instance().installTranslator(langid);
		}
	}
	else {
		QtExt::LanguageHandler::instance().installTranslator(QtExt::LanguageHandler::langId());
	}

	// set default language in IBK MultiLanguageString
	IBK::MultiLanguageString::m_language = QtExt::LanguageHandler::langId().toStdString();


	// *** Create and show splash-screen ***
	std::unique_ptr<QSplashScreen> splash;

	if (!settings.m_flags[SVSettings::NoSplashScreen]) {
		QPixmap pixmap;
		pixmap.load(":/gfx/splashscreen/SIM-VICUS.jpg");
		splash.reset(new QSplashScreen(pixmap, Qt::WindowStaysOnTopHint | Qt::SplashScreen));
		splash->show();
		QTimer::singleShot(5000, splash.get(), SLOT(close()));
	}


	// *** Setup and show MainWindow and start event loop ***
	int res;
	try { // open scope to control lifetime of main window, ensure that main window instance dies before settings or project handler

		SVMainWindow w;

		// start event loop
		res = a.exec();
	} // here our mainwindow dies, main window goes out of scope and UI goes down -> destructor does ui and thread cleanup
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		return EXIT_FAILURE;
	}

	// return exit code to environment
	return res;
}
