#include "SVDBZoneControlShadingTableModel.h"

#include <QTableView>
#include <QHeaderView>

#include <VICUS_KeywordListQt.h>

#include <QtExt_LanguageHandler.h>

#include "SVConstants.h"
#include "SVStyle.h"

SVDBZoneControlShadingTableModel::SVDBZoneControlShadingTableModel(QObject * parent, SVDatabase & db) :
	SVAbstractDatabaseTableModel(parent),
	m_db(&db)
{
	Q_ASSERT(m_db != nullptr);
}


int SVDBZoneControlShadingTableModel::rowCount ( const QModelIndex & ) const {
	return (int)m_db->m_zoneControlShading.size();
}


QVariant SVDBZoneControlShadingTableModel::data ( const QModelIndex & index, int role) const {
	if (!index.isValid())
		return QVariant();

	// readability improvement
	const VICUS::Database<VICUS::ZoneControlShading> & ctrl = m_db->m_zoneControlShading;

	int row = index.row();
	if (row >= (int)ctrl.size())
		return QVariant();

	std::map<unsigned int, VICUS::ZoneControlShading>::const_iterator it = ctrl.begin();
	std::advance(it, row);

	switch (role) {
		case Qt::DisplayRole : {
			// Note: when accessing multilanguage strings below, take name in current language or if missing, "all"
			std::string langId = QtExt::LanguageHandler::instance().langId().toStdString();
			std::string fallBackLangId = "en";

			switch (index.column()) {
				case ColId					: return it->first;
				case ColName				: return QString::fromStdString(it->second.m_displayName.string(langId, fallBackLangId));
//				case ColCategory			:
//					try {
//						return VICUS::KeywordListQt::Keyword("Material::Category",it->second.m_category);
//					} catch (...) {
//						return "";
//					}
				case ColSource				: return QString::fromStdString(it->second.m_dataSource.string(langId, fallBackLangId));
			}
		} break;

		case Qt::DecorationRole : {
			if (index.column() == ColCheck) {
				if (it->second.isValid()) // for now just check for validity
					return QIcon("://gfx/actions/16x16/ok.png");
				else
					return QIcon("://gfx/actions/16x16/error.png");
			}
		} break;

		case Qt::TextAlignmentRole :
			//if (index.column() >= ColRho && index.column() < NumColumns)
			//	return int(Qt::AlignRight | Qt::AlignVCenter);
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


QVariant SVDBZoneControlShadingTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation == Qt::Vertical)
		return QVariant();
	switch (role) {
		case Qt::DisplayRole: {
			switch ( section ) {
				case ColId					: return tr("Id");
				case ColName				: return tr("Name");
				//case ColCategory			: return tr("Category");
				case ColSource				: return tr("Source");
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


void SVDBZoneControlShadingTableModel::resetModel() {
	beginResetModel();
	endResetModel();
}


QModelIndex SVDBZoneControlShadingTableModel::addNewItem() {
	VICUS::ZoneControlShading ctrl;
	ctrl.m_displayName.setEncodedString("en:<new zone control Shading model>");

	// set default parameters

	ctrl.m_category = VICUS::ZoneControlShading::C_GlobalHorizontalSensor;
	VICUS::KeywordList::setParameter(ctrl.m_para, "ZoneControlShading::para_t", VICUS::ZoneControlShading::P_GlobalEast, 150);
	VICUS::KeywordList::setParameter(ctrl.m_para, "ZoneControlShading::para_t", VICUS::ZoneControlShading::P_GlobalWest, 150);
	VICUS::KeywordList::setParameter(ctrl.m_para, "ZoneControlShading::para_t", VICUS::ZoneControlShading::P_GlobalNorth, 150);
	VICUS::KeywordList::setParameter(ctrl.m_para, "ZoneControlShading::para_t", VICUS::ZoneControlShading::P_GlobalSouth, 150);
	VICUS::KeywordList::setParameter(ctrl.m_para, "ZoneControlShading::para_t", VICUS::ZoneControlShading::P_GlobalHorizontal, 150);
	VICUS::KeywordList::setParameter(ctrl.m_para, "ZoneControlShading::para_t", VICUS::ZoneControlShading::P_DeadBand, 0);

	ctrl.m_color = SVStyle::randomColor();

	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	unsigned int id = m_db->m_zoneControlShading.add( ctrl );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


QModelIndex SVDBZoneControlShadingTableModel::copyItem(const QModelIndex & existingItemIndex) {
	// lookup existing item
	const VICUS::Database<VICUS::ZoneControlShading> & db = m_db->m_zoneControlShading;
	Q_ASSERT(existingItemIndex.isValid() && existingItemIndex.row() < (int)db.size());
	std::map<unsigned int, VICUS::ZoneControlShading>::const_iterator it = db.begin();
	std::advance(it, existingItemIndex.row());
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	// create new item and insert into DB
	VICUS::ZoneControlShading newItem(it->second);
	newItem.m_color = SVStyle::randomColor();
	unsigned int id = m_db->m_zoneControlShading.add( newItem );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


void SVDBZoneControlShadingTableModel::deleteItem(const QModelIndex & index) {
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	beginRemoveRows(QModelIndex(), index.row(), index.row());
	m_db->m_zoneControlShading.remove(id);
	endRemoveRows();
}


void SVDBZoneControlShadingTableModel::setColumnResizeModes(QTableView * tableView) {
	tableView->horizontalHeader()->setSectionResizeMode(SVDBZoneControlShadingTableModel::ColId, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBZoneControlShadingTableModel::ColCheck, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBZoneControlShadingTableModel::ColName, QHeaderView::Stretch);
}


void SVDBZoneControlShadingTableModel::setItemModified(unsigned int id) {
	QModelIndex idx = indexById(id);
	QModelIndex left = index(idx.row(), 0);
	QModelIndex right = index(idx.row(), NumColumns-1);
	emit dataChanged(left, right);
}


QModelIndex SVDBZoneControlShadingTableModel::indexById(unsigned int id) const {
	for (int i=0; i<rowCount(); ++i) {
		QModelIndex idx = index(i, 0);
		if (data(idx, Role_Id).toUInt() == id)
			return idx;
	}
	return QModelIndex();
}
