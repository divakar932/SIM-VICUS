#include "SVDBZoneControlVentilationNaturalTableModel.h"

#include <QTableView>
#include <QHeaderView>

#include <VICUS_KeywordListQt.h>

#include <QtExt_LanguageHandler.h>

#include "SVConstants.h"
#include "SVStyle.h"

SVDBZoneControlVentilationNaturalTableModel::SVDBZoneControlVentilationNaturalTableModel(QObject * parent, SVDatabase & db) :
	SVAbstractDatabaseTableModel(parent),
	m_db(&db)
{
	Q_ASSERT(m_db != nullptr);
}

int SVDBZoneControlVentilationNaturalTableModel::rowCount ( const QModelIndex & ) const {
	return (int)m_db->m_zoneControlVentilationNatural.size();
}


QVariant SVDBZoneControlVentilationNaturalTableModel::data ( const QModelIndex & index, int role) const {
	if (!index.isValid())
		return QVariant();

	// readability improvement
	const VICUS::Database<VICUS::ZoneControlNaturalVentilation> & ctrl = m_db->m_zoneControlVentilationNatural;

	int row = index.row();
	if (row >= (int)ctrl.size())
		return QVariant();

	std::map<unsigned int, VICUS::ZoneControlNaturalVentilation>::const_iterator it = ctrl.begin();
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


QVariant SVDBZoneControlVentilationNaturalTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
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


void SVDBZoneControlVentilationNaturalTableModel::resetModel() {
	beginResetModel();
	endResetModel();
}


QModelIndex SVDBZoneControlVentilationNaturalTableModel::addNewItem() {
	VICUS::ZoneControlNaturalVentilation venti;
	venti.m_displayName.setEncodedString("en:<new zone control VentilationNatural model>");

	// set default parameters


	VICUS::KeywordList::setParameter(venti.m_para, "ZoneControlNaturalVentilation::ScheduleType", VICUS::ZoneControlNaturalVentilation::ST_TemperatureAirMax, 60.0);
	VICUS::KeywordList::setParameter(venti.m_para, "ZoneControlNaturalVentilation::ScheduleType", VICUS::ZoneControlNaturalVentilation::ST_TemperatureAirMin, 20.0);
	VICUS::KeywordList::setParameter(venti.m_para, "ZoneControlNaturalVentilation::ScheduleType", VICUS::ZoneControlNaturalVentilation::ST_TemperatureOutsideMax, 60.0);
	VICUS::KeywordList::setParameter(venti.m_para, "ZoneControlNaturalVentilation::ScheduleType", VICUS::ZoneControlNaturalVentilation::ST_TemperatureOutsideMin, 0.0);
	VICUS::KeywordList::setParameter(venti.m_para, "ZoneControlNaturalVentilation::ScheduleType", VICUS::ZoneControlNaturalVentilation::ST_TemperatureDifference, 2.0);
	VICUS::KeywordList::setParameter(venti.m_para, "ZoneControlNaturalVentilation::ScheduleType", VICUS::ZoneControlNaturalVentilation::ST_TemperatureDifference, 2.0);

	venti.m_color = SVStyle::randomColor();

	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	unsigned int id = m_db->m_zoneControlVentilationNatural.add( venti );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


QModelIndex SVDBZoneControlVentilationNaturalTableModel::copyItem(const QModelIndex & existingItemIndex) {
	// lookup existing item
	const VICUS::Database<VICUS::ZoneControlNaturalVentilation> & db = m_db->m_zoneControlVentilationNatural;
	Q_ASSERT(existingItemIndex.isValid() && existingItemIndex.row() < (int)db.size());
	std::map<unsigned int, VICUS::ZoneControlNaturalVentilation>::const_iterator it = db.begin();
	std::advance(it, existingItemIndex.row());
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	// create new item and insert into DB
	VICUS::ZoneControlNaturalVentilation newItem(it->second);
	newItem.m_color = SVStyle::randomColor();
	unsigned int id = m_db->m_zoneControlVentilationNatural.add( newItem );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


void SVDBZoneControlVentilationNaturalTableModel::deleteItem(const QModelIndex & index) {
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	beginRemoveRows(QModelIndex(), index.row(), index.row());
	m_db->m_zoneControlVentilationNatural.remove(id);
	endRemoveRows();
}


void SVDBZoneControlVentilationNaturalTableModel::setColumnResizeModes(QTableView * tableView) {
	tableView->horizontalHeader()->setSectionResizeMode(SVDBZoneControlVentilationNaturalTableModel::ColId, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBZoneControlVentilationNaturalTableModel::ColCheck, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBZoneControlVentilationNaturalTableModel::ColName, QHeaderView::Stretch);
}


void SVDBZoneControlVentilationNaturalTableModel::setItemModified(unsigned int id) {
	QModelIndex idx = indexById(id);
	QModelIndex left = index(idx.row(), 0);
	QModelIndex right = index(idx.row(), NumColumns-1);
	emit dataChanged(left, right);
}


QModelIndex SVDBZoneControlVentilationNaturalTableModel::indexById(unsigned int id) const {
	for (int i=0; i<rowCount(); ++i) {
		QModelIndex idx = index(i, 0);
		if (data(idx, Role_Id).toUInt() == id)
			return idx;
	}
	return QModelIndex();
}
