#include "SVDBMaterialTableModel.h"

#include <map>
#include <algorithm>

#include <QIcon>
#include <QFont>

#include <VICUS_Material.h>
#include <VICUS_Database.h>
#include <VICUS_KeywordListQt.h>

#include "SVConstants.h"


SVDBMaterialTableModel::SVDBMaterialTableModel(QObject * parent, SVDatabase & db) :
	QAbstractTableModel(parent),
	m_db(&db)
{
	Q_ASSERT(m_db != nullptr);
}


SVDBMaterialTableModel::~SVDBMaterialTableModel() {
}


int SVDBMaterialTableModel::columnCount ( const QModelIndex & ) const {
	return NumColumns;
}


QVariant SVDBMaterialTableModel::data ( const QModelIndex & index, int role) const {
	if (!index.isValid())
		return QVariant();

	// readibility improvement
	const VICUS::Database<VICUS::Material> & matDB = m_db->m_materials;

	int row = index.row();
	if (row >= (int)matDB.size())
		return QVariant();

	std::map<unsigned int, VICUS::Material>::const_iterator it = matDB.begin();
	std::advance(it, row);

	switch (role) {
		case Qt::DisplayRole : {
			switch (index.column()) {
				case ColId					: return it->first;
				case ColName				: return QString::fromStdString(it->second.m_displayName.string("de", true)); // Note: take name in current language or if missing, "all"
			}
		} break;

		case Qt::DecorationRole : {
			if (index.column() == ColCheck) {
				if (it->second.isValid(false)) // for now just check the thermal properties
					return QIcon("://gfx/actions/16x16/ok.png");
				else
					return QIcon("://gfx/actions/16x16/error.png");
			}
		} break;

		case Qt::TextAlignmentRole :
			if (index.column() >= ColRho && index.column() < NumColumns)
				return int(Qt::AlignRight | Qt::AlignVCenter);
			break;

		case Qt::SizeHintRole :
			switch (index.column()) {
				case ColCheck :
					return QSize(22, 16);
			} // switch
			break;

		case Role_Id :
			return it->first;

		case Role_BuiltIn :
			return it->second.m_builtIn;
	}

	return QVariant();
}


int SVDBMaterialTableModel::rowCount ( const QModelIndex & ) const {
	return (int)m_db->m_materials.size();
}


QVariant SVDBMaterialTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation == Qt::Vertical)
		return QVariant();
	switch (role) {
		case Qt::DisplayRole: {
			switch ( section ) {
				case ColId					: return tr("Id");
				case ColName				: return tr("Name");
				default: ;
			}
		} break;

		case Qt::FontRole : {
			QFont f;
			f.setBold(true);
			f.setPointSizeF(f.pointSizeF()*0.8);
			return f;
		}
	} // switch
	return QVariant();
}


QModelIndex SVDBMaterialTableModel::addNewItem() {
	VICUS::Material m;
	m.m_displayName.setEncodedString("en:<new material>");
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	unsigned int id = m_db->m_materials.add( m );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


QModelIndex SVDBMaterialTableModel::addNewItem(VICUS::Material c) {
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	unsigned int id = m_db->m_materials.add( c );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


bool SVDBMaterialTableModel::deleteItem(QModelIndex index) {
	if (!index.isValid())
		return false;
	unsigned int id = data(index, Role_Id).toUInt();
	beginRemoveRows(QModelIndex(), index.row(), index.row());
	m_db->m_materials.remove(id);
	endRemoveRows();
	return true;
}


void SVDBMaterialTableModel::resetModel() {
	beginResetModel();
	endResetModel();
}


void SVDBMaterialTableModel::setItemModified(unsigned int id) {
	QModelIndex idx = indexById(id);
	QModelIndex left = index(idx.row(), 0);
	QModelIndex right = index(idx.row(), NumColumns-1);
	emit dataChanged(left, right);
}


QModelIndex SVDBMaterialTableModel::indexById(unsigned int id) const {
	for (int i=0; i<rowCount(); ++i) {
		QModelIndex idx = index(i, 0);
		if (data(idx, Role_Id).toUInt() == id)
			return idx;
	}
	return QModelIndex();
}
