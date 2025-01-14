#include "SVPropVertexListWidget.h"
#include "ui_SVPropVertexListWidget.h"

#include <QMessageBox>
#include <QInputDialog>

#include <IBKMK_Vector3D.h>

#include <QtExt_LanguageHandler.h>

#include <VICUS_Project.h>
#include <QtExt_Conversions.h>
#include <VICUS_KeywordList.h>
#include <VICUS_ComponentInstance.h>

#include "SVProjectHandler.h"
#include "SVViewStateHandler.h"
#include "SVUndoAddSurface.h"
#include "SVGeometryView.h"
#include "SVSettings.h"
#include "SVMainWindow.h"
#include "SVUndoAddBuilding.h"
#include "SVUndoAddBuildingLevel.h"
#include "SVUndoAddZone.h"

#include "Vic3DNewGeometryObject.h"
#include "Vic3DCoordinateSystemObject.h"

SVPropVertexListWidget::SVPropVertexListWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropVertexListWidget)
{
	m_ui->setupUi(this);

	QSizePolicy sp_retain = m_ui->groupBoxPolygonVertexes->sizePolicy();
	sp_retain.setRetainSizeWhenHidden(true);
	m_ui->groupBoxPolygonVertexes->setSizePolicy(sp_retain);

	m_ui->lineEditZoneHeight->setup(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(),
									tr("Zone height in [m]."),true, true);


	SVViewStateHandler::instance().m_propVertexListWidget = this;

	connect(m_ui->toolButtonEditComponents1, &QToolButton::clicked,
			this, &SVPropVertexListWidget::onEditComponents);
	connect(m_ui->toolButtonEditComponents2, &QToolButton::clicked,
			this, &SVPropVertexListWidget::onEditComponents);
	connect(m_ui->toolButtonEditComponents3, &QToolButton::clicked,
			this, &SVPropVertexListWidget::onEditComponents);
	connect(m_ui->toolButtonEditComponents4, &QToolButton::clicked,
			this, &SVPropVertexListWidget::onEditComponents);

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVPropVertexListWidget::onModified);
}


SVPropVertexListWidget::~SVPropVertexListWidget() {
	delete m_ui;
}


void SVPropVertexListWidget::setup(int newGeometryType) {
	m_ui->groupBoxPolygonVertexes->setVisible(false);
	m_ui->groupBoxPolygonVertexes->setEnabled(true);
	m_ui->groupBoxZoneProperties->setVisible(false);
	m_ui->groupBoxSurfaceProperties->setVisible(false);
	m_ui->checkBoxAnnonymousGeometry->setVisible(false);
	QString baseName(tr("New surface"));
	switch (newGeometryType) {
		case Vic3D::NewGeometryObject::NGM_Rect :
			m_ui->groupBoxSurfaceProperties->setVisible(true);
			m_ui->checkBoxAnnonymousGeometry->setVisible(true);
			m_ui->pushButtonFinish->setText(tr("Create surface"));
		break;
		case Vic3D::NewGeometryObject::NGM_Polygon :
			m_ui->groupBoxPolygonVertexes->setVisible(true);
			m_ui->groupBoxSurfaceProperties->setVisible(true);
			m_ui->checkBoxAnnonymousGeometry->setVisible(true);
			m_ui->pushButtonFinish->setText(tr("Create surface"));
		break;
		case Vic3D::NewGeometryObject::NGM_ZoneFloor :
			m_ui->groupBoxPolygonVertexes->setVisible(true);
			m_ui->groupBoxZoneProperties->setVisible(true);
			baseName = tr("New zone");
			m_ui->pushButtonFinish->setText(tr("Create zone"));
		break;

		// we do not have other geometries, yet
		default:;
	}

	// populate component combo boxes
	updateBuildingComboBox(); // this will also update the other combo boxes
	updateComponentComboBoxes();

	// compose object names until we found a unique object name
	std::set<QString> existingNames;
	for (const VICUS::Surface & s : project().m_plainGeometry)
		existingNames.insert(s.m_displayName);

	// set new unique object/surface name
	m_ui->lineEditName->setText( VICUS::Project::uniqueName(baseName, existingNames));
	clearPolygonVertexList();
}


bool SVPropVertexListWidget::reselectById(QComboBox * combo, int id) const {
	combo->setEnabled(true);
	if (id != -1) {
		id = combo->findData(id);
		if (id != -1) {
			combo->setCurrentIndex(id);
			return true;
		}
	}
	if (combo->count() != 0)
		combo->setCurrentIndex(0);
	else {
		combo->setEnabled(false);
	}
	return false;
}


void SVPropVertexListWidget::updateBuildingComboBox() {
	// populate the combo boxes
	m_ui->comboBoxBuilding->blockSignals(true);
	unsigned int currentUniqueId = m_ui->comboBoxBuilding->currentData().toUInt();
	m_ui->comboBoxBuilding->clear();

	const VICUS::Project & prj = project();
	int rowOfCurrent = -1;
	for (unsigned int i=0; i<prj.m_buildings.size(); ++i) {
		const VICUS::Building & b = prj.m_buildings[i];
		m_ui->comboBoxBuilding->addItem(b.m_displayName, b.uniqueID());
		if (b.uniqueID() == currentUniqueId)
			rowOfCurrent = (int)i;
	}

	if (rowOfCurrent != -1) {
		m_ui->comboBoxBuilding->setCurrentIndex(rowOfCurrent);
	}
	else {
		m_ui->comboBoxBuilding->setCurrentIndex(m_ui->comboBoxBuilding->count()-1); // Note: if no buildings, nothing will be selected
	}
	m_ui->comboBoxBuilding->blockSignals(false);

	// also update the building levels combo box
	updateBuildingLevelsComboBox();
}


void SVPropVertexListWidget::updateBuildingLevelsComboBox() {
	m_ui->comboBoxBuildingLevel->blockSignals(true);
	unsigned int currentUniqueId = m_ui->comboBoxBuildingLevel->currentData().toUInt();
	m_ui->comboBoxBuildingLevel->clear();
	// only add items if we have a building selected
	if (m_ui->comboBoxBuilding->count() != 0) {
		const VICUS::Project & prj = project();
		unsigned int buildingUniqueID = m_ui->comboBoxBuilding->currentData().toUInt();
		const VICUS::Building * b = dynamic_cast<const VICUS::Building*>(prj.objectById(buildingUniqueID));
		Q_ASSERT(b != nullptr);
		int rowOfCurrent = -1;
		for (unsigned int i=0; i<b->m_buildingLevels.size(); ++i) {
			const VICUS::BuildingLevel & bl = b->m_buildingLevels[i];
			m_ui->comboBoxBuildingLevel->addItem(bl.m_displayName, bl.uniqueID());
			if (bl.uniqueID() == currentUniqueId)
				rowOfCurrent = (int)i;
		}
		if (rowOfCurrent != -1) {
			m_ui->comboBoxBuildingLevel->setCurrentIndex(rowOfCurrent);
		}
		else {
			m_ui->comboBoxBuildingLevel->setCurrentIndex(m_ui->comboBoxBuildingLevel->count()-1); // Note: if none, nothing will be selected
		}

	}
	m_ui->comboBoxBuildingLevel->blockSignals(false);

	// also update the zones combo box
	on_comboBoxBuildingLevel_currentIndexChanged(m_ui->comboBoxBuildingLevel->currentIndex());
}


void SVPropVertexListWidget::updateZoneComboBox() {
	m_ui->comboBoxZone->blockSignals(true);
	unsigned int currentUniqueId = m_ui->comboBoxZone->currentData().toUInt();
	m_ui->comboBoxZone->clear();
	// only add items if we have a building level selected
	if (m_ui->comboBoxBuildingLevel->count() != 0) {
		const VICUS::Project & prj = project();
		unsigned int buildingLevelUniqueID = m_ui->comboBoxBuildingLevel->currentData().toUInt();
		const VICUS::BuildingLevel * bl = dynamic_cast<const VICUS::BuildingLevel*>(prj.objectById(buildingLevelUniqueID));
		Q_ASSERT(bl != nullptr);
		int rowOfCurrent = -1;
		for (unsigned int i=0; i<bl->m_rooms.size(); ++i) {
			const VICUS::Room & r = bl->m_rooms[i];
			m_ui->comboBoxZone->addItem(r.m_displayName, r.uniqueID());
			if (r.uniqueID() == currentUniqueId)
				rowOfCurrent = (int)i;
		}
		if (rowOfCurrent != -1) {
			m_ui->comboBoxZone->setCurrentIndex(rowOfCurrent);
		}
		else {
			m_ui->comboBoxZone->setCurrentIndex(m_ui->comboBoxZone->count()-1); // Note: if none, nothing will be selected
		}
	}
	m_ui->comboBoxZone->blockSignals(false);
	updateEnabledStates();
}


void SVPropVertexListWidget::updateComponentComboBoxes() {
	// remember currently selected component IDs
	int floorCompID = -1;
	int ceilingCompID = -1;
	int wallCompID = -1;
	int surfaceCompID = -1;
	if (m_ui->comboBoxComponentFloor->currentIndex() != -1)
		floorCompID = m_ui->comboBoxComponentFloor->currentData().toInt();
	if (m_ui->comboBoxComponentCeiling->currentIndex() != -1)
		ceilingCompID = m_ui->comboBoxComponentCeiling->currentData().toInt();
	if (m_ui->comboBoxComponentWalls->currentIndex() != -1)
		wallCompID = m_ui->comboBoxComponentWalls->currentData().toInt();
	if (m_ui->comboBoxComponent->currentIndex() != -1)
		surfaceCompID = m_ui->comboBoxComponent->currentData().toInt();

	m_ui->comboBoxComponentFloor->clear();
	m_ui->comboBoxComponentCeiling->clear();
	m_ui->comboBoxComponentWalls->clear();
	m_ui->comboBoxComponent->clear();

	std::string langID = QtExt::LanguageHandler::instance().langId().toStdString();
	for (auto & c : SVSettings::instance().m_db.m_components) {
		switch (c.second.m_type) {
			case VICUS::Component::CT_OutsideWall :
			case VICUS::Component::CT_OutsideWallToGround :
			case VICUS::Component::CT_InsideWall :
				m_ui->comboBoxComponentWalls->addItem( QString::fromStdString(c.second.m_displayName.string(langID, "en")), c.first);
			break;

			case VICUS::Component::CT_FloorToCellar :
			case VICUS::Component::CT_FloorToAir :
			case VICUS::Component::CT_FloorToGround :
				m_ui->comboBoxComponentFloor->addItem( QString::fromStdString(c.second.m_displayName.string(langID, "en")), c.first);
			break;

			case VICUS::Component::CT_Ceiling :
			case VICUS::Component::CT_SlopedRoof :
			case VICUS::Component::CT_FlatRoof :
			case VICUS::Component::CT_ColdRoof :
			case VICUS::Component::CT_WarmRoof :
				m_ui->comboBoxComponentCeiling->addItem( QString::fromStdString(c.second.m_displayName.string(langID, "en")), c.first);
			break;

			case VICUS::Component::CT_Miscellaneous :
			case VICUS::Component::NUM_CT:
				m_ui->comboBoxComponentFloor->addItem( QString::fromStdString(c.second.m_displayName.string(langID, "en")), c.first);
				m_ui->comboBoxComponentCeiling->addItem( QString::fromStdString(c.second.m_displayName.string(langID, "en")), c.first);
				m_ui->comboBoxComponentWalls->addItem( QString::fromStdString(c.second.m_displayName.string(langID, "en")), c.first);
			break;
		}

		m_ui->comboBoxComponent->addItem( QString::fromStdString(c.second.m_displayName.string(langID, "en")), c.first);
	}

	// reselect previously selected components
	reselectById(m_ui->comboBoxComponentFloor, floorCompID);
	reselectById(m_ui->comboBoxComponentCeiling, ceilingCompID);
	reselectById(m_ui->comboBoxComponentWalls, wallCompID);
	reselectById(m_ui->comboBoxComponent, surfaceCompID);
}


void SVPropVertexListWidget::addVertex(const IBKMK::Vector3D & p) {
	// Note: the vertex is already in the NewGeometryObject, we only
	//       modify the table widget and update the button enabled states
	int row = m_ui->tableWidgetVertexes->rowCount();
	m_ui->tableWidgetVertexes->setRowCount(row + 1);
	QTableWidgetItem * item = new QTableWidgetItem(QString("%1").arg(row+1));
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	m_ui->tableWidgetVertexes->setItem(row,0,item);
	item = new QTableWidgetItem(QString("%L1,%L2,%L3").arg(p.m_x).arg(p.m_y).arg(p.m_z));
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	m_ui->tableWidgetVertexes->setItem(row,1,item);

	m_ui->pushButtonDeleteLast->setEnabled(true);

	m_ui->pushButtonFinish->setEnabled(SVViewStateHandler::instance().m_newGeometryObject->canComplete());
	if (SVViewStateHandler::instance().m_newGeometryObject->planeGeometry().isValid()) {
		// only enabled floor done button (i.e. start extrusion) if we are in NGM_ZoneFloor mode
		if (SVViewStateHandler::instance().m_newGeometryObject->newGeometryMode() == Vic3D::NewGeometryObject::NGM_ZoneFloor)
			m_ui->pushButtonFloorDone->setEnabled(true);
	}
}


void SVPropVertexListWidget::removeVertex(unsigned int idx) {
	// Note: the vertex has already been removed in the NewGeometryObject, we only
	//       modify the table widget and update the button enabled states
	int rows = m_ui->tableWidgetVertexes->rowCount();
	Q_ASSERT(rows > 0);
	Q_ASSERT((int)idx < m_ui->tableWidgetVertexes->rowCount());
	// now remove selected row from table widget
	m_ui->tableWidgetVertexes->removeRow((int)idx);
	m_ui->pushButtonDeleteLast->setEnabled(rows > 1);
	m_ui->pushButtonFinish->setEnabled(SVViewStateHandler::instance().m_newGeometryObject->canComplete());
	if (SVViewStateHandler::instance().m_newGeometryObject->planeGeometry().isValid()) {
		// only enabled floor done button (i.e. start extrusion) if we are in NGM_ZoneFloor mode
		if (SVViewStateHandler::instance().m_newGeometryObject->newGeometryMode() == Vic3D::NewGeometryObject::NGM_ZoneFloor)
			m_ui->pushButtonFloorDone->setEnabled(true);
	}

	// continue in place-vertex mode (setting the viewstate also triggers a repaint)
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_sceneOperationMode = SVViewState::OM_PlaceVertex;
	vs.m_propertyWidgetMode = SVViewState::PM_VertexList;
	SVViewStateHandler::instance().setViewState(vs);
	SVViewStateHandler::instance().m_geometryView->focusSceneView();
}


void SVPropVertexListWidget::setExtrusionDistance(double dist) {
	if (m_ui->groupBoxZoneProperties->isVisibleTo(this)) {
		m_ui->lineEditZoneHeight->setText(QString("%L1").arg(dist));
	}
}


void SVPropVertexListWidget::onModified(int modificationType, ModificationInfo * /*data*/) {
	// only do something here, if this widget is actually visible
	if (!isVisibleTo(qobject_cast<QWidget*>(parent())) )
		return;
	SVProjectHandler::ModificationTypes mod = (SVProjectHandler::ModificationTypes)modificationType;
	switch (mod) {
		// We only need to handle changes of the building topology, in all other cases
		// the "create new geometry" action is aborted and the widget will be hidden.
		case SVProjectHandler::BuildingTopologyChanged:
			updateBuildingComboBox(); // this will also update the other combo boxes
		break;
		default:;
	}
}


void SVPropVertexListWidget::clearPolygonVertexList() {
	// clear table widget and disable "delete" and "finish" buttons
	m_ui->tableWidgetVertexes->setRowCount(0);
	m_ui->pushButtonFinish->setEnabled(false);
	m_ui->pushButtonFloorDone->setEnabled(false);
	m_ui->pushButtonDeleteLast->setEnabled(false);
	m_ui->pushButtonDeleteSelected->setEnabled(false);
}


void SVPropVertexListWidget::on_pushButtonDeleteLast_clicked() {
	// remove last vertex from polygon
	Vic3D::NewGeometryObject * po = SVViewStateHandler::instance().m_newGeometryObject;
	po->removeLastVertex();
}


void SVPropVertexListWidget::on_tableWidgetVertexes_itemSelectionChanged() {
	m_ui->pushButtonDeleteSelected->setEnabled( m_ui->tableWidgetVertexes->currentRow() != -1);
}


void SVPropVertexListWidget::on_pushButtonCancel_clicked() {
	// reset new polygon object, so that it won't be drawn anylonger
	SVViewStateHandler::instance().m_newGeometryObject->clear();
	// signal, that we are no longer in "add vertex" mode
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_sceneOperationMode = SVViewState::NUM_OM;
	vs.m_propertyWidgetMode = SVViewState::PM_AddGeometry;
	// reset locks
	vs.m_locks = SVViewState::NUM_L;

	// take xy plane out of snap option mask
	vs.m_snapEnabled = true;
	// now tell all UI components to toggle their view state
	SVViewStateHandler::instance().setViewState(vs);
}


void SVPropVertexListWidget::on_pushButtonDeleteSelected_clicked() {
	int currentRow = m_ui->tableWidgetVertexes->currentRow();
	Q_ASSERT(currentRow != -1);
	// remove selected vertex from polygon
	Vic3D::NewGeometryObject * po = SVViewStateHandler::instance().m_newGeometryObject;
	po->removeVertex((unsigned int)currentRow); // this will in turn call removeVertex() above
}


void SVPropVertexListWidget::on_pushButtonFinish_clicked() {
	if (m_ui->lineEditName->text().trimmed().isEmpty()) {
		QMessageBox::critical(this, QString(), tr("Please enter a descriptive name!"));
		m_ui->lineEditName->selectAll();
		m_ui->lineEditName->setFocus();
		return;
	}
	// depending on the type of geometry that's being created,
	// perform additional checks
	Vic3D::NewGeometryObject * po = SVViewStateHandler::instance().m_newGeometryObject;
	switch (po->newGeometryMode()) {
		case Vic3D::NewGeometryObject::NGM_Rect:
		case Vic3D::NewGeometryObject::NGM_Polygon: {
			// compose a surface object based on the current content of the new polygon object
			VICUS::Surface s;
			s.m_displayName = m_ui->lineEditName->text().trimmed();
			s.m_geometry = po->planeGeometry();

			// we need all properties, unless we create annonymous geometry
			if (m_ui->checkBoxAnnonymousGeometry->isChecked()) {
				s.m_color = QColor("silver");
				s.m_id = VICUS::Project::uniqueId(project().m_plainGeometry);
				// modify project
				SVUndoAddSurface * undo = new SVUndoAddSurface(tr("Added surface '%1'").arg(s.m_displayName), s, 0);
				undo->push();
			}
			else {
				// we need inputs for room (if there is a room, there is also a building and level)
				if (m_ui->comboBoxZone->currentIndex() == -1) {
					QMessageBox::critical(this, QString(), tr("First select a zone to add the surface to!"));
					return;
				}
				unsigned int zoneUUID = m_ui->comboBoxZone->currentData().toUInt();
				Q_ASSERT(zoneUUID != 0);

				s.updateColor(); // set color based on orientation
				// the surface will get the unique ID as persistant ID
				s.m_id = s.uniqueID();
				// also store component information
				VICUS::ComponentInstance compInstance;
				compInstance.m_id = VICUS::Project::uniqueId(project().m_componentInstances);
				compInstance.m_componentID = m_ui->comboBoxComponent->currentData().toUInt();
				// for now we assume that the zone's surface is connected to the b-side of the component
				compInstance.m_sideBSurfaceID = s.m_id;
				// modify project
				SVUndoAddSurface * undo = new SVUndoAddSurface(tr("Added surface '%1'").arg(s.m_displayName), s, zoneUUID, &compInstance);
				undo->push();
			}
		} break;

		case Vic3D::NewGeometryObject::NGM_ZoneExtrusion : {

			// we need a building level
			if (m_ui->comboBoxBuildingLevel->currentIndex() == -1) {
				QMessageBox::critical(this, QString(), tr("First select a building level to add the zone/room to!"));
				return;
			}
			// tricky part starts now
			// 1. we need to get a list of surfaces and make their normal vectors point outwards
			// 2. we need to assign colors to the surfaces and default components based on
			//    inclination
			// 3. we need to create an undo-action

			// take the polygon
			VICUS::PlaneGeometry floor = po->planeGeometry();
			VICUS::PlaneGeometry ceiling = po->offsetPlaneGeometry();
			// Note: both polygons still have the same normal vector!

			// compute offset vector
			IBKMK::Vector3D offset = ceiling.vertexes()[0] - floor.vertexes()[0];
			// now check if ceiling is offset in same direction as normal vector of floor plane?
			double dotProduct = offset.scalarProduct(floor.normal());
			if (dotProduct > 0) {
				// same direction, we need to reverse floor polygon
				floor.flip();
			}
			else {
				// opposite direction, we need to reverse the ceiling polygon
				ceiling.flip();
			}

			std::vector<VICUS::ComponentInstance> componentInstances;
			VICUS::Room r;
			r.m_displayName = m_ui->lineEditName->text().trimmed();
			// now we can create the surfaces for top and bottom
			// compose a surface object based on the current content of the new polygon object
			VICUS::Surface sFloor;
			sFloor.m_displayName = QString("Floor");
			sFloor.m_id = sFloor.uniqueID();
			VICUS::Surface sCeiling;
			sCeiling.m_displayName = QString("Ceiling");
			sCeiling.m_id = sCeiling.uniqueID();
			// if the ceiling has a normal vector pointing up, we take it as ceiling, otherwise it's going to be the floor
			if (IBKMK::Vector3D(0,0,1).scalarProduct(ceiling.normal()) > 0) {
				sCeiling.m_geometry = ceiling;
				sFloor.m_geometry = floor;
			}
			else {
				sCeiling.m_geometry = floor;
				sFloor.m_geometry = ceiling;
			}

			sFloor.updateColor();
			// get the smallest yet free ID for component instances/construction instances
			unsigned int conInstID = VICUS::Project::largestUniqueId(project().m_componentInstances);
			// Note: surface is attached to "Side A"
			componentInstances.push_back(VICUS::ComponentInstance(++conInstID,
				 m_ui->comboBoxComponentFloor->currentData().toUInt(), sFloor.m_id, VICUS::INVALID_ID));

			sCeiling.updateColor();
			// Note: surface is attached to "Side A"
			componentInstances.push_back(VICUS::ComponentInstance(++conInstID,
				 m_ui->comboBoxComponentCeiling->currentData().toUInt(), sCeiling.m_id, VICUS::INVALID_ID));

			r.m_id = r.uniqueID();
			r.m_surfaces.push_back(sFloor);
			r.m_surfaces.push_back(sCeiling);

			// now loop around the circle and create planes for wall segments
			// we take the floor polygon

			unsigned int nVert = floor.vertexes().size();
			unsigned int wallComponentID = m_ui->comboBoxComponentWalls->currentData().toUInt();
			for (unsigned int i=0; i<nVert; ++i) {
				// mind the winding order
				// when looked from above, floor vertexes go clock-wise,
				// and ceiling vertices go anti-clockwise
				unsigned int vIdx2 = (i+1) % nVert;
				//IBKMK::Vector3D p0 = floor.vertexes()[ i ];
				//IBKMK::Vector3D p1 = floor.vertexes()[ vIdx2 ];
				//IBKMK::Vector3D p2 = floor.vertexes()[ i ] + offset;	//take offset as last point for rectangle; rounding errors by vector-sum?

				IBKMK::Vector3D p0 = floor.vertexes()[ vIdx2 ];
				IBKMK::Vector3D p1 = floor.vertexes()[ i ];
				IBKMK::Vector3D p2 = floor.vertexes()[ vIdx2 ] + offset;	//take offset as last point for rectangle; rounding errors by vector-sum?

				//				IBKMK::Vector3D a = p1-p0;
				//				IBKMK::Vector3D b = p2-p0;

				//				qDebug() << "Plane: " << VICUS::IBKVector2String(a) << " : " << VICUS::IBKVector2String(b);

				VICUS::Surface sWall;
				sWall.m_id = sWall.uniqueID();
				sWall.m_displayName = tr("Wall %1").arg(i+1);
				sWall.m_geometry = VICUS::PlaneGeometry( VICUS::PlaneGeometry::T_Rectangle, p0, p1, p2);
				sWall.updateColor();
				// wall surface is attached to "Side A"
				componentInstances.push_back(VICUS::ComponentInstance(++conInstID,
															  wallComponentID, sWall.m_id, VICUS::INVALID_ID));

				r.m_surfaces.push_back(sWall);
			}

			double area = sFloor.m_geometry.area();
			VICUS::KeywordList::setParameter(r.m_para, "Room::para_t", VICUS::Room::P_Area, area);
			VICUS::KeywordList::setParameter(r.m_para, "Room::para_t", VICUS::Room::P_Volume, area*offset.magnitude());

			// now create the undo action
			unsigned int buildingLevelUid = m_ui->comboBoxBuildingLevel->currentData().toUInt();
			Q_ASSERT(buildingLevelUid != 0);
			SVUndoAddZone * undo = new SVUndoAddZone(tr("Adding new zone '%1'").arg(r.m_displayName),
													 buildingLevelUid,
													 r, false, &componentInstances);
			undo->push();
		}
		break;

		case Vic3D::NewGeometryObject::NGM_ZoneFloor:
		case Vic3D::NewGeometryObject::NUM_NGM:
			Q_ASSERT(false); // invalid operation
			return;
	}

	// reset view
	on_pushButtonCancel_clicked();
}


void SVPropVertexListWidget::onEditComponents() {
	// ask main window to show database dialog, afterwards update component combos
	SVMainWindow::instance().on_actionDBComponents_triggered();
	// Note: SVMainWindow::instance().on_actionDBComponents_triggered() calls updateComponentCombos() itself, so
	//       no need to call this here
}


void SVPropVertexListWidget::on_toolButtonAddBuilding_clicked() {
	std::set<QString> existingNames;
	for (const VICUS::Building & b : project().m_buildings)
		existingNames.insert(b.m_displayName);
	QString defaultName = VICUS::Project::uniqueName(tr("Building"), existingNames);
	QString text = QInputDialog::getText(this, tr("Add building"), tr("New building name:"), QLineEdit::Normal, defaultName).trimmed();
	if (text.isEmpty()) return;
	// modify project
	VICUS::Building b;
	b.m_id = VICUS::Project::uniqueId(project().m_buildings);
	b.m_displayName = text;
	SVUndoAddBuilding * undo = new SVUndoAddBuilding(tr("Adding building '%1'").arg(b.m_displayName), b, true);
	undo->push(); // this will update our combo boxes

	// now also select the matching item
	reselectById(m_ui->comboBoxBuilding, (int)b.uniqueID());
}


void SVPropVertexListWidget::on_toolButtonAddBuildingLevel_clicked() {
	// get currently selected building
	unsigned int buildingUniqueID = m_ui->comboBoxBuilding->currentData().toUInt();
	const VICUS::Building * b = dynamic_cast<const VICUS::Building*>(project().objectById(buildingUniqueID));
	Q_ASSERT(b != nullptr);

	std::set<QString> existingNames;
	for (const VICUS::BuildingLevel & bl : b->m_buildingLevels)
		existingNames.insert(bl.m_displayName);
	QString defaultName = VICUS::Project::uniqueName(tr("Level"), existingNames);
	QString text = QInputDialog::getText(this, tr("Add building level"), tr("New building level/floor name:"), QLineEdit::Normal, defaultName).trimmed();
	if (text.isEmpty()) return;

	// modify project
	VICUS::BuildingLevel bl;
	bl.m_id = VICUS::Project::uniqueId(b->m_buildingLevels);
	bl.m_displayName = text;
	SVUndoAddBuildingLevel * undo = new SVUndoAddBuildingLevel(tr("Adding building level '%1'").arg(bl.m_displayName), buildingUniqueID, bl, true);
	undo->push(); // this will update our combo boxes

	// now also select the matching item
	reselectById(m_ui->comboBoxBuildingLevel, (int)bl.uniqueID());
}


void SVPropVertexListWidget::on_toolButtonAddZone_clicked() {
	// get currently selected building
	unsigned int buildingLevelUniqueID = m_ui->comboBoxBuildingLevel->currentData().toUInt();
	const VICUS::BuildingLevel * bl = dynamic_cast<const VICUS::BuildingLevel*>(project().objectById(buildingLevelUniqueID));
	Q_ASSERT(bl != nullptr);

	std::set<QString> existingNames;
	for (const VICUS::Room & r : bl->m_rooms)
		existingNames.insert(r.m_displayName);
	QString defaultName = VICUS::Project::uniqueName(tr("Room"), existingNames);
	QString text = QInputDialog::getText(this, tr("Add room/zone"), tr("New room/zone name:"), QLineEdit::Normal, defaultName).trimmed();
	if (text.isEmpty()) return;

	// modify project
	VICUS::Room r;
	r.m_id = VICUS::Project::uniqueId(bl->m_rooms);
	r.m_displayName = text;
	SVUndoAddZone * undo = new SVUndoAddZone(tr("Adding building zone '%1'").arg(r.m_displayName), buildingLevelUniqueID, r, true);
	undo->push(); // this will update our combo boxes

	// now also select the matching item
	reselectById(m_ui->comboBoxZone, (int)r.uniqueID());
}


void SVPropVertexListWidget::on_checkBoxAnnonymousGeometry_stateChanged(int /*arg1*/) {
	updateEnabledStates();
}


bool SVPropVertexListWidget::createAnnonymousGeometry() const {
	return (m_ui->checkBoxAnnonymousGeometry->isVisibleTo(this) && m_ui->checkBoxAnnonymousGeometry->isChecked());
}

void SVPropVertexListWidget::updateEnabledStates() {
	// if checkbox is visible, we adjust the enabled state of other inputs
	bool annonymousGeometry = createAnnonymousGeometry();
	if (annonymousGeometry) {
		m_ui->groupBoxSurfaceProperties->setEnabled(false);

		m_ui->labelBuilding->setEnabled(false);
		m_ui->comboBoxBuilding->setEnabled(false);
		m_ui->toolButtonAddBuilding->setEnabled(false);

		m_ui->labelBuildingLevel->setEnabled(false);
		m_ui->comboBoxBuildingLevel->setEnabled(false);
		m_ui->toolButtonAddBuildingLevel->setEnabled(false);

		m_ui->labelZone->setEnabled(false);
		m_ui->comboBoxZone->setEnabled(false);
		m_ui->toolButtonAddZone->setEnabled(false);
	}
	else {
		m_ui->groupBoxSurfaceProperties->setEnabled(true);

		// building controls
		if (m_ui->comboBoxBuilding->count() == 0) {
			m_ui->comboBoxBuilding->setEnabled(false);
		}
		else {
			m_ui->comboBoxBuilding->setEnabled(true);
		}
		m_ui->labelBuilding->setEnabled(true);
		m_ui->toolButtonAddBuilding->setEnabled(true);


		// building level controls
		if (m_ui->comboBoxBuildingLevel->count() == 0) {
			m_ui->comboBoxBuildingLevel->setEnabled(false);
		}
		else {
			m_ui->comboBoxBuildingLevel->setEnabled(true);
		}
		// enable tool button to add new levels
		m_ui->toolButtonAddBuildingLevel->setEnabled(m_ui->comboBoxBuilding->count() != 0);
		m_ui->labelBuildingLevel->setEnabled(m_ui->comboBoxBuilding->count() != 0);


		// room controls
		// never enabled when we create zones
		if (m_ui->groupBoxZoneProperties->isVisibleTo(this)) {
			m_ui->labelZone->setEnabled(false);
			m_ui->comboBoxZone->setEnabled(false);
			m_ui->toolButtonAddZone->setEnabled(false);
		}
		else {
			if (m_ui->comboBoxZone->count() == 0) {
				m_ui->comboBoxZone->setEnabled(false);
			}
			else {
				m_ui->comboBoxZone->setEnabled(true);
			}
			// enable tool button to add new zones
			m_ui->toolButtonAddZone->setEnabled(m_ui->comboBoxBuildingLevel->count() != 0);
			m_ui->labelZone->setEnabled(m_ui->comboBoxBuildingLevel->count() != 0);
		}
	}
}


void SVPropVertexListWidget::on_comboBoxBuilding_currentIndexChanged(int /*index*/) {
	updateBuildingLevelsComboBox();
}


void SVPropVertexListWidget::on_comboBoxBuildingLevel_currentIndexChanged(int /*index*/) {
	updateZoneComboBox();
	unsigned int buildingLevelUniqueID = m_ui->comboBoxBuildingLevel->currentData().toUInt();
	const VICUS::BuildingLevel * bl = dynamic_cast<const VICUS::BuildingLevel*>(project().objectById(buildingLevelUniqueID));
	// also transfer nominal height into zone-height line edit
	if (bl != nullptr) {
		m_ui->lineEditZoneHeight->setValue(bl->m_height);
		on_lineEditZoneHeight_editingFinishedSuccessfully();
	}
}


void SVPropVertexListWidget::on_pushButtonFloorDone_clicked() {
	// we switch to floor-extrusion mode now
	Vic3D::NewGeometryObject * po = SVViewStateHandler::instance().m_newGeometryObject;
	po->switchTo(Vic3D::NewGeometryObject::NGM_ZoneExtrusion);
	m_ui->pushButtonFloorDone->setEnabled(false);
	m_ui->pushButtonFinish->setEnabled(true);
	m_ui->groupBoxPolygonVertexes->setEnabled(false);

	// we now must align the local coordinate system to the newly created plane

	IBKMK::Vector3D x = po->planeGeometry().localX();
	IBKMK::Vector3D y = po->planeGeometry().localY();
	IBKMK::Vector3D z = po->planeGeometry().normal();

	// special handling - normal vector of a horizontal plane should always point upwards

	double proj = z.scalarProduct(IBKMK::Vector3D(0,0,1));
	if (std::fabs(proj) > 0.999999) {
		if (proj < 0) {
			// invert floor polygon and set inverted polygon in polygon object
			qDebug() << "Zone with horizontal floor drawn, but upside-down normal. Flipping surface.";
			po->flipGeometry();
			x = po->planeGeometry().localX();
			y = po->planeGeometry().localY();
			z = po->planeGeometry().normal();
		}
	}


	QQuaternion q2 = QQuaternion::fromAxes(QtExt::IBKVector2QVector(x.normalized()),
										   QtExt::IBKVector2QVector(y.normalized()),
										   QtExt::IBKVector2QVector(z.normalized()));
	SVViewStateHandler::instance().m_coordinateSystemObject->setRotation(q2);

	// now also enable the z snap operation
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_locks = SVViewState::L_LocalZ; // local Z axis is locked
	SVViewStateHandler::instance().setViewState(vs);
	// now also transfer the zone height to the zone object
	if (m_ui->lineEditZoneHeight->isValid())
		on_lineEditZoneHeight_editingFinishedSuccessfully();
}


void SVPropVertexListWidget::on_lineEditZoneHeight_editingFinishedSuccessfully() {
	// read entered line height and if valid move local coordinate system to new height
	double val = m_ui->lineEditZoneHeight->value();
	Vic3D::NewGeometryObject * po = SVViewStateHandler::instance().m_newGeometryObject;
	if (po->newGeometryMode() == Vic3D::NewGeometryObject::NGM_ZoneExtrusion) {
		po->setZoneHeight(val);
		// we need to trigger a redraw here
		SVViewStateHandler::instance().m_geometryView->refreshSceneView();
	}
}


void SVPropVertexListWidget::on_pushButtonPickZoneHeight_clicked() {
	// enable interactive zone extrusion mode
	Vic3D::NewGeometryObject * po = SVViewStateHandler::instance().m_newGeometryObject;
	// check if interactive mode is already enabled
	po->m_interactiveZoneExtrusionMode = !po->m_interactiveZoneExtrusionMode;
}
