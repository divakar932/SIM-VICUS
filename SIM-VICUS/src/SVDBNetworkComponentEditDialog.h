#ifndef SVDBNetworkComponentEditDialogH
#define SVDBNetworkComponentEditDialogH

#include <QDialog>

namespace Ui {
class SVDBNetworkComponentEditDialog;
}

class QSortFilterProxyModel;
class QModelIndex;

class SVDBNetworkComponentTableModel;

/*! The edit dialog for component types. */
class SVDBNetworkComponentEditDialog : public QDialog {
	Q_OBJECT

public:
	explicit SVDBNetworkComponentEditDialog(QWidget *parent = nullptr);
	~SVDBNetworkComponentEditDialog() override;

	/*! Starts the dialog in "edit" mode.
		\param id Optional id of element to be initially selected.
	*/
	void edit(unsigned int initialId = 0);

	/*! Starts the dialog in "select mode".
		\param initialId The component indicated by this ID is initially selected.
		\return If a component was selected and double-clicked/or the "Select" button was
				pressed, the function returns the ID of the selected component. Otherwise, if the
				dialog was aborted, the function returns -1.
	*/
	int select(unsigned int initialId);

protected:
	void showEvent(QShowEvent * event) override;

private slots:
	void on_pushButtonSelect_clicked();
	void on_pushButtonCancel_clicked();
	void on_pushButtonClose_clicked();

	void on_toolButtonAdd_clicked();
	void on_toolButtonCopy_clicked();
	void on_toolButtonRemove_clicked();

	/*! Connected to the respective signal in the table view.
		Enables/disables the remove button.
	*/
	void onCurrentIndexChanged(const QModelIndex &current, const QModelIndex &/*previous*/);

	void on_pushButtonReloadUserDB_clicked();

private:
	Ui::SVDBNetworkComponentEditDialog *m_ui;

	/*! The component table model (owned). */
	SVDBNetworkComponentTableModel	*m_dbModel		= nullptr;
	/*! The sort filter model (owned). */
	QSortFilterProxyModel			*m_proxyModel	= nullptr;

};

#endif // SVDBNetworkComponentEditDialogH