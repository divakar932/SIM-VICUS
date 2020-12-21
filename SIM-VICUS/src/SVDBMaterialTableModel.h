#ifndef SVDBMaterialTableModelH
#define SVDBMaterialTableModelH

#include <QAbstractTableModel>

#include "SVDatabase.h"

/*! Model for accessing the materials in the materials database. */
class SVDBMaterialTableModel : public QAbstractTableModel {
	Q_OBJECT
public:

	/*! Columns shown in the table view. */
	enum Columns {
		ColId,
		ColCheck,
		ColName,
		ColCategory,
		ColProductID,
		ColProducer,
		ColSource,
		ColRho,
		ColCet,
		ColLambda,
		ColMew,
		ColW80,
		ColWSat,
		ColAw,
		ColSd,
		ColKAir,
		NumColumns
	};


	/*! Constructor, requires a read/write pointer to the central database object.
		\note Pointer to database must be valid throughout the lifetime of the Model!
		*/
	SVDBMaterialTableModel(QObject * parent, SVDatabase & db);
	virtual ~SVDBMaterialTableModel();

	// ** QAbstractItemModel interface **

	virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
	virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
	virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
	virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

	// ** other members **

	/*! Tells the model that an item has been modified, triggers a dataChanged() signal. */
	void setItemModified(unsigned int id);

	/*! Inserts a new item and returns the model index of the new item. */
	QModelIndex addNewItem();

	/*! Inserts a new item and returns the model index of the new item.
		\note Pass-by-value is intended.
	*/
	QModelIndex addNewItem(VICUS::Material c);

	/*! Removes a selected item.
		\return Returns true on success, false if the item wasn't deleted (invalid index etc.)
	*/
	bool deleteItem(QModelIndex index);

	void resetModel();

private:
	/*! Returns an index for a given Id. */
	QModelIndex indexById(unsigned int id) const;

	/*! Pointer to the entire database (not owned). */
	SVDatabase	* m_db;
};

#endif // SVDBMaterialTableModelH