#include "SVPropertyWidget.h"

#include <QVBoxLayout>
#include <QToolBox>

#include <IBKMK_Vector3D.h>

#include "SVViewStateHandler.h"
#include "SVProjectHandler.h"
#include "SVConstants.h"

#include "SVPropVertexListWidget.h"
#include "SVPropEditGeometry.h"
#include "SVPropSiteWidget.h"
#include "SVPropNetworkEditWidget.h"
#include "SVPropModeSelectionWidget.h"
#include "SVPropBuildingEditWidget.h"
#include "SVPropFloorManagerWidget.h"


#include "Vic3DNewGeometryObject.h"
#include "Vic3DCoordinateSystemObject.h"
#include "Vic3DWireFrameObject.h"

SVPropertyWidget::SVPropertyWidget(QWidget * parent) :
	QWidget(parent)
{
	m_layout = new QVBoxLayout(this);
	m_layout->setMargin(0);
	setLayout(m_layout);
	for (QWidget * &w : m_propWidgets)
		w = nullptr;

	m_propModeSelectionWidget = new SVPropModeSelectionWidget(this);
	m_layout->addWidget(m_propModeSelectionWidget);


	setMinimumWidth(200);

	connect(&SVViewStateHandler::instance(), &SVViewStateHandler::viewStateChanged,
			this, &SVPropertyWidget::onViewStateChanged);

	onViewStateChanged();
}



void SVPropertyWidget::onViewStateChanged() {

	// hide all already created widgets
	for (QWidget * w : m_propWidgets) {
		if (w != nullptr)
			w->setVisible(false);
	}

	SVViewState vs = SVViewStateHandler::instance().viewState();

	switch (vs.m_viewMode) {
		case SVViewState::VM_GeometryEditMode:
			m_propModeSelectionWidget->setVisible(false);
		break;
		case SVViewState::VM_PropertyEditMode:
			m_propModeSelectionWidget->setVisible(true);
		break;
		case SVViewState::NUM_VM: break; // just to make compiler happy
	}


	// now show the respective property widget
	SVViewState::PropertyWidgetMode m = vs.m_propertyWidgetMode;
	switch (m) {
		case SVViewState::PM_EditGeometry :
		case SVViewState::PM_AddGeometry : {
			showPropertyWidget<SVPropEditGeometry>(M_Geometry);

			// Note: we do not use the slot for SVViewState::PM_AddGeometry; instead we just show a different tab
			if (m == SVViewState::PM_EditGeometry)
				qobject_cast<SVPropEditGeometry *>(m_propWidgets[M_Geometry])->setCurrentPage(SVPropEditGeometry::O_EditGeometry);
			else
				qobject_cast<SVPropEditGeometry *>(m_propWidgets[M_Geometry])->setCurrentPage(SVPropEditGeometry::O_AddGeometry);
		} break;

		case SVViewState::PM_VertexList:
			showPropertyWidget<SVPropVertexListWidget>(M_VertexListWidget);
			setMinimumWidth(500);
		break;

		case SVViewState::PM_SiteProperties :
			showPropertyWidget<SVPropSiteWidget>(M_SiteProperties);
		break;

		case SVViewState::PM_BuildingProperties : {
			// select highlighting/edit mode -> this will send a signal to update the scene's geometry coloring
			BuildingPropertyTypes buildingPropertyType = m_propModeSelectionWidget->currentBuildingPropertyType();
			if (buildingPropertyType == BT_FloorManager) {
				showPropertyWidget<SVPropFloorManagerWidget>(M_FloorManager);
			}
			else {
				showPropertyWidget<SVPropBuildingEditWidget>(M_BuildingProperties);
				qobject_cast<SVPropBuildingEditWidget*>(m_propWidgets[M_BuildingProperties])->setPropertyType(buildingPropertyType);
			}
		} break;

		case SVViewState::PM_NetworkProperties : {
			showPropertyWidget<SVPropNetworkEditWidget>(M_NetworkProperties);
			// select highlighting/edit mode -> this will send a signal to update the scene's geometry coloring
			int networkPropertyType = m_propModeSelectionWidget->currentNetworkPropertyType();
			qobject_cast<SVPropNetworkEditWidget*>(m_propWidgets[M_NetworkProperties])->setPropertyMode(networkPropertyType);
		} break;
	}
}


