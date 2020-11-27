#ifndef SVNETWORKEDITDIALOG_H
#define SVNETWORKEDITDIALOG_H

#include <QDialog>
#include <QMap>

#include "VICUS_Network.h"

namespace Ui {
class SVNetworkEditDialog;
}

class SVNetworkEditDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SVNetworkEditDialog(QWidget *parent = nullptr);
	~SVNetworkEditDialog();

	void edit();

private slots:
	void on_pushButtonGenerateIntersections_clicked();

	void on_comboBoxSelectNetwork_activated(const QString &arg1);

private:
	Ui::SVNetworkEditDialog *m_ui;

	void updateStatus();

	void setNetwork();

	QMap<QString, unsigned> m_existingNetworksMap;

	VICUS::Network m_network;
};

#endif // SVNETWORKEDITDIALOG_H