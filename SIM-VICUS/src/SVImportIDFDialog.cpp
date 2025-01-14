#include "SVImportIDFDialog.h"
#include "ui_SVImportIDFDialog.h"

#include <QMessageBox>
#include <QProgressDialog>
#include <QElapsedTimer>

#include "SVProjectHandler.h"

#include <EP_Project.h>
#include <EP_IDFParser.h>

SVImportIDFDialog::SVImportIDFDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVImportIDFDialog)
{
	m_ui->setupUi(this);
}


SVImportIDFDialog::~SVImportIDFDialog() {
	delete m_ui;
}


SVImportIDFDialog::ImportResults SVImportIDFDialog::import(const QString & fname) {

	// read IDF file

	try {
		EP::IDFParser parser;
		parser.read(IBK::Path(fname.toStdString()));

		EP::Project prj;
		prj.readIDF(parser);

		// now transfer data to temporary VICUS project structure.
		m_importedProject = VICUS::Project(); // clear data from previous import
		transferData(prj);
	}
	catch (IBK::Exception & ex) {
		QMessageBox::critical((QWidget*)parent(), tr("Import error"), tr("Error parsing IDF file:\n%1").arg(ex.what()));
		return ImportCancelled;
	}

	// if successful, show dialog

	// merge project is only active if we have a project

	m_ui->pushButtonMerge->setEnabled( SVProjectHandler::instance().isValid() );

	int res = exec();
	if (res == QDialog::Rejected)
		return ImportCancelled;

	// TODO : apply coordinate shift to imported building geometry


	return m_returnCode;

}


void SVImportIDFDialog::transferData(const EP::Project & prj) {
	FUNCID(SVImportIDFDialog::transferData);

	VICUS::Project & vp = m_importedProject; // readability improvement
	vp.m_buildings.resize(1);
	vp.m_buildings[0].m_buildingLevels.resize(1);
	VICUS::BuildingLevel & bl = vp.m_buildings[0].m_buildingLevels[0];

	vp.m_buildings[0].m_displayName = "Import_IDF_Building";
	bl.m_displayName = "Floor0";

	std::map<std::string, unsigned int> mapZoneID;
	std::map<std::string, unsigned int>	mapZoneNameToIdx;

	unsigned int transferedBSDCounter = 0;

	QProgressDialog dlg(tr("Importing IDF project"), tr("Abort"), 0, prj.m_zones.size(), this);
	dlg.setWindowModality(Qt::WindowModal);
	dlg.setValue(0);
	qApp->processEvents();

	QElapsedTimer progressTimer;
	progressTimer.start();

	// import all zones
	unsigned int count = 0;
	// TODO : Andreas, add OpenMP parallelization just to show off :-)
	for (int i=0; i<prj.m_zones.size(); ++i) {
		const EP::Zone & z = prj.m_zones[i];
		++count;
		if (progressTimer.elapsed() > 100) {
			dlg.setValue(count);
			if (dlg.wasCanceled())
				throw IBK::Exception("Import canceled.", FUNC_ID);
			progressTimer.start();
		}

		VICUS::Room r;
		r.m_id = r.uniqueID();
		r.m_displayName = QString::fromStdString(z.m_name);

		// remember zone name - id association
		if (mapZoneID.find(z.m_name) != mapZoneID.end())
			throw IBK::Exception(IBK::FormatString("Duplicate zone ID name '%1'").arg(z.m_name), FUNC_ID);
		mapZoneID[z.m_name] = r.m_id;

		// transfer attributes


		// ceiling height is not taken into account
		if(z.m_floorArea > 0)
			r.m_para[VICUS::Room::P_Area].set("Area", z.m_floorArea, "m2" );
		if(z.m_volume > 0)
			r.m_para[VICUS::Room::P_Volume].set("Volume", z.m_volume, "m3" );


		// TODO : Dirk

		// add zone
		mapZoneNameToIdx[z.m_name] = bl.m_rooms.size();
		bl.m_rooms.push_back(r);

		//import all building surface detailed -> opaque surfaces
		for(const EP::BuildingSurfaceDetailed &bsd : prj.m_bsd){

			if(IBK::tolower_string(bsd.m_zoneName) != IBK::tolower_string(z.m_name))
				continue;

			if(mapZoneNameToIdx.find(bsd.m_zoneName) == mapZoneNameToIdx.end())
				throw IBK::Exception(IBK::FormatString("Zone name '%1' does not exist, which is "
													   "referenced in Building Surface Detailed '%2'").arg(bsd.m_zoneName)
														.arg(bsd.m_name), FUNC_ID);
			unsigned idx = mapZoneNameToIdx[bsd.m_zoneName];

			VICUS::Surface surf;
			surf.m_id = surf.uniqueID();
			surf.m_displayName = QString::fromStdString(bsd.m_name);
			surf.m_geometry = VICUS::PlaneGeometry(VICUS::PlaneGeometry::T_Polygon);
			surf.m_geometry.setVertexes(bsd.m_polyline);

			surf.updateColor();
			bl.m_rooms[idx].m_surfaces.push_back(surf);
			++transferedBSDCounter;
		}

		//add surfaces windows, doors, ...
		//add constructions, materials
		//add internal loads ...
	}
	dlg.setValue(count);

	if(transferedBSDCounter != prj.m_bsd.size())
	{
		//nicht alle BSD's wurden transferiert.
	}
}

void SVImportIDFDialog::on_pushButtonReplace_clicked() {
	m_returnCode = ReplaceProject;
	accept();
}

void SVImportIDFDialog::on_pushButtonMerge_clicked() {
	m_returnCode = MergeProjects;
	accept();
}
