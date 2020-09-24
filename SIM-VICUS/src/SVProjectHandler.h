#ifndef SVProjectHandlerH
#define SVProjectHandlerH

#include <QObject>
#include <QCoreApplication>
#include <QString>
#include <QDateTime>

#include <IBK_Path.h>

namespace VICUS {
	class Project;
}

/*! Utility class that manages common project-related functions, in particular
	modification state setting, project file reading/writing and other.
	A project handler maintains a project instance with all project-related data.

	It provides GUI functionality like loadProject(), saveProject() and
	saveWithNewFilename(). These functions require a parent widget as function
	argument.
*/
class SVProjectHandler : public QObject {
	Q_OBJECT
	Q_DISABLE_COPY(SVProjectHandler)
public:

	/*! Returns singleton instance of project handler. */
	static SVProjectHandler & instance();

	/*! Modification types used in undo actions.
		\see setModified()
	*/
	enum ModificationTypes {

		/*! Used whenever a solver/model parameter (NANDRAD::SolverParameters) has changed. */
		SolverParametersModified,
		/*! Used when a climate location parameter (CCM file path, longitude, latitude, timezone) has changed. */
		ClimateLocationModified,
		/*! Used whenever the project data changes completely (new project created, project loaded etc.)
			and a complete reset of all views and models is needed. */
		AllModified = 0x00FF0001
	};

	/*! Abstract base class for all modification data containers. */
	class ModificationInfo {
	public:

		ModificationTypes	m_type = AllModified;
	};

	/*! Possible outcomes of the saveWithNewFilename() function. */
	enum SaveResult {
		SaveOK,			///< All ok.
		SaveFailed,		///< Couldn't write file.
		SaveCanceled	///< User canceled file name request dialog.
	};

	/*! Constructor, only to be used by SVMainWindow (but SVMainWindow must not be a friend of us). */
	SVProjectHandler();

	/*! Destructor, destroys managed project. */
	~SVProjectHandler();

	/*! Returns whether or not a project had been created yet. */
	bool isValid() const { return m_project != nullptr; }

	/*! Returns 'true' if some part of the project was modified.*/
	bool isModified() const { return m_modified; }

	/*! Return the project file of the currently managed project.
		\return Returns a file path when the current project has been
				saved already. Returns an empty string when the project
				is newly created and hasn't been saved yet.
	*/
	QString projectFile() const { return m_projectFile; }

	/*! Creates a new project instance (must not have one already) and resets project file name.
		Emits updateActions() to signal that view state has changed.
		\return Returns false if New Project Wizard was aborted by user.
	*/
	bool newProject(QWidget * parent);

	/*! Checks for modifications and asks user to confirm saving, then closes project.
		Also destroys project object.
		Emits updateActions() signal.
		\note The function can be safely called regardless of the valid state of the project handler.
			  Function does nothing of no project exists.
	*/
	bool closeProject(QWidget * parent);

	/*! Creates a new project and loads a project file with given filename.
		This function calles internally setModified(AllModified) when reading
		was successful.
		If project cannot be loaded, the project is destroyed again and the
		function returns with invalid state of project manager.
		Emits updateActions() signal on success.
		\param parent Parent widget, needed for QMessageBox
		\param filename Filename of project file to read.
		\param silent If true, error messages won't pop-up as dialog box but rather be
					  sent to IBK::IBK_Message().
	*/
	void loadProject(	QWidget * parent,
						const QString & filename,
						bool silent);

	/*! Closes project (discarding modifications) and reopens the project.
		Project must have a valid filename already.
	*/
	void reloadProject(QWidget *parent);

	/*! Saves project with new filename (interactive function, asks user to input filename).
		Calls saveProject() internally.
	*/
	SaveResult saveWithNewFilename(QWidget * parent);

	/*! Saves project with new filename into the template folder (interactive function, asks user to input filename).
		Calls saveProject() internally.
	*/
	SaveResult saveAsTemplate(QWidget * parent);

	/*! Saves the project with the given filename (non-interactive).
		Emits updateActions() signal on success.
		\param parent Pointe to parent widget. Used in MessageBox.
		\param fileName Filepath for the project file. Should not contain placeholder. A missing project file extension will be added.
		\param addToRecentFilesList If true file will be added to recent file list if save was successful.
	*/
	SaveResult saveProject(QWidget * parent, const QString & fileName, bool addToRecentFilesList = true);

	/*! Interface function for the user interface that allows
		different levels of modifications to be signalled to the project.

		\param modificationType An integer value defining the type of
			modification. The implementation should handle the default
			modification types (\sa DefaultModificationTypes).

		\param data The optional data argument can contain additional
			information about the specifics of the modification event.
			The implementation should cast the data pointer into
			corresponding types according to modification type.

		This function should be re-implemented in the derived class to
		handle all modifications to the project data.

		This function emits the modified() signal.

		If NotModified is passed as argument, the modification flag is reset.
		In this case the modified() signal is not emitted.

		The default implementation emits the modified() signal passing
		the function arguments and signal arguments.

		Emits updateActions() signal.
	*/
	void setModified(int modificationType, ModificationInfo * data = nullptr);

	/*! Returns a const reference to the internal project.
		Users of this function must include 'VICUS_project.h' directly.
		\warning This function throws an exception if there isn't a valid project loaded.
	*/
	const VICUS::Project & project() const;

	/*! Returns the time stamp of the last modification of the current project. */
	const QDateTime lastReadTime() const { return m_lastReadTime; }

	/*! Updates time stamp of the last modification of the current project, but only if project is present. */
	void updateLastReadTime();

	/*! Convenience function to be used for path placeholder substitution.
		This function takes the project's path placeholders, adds/overwrites default path placeholders
		and the "Project Directory" placeholder and then calls the IBK::replace_path_placeholders()
		function.
	*/
	IBK::Path replacePathPlaceholders(const IBK::Path & stringWithPlaceholders);

	/*! Set the reload state to true.*/
	void setReload() { m_reload = true; }

signals:
	/*! Emitted when the project has been modified.

		The signal should be emitted from the setModified() function and passes the
		modification event info.
		\param modificationType Modification type (you need to cast the int into ModificationType).
		\param data The optional data argument (nullptr if unused)
		\sa ModificationTypes
	*/
	void modified( int modificationType, ModificationInfo * data );

	/*! Emitted from closeProject(), loadProject(), saveProject() and saveWithNewFilename(), whenever
		file name or modification state changes.
		This signal is connected to the corresponding function in the SVMainWindow to update the ui-state.
	*/
	void updateActions();

	/*! A signal emitted from addToRecentFiles()

		Updates the menu entries in the 'Recent Projects' submenu.
		This is a slot because we need to update the menu with the actions
		a bit delayed. When the user clicks on a recent project menu entry, the
		loadProject() function is indirectly called which in turn calls
		updateRecentProjects(). Since the menu actions in the recent projects
		menu are deleted, this would mean that the action currently processed is
		being deleted - causing a crash. Therefore we don't call updateRecentProjects()
		directly, but via a QTimer::singleShot() and thus ensure that the
		action handler function is completed before the action is touched.
	*/
	void updateRecentProjects();

	/*! Emitted after project was read successfully, connected to corresponding slot in main window
		that creates undo-actions based on user-agreed modifications.

		In this slot you can implement any user-dialogs where you ask for optional project adjustments/fixes.
	*/
	void fixProjectAfterRead();

private:

	/*! Creates a new project instance (must not have one already) and resets project file name.
		This function is used internally.
		\note Does not emit any signals and does not call setModified()
	*/
	void createProject();

	/*! Deletes the project being wrapped and sets internal m_project to nullptr.
		This function is used internally and called from closeProject().
		Afterwards the project handler has invalid state (no project).
		\note Does not emit any signals and does not call setModified()
	*/
	void destroyProject();

	/*! Attempts to read the project with the given filename.
		If the reading was successful, the member variable projectFile is updated to 'fname'.
		\param fname The filename of the project file to be read.
		\return Returns true on success, false on error (error messages are written to IBK::IBK_Message).
		\note Does not emit any signals and does not call setModified()
	*/
	bool read(const QString & fname);

	/*! Writes the project with the given filename.
		If the writing was successful, the member variable m_projectFile is updated to 'fname'.
		\param fname The filename of the project file to be read.
		\return Returns true on success, false on error (error messages are written to IBK::IBK_Message).
		\note Does not emit any signals and does not call setModified()
	*/
	bool write(const QString & fname) const;

	/*! Adds the file fname to the list of most recently used files.
		The default implementation also emits the signal updateRecentProjects() which
		triggers a menu update.
		\sa updateRecentProjects()
	*/
	void addToRecentFiles(const QString& fname);


	// *** PRIVATE DATA MEMBERS ***

	/*! Pointer to self instance.
		Returned from instance().
	*/
	static SVProjectHandler	*m_self;

	/*! Pointer to the general project data storage class.
		This pointer is set and unset via the member functions create() and destroy().
		Access to the project instance is strictly forbidden and should only be granted
		to undo actions via a suitable mechanism.
	*/
	VICUS::Project		*m_project;

	/*! Holds the time stamp of the last time the project was read.
		This time stamp is updated in read() and used to check for external project modifications.
	*/
	QDateTime				m_lastReadTime;

	/*! Contains the state whether the project was modified or not. */
	bool					m_modified;


	/*! Contains the file name of the current project.
		Newly created projects have an empty file name (this can be used to
		determine whether a SaveAs dialog should be shown on first save).
		The variable is only set in the read() and write() functions.
		*/
	mutable QString			m_projectFile;

	/*! If true the project will be reloaded after closing log file dialog.*/
	bool					m_reload;
};

/*! Convenience function for accessing the VICUS::Project data directly with a shorter synopsis. */
inline const VICUS::Project & project() { return SVProjectHandler::instance().project(); }

#endif // SVProjectHandlerH