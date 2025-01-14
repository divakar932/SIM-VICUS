#include "SVDBZoneControlVentilationNaturalEditWidget.h"
#include "ui_SVDBZoneControlVentilationNaturalEditWidget.h"

#include <VICUS_KeywordListQt.h>
#include <VICUS_Schedule.h>

#include <QtExt_Conversions.h>
#include <QtExt_LanguageHandler.h>

#include "SVDBZoneControlVentilationNaturalTableModel.h"
#include "SVMainWindow.h"
#include "SVConstants.h"
#include "SVDatabaseEditDialog.h"

SVDBZoneControlVentilationNaturalEditWidget::SVDBZoneControlVentilationNaturalEditWidget(QWidget *parent) :
	SVAbstractDatabaseEditWidget(parent),
	m_ui(new Ui::SVDBZoneControlVentilationNaturalEditWidget)
{
	m_ui->setupUi(this);
	m_ui->gridLayoutMaster->setMargin(4);


	m_ui->lineEditName->initLanguages(QtExt::LanguageHandler::instance().langId().toStdString(), THIRD_LANGUAGE, true);
	m_ui->lineEditName->setDialog3Caption(tr("Zone control VentilationNatural model name"));

	m_ui->lineEditTemperatureAirRoomMaximum->setup(-100, 100, tr("Maximum room air temperature above which ventilation stops."), true, true);
	m_ui->lineEditTemperatureAirRoomMinimum->setup(-100, 100, tr("Minimum room air temperature below which ventilation stops."), true, true);
	m_ui->lineEditTemperatureAirOutsideMaximum->setup(-100, 100, tr("Maximum outside air temperature above which ventilation stops."), true, true);
	m_ui->lineEditTemperatureAirOutsideMinimum->setup(-100, 100, tr("Minimum outside air temperature below which ventilation stops."), true, true);
	m_ui->lineEditTemperatureDifference->setup(-100, 100, tr("Temperature Difference of Room - Outside. Is Difference lower ventilation stops."), true, true);
	m_ui->lineEditWindSpeedMax->setup(0, 40, tr("Maximum wind speed. Values above stops ventilation."), true, true);

	// initial state is "nothing selected"
	updateInput(-1);
}


SVDBZoneControlVentilationNaturalEditWidget::~SVDBZoneControlVentilationNaturalEditWidget() {
	delete m_ui;
}


void SVDBZoneControlVentilationNaturalEditWidget::setup(SVDatabase * db, SVAbstractDatabaseTableModel * dbModel) {
	m_db = db;
	m_dbModel = dynamic_cast<SVDBZoneControlVentilationNaturalTableModel*>(dbModel);
}


void SVDBZoneControlVentilationNaturalEditWidget::updateInput(int id) {
	m_current = nullptr; // disable edit triggers

	m_ui->labelTemperatureAirRoomMaximum->setText(tr("Maximum room air Temperature:"));
	m_ui->labelTemperatureAirRoomMinimum->setText(tr("Minimum room air Temperature:"));
	m_ui->labelTemperatureAirOutsideMaximum->setText(tr("Maximum outside air Temperature:"));
	m_ui->labelTemperatureAirOutsideMinimum->setText(tr("Minimum outside air Temperature:"));
	m_ui->labelTemperatureDifference->setText(tr("Temperature difference (in - out):"));
	m_ui->labelWindSpeedMax->setText(tr("Maximum wind speed:"));

	if (id == -1) {
		// clear input controls
		m_ui->lineEditName->setString(IBK::MultiLanguageString());
		m_ui->lineEditTemperatureAirOutsideMaximum->setText("");
		m_ui->lineEditTemperatureAirOutsideMinimum->setText("");
		m_ui->lineEditTemperatureAirRoomMaximum->setText("");
		m_ui->lineEditTemperatureAirRoomMinimum->setText("");
		m_ui->lineEditTemperatureDifference->setText("");
		m_ui->lineEditWindSpeedMax->setText("");
		return;
	}

	m_current = const_cast<VICUS::ZoneControlNaturalVentilation *>(m_db->m_zoneControlVentilationNatural[(unsigned int) id ]);

	// we must have a valid internal load model pointer
	Q_ASSERT(m_current != nullptr);

	m_ui->lineEditName->setString(m_current->m_displayName);
	m_ui->pushButtonColor->setColor(m_current->m_color);
	try {
		m_ui->lineEditTemperatureAirOutsideMaximum->setValue(m_current->m_para[VICUS::ZoneControlNaturalVentilation::ST_TemperatureOutsideMax].get_value("C"));
	}  catch (IBK::Exception &ex) {
		m_ui->lineEditTemperatureAirOutsideMaximum->setValue(0);
	}
	try {
		m_ui->lineEditTemperatureAirOutsideMinimum->setValue(m_current->m_para[VICUS::ZoneControlNaturalVentilation::ST_TemperatureOutsideMin].get_value("C"));
	}  catch (IBK::Exception &ex) {
		m_ui->lineEditTemperatureAirOutsideMinimum->setValue(0);
	}
	try {
		m_ui->lineEditTemperatureAirRoomMaximum->setValue(m_current->m_para[VICUS::ZoneControlNaturalVentilation::ST_TemperatureAirMax].get_value("C"));

	}  catch (IBK::Exception &ex) {
		m_ui->lineEditTemperatureAirRoomMaximum->setValue(0);
	}
	try {
		m_ui->lineEditTemperatureAirRoomMinimum->setValue(m_current->m_para[VICUS::ZoneControlNaturalVentilation::ST_TemperatureAirMin].get_value("C"));

	}  catch (IBK::Exception &ex) {
		m_ui->lineEditTemperatureAirRoomMinimum->setValue(0);
	}
	try {
		m_ui->lineEditTemperatureDifference->setValue(m_current->m_para[VICUS::ZoneControlNaturalVentilation::ST_TemperatureDifference].get_value("K"));

	}  catch (IBK::Exception &ex) {
		m_ui->lineEditTemperatureDifference->setValue(0);
	}
	try {
		m_ui->lineEditWindSpeedMax->setValue(m_current->m_para[VICUS::ZoneControlNaturalVentilation::ST_WindSpeedMax].get_value("m/s"));

	}  catch (IBK::Exception &ex) {
		m_ui->lineEditWindSpeedMax->setValue(0);
	}

//	VICUS::Schedule * sched = const_cast<VICUS::Schedule *>(m_db->m_schedules[(unsigned int) m_current->m_heatingSetpointScheduleId]);
//	if (sched != nullptr)
//		m_ui->lineEditHeatingScheduleName->setText(QtExt::MultiLangString2QString(sched->m_displayName));
//	else
//		m_ui->lineEditHeatingScheduleName->setText(tr("<select schedule>"));


	// for built-ins, disable editing/make read-only
	bool isbuiltIn = m_current->m_builtIn;
	m_ui->lineEditName->setReadOnly(isbuiltIn);
	m_ui->pushButtonColor->setReadOnly(isbuiltIn);
	m_ui->lineEditTemperatureAirOutsideMaximum->setEnabled(!isbuiltIn);
	m_ui->lineEditTemperatureAirOutsideMinimum->setEnabled(!isbuiltIn);
	m_ui->lineEditTemperatureAirRoomMaximum->setEnabled(!isbuiltIn);
	m_ui->lineEditTemperatureAirRoomMinimum->setEnabled(!isbuiltIn);
	m_ui->lineEditTemperatureDifference->setEnabled(!isbuiltIn);
	m_ui->lineEditWindSpeedMax->setEnabled(!isbuiltIn);
}


void SVDBZoneControlVentilationNaturalEditWidget::on_lineEditName_editingFinished() {
	Q_ASSERT(m_current != nullptr);
	if (m_current->m_displayName != m_ui->lineEditName->string()) {  // currentdisplayname is multilanguage string
		m_current->m_displayName = m_ui->lineEditName->string();
		modelModify();
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
}


void SVDBZoneControlVentilationNaturalEditWidget::on_lineEditTemperatureAirOutsideMaximum_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	//change this only:
	auto *lineEdit = m_ui->lineEditTemperatureAirOutsideMaximum;
	VICUS::ZoneControlNaturalVentilation::ScheduleType paraName = VICUS::ZoneControlNaturalVentilation::ST_TemperatureOutsideMax;
	std::string keywordList = "ZoneControlNaturalVentilation::ScheduleType";

	if(lineEdit->isValid()){
		double val = lineEdit->value();

		if (m_current->m_para[paraName].empty() ||
			val != m_current->m_para[paraName].value)
		{
			VICUS::KeywordList::setParameter(m_current->m_para, keywordList.c_str(), paraName, val);
			modelModify();
			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		}
	}
}

void SVDBZoneControlVentilationNaturalEditWidget::on_lineEditTemperatureAirOutsideMinimum_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	//change this only:
	auto *lineEdit = m_ui->lineEditTemperatureAirOutsideMinimum;
	VICUS::ZoneControlNaturalVentilation::ScheduleType paraName = VICUS::ZoneControlNaturalVentilation::ST_TemperatureOutsideMin;
	std::string keywordList = "ZoneControlNaturalVentilation::ScheduleType";

	if(lineEdit->isValid()){
		double val = lineEdit->value();

		if (m_current->m_para[paraName].empty() ||
				val != m_current->m_para[paraName].value)
		{
			VICUS::KeywordList::setParameter(m_current->m_para, keywordList.c_str(), paraName, val);
			modelModify();
			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		}
	}
}

void SVDBZoneControlVentilationNaturalEditWidget::on_lineEditTemperatureAirRoomMaximum_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	//change this only:
	auto *lineEdit = m_ui->lineEditTemperatureAirRoomMaximum;
	VICUS::ZoneControlNaturalVentilation::ScheduleType paraName = VICUS::ZoneControlNaturalVentilation::ST_TemperatureAirMax;
	std::string keywordList = "ZoneControlNaturalVentilation::ScheduleType";

	if(lineEdit->isValid()){
		double val = lineEdit->value();

		if (m_current->m_para[paraName].empty() ||
				val != m_current->m_para[paraName].value)
		{
			VICUS::KeywordList::setParameter(m_current->m_para, keywordList.c_str(), paraName, val);
			modelModify();
			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		}
	}
}

void SVDBZoneControlVentilationNaturalEditWidget::on_lineEditTemperatureAirRoomMinimum_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	//change this only:
	auto *lineEdit = m_ui->lineEditTemperatureAirRoomMaximum;
	VICUS::ZoneControlNaturalVentilation::ScheduleType paraName = VICUS::ZoneControlNaturalVentilation::ST_TemperatureAirMin;
	std::string keywordList = "ZoneControlNaturalVentilation::ScheduleType";

	if(lineEdit->isValid()){
		double val = lineEdit->value();

		if (m_current->m_para[paraName].empty() ||
				val != m_current->m_para[paraName].value)
		{
			VICUS::KeywordList::setParameter(m_current->m_para, keywordList.c_str(), paraName, val);
			modelModify();
			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		}
	}
}

void SVDBZoneControlVentilationNaturalEditWidget::on_lineEditTemperatureDifference_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	//change this only:
	auto *lineEdit = m_ui->lineEditTemperatureDifference;
	VICUS::ZoneControlNaturalVentilation::ScheduleType paraName = VICUS::ZoneControlNaturalVentilation::ST_TemperatureDifference;
	std::string keywordList = "ZoneControlNaturalVentilation::ScheduleType";

	if(lineEdit->isValid()){
		double val = lineEdit->value();

		if (m_current->m_para[paraName].empty() ||
				val != m_current->m_para[paraName].value)
		{
			VICUS::KeywordList::setParameter(m_current->m_para, keywordList.c_str(), paraName, val);
			modelModify();
			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		}
	}
}

void SVDBZoneControlVentilationNaturalEditWidget::on_lineEditWindSpeedMax_editingFinished() {
	Q_ASSERT(m_current != nullptr);

	//change this only:
	auto *lineEdit = m_ui->lineEditWindSpeedMax;
	VICUS::ZoneControlNaturalVentilation::ScheduleType paraName = VICUS::ZoneControlNaturalVentilation::ST_WindSpeedMax;
	std::string keywordList = "ZoneControlNaturalVentilation::ScheduleType";

	if(lineEdit->isValid()){
		double val = lineEdit->value();

		if (m_current->m_para[paraName].empty() ||
				val != m_current->m_para[paraName].value)
		{
			VICUS::KeywordList::setParameter(m_current->m_para, keywordList.c_str(), paraName, val);
			modelModify();
			m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
		}
	}
}



void SVDBZoneControlVentilationNaturalEditWidget::modelModify() {
	m_db->m_zoneControlVentilationNatural.m_modified = true;
}

void SVDBZoneControlVentilationNaturalEditWidget::on_pushButtonColor_colorChanged() {
	if (m_current->m_color != m_ui->pushButtonColor->color()) {
		m_current->m_color = m_ui->pushButtonColor->color();
		modelModify();
		m_dbModel->setItemModified(m_current->m_id); // tell model that we changed the data
	}
}



