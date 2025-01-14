#ifndef SVDBConstructionEditWidgetH
#define SVDBConstructionEditWidgetH

#include "SVAbstractDatabaseEditWidget.h"

#include <QtExt_ConstructionLayer.h>

namespace Ui {
	class SVDBConstructionEditWidget;
}

class QModelIndex;
class QComboBox;
class QTableWidgetItem;

namespace VICUS {
	class Construction;
}

class SVDBConstructionTableModel;
class SVDatabase;

/*! Edit widget for construction types.

	A call to updateInput() initializes the widget and fill the GUI controls with data.
	As long as the widget is visible the pointer to the data must be valid. Keep this
	in mind if you change the container that the data object belongs to! If the pointer
	is no longer valid or you want to resize the container (through adding new items)
	call updateInput() with an invalid index and/or nullptr pointer to the model.
*/
class SVDBConstructionEditWidget : public SVAbstractDatabaseEditWidget {
	Q_OBJECT
public:
	/*! Constructor, requires read/write access to database object. */
	SVDBConstructionEditWidget(QWidget * parent);
	~SVDBConstructionEditWidget() override;

	/*! Needs to be called once, before the widget is being used. */
	void setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) override;

	/*! Called whenever the user selects a new construction type. */
	void updateInput(int id) override;

private:
	/*! Updates the content of the table. */
	void updateTable();

	/*! Updates the graphical construction view.*/
	void updateConstructionView();

	/*! Updates the U value and thermal storage mass line edits. */
	void updateUValue();

	/*! Show a material database view in order to select a new material and change the current selection.
		\param index Index of the layer for material change.
	*/
	void showMaterialSelectionDialog(int index);

	/*! The UI class. */
	Ui::SVDBConstructionEditWidget	*m_ui;

	/*! Cached pointer to database object. */
	SVDatabase						*m_db;

	/*! Pointer to the database model, to modify items when data has changed in the widget. */
	SVDBConstructionTableModel		*m_dbModel;

	/*! Pointer to currently edited construction.
		The pointer is updated whenever updateInput() is called.
		A nullptr pointer means that there is no construction to edit.
	*/
	VICUS::Construction				*m_current;

private slots:
	void on_lineEditName_editingFinished();
	void on_comboBoxInsulationKind_currentIndexChanged(int index);
	void on_comboBoxMaterialKind_currentIndexChanged(int index);
	void on_comboBoxConstructionUsage_currentIndexChanged(int index);
	void on_spinBoxLayerCount_valueChanged(int);
	/*! Triggered when user modifies a table cell, only needed for 'width'-column. */
	void tableItemChanged(QTableWidgetItem *);
	/*! Triggered when user modifies a table cell, only needed for 'width'-column. */
	void tableItemClicked(QTableWidgetItem *);
	/*! Slot for react on double click on table cell.*/
	void onCellDoubleClicked(int row, int col);

	/*! Connected with construction view widget layer selection changed.*/
	void constructionViewlayerSelected(int index);

	/*! Connected with construction view widget assign material signal.*/
	void constructionViewAssign_material(int index);

	/*! Connected with construction view widget insert layer signal.*/
	void constructionViewInsert_layer(int index, bool left);

	/*! Connected with construction view widget remove layer signal.*/
	void constructionViewRemove_layer(int index);

	/*! Connected with construction view widget move layer signal.*/
	void constructionViewMove_layer(int index, bool left);
};

#endif // SVDBConstructionEditWidgetH
