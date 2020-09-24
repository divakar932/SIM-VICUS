#ifndef SVMainWindow_H
#define SVMainWindow_H

#include <QMainWindow>
#include <QUndoStack>
#include <QProcess>

#include <map>

#include "SVProjectHandler.h"

namespace Ui {
	class SVMainWindow;
}

class QProgressDialog;
class SVThreadBase;
class SVWelcomeScreen;
class SVButtonBar;
class SVPreferencesDialog;
class SVPostProcHandler;
class SVLogWidget;

/*! Main window class. */
class SVMainWindow : public QMainWindow {
	Q_OBJECT
public:

	/*! Returns a pointer to the SVMainWindow instance.
		Only access this function during the lifetime of the
		SVMainWindow instance.
	*/
	static SVMainWindow & instance();

	/*! Adds an undo command to the global undo stack.
		Ownership of the command object will be transferred to the stack.
	*/
	static void addUndoCommand(QUndoCommand * command);

	/*! Default SVMainWindow constructor. */
	explicit SVMainWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = 0);

	/*! Default destructor. */
	~SVMainWindow();

	/*! Public access function to save project file (called from simulation view).
		\return Returns true if project was saved and project handler
				has now an unmodified project with valid project filename.
	*/
	bool saveProject();

	/*! Exports the current project to the selected exportFilePath path.
		Opens an (internationalized) message box on the first error encountered.
		\param exportFilePath Path to resultant project package
		\param withTopLevelDir If true, the files are stored within a top-level directory.
		\return Returns true on success, false on error.

		The following conventions on the directories within the zip-file are used:
		Suppose the project lies at: ../data/test.vicus
		Suppose exportFilePath = ../data/exports/testPackage.vicus

		When withTopLevelDir==false, Zip content:

		  db/
		  climate/
		  test.vicus

		When withTopLevelDir==true, Zip content:

		  test_package/
			db/
			climate/
			test.vicus

	*/
	bool exportProjectPackage(const QString & exportFilePath, bool withTopLevelDir);

protected:
	/*! Checks if project file has been changed by external application. */
	void changeEvent(QEvent *event);
	/*! Does the confirm-saving-before-close stuff. */
	void closeEvent(QCloseEvent * event);

private slots:
	void on_actionFileNew_triggered();
	void on_actionFileOpen_triggered();
	void on_actionFileSave_triggered();
	void on_actionFileSaveAs_triggered();
	void on_actionFileReload_triggered();
	void on_actionFileExport_triggered();
	void on_actionFileClose_triggered();
	void on_actionFileQuit_triggered();

	void on_actionEditTextEditProject_triggered();
	void on_actionEditPreferences_triggered();
	void on_actionEditCleanProject_triggered();

	void on_actionViewExternalPostProcessing_triggered();
	void on_actionViewCCMeditor_triggered();

	void on_actionHelpAboutQt_triggered();
	void on_actionHelpAbout_triggered();

	/*! Triggered when a recent file menu entry was clicked. */
	void onActionOpenRecentFile();

	/*! Triggered when a language menu entry was clicked. */
	void onActionSwitchLanguage();

	/*! Updates the state of all actions based on the current condition of the project.
		This slot is connected to the signal updateActions() from SVProjectHandler.
	*/
	void onUpdateActions();

	/*! Updates the menu entries in the 'Recent Projects' submenu.
		This is a slot because we need to update the menu with the actions
		a bit delayed. When the user clicks on a recent project menu entry, the
		loadProject() function is indirectly called which in turn calls
		updateRecentProjects(). Since the menu actions in the recent projects
		menu are deleted, this would mean that the action currently process is
		being deleted - causing a crash. Therefore we don't call updateRecentProjects()
		directly, but via a QTimer::singleShot() and thus ensure that the
		action handler function is completed before the action is touched.
	*/
	void onUpdateRecentProjects();

	/*! Opens a project with filename.
		Called from onActionOpenRecentFile() and from welcome screen.
	*/
	void onOpenProjectByFilename(const QString & filename);

	/*! Copies an example project (and its directory content) to a new directory (interactive) and
		opens the copied example project.
		Called only from welcome screen.
	*/
	void onOpenExampleByFilename(const QString & filename);

	/*! Copies an template project (and its directory content) to a new directory (interactive) and
		opens the copied template project.
		Called only from welcome screen.
	*/
	void onOpenTemplateByFilename(const QString & filename);

	/*! Connected to currentViewChanged() in navigation bar. */
	void onNavigationBarViewChanged(int view);

	/*! Triggered whenever a worker thread finishes. */
	void onWorkerThreadFinished();

	/*! Triggered whenever a project was read successfully. */
	void onFixProjectAfterRead();

	/*! Opens a web browser with a window for adding bug reports and feature wishes. */
	void on_actionHelpBugReport_triggered();

	/*! Triggered by the "Run simulation" shortcut (F9), launches simulation right away. */
	void on_actionRunSimulation_triggered();

	void on_actionHelpVisitDiscussionForum_triggered();

	void on_actionFileExportFMU_triggered();

private:
	/*! Sets up all dock widgets with definition lists. */
	void setupDockWidgets();

	/*! Updates the window title. */
	void updateWindowTitle();

	/*! Imports the project package to the selected import directory.
		Opens an (internationalized) message box on the first error encountered.
		\param packageFilePath Full path to package file.
		\param targetDirectory Target dir to extract package content into (no subdirectory is created)
		\param projectFilePath If package contains a project file, this file name (full path) will be stored
		\param isPackage True if package is a vicus package and should contain a vicus project file.
		\return Returns true on success, false on error.
	*/
	bool importProjectPackage(const QString & packageFilePath, const QString & targetDirectory,
							  QString & projectFilePath, bool isPackage);

	/*! Creates a thumbnail-image of the current project sketch.
		\return Returns full path to created thumbnail.
	*/
	QString saveThumbNail();

	/*! Adds another language setting action, when the corresponding language files exist. */
	void addLanguageAction(const QString & langId, const QString & actionCaption);

	/*! Appends the thread base to the thread pool and starts processing the threads.
		This function is usually called from the static function SVThreadBase::runWorkerThread().
	*/
	void runWorkerThreadImpl(SVThreadBase * base);

	/*! This will start the processing of the thread pool.
		If a thread is currently running, the call is ignored. */
	void processThread();

	/*! Caches visibility information of dock widgets. */
	void storeDockWidgetVisibility();

	/*! Tests of file is a project package, and if yes, asks user to select an extraction
		directory, imports the project and adjusts filename to point to the extracted
		project file.
		In case of error, an error message pops up (this is an interactive function!) and
		the function returns false. If all goes well, the function returns true.
		\param filename Full path to package file (if file has invalid extension, function silently returns).
		\param renameProjectFileAfterwards If set to true, the user is requested to specify target filename instead of
			just the extraction directory. After project is extracted it will then be renamed to match the selected file.
	*/
	bool processProjectPackage(QString & filename, bool renameProjectFileAfterwards);

	/*! This function does the actual copying of the project dependencies to the 'materials' and 'climate' subdirectories.
		\param targetDirPath Directory to export the project to (must exist and be empty).
		\param project The project to export.
	*/
	bool exportProjectCopy(QString targetDirPath, const VICUS::Project & project);


	/*! Global pointer to main window instance.
		Initialized in the constructor of SVMainWindow and
		reset to nullptr in the destructor. So be sure that the main window
		exists before accessing SVMainWindow::instance()
	*/
	static SVMainWindow			*m_self;

	/*! Map to store dock widget visibility.
		Initialized, after stored view configuration was read from file.
		Modified and applied in onNavigationBarViewChanged();
	*/
	std::map<QDockWidget*, bool>	m_dockWidgetVisibility;

	/*! Main user interface pointer. */
	Ui::SVMainWindow			*m_ui													= nullptr;
	/*! The global undo stack in the program. */
	QUndoStack					*m_undoStack											= nullptr;
	/*! Menu for the recent projects entries. */
	QMenu						*m_recentProjectsMenu									= nullptr;
	/*! List with action objects for each recent project in the main menu. */
	QList<QAction*>				m_recentProjectActions;
	/*! List of language actions shown in the languages menu. */
	QList<QAction*>				m_languageActions;
	/*! The project handler that manages the actual project. */
	SVProjectHandler			m_projectHandler;
	/*! The thread pool managed by the main window. */
	QList<SVThreadBase*>		m_threadPool;

	/*! The welcome screen. */
	SVWelcomeScreen				*m_welcomeScreen										= nullptr;

	/*! The navigation bar. */
	SVButtonBar					*m_buttonBar											= nullptr;

	/*! User preferences. */
	SVPreferencesDialog			*m_preferencesDialog									= nullptr;

	/*! FMI Export dialog. */
//	SVFMIExportDialog			*m_fmiExportDialog										= nullptr;

	QDockWidget					*m_logDockWidget;
	SVLogWidget					*m_logWidget;

	/*! Handles spawning/activating of PostProc executable. */
	SVPostProcHandler			*m_postProcHandler										= nullptr;

	friend class SVThreadBase;

};

#endif // SVMainWindow_H