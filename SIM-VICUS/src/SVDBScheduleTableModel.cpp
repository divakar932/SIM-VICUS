#include "SVDBScheduleTableModel.h"

#include <QIcon>
#include <QFont>
#include <QTableView>
#include <QHeaderView>

#include <VICUS_Schedule.h>
#include <VICUS_Database.h>
#include <VICUS_KeywordListQt.h>

#include <NANDRAD_KeywordList.h>

#include <QtExt_LanguageHandler.h>

#include "SVConstants.h"

SVDBScheduleTableModel::SVDBScheduleTableModel(QObject *parent, SVDatabase &db) :
	SVAbstractDatabaseTableModel(parent),
	m_db(&db)
{
	Q_ASSERT(m_db != nullptr);
}


QVariant SVDBScheduleTableModel::data ( const QModelIndex & index, int role) const {
	if (!index.isValid())
		return QVariant();

	// readability improvement
	const VICUS::Database<VICUS::Schedule> & db = m_db->m_schedules;

	int row = index.row();
	if (row >= (int)db.size())
		return QVariant();

	std::map<unsigned int, VICUS::Schedule>::const_iterator it = db.begin();
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

		case Qt::SizeHintRole :
			switch (index.column()) {
				case ColCheck :
				case ColAnnualSplineData :
				case ColInterpolation :
					return QSize(22, 16);
			} // switch
		break;

		case Qt::TextAlignmentRole :
			switch (index.column()) {
				case ColId					: return int(Qt::AlignLeft | Qt::AlignVCenter);
				case ColName				: return int(Qt::AlignLeft | Qt::AlignVCenter);
				case ColCheck				: return int(Qt::AlignCenter | Qt::AlignVCenter);
				case ColAnnualSplineData	: return int(Qt::AlignCenter | Qt::AlignVCenter);
				case ColInterpolation		: return int(Qt::AlignCenter | Qt::AlignVCenter);
			}
			break;

		case Qt::DecorationRole : {
			switch (index.column()) {
				case ColCheck:
					if (it->second.isValid())
						return QIcon("://gfx/actions/16x16/ok.png");
					else
						return QIcon("://gfx/actions/16x16/error.png");

				case ColAnnualSplineData :
					if(it->second.m_periods.empty())
						return QIcon("");			///TODO hier brauchen wir ein Bild für einen jährliche Daten
					else
						return QIcon("");			///TODO hier brauchen wir ein Bild für einen periodische Daten

				case ColInterpolation :
					if(it->second.m_useLinearInterpolation)
						return QIcon("://gfx/icons/16x16/interpolationMode_linear_bright_16.png");		///TODO hier brauchen wir ein Bild für einen LINEARE Interpolationsdarstellung
					else
						return QIcon("://gfx/icons/16x16/interpolationMode_constant_bright_16.png");		///TODO hier brauchen wir ein Bild für einen KONSTANTE Interpolationsdarstellung

			} // switch
		} break;

		case Role_Id :
			return it->first;

		case Role_BuiltIn :
			return it->second.m_builtIn;
	}

	return QVariant();
}


int SVDBScheduleTableModel::rowCount ( const QModelIndex & ) const {
	return (int)m_db->m_schedules.size();
}


QVariant SVDBScheduleTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation == Qt::Vertical)
		return QVariant();
	switch (role) {
		case Qt::DisplayRole: {
			switch ( section ) {
				case ColId					: return tr("Id");
				case ColName				: return tr("Name");
				case ColCheck				:
				case ColAnnualSplineData	:
				case ColInterpolation		: break; // icon columns are self-explanatory and do not need a caption
			}
		} break;

		case Qt::ToolTipRole:{
			switch (section) {
				case ColCheck				: return QString(tr("Indicates if schedule data is valid."));
				case ColInterpolation		: return QString(tr("Indicates if schedule data is linearly interpolated."));
				case ColAnnualSplineData	: return QString(tr("Indicates if schedule data is provided as annual data set."));
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


void SVDBScheduleTableModel::resetModel() {
	beginResetModel();
	endResetModel();
}


QModelIndex SVDBScheduleTableModel::addNewItem() {
	VICUS::Schedule sched;
	sched.m_displayName.setEncodedString("en:<new schedule>");

	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	unsigned int id = m_db->m_schedules.add( sched );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


QModelIndex SVDBScheduleTableModel::copyItem(const QModelIndex & existingItemIndex) {
	// lookup existing item
	const VICUS::Database<VICUS::Schedule> & db = m_db->m_schedules;
	Q_ASSERT(existingItemIndex.isValid() && existingItemIndex.row() < (int)db.size());
	std::map<unsigned int, VICUS::Schedule>::const_iterator it = db.begin();
	std::advance(it, existingItemIndex.row());
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	// create new item and insert into DB
	VICUS::Schedule newItem(it->second);
	unsigned int id = m_db->m_schedules.add( newItem );
	endInsertRows();
	QModelIndex idx = indexById(id);
	return idx;
}


void SVDBScheduleTableModel::deleteItem(const QModelIndex & index) {
	if (!index.isValid())
		return;
	unsigned int id = data(index, Role_Id).toUInt();
	beginRemoveRows(QModelIndex(), index.row(), index.row());
	m_db->m_schedules.remove(id);
	endRemoveRows();
}


void SVDBScheduleTableModel::setColumnResizeModes(QTableView * tableView) {
	tableView->horizontalHeader()->setSectionResizeMode(SVDBScheduleTableModel::ColId, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBScheduleTableModel::ColCheck, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBScheduleTableModel::ColAnnualSplineData, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBScheduleTableModel::ColInterpolation, QHeaderView::Fixed);
	tableView->horizontalHeader()->setSectionResizeMode(SVDBScheduleTableModel::ColName, QHeaderView::Stretch);
}


void SVDBScheduleTableModel::setItemModified(unsigned int id) {
	QModelIndex idx = indexById(id);
	QModelIndex left = index(idx.row(), 0);
	QModelIndex right = index(idx.row(), NumColumns-1);
	emit dataChanged(left, right);
}


QModelIndex SVDBScheduleTableModel::indexById(unsigned int id) const {
	for (int i=0; i<rowCount(); ++i) {
		QModelIndex idx = index(i, 0);
		if (data(idx, Role_Id).toUInt() == id)
			return idx;
	}
	return QModelIndex();
}
