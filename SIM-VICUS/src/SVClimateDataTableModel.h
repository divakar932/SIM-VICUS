#ifndef SVClimateDataTableModelH
#define SVClimateDataTableModelH

#include <QAbstractTableModel>

#include "SVClimateFileInfo.h"

/*! A table model that provides a table/list of available climate data files.
	Use the Role_FileName to get just the filename. Use Role_FilePath to get the
	file path including a database placeholder (this should be used to store the location in the project file).
	Finally, use Role_AbsoluteFilePath to get the absolute file path, in case the climate data file needs to be
	read for diagram display.
*/
class SVClimateDataTableModel : public QAbstractTableModel {
public:
	/*! Different columns provided by the model.
		Country and City are stored as meta data in the climate data containers.
		Region and sub are from the file hierarchy:
		DB_climate/<region or continent>/<country>/<sub>/climateFile.c6b

		<sub> is optional. <region> can be "generic"
	*/
	enum Columns {
		C_Region,
		C_Country,
		C_Sub,
		C_City,
		C_Longitude,
		C_Latitude,
		C_TimeZone,
		C_Elevation,
		NUM_C
	};

	SVClimateDataTableModel(QObject * parent);

	// QAbstractItemModel interface

	int rowCount(const QModelIndex & parent) const override;
	int columnCount(const QModelIndex & parent) const override;
	QVariant data(const QModelIndex & index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	/*! Returns access to the raw climate data files used by the model. */
	QList<SVClimateFileInfo> climateFiles() const {	return m_climateFiles; }

	/*! Parses the climate data base directories and refreshs the list of climate data files.
		This also resets the model.
	*/
	void updateClimateFileList();

private:
	/*! Available climate data files (updated in updateClimateFileList()). */
	QList<SVClimateFileInfo>	m_climateFiles;

};

#endif // SVClimateDataTableModelH
