#include "SVDBZoneTemplateTreeModel.h"

#include <QTableView>
#include <QHeaderView>

#include <VICUS_ZoneTemplate.h>
#include <VICUS_Database.h>
#include <VICUS_KeywordListQt.h>

#include <QtExt_LanguageHandler.h>

#include "SVConstants.h"
#include "SVStyle.h"

SVDBZoneTemplateTreeModel::SVDBZoneTemplateTreeModel(QObject * parent, SVDatabase & db) :
	QAbstractItemModel(parent),
	m_db(&db)
{
	Q_ASSERT(m_db != nullptr);
}


QVariant SVDBZoneTemplateTreeModel::data ( const QModelIndex & index, int role) const {
	if (!index.isValid())
		return QVariant();

	// readability improvement
	const VICUS::Database<VICUS::ZoneTemplate> & db = m_db->m_zoneTemplates;

	// index is a top-level item
	if (index.internalPointer() == nullptr) {

		int row = index.row();
		if (row >= (int)db.size())
			return QVariant();

		std::map<unsigned int, VICUS::ZoneTemplate>::const_iterator it = db.begin();
		std::advance(it, row);

		switch (role) {
			case Qt::DisplayRole : {
				// Note: when accessing multilanguage strings below, take name in current language or if missing, "all"
				std::string langId = QtExt::LanguageHandler::instance().langId().toStdString();
				std::string fallBackLangId = "en";

				switch (index.column()) {
					case ColId					: return it->first;
					case ColName				: return QString::fromStdString(it->second.m_displayName.string(langId, fallBackLangId));
				}
			} break;

			case Qt::DecorationRole : {
				if (index.column() == ColCheck) {
					if (it->second.isValid())
						return QIcon("://gfx/actions/16x16/ok.png");
					else
						return QIcon("://gfx/actions/16x16/error.png");
				}
			} break;

			case Qt::BackgroundRole : {
				if (index.column() == ColColor) {
					return it->second.m_color;
				}
			} break;

			case Qt::SizeHintRole :
				switch (index.column()) {
					case ColCheck :
					case ColColor :
						return QSize(22, 16);
				} // switch
				break;

			case Role_Id :
				return it->first;

			case Role_BuiltIn :
				return it->second.m_builtIn;
		}
	}
	return QVariant();
}


int SVDBZoneTemplateTreeModel::rowCount ( const QModelIndex & parent) const {
	// top-level - number of zone templates
	if (!parent.isValid())
		return (int)m_db->m_zoneTemplates.size();
	// lookup currently selected zone template
	int row = parent.row();
	const VICUS::Database<VICUS::ZoneTemplate> & db = m_db->m_zoneTemplates;
	Q_ASSERT ((unsigned int)row < db.size());

	std::map<unsigned int, VICUS::ZoneTemplate>::const_iterator it = db.begin();
	std::advance(it, row);

	// return number of assigned sub-templates
	return (int)it->second.subTemplateCount();
}


QVariant SVDBZoneTemplateTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
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


QModelIndex SVDBZoneTemplateTreeModel::index(int row, int column, const QModelIndex & parent) const {
	if (!parent.isValid()) {
		return createIndex(row, column, nullptr); // top-level items have a nullptr for identification
	}
	// child item,
	int parentRow = parent.row();
	// take pointer to item
	const VICUS::Database<VICUS::ZoneTemplate> & db = m_db->m_zoneTemplates;
	std::map<unsigned int, VICUS::ZoneTemplate>::const_iterator it = db.begin();
	std::advance(it, parentRow);
	// child items have a pointer to the zone template they belong to as identification
	return createIndex(row, column, (void*)(&it->second));
}


QModelIndex SVDBZoneTemplateTreeModel::parent(const QModelIndex & child) const {
	// nullptr means top-level it
	if (child.internalPointer() == nullptr)
		return QModelIndex();
	else {
		// get internal pointer and lookup item by id
		const VICUS::ZoneTemplate * ptr = reinterpret_cast<const VICUS::ZoneTemplate *>(child.internalPointer());
		// search DB and get the row index of this item
		std::map<unsigned int, VICUS::ZoneTemplate>::const_iterator it = m_db->m_zoneTemplates.begin();
		int i=0;
		for (; ptr != &it->second && (unsigned int)i<m_db->m_zoneTemplates.size(); ++i, ++it);
		Q_ASSERT((unsigned int)i != m_db->m_zoneTemplates.size());
		return createIndex(i, 0, nullptr);
	}
}


void SVDBZoneTemplateTreeModel::resetModel() {
	beginResetModel();
	endResetModel();
}


QModelIndex SVDBZoneTemplateTreeModel::addNewItem() {
	VICUS::ZoneTemplate c;
	c.m_displayName.setEncodedString("en:<new zone template>");
	c.m_color = SVStyle::randomColor();
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	unsigned int id = m_db->m_zoneTemplates.add( c );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


QModelIndex SVDBZoneTemplateTreeModel::copyItem(const QModelIndex & existingItemIndex) {
	// lookup existing item
	const VICUS::Database<VICUS::ZoneTemplate> & db = m_db->m_zoneTemplates;
	Q_ASSERT(existingItemIndex.isValid() && existingItemIndex.row() < (int)db.size());
	std::map<unsigned int, VICUS::ZoneTemplate>::const_iterator it = db.begin();
	std::advance(it, existingItemIndex.row());
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	// create new item and insert into DB
	VICUS::ZoneTemplate newItem(it->second);
	newItem.m_color = SVStyle::randomColor();
	unsigned int id = m_db->m_zoneTemplates.add( newItem );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


void SVDBZoneTemplateTreeModel::deleteItem(const QModelIndex & index) {
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	beginRemoveRows(QModelIndex(), index.row(), index.row());
	m_db->m_zoneTemplates.remove(id);
	endRemoveRows();
}


void SVDBZoneTemplateTreeModel::setItemModified(unsigned int id) {
	QModelIndex idx = indexById(id);
	QModelIndex left = index(idx.row(), 0, QModelIndex());
	QModelIndex right = index(idx.row(), NumColumns-1, QModelIndex());
	emit dataChanged(left, right);
}


QModelIndex SVDBZoneTemplateTreeModel::indexById(unsigned int id) const {
	for (int i=0; i<rowCount(); ++i) {
		QModelIndex idx = index(i, 0, QModelIndex());
		if (data(idx, Role_Id).toUInt() == id)
			return idx;
	}
	return QModelIndex();
}
