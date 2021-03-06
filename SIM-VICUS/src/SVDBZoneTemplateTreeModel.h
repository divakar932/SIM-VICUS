#ifndef SVDBZoneTemplateTreeModelH
#define SVDBZoneTemplateTreeModelH

#include "SVAbstractDatabaseEditWidget.h"

#include "SVDatabase.h"


/*! Model for accessing the components in the component database. */
class SVDBZoneTemplateTreeModel : public QAbstractItemModel {
	Q_OBJECT
public:
	/*! Columns shown in the table view. */
	enum Columns {
		ColId,
		ColColor,
		ColCheck,
		ColName,
		NumColumns
	};

	/*! Constructor, requires a read/write pointer to the central database object.
		\note Pointer to database must be valid throughout the lifetime of the Model!
		*/
	SVDBZoneTemplateTreeModel(QObject * parent, SVDatabase & db);

	// ** QAbstractItemModel interface **

	virtual int columnCount ( const QModelIndex & ) const override { return NumColumns; }
	virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const override;
	virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const override;
	virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
	QModelIndex index(int row, int column, const QModelIndex & parent) const override;
	QModelIndex parent(const QModelIndex & child) const override;

	// ** SVAbstractDatabaseTableModel interface **

	virtual void resetModel();
	QModelIndex addNewItem();
	QModelIndex copyItem(const QModelIndex & index);
	void deleteItem(const QModelIndex & index);

	// ** other members **

	/*! Tells the model that an item has been modified, triggers a dataChanged() signal. */
	void setItemModified(unsigned int id);

private:
	/*! Returns an index for a given Id. */
	QModelIndex indexById(unsigned int id) const;

	/*! Pointer to the entire database (not owned). */
	SVDatabase	* m_db;

};


#endif // SVDBZoneTemplateTreeModelH