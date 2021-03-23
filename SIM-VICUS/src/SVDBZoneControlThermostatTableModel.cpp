#include "SVDBZoneControlThermostatTableModel.h"

#include <QTableView>
#include <QHeaderView>

#include <VICUS_KeywordListQt.h>

#include <QtExt_LanguageHandler.h>

#include "SVConstants.h"
#include "SVStyle.h"

SVDBZoneControlThermostatTableModel::SVDBZoneControlThermostatTableModel(QObject * parent, SVDatabase & db) :
	SVAbstractDatabaseTableModel(parent),
	m_db(&db)
{
	Q_ASSERT(m_db != nullptr);
}


int SVDBZoneControlThermostatTableModel::rowCount ( const QModelIndex & ) const {
	return (int)m_db->m_zoneControlThermostat.size();
}


QVariant SVDBZoneControlThermostatTableModel::data ( const QModelIndex & index, int role) const {
	if (!index.isValid())
		return QVariant();

	// readability improvement
	const VICUS::Database<VICUS::ZoneControlThermostat> & ctrl = m_db->m_zoneControlThermostat;

	int row = index.row();
	if (row >= (int)ctrl.size())
		return QVariant();

	std::map<unsigned int, VICUS::ZoneControlThermostat>::const_iterator it = ctrl.begin();
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


QVariant SVDBZoneControlThermostatTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
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


void SVDBZoneControlThermostatTableModel::resetModel() {
	beginResetModel();
	endResetModel();
}


QModelIndex SVDBZoneControlThermostatTableModel::addNewItem() {
	VICUS::ZoneControlThermostat ctrl;
	ctrl.m_displayName.setEncodedString("en:<new zone control thermostat model>");

	// set default parameters

	ctrl.m_ctrlVal = VICUS::ZoneControlThermostat::CV_AirTemperature;
	VICUS::KeywordList::setParameter(ctrl.m_para, "ZoneControlThermostat::para_t", VICUS::ZoneControlThermostat::P_ToleranceHeating, 0.0);
	VICUS::KeywordList::setParameter(ctrl.m_para, "ZoneControlThermostat::para_t", VICUS::ZoneControlThermostat::P_ToleranceCooling, 0.0);

	ctrl.m_color = SVStyle::randomColor();

	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	unsigned int id = m_db->m_zoneControlThermostat.add( ctrl );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


QModelIndex SVDBZoneControlThermostatTableModel::copyItem(const QModelIndex & existingItemIndex) {
	// lookup existing item
	const VICUS::Database<VICUS::ZoneControlThermostat> & db = m_db->m_zoneControlThermostat;
	Q_ASSERT(existingItemIndex.isValid() && existingItemIndex.row() < (int)db.size());
	std::map<unsigned int, VICUS::ZoneControlThermostat>::const_iterator it = db.begin();
	std::advance(it, existingItemIndex.row());
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	// create new item and insert into DB
	VICUS::ZoneControlThermostat newItem(it->second);
	newItem.m_color = SVStyle::randomColor();
	unsigned int id = m_db->m_zoneControlThermostat.add( newItem );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


void SVDBZoneControlThermostatTableModel::deleteItem(const QModelIndex & index) {
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	beginRemoveRows(QModelIndex(), index.row(), index.row());
	m_db->m_zoneControlThermostat.remove(id);
	endRemoveRows();
}


void SVDBZoneControlThermostatTableModel::setColumnResizeModes(QTableView * tableView) {
	tableView->horizontalHeader()->setSectionResizeMode(SVDBZoneControlThermostatTableModel::ColId, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBZoneControlThermostatTableModel::ColCheck, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBZoneControlThermostatTableModel::ColName, QHeaderView::Stretch);
}


void SVDBZoneControlThermostatTableModel::setItemModified(unsigned int id) {
	QModelIndex idx = indexById(id);
	QModelIndex left = index(idx.row(), 0);
	QModelIndex right = index(idx.row(), NumColumns-1);
	emit dataChanged(left, right);
}


QModelIndex SVDBZoneControlThermostatTableModel::indexById(unsigned int id) const {
	for (int i=0; i<rowCount(); ++i) {
		QModelIndex idx = index(i, 0);
		if (data(idx, Role_Id).toUInt() == id)
			return idx;
	}
	return QModelIndex();
}