#ifndef SVDBZoneTemplateEditWidgetH
#define SVDBZoneTemplateEditWidgetH

#include <QWidget>

namespace Ui {
	class SVDBZoneTemplateEditWidget;
}

namespace VICUS {
	class ZoneTemplate;
}

class SVDBZoneTemplateTreeModel;
class SVDatabase;

/*! Edit widget for zone template.

	A call to updateInput() initializes the widget and fill the GUI controls with data.
	As long as the widget is visible the pointer to the data must be valid. Keep this
	in mind if you change the container that the data object belongs to! If the pointer
	is no longer valid or you want to resize the container (through adding new items)
	call updateInput() with an invalid index and/or nullptr pointer to the model.
*/
class SVDBZoneTemplateEditWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVDBZoneTemplateEditWidget(QWidget *parent = nullptr);
	~SVDBZoneTemplateEditWidget() override;

	/*! Needs to be called once, before the widget is being used. */
	void setup(SVDatabase * db, SVDBZoneTemplateTreeModel * dbModel);

	/*! Update widget with this. */
	void updateInput(int id, int subTemplateId, int subTemplateType);

public slots:
	void on_toolButtonRemoveSubComponent_clicked();

signals:
	/*! Emitted when the action in the widget shall cause the tree view to change selection.
		In case of removal of a sub-template, the subTemplateType matches ZoneTemplate::NUM_ST and
		in this case the top-level ZoneTemplate node shall be selected.
		The selection change in the tree must cause a call to updateInput() afterwards, even if
		the same item was re-selected.
	*/
	void selectSubTemplate(unsigned int id, int subTemplateType);

private slots:
	void on_lineEditName_editingFinished();
	void on_pushButtonColor_colorChanged();

	void on_toolButtonSelectSubComponent_clicked();;


	void on_pushButtonAddPersonLoad_clicked();
	void on_pushButtonAddElectricLoad_clicked();
	void on_pushButtonAddLightLoad_clicked();



private:
	Ui::SVDBZoneTemplateEditWidget *m_ui;

	/*! Cached pointer to database object. */
	SVDatabase							*m_db;

	/*! Pointer to the database model, to modify items when data has changed in the widget. */
	SVDBZoneTemplateTreeModel			*m_dbModel;

	/*! Pointer to currently edited element.
		The pointer is updated whenever updateInput() is called.
		A nullptr pointer means that there is nothing to edit.
	*/
	VICUS::ZoneTemplate					*m_current;

	int									m_currentSubTemplateType;
};

#endif // SVDBZoneTemplateEditWidgetH
