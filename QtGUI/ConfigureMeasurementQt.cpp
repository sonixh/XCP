#include "ConfigureMeasurementQt.h"
#include <XCPMaster.h>

using ModeFieldBits = SetDaqListModePacket::ModeFieldBits;

ConfigureMeasurementQt::ConfigureMeasurementQt(QWidget* parent) : QDialog(parent)
{
	ui.setupUi(this);
	ui.treeWidget->setColumnCount(1);
	connect(ui.AddDAQBtn, SIGNAL(clicked()), this, SLOT(AddDaqBtnClicked()));
	connect(ui.AddODTBtn, SIGNAL(clicked()), this, SLOT(AddOdtBtnClicked()));
	connect(ui.AddEntryBtn, SIGNAL(clicked()), this, SLOT(AddEntryBtnClicked()));
	connect(ui.setDAQBtn, SIGNAL(clicked()), this, SLOT(SetDAQSettingsClicked()));
	connect(ui.setEntryBtn, SIGNAL(clicked()), this, SLOT(SetEntrySettingsClicked()));
	connect(ui.colorBtn, SIGNAL(clicked()), this, SLOT(ColorPickerButtonClicked()));
	//connect(ui.treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(ItemClicked(QTreeWidgetItem*, int)));
	connect(ui.treeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(ItemSelected()));
	SelectedDAQ = nullptr;
	SelectedODT = nullptr;
	SelectedEntry = nullptr;

	/*DAQLayout daqlayout;
	DAQ daq0;
	DAQ daq1;
	ODT daq0odt0;
	ODT daq1odt0;
	ODTEntry daq0odt0entry0(0x21A1BD, 0, 1);
	ODTEntry daq1odt0entry0(0x21A08D, 0, 1);

	daq0odt0.AddEntry(daq0odt0entry0);
	daq1odt0.AddEntry(daq1odt0entry0);
	daq0.AddODT(daq0odt0);
	daq1.AddODT(daq1odt0);
	daq0.SetEventChannel(1);
	daq0.SetMode(ModeFieldBits::TIMESTAMP);
	daq0.SetPrescaler(1);
	daq0.SetPriority(1);
	daq1.SetEventChannel(2);
	daq1.SetMode(ModeFieldBits::TIMESTAMP);
	daq1.SetPrescaler(1);
	daq1.SetPriority(2);
	daqlayout.AddDAQ(daq0);
	daqlayout.AddDAQ(daq1);
	daqlayout.SetInitialized(true);
	this->daq_layout = daqlayout;*/
}


ConfigureMeasurementQt::~ConfigureMeasurementQt()
{
}

const DAQLayout & ConfigureMeasurementQt::GetDaqLayout()
{
	return daq_layout;
}

void ConfigureMeasurementQt::AddDaqBtnClicked()
{
	AddDAQToTree();
}

void ConfigureMeasurementQt::AddOdtBtnClicked()
{
	if (SelectedDAQ)
	{
		AddODTToDAQ(SelectedDAQ);
	}
}

void ConfigureMeasurementQt::AddEntryBtnClicked()
{
	if (SelectedODT)
	{
		AddEntryToODT(SelectedODT);
	}
}

void ConfigureMeasurementQt::ItemClicked(QTreeWidgetItem *item, int column)
{
	if (item->parent())
	{
		printf("%d", item->parent()->indexOfChild(item));
	}
	else
	{
		printf("%d", ui.treeWidget->indexOfTopLevelItem(item));
	}

	if (item->parent() == 0) //DAQ
	{
		SelectedDAQ = item;
		SelectedODT = nullptr;
		SelectedEntry = nullptr;
		SelectedDAQId = ui.treeWidget->indexOfTopLevelItem(item);

		DAQ d = daq_layout.GetDAQ(SelectedDAQId);
		ui.timestampBox->setChecked(d.GetMode()&ModeFieldBits::TIMESTAMP);
		ui.pidBox->setChecked(d.GetMode()&ModeFieldBits::PID_OFF);
		ui.ctrBox->setChecked(d.GetMode()&ModeFieldBits::DTO_CTR);
		ui.alternatingBox->setChecked(d.GetMode()&ModeFieldBits::ALTERNATING);
		ui.eventBox->setValue(d.GetEventChannel());
		ui.priorityBox->setValue(d.GetPriority());
		ui.prescalerBox->setValue(d.GetPrescaler());

		ui.DAQbutton->setChecked(!(d.GetMode()&ModeFieldBits::DIRECTION));
		ui.stimButton->setChecked((d.GetMode()&ModeFieldBits::DIRECTION));
	}
	else if (item->parent() && item->parent()->parent() == 0) //ODT
	{
		SelectedDAQ = item->parent();
		SelectedDAQId = ui.treeWidget->indexOfTopLevelItem(item->parent());
		SelectedODT = item;
		SelectedEntry = nullptr;
		SelectedODTId = item->parent()->indexOfChild(item);
	}
	else
	{
		SelectedEntry = item;
		SelectedEntryId= item->parent()->indexOfChild(item);
		SelectedODT = item->parent();
		SelectedODTId = item->parent()->parent()->indexOfChild(item->parent());
		SelectedDAQ = item->parent()->parent();
		SelectedDAQId = ui.treeWidget->indexOfTopLevelItem(item->parent()->parent());
		ODTEntry e = daq_layout.GetDAQ(SelectedDAQId).GetOdt(SelectedODTId).GetEntry(SelectedEntryId);
		ui.addressInput->setText(QString::number(e.GetAddress(),16));
		ui.addressExtInput->setText(QString::number(e.GetAddressExtension(), 16));
		SeriesProperties p;
		if (ChartSeries.find({ SelectedDAQId, SelectedODTId, SelectedEntryId }) != ChartSeries.end())
		{
			p = ChartSeries[{SelectedDAQId, SelectedODTId, SelectedEntryId}];
		}	
		QColor c(p.r,p.g,p.b);
		ui.colorBtn->setAutoFillBackground(true);
		QPalette pal;
		pal.setColor(QPalette::Button, c);
		ui.colorBtn->setPalette(pal);
		ui.colorBtn->setFlat(true);
		ui.colorBtn->update();
		ui.typeInput->setCurrentIndex(e.GetDataType());
//		ui.lengthInput->setText(QString::number(e.GetLength()));
	}
}

void ConfigureMeasurementQt::ItemSelected()
{
	ItemClicked(ui.treeWidget->selectedItems()[0],0);
}

void ConfigureMeasurementQt::SetDAQSettingsClicked()
{
	if (SelectedDAQ)
	{
		DAQ& d = daq_layout.GetDAQ(SelectedDAQId);
		d.SetEventChannel(ui.eventBox->value());
		d.SetPriority(ui.priorityBox->value());
		d.SetPrescaler(ui.prescalerBox->value());
		uint8_t Mode = 0;
		if (ui.pidBox->isChecked())
		{
			Mode |= ModeFieldBits::PID_OFF;
		}
		if (ui.timestampBox->isChecked())
		{
			Mode |= ModeFieldBits::TIMESTAMP;
		}
		if (ui.ctrBox->isChecked())
		{
			Mode |= ModeFieldBits::DTO_CTR;
		}
		if (ui.alternatingBox->isChecked())
		{
			Mode |= ModeFieldBits::ALTERNATING;
		}
		if (ui.stimButton->isChecked())
		{
			Mode |= ModeFieldBits::DIRECTION; //Sets STIM direction, otherwise it stays at 0 (DAQ)
		}
		d.SetMode(Mode);
	}
}

void ConfigureMeasurementQt::SetEntrySettingsClicked()
{
	if (SelectedEntry)
	{
		ODTEntry& e = daq_layout.GetDAQ(SelectedDAQId).GetOdt(SelectedODTId).GetEntry(SelectedEntryId);
		bool ok;
		uint32_t address = ui.addressInput->text().toUInt(&ok,16);
		e.SetAddress(address);
		uint8_t addressExt = ui.addressExtInput->text().toUInt(&ok, 16);
		e.SetAddressExtension(addressExt);
		e.SetDataType(ui.typeInput->currentIndex());
		//uint8_t length = 1;//ui.lengthInput->text().toUInt(&ok, 16);
		//e.SetLength(length);
	}
}

void ConfigureMeasurementQt::ColorPickerButtonClicked()
{
	ColorDialog = new QColorDialog(this);
	ColorDialog->exec();
	SeriesProperties p;
	QColor c = ColorDialog->selectedColor();
	if (ChartSeries.find({ SelectedDAQId, SelectedODTId, SelectedEntryId }) == ChartSeries.end())
	{
		p.SeriesIndex = ChartSeries.size();
	}
	else
	{
		p = ChartSeries[{ SelectedDAQId, SelectedODTId, SelectedEntryId }];
	}
	p.r = c.red();
	p.g = c.green();
	p.b = c.blue();
	ChartSeries[{SelectedDAQId, SelectedODTId, SelectedEntryId}] = p;
	ui.colorBtn->setAutoFillBackground(true);
	QPalette pal;
	pal.setColor(QPalette::Button, c);
	ui.colorBtn->setPalette(pal);
	ui.colorBtn->setFlat(true);
	ui.colorBtn->update();
}

void ConfigureMeasurementQt::AddDAQToTree()
{
	QTreeWidgetItem* item = new QTreeWidgetItem(ui.treeWidget);
	static int i = 0;
	item->setText(0, QString("DAQ ")+QString::number(i++));
	DAQ d;
	d.SetPriority(0);
	d.SetEventChannel(1);
	d.SetMode(ModeFieldBits::TIMESTAMP);
	d.SetPrescaler(1);
	daq_layout.AddDAQ(d);
}

void ConfigureMeasurementQt::AddODTToDAQ(QTreeWidgetItem * parent)
{
	QTreeWidgetItem *treeItem = new QTreeWidgetItem();
	int i = parent->childCount();
	treeItem->setText(0, QString("ODT ") + QString::number(i));
	parent->addChild(treeItem);
	parent->setExpanded(true);

	ODT o;
	daq_layout.GetDAQ(SelectedDAQId).AddODT(o);
}

void ConfigureMeasurementQt::AddEntryToODT(QTreeWidgetItem * parent)
{
	QTreeWidgetItem *treeItem = new QTreeWidgetItem();
	int i = parent->childCount();
	treeItem->setText(0, QString("Entry ") + QString::number(i));
	parent->addChild(treeItem);
	parent->setExpanded(true);

	ODTEntry e;
	e.SetLength(1);
	daq_layout.GetDAQ(SelectedDAQId).GetOdt(SelectedODTId).AddEntry(e);
}

const std::map<std::tuple<uint16_t, uint8_t, uint32_t>, SeriesProperties>& ConfigureMeasurementQt::GetChartSeries()
{
	return this->ChartSeries;
}

void ConfigureMeasurementQt::reject()
{
	QDialog::reject();
}

void ConfigureMeasurementQt::accept()
{
	this->daq_layout.SetInitialized(true);
	QDialog::accept();
}
