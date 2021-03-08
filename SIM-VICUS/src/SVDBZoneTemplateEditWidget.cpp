#include "SVDBZoneTemplateEditWidget.h"
#include "ui_SVDBZoneTemplateEditWidget.h"

#include <NANDRAD_KeywordListQt.h>
#include <NANDRAD_KeywordList.h>

#include <VICUS_KeywordList.h>

#include <QtExt_LanguageHandler.h>
#include <QtExt_Conversions.h>

#include "SVDBZoneTemplateTreeModel.h"
#include "SVConstants.h"
#include "SVSettings.h"
#include "SVMainWindow.h"
#include "SVDatabaseEditDialog.h"

SVDBZoneTemplateEditWidget::SVDBZoneTemplateEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVDBZoneTemplateEditWidget)
{
	m_ui->setupUi(this);
	m_ui->verticalLayout->setMargin(4);
	m_ui->verticalLayoutWidget->setMargin(0);

	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), "fr", true);
	m_ui->lineEditName->setDialog3Caption("Boundary condition identification name");

	// initial state is "nothing selected"
	updateInput(-1, -1, 0);
}


SVDBZoneTemplateEditWidget::~SVDBZoneTemplateEditWidget() {
	delete m_ui;
}


void SVDBZoneTemplateEditWidget::setup(SVDatabase * db, SVDBZoneTemplateTreeModel * dbModel) {
	m_db = db;
	m_dbModel = dbModel;
}


void SVDBZoneTemplateEditWidget::updateInput(int id, int subTemplateId, int subTemplateType) {
	m_current = nullptr; // disable edit triggers

	bool isEnabled = id == -1 ? false : true;

	m_currentSubTemplateType = subTemplateType;

	m_ui->lineEditName->setEnabled(isEnabled);
	m_ui->pushButtonColor->setEnabled(isEnabled);


	if (!isEnabled) {
		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->lineEditSubComponent->setText(QString());
		// hide widget with references
		m_ui->widget->setVisible(false);
		return;
	}

	VICUS::ZoneTemplate * item = const_cast<VICUS::ZoneTemplate*>(m_db->m_zoneTemplates[(unsigned int)id]);
	m_current = item;

	// now update the GUI controls
	m_ui->lineEditName->setString(m_current->m_displayName);

	m_ui->pushButtonColor->blockSignals(true);
	m_ui->pushButtonColor->setColor(m_current->m_color);
	m_ui->pushButtonColor->blockSignals(false);

	// for built-ins, disable editing/make read-only
	bool isEditable = true;
	if (m_current->m_builtIn)
		isEditable = false;

	m_ui->lineEditName->setReadOnly(!isEditable);
	m_ui->pushButtonColor->setReadOnly(!isEditable);
	m_ui->pushButtonAddSubTemplate->setEnabled(isEditable);

	// now the sub-template stuff
	if (subTemplateId == -1) {
		// hide widget with references
		m_ui->widget->setVisible(false);
		return;
	}
	// show widget with references
	m_ui->widget->setVisible(true);

	// determine which sub-template was selected
	switch ((VICUS::ZoneTemplate::SubTemplateType)subTemplateType) {
		case VICUS::ZoneTemplate::ST_IntLoadPerson: {
			m_ui->labelSubTemplate->setText(tr("Internal Loads - Person loads:"));
			// lookup corresponding dataset entry in database
			const VICUS::InternalLoad * iload = m_db->m_internalLoads[(unsigned int)subTemplateId];
			if (iload == nullptr) {
				m_ui->lineEditSubComponent->setText(tr("<select>"));
			}
			else {
				m_ui->lineEditSubComponent->setText( QtExt::MultiLangString2QString(iload->m_displayName) );
			}
		} break;
		case VICUS::ZoneTemplate::ST_IntLoadEquipment:
		break;
		case VICUS::ZoneTemplate::ST_IntLoadLighting:
		break;
		case VICUS::ZoneTemplate::ST_IntLoadOther:
		break;
		case VICUS::ZoneTemplate::ST_ControlThermostat:
		break;

		case VICUS::ZoneTemplate::NUM_ST:
		break;
	}

}


void SVDBZoneTemplateEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_displayName != m_ui->lineEditName->string()) {
		m_current->m_displayName = m_ui->lineEditName->string();
		m_db->m_zoneTemplates.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
}


void SVDBZoneTemplateEditWidget::on_pushButtonColor_colorChanged() {
	Q_ASSERT(m_current != nullptr);

	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		m_db->m_zoneTemplates.m_modified = true;
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
}



void SVDBZoneTemplateEditWidget::on_toolButtonSelectSubComponent_clicked() {

}


void SVDBZoneTemplateEditWidget::on_toolButtonRemoveSubComponent_clicked() {
	switch ((VICUS::ZoneTemplate::SubTemplateType)m_currentSubTemplateType) {
		case VICUS::ZoneTemplate::ST_IntLoadPerson:
			// remove the zone template association
			m_current->m_idReferences[VICUS::ZoneTemplate::ST_IntLoadPerson] = VICUS::INVALID_ID;
			m_db->m_zoneTemplates.m_modified = true;
			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
			emit selectSubTemplate(m_current->m_id, -1);
		break;

		// TODO Dirk: implement below

		case VICUS::ZoneTemplate::ST_IntLoadEquipment:
		case VICUS::ZoneTemplate::ST_IntLoadLighting:
		case VICUS::ZoneTemplate::ST_IntLoadOther:
		case VICUS::ZoneTemplate::ST_ControlThermostat:
		case VICUS::ZoneTemplate::NUM_ST:
		break;
	}
}


void SVDBZoneTemplateEditWidget::on_pushButtonAddSubTemplate_clicked() {
	Q_ASSERT(m_current != nullptr);
	// TODO Dirk: ask user to select which sub-template to edit
	// for now, just open the Internal Loads DB dialog and let user select one

	unsigned int id = SVMainWindow::instance().dbInternalLoadsPersonEditDialog()->select(VICUS::INVALID_ID);
	if (id == VICUS::INVALID_ID) return;
	if (m_current->m_idReferences[VICUS::ZoneTemplate::ST_IntLoadPerson] != id) {
		if (m_current->m_idReferences[VICUS::ZoneTemplate::ST_IntLoadPerson] == VICUS::INVALID_ID) {
			// add new child
			m_dbModel->addChildItem( m_dbModel->indexById(m_current->m_id), VICUS::ZoneTemplate::ST_IntLoadPerson, id);
		}
		else {
			// modify existing
			m_current->m_idReferences[VICUS::ZoneTemplate::ST_IntLoadPerson] = id;
			m_db->m_zoneTemplates.m_modified = true;
			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
			emit selectSubTemplate(m_current->m_id, (int)VICUS::ZoneTemplate::ST_IntLoadPerson);
		}
	}
}
