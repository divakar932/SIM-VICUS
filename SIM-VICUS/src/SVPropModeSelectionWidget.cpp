#include "SVPropModeSelectionWidget.h"
#include "ui_SVPropModeSelectionWidget.h"

#include <VICUS_Project.h>

#include "SVProjectHandler.h"
#include "SVViewStateHandler.h"
#include "SVUndoModifySiteData.h"

SVPropModeSelectionWidget::SVPropModeSelectionWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropModeSelectionWidget)
{
	// make us known to the world
	SVViewStateHandler::instance().m_propModeSelectionWidget = this;

	m_ui->setupUi(this);
	m_ui->verticalLayout->setMargin(0);

	m_ui->comboBoxBuildingProperties->blockSignals(true);
	m_ui->comboBoxBuildingProperties->addItem(tr("Component"), BT_Components);
	m_ui->comboBoxBuildingProperties->addItem(tr("Construction orientation"), BT_ComponentOrientation);
	m_ui->comboBoxBuildingProperties->addItem(tr("Boundary conditions"), BT_BoundaryConditions);
	m_ui->comboBoxBuildingProperties->addItem(tr("Building levels"), BT_FloorManager);
	m_ui->comboBoxBuildingProperties->addItem(tr("Zone templates"), BT_ZoneTemplates);
	m_ui->comboBoxBuildingProperties->blockSignals(false);

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVPropModeSelectionWidget::onModified);

	// we are in "Site" mode initially, so the rest is hidden
	updateWidgetVisibility();
}


SVPropModeSelectionWidget::~SVPropModeSelectionWidget() {
	delete m_ui;
}


void SVPropModeSelectionWidget::setBuildingPropertyType(BuildingPropertyTypes pt) {
	m_ui->pushButtonSite->blockSignals(true);
	m_ui->pushButtonNetwork->blockSignals(true);
	m_ui->pushButtonBuilding->blockSignals(true);

	m_ui->pushButtonSite->setChecked(false);
	m_ui->pushButtonNetwork->setChecked(false);
	m_ui->pushButtonBuilding->setChecked(true);

	m_ui->pushButtonSite->blockSignals(false);
	m_ui->pushButtonNetwork->blockSignals(false);
	m_ui->pushButtonBuilding->blockSignals(false);

	m_ui->comboBoxBuildingProperties->blockSignals(true);
	m_ui->comboBoxBuildingProperties->setCurrentIndex( m_ui->comboBoxBuildingProperties->findData(pt) );
	m_ui->comboBoxBuildingProperties->blockSignals(false);
}


void SVPropModeSelectionWidget::onModified(int modificationType, ModificationInfo * ) {
	SVProjectHandler::ModificationTypes modType((SVProjectHandler::ModificationTypes)modificationType);
	switch (modType) {
		case SVProjectHandler::NetworkModified:
		case SVProjectHandler::AllModified:
		case SVProjectHandler::NodeStateModified:
			// Based on current selection, switch into a specific building/network edit mode. For example, when
			// "Network" mode is selected and a node is selected, we automatically switch to "Node" edit mode.
			selectionChanged();
			// Note: the call above may results in a view state change, when the selection causes
			//		 the network property selection to change.
			break;

		default: ; // just to make compiler happy
	}
}


void SVPropModeSelectionWidget::selectionChanged() {
	if (m_ui->pushButtonNetwork->isChecked()) {
		// now check the selected objects and if we have only nodes - go to node edit more,
		// if we have only edges - go to edge edit mode

		std::set<const VICUS::Object*> objs;
		project().selectObjects(objs, VICUS::Project::SG_Network, true, true);
//		bool haveNode = false;
//		bool haveNetwork = false;
//		blockSignals(true);
//		for (const VICUS::Object* o : objs) {
//			if (dynamic_cast<const VICUS::Network*>(o) != nullptr){
//				m_ui->comboBoxNetworkProperties->setCurrentIndex(0);
//				haveNetwork = true;
//				break;
//			}
//		}
//		if (!haveNetwork){
//			for (const VICUS::Object* o : objs) {
//				if (dynamic_cast<const VICUS::NetworkNode*>(o) != nullptr){
//					m_ui->comboBoxNetworkProperties->setCurrentIndex(1);
//					break;
//				}
//				if (dynamic_cast<const VICUS::NetworkEdge*>(o) != nullptr){
//					m_ui->comboBoxNetworkProperties->setCurrentIndex(2);
//					break;
//				}
//			}
//		}
//		blockSignals(false);
		updateViewState();
	}

	// TODO : what about building editing default mode?

}


BuildingPropertyTypes SVPropModeSelectionWidget::currentBuildingPropertyType() const {
	return (BuildingPropertyTypes)m_ui->comboBoxBuildingProperties->currentData().toInt();
}


int SVPropModeSelectionWidget::currentNetworkPropertyType() const {
	return m_ui->comboBoxNetworkProperties->currentIndex();
}


void SVPropModeSelectionWidget::viewStateProperties(SVViewState & vs) const {
	if (m_ui->pushButtonBuilding->isChecked())
		vs.m_propertyWidgetMode = SVViewState::PM_BuildingProperties;
	else if (m_ui->pushButtonNetwork->isChecked())
		vs.m_propertyWidgetMode = SVViewState::PM_NetworkProperties;
	else
		vs.m_propertyWidgetMode = SVViewState::PM_SiteProperties;

	// also set the scene coloring mode
	switch (vs.m_propertyWidgetMode) {
		case SVViewState::PM_SiteProperties:
			// clear scene coloring
			vs.m_objectColorMode = SVViewState::OCM_None;
		break;

		case SVViewState::PM_BuildingProperties:
			switch ((BuildingPropertyTypes)m_ui->comboBoxBuildingProperties->currentData().toInt()) {
				case BT_Components:				vs.m_objectColorMode = SVViewState::OCM_Components; break;
				case BT_ComponentOrientation:	vs.m_objectColorMode = SVViewState::OCM_ComponentOrientation; break;
				case BT_BoundaryConditions:		vs.m_objectColorMode = SVViewState::OCM_BoundaryConditions; break;
				case BT_FloorManager:			vs.m_objectColorMode = SVViewState::OCM_None; break;
				case BT_ZoneTemplates:			vs.m_objectColorMode = SVViewState::OCM_ZoneTemplates; break;
			}
		break;

		case SVViewState::PM_NetworkProperties:
			switch (m_ui->comboBoxNetworkProperties->currentIndex()) {
				// network
				case 0 : vs.m_objectColorMode = SVViewState::OCM_Network; break;

				// node: show node association
				case 1 : vs.m_objectColorMode = SVViewState::OCM_NetworkNode; break;

				// pipe : show pipe association
				case 2 : vs.m_objectColorMode = SVViewState::OCM_NetworkEdge; break;

				// component: show network component association
				case 3 : vs.m_objectColorMode = SVViewState::OCM_NetworkComponents; break;
			}
		break;

		default:; // just to make compiler happy
	}
}


void SVPropModeSelectionWidget::on_pushButtonBuilding_toggled(bool on) {
	// Note: this slot is also called when the button is turned off due to another
	//       button being turned on. But we don't want to change the view state twice,
	//       so we only handle the signal if the button is turned on
	if (!on) return;

	updateWidgetVisibility();

	// block signals in this widget, since we do not want to let selectionChanged() change the viewstate already
	blockSignals(true);
	selectionChanged();
	blockSignals(false);

	updateViewState(); // tell the world that our edit mode has changed
}


void SVPropModeSelectionWidget::on_pushButtonNetwork_toggled(bool on) {
	// Note: this slot is also called when the button is turned off due to another
	//       button being turned on. But we don't want to change the view state twice,
	//       so we only handle the signal if the button is turned on
	if (!on) return;

	updateWidgetVisibility();

	// block signals in this widget, since we do not want to let selectionChanged() change the viewstate already
	blockSignals(true);
	selectionChanged();
	blockSignals(false);

	updateViewState(); // tell the world that our edit mode has changed
}


void SVPropModeSelectionWidget::on_pushButtonSite_toggled(bool on) {
	// Note: this slot is also called when the button is turned off due to another
	//       button being turned on. But we don't want to change the view state twice,
	//       so we only handle the signal if the button is turned on
	if (!on) return;

	updateWidgetVisibility();

	updateViewState(); // tell the world that our edit mode has changed
}



void SVPropModeSelectionWidget::on_comboBoxNetworkProperties_currentIndexChanged(int) {
	updateViewState();
}


void SVPropModeSelectionWidget::on_comboBoxBuildingProperties_currentIndexChanged(int) {
	updateViewState();
}


void SVPropModeSelectionWidget::updateWidgetVisibility() {
	bool showBuildingProps = m_ui->pushButtonBuilding->isChecked();
	bool showNetworkProps = m_ui->pushButtonNetwork->isChecked();
	m_ui->labelNetworkProperties->setVisible(showNetworkProps);
	m_ui->comboBoxNetworkProperties->setVisible(showNetworkProps);

	m_ui->labelBuildingProperties->setVisible(showBuildingProps);
	m_ui->comboBoxBuildingProperties->setVisible(showBuildingProps);
}


void SVPropModeSelectionWidget::updateViewState() {
	// this function is only called, when indeed there was a change (one of the buttons was pressed,
	// user has selected a property in the combo box, property combo was changed due to selection change)

	SVViewState vs = SVViewStateHandler::instance().viewState();
	viewStateProperties(vs);

	// now set the new viewstate to update property widgets and scene coloring at the same time
	SVViewStateHandler::instance().setViewState(vs);
}
