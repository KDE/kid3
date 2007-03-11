/**
 * \file exportdialog.cpp
 * Export dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 May 2006
 */

#include "exportdialog.h"
#include "config.h"
#ifdef CONFIG_USE_KDE
#include <kconfig.h>
#include <kfiledialog.h>
#else
#include <qfiledialog.h>
#endif

#include "taggedfile.h"
#include "genres.h"
#include "kid3.h"
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qstring.h>
#include <qtextedit.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qdir.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qurl.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <Q3GroupBox>
#include <QTextStream>
#include <QHBoxLayout>
#include <QVBoxLayout>
#else
#include <qgroupbox.h>
#endif

/**
 * Constructor.
 *
 * @param parent        parent widget
 */
ExportDialog::ExportDialog(QWidget* parent) :
	QDialog(parent, "export", true)
{
	setCaption(i18n("Export"));

	QVBoxLayout* vlayout = new QVBoxLayout(this, 6, 6, "vlayout");
	if (vlayout) {
		m_edit = new QTextEdit(this);
		if (m_edit) {
			m_edit->setTextFormat(Qt::PlainText);
			vlayout->addWidget(m_edit);
		}

		Q3GroupBox* fmtbox = new Q3GroupBox(4, Qt::Vertical, i18n("&Format"), this);
		if (fmtbox) {
#if QT_VERSION >= 0x040000
			fmtbox->setInsideMargin(5);
#endif
			m_formatComboBox = new QComboBox(false, fmtbox, "formatComboBox");
			m_formatComboBox->setEditable(true);
			m_headerLineEdit = new QLineEdit(fmtbox);
			m_trackLineEdit = new QLineEdit(fmtbox);
			m_trailerLineEdit = new QLineEdit(fmtbox);
			vlayout->addWidget(fmtbox);
			connect(m_formatComboBox, SIGNAL(activated(int)), this,
							SLOT(setFormatLineEdit(int)));
			connect(m_headerLineEdit, SIGNAL(returnPressed()), this, SLOT(showPreview()));
			connect(m_trackLineEdit, SIGNAL(returnPressed()), this, SLOT(showPreview()));
			connect(m_trailerLineEdit, SIGNAL(returnPressed()), this, SLOT(showPreview()));
		}

		QHBoxLayout* butlayout = new QHBoxLayout(vlayout, 6, "butlayout");
		if (butlayout) {
			m_fileButton = new QPushButton(i18n("To F&ile"), this);
			if (m_fileButton) {
				m_fileButton->setAutoDefault(false);
				butlayout->addWidget(m_fileButton);
				connect(m_fileButton, SIGNAL(clicked()), this, SLOT(slotToFile()));
			}
			m_clipButton = new QPushButton(i18n("To Clip&board"), this);
			if (m_clipButton) {
				m_clipButton->setAutoDefault(false);
				butlayout->addWidget(m_clipButton);
				connect(m_clipButton, SIGNAL(clicked()), this, SLOT(slotToClipboard()));
			}
			QSpacerItem* butspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
																							 QSizePolicy::Minimum);
			butlayout->addItem(butspacer);

			QLabel* srcLabel = new QLabel(i18n("&Source:"), this, "srcLabel");
			butlayout->addWidget(srcLabel);
			m_srcComboBox = new QComboBox(false, this, "srcComboBox");
			if (m_srcComboBox) {
				m_srcComboBox->insertItem(i18n("Tag 1"), SrcV1);
				m_srcComboBox->insertItem(i18n("Tag 2"), SrcV2);
				srcLabel->setBuddy(m_srcComboBox);
				butlayout->addWidget(m_srcComboBox);
				connect(m_srcComboBox, SIGNAL(activated(int)),
								this, SIGNAL(exportDataRequested(int)));
			}
		}

		QHBoxLayout* hlayout = new QHBoxLayout(vlayout, 6, "hlayout");
		if (hlayout) {
			QPushButton* helpButton = new QPushButton(i18n("&Help"), this);
			if (helpButton) {
				helpButton->setAutoDefault(false);
				hlayout->addWidget(helpButton);
				connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
			}
			QPushButton* saveButton = new QPushButton(i18n("&Save Settings"), this);
			if (saveButton) {
				saveButton->setAutoDefault(false);
				hlayout->addWidget(saveButton);
				connect(saveButton, SIGNAL(clicked()), this, SLOT(saveConfig()));
			}
			QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
																						 QSizePolicy::Minimum);
			hlayout->addItem(hspacer);

			QPushButton* closeButton = new QPushButton(i18n("&Close"), this);
			if (closeButton) {
				closeButton->setAutoDefault(false);
				hlayout->addWidget(closeButton);
				connect(closeButton, SIGNAL(clicked()), this, SLOT(accept()));
			}
		}
	}
}

/**
 * Destructor.
 */
ExportDialog::~ExportDialog()
{}

/**
 * Export to a file.
 */
void ExportDialog::slotToFile()
{
	QString fn =
#ifdef CONFIG_USE_KDE
		KFileDialog::getSaveFileName(QString::null, QString::null, this);
#else
		QFileDialog::getSaveFileName(QString::null, QString::null, this);
#endif
	if (!fn.isEmpty()) {
		QFile file(fn);
		if (file.open(QCM_WriteOnly)) {
			QTextStream stream(&file);
			stream.setEncoding(QTextStream::Locale);
			stream << m_edit->text();
			file.close();
		}
	}
}

/**
 * Export to clipboard.
 */
void ExportDialog::slotToClipboard()
{
	QApplication::clipboard()->setText(
		m_edit->text()
#if QT_VERSION >= 0x030100
		, QClipboard::Clipboard
#endif
		);
}

/**
 * Set the format lineedits to the format selected in the combo box.
 *
 * @param index current index of the combo box
 */
void ExportDialog::setFormatLineEdit(int index)
{
	if (index < static_cast<int>(m_formatHeaders.size())) {
		m_headerLineEdit->setText(m_formatHeaders[index]);
		m_trackLineEdit->setText(m_formatTracks[index]);
		m_trailerLineEdit->setText(m_formatTrailers[index]);
		showPreview();
	}
}

/**
 * Format a string from track data.
 * Supported format fields:
 * Those supported by ImportTrackData::formatString()
 * %n total number of tracks
 *
 * @param trackData track data
 * @param format    format specification
 * @param numTracks total number of tracks
 *
 * @return formatted string.
 */
static QString trackDataToString(
	const ImportTrackData& trackData, const QString& format, int numTracks)
{
	QString fmt = trackData.formatString(format);
	if (!fmt.isEmpty()) {
		const int numTagCodes = 1;
		const QChar tagCode[numTagCodes] = { 'n' };
		QString tagStr[numTagCodes];
		tagStr[0] = QString::number(numTracks);

		fmt = StandardTags::replacePercentCodes(
			fmt, tagCode, tagStr, numTagCodes);
	}
	return fmt;
}

/**
 * Show exported text as preview in editor.
 */
void ExportDialog::showPreview()
{
	m_edit->clear();
	unsigned numTracks = m_trackDataVector.size();
	unsigned trackNr = 0;
	QString headerFormat(m_headerLineEdit->text());
	QString trackFormat(m_trackLineEdit->text());
	QString trailerFormat(m_trailerLineEdit->text());
	for (ImportTrackDataVector::const_iterator it = m_trackDataVector.begin();
			 it != m_trackDataVector.end();
			 ++it) {
		if (trackNr == 0 && !headerFormat.isEmpty()) {
			m_edit->append(trackDataToString(*it, headerFormat, numTracks));
		}
		if (!trackFormat.isEmpty()) {
			m_edit->append(trackDataToString(*it, trackFormat, numTracks));
		}
		if (trackNr == numTracks - 1 && !trailerFormat.isEmpty()) {
			m_edit->append(trackDataToString(*it, trailerFormat, numTracks));
		}
		++trackNr;
	}
	m_edit->append(""); // terminate last line
}

/**
 * Set data to be exported.
 *
 * @param trackDataVector data to export
 */
void ExportDialog::setExportData(const ImportTrackDataVector& trackDataVector)
{
	m_trackDataVector = trackDataVector;
	showPreview();
}

/**
 * Read the local settings from the configuration.
 */
void ExportDialog::readConfig()
{
	m_srcComboBox->setCurrentItem(Kid3App::s_genCfg.m_exportSrcV1 ? SrcV1 : SrcV2);

	m_formatHeaders = Kid3App::s_genCfg.m_exportFormatHeaders;
	m_formatTracks = Kid3App::s_genCfg.m_exportFormatTracks;
	m_formatTrailers = Kid3App::s_genCfg.m_exportFormatTrailers;
	m_formatComboBox->clear();
	m_formatComboBox->insertStringList(Kid3App::s_genCfg.m_exportFormatNames);
	m_formatComboBox->setCurrentItem(Kid3App::s_genCfg.m_exportFormatIdx);
	setFormatLineEdit(Kid3App::s_genCfg.m_exportFormatIdx);

	if (Kid3App::s_genCfg.m_exportWindowWidth > 0 &&
			Kid3App::s_genCfg.m_exportWindowHeight > 0) {
		resize(Kid3App::s_genCfg.m_exportWindowWidth,
					 Kid3App::s_genCfg.m_exportWindowHeight);
	}
}

/**
 * Save the local settings to the configuration.
 */
void ExportDialog::saveConfig()
{
	Kid3App::s_genCfg.m_exportSrcV1 = (m_srcComboBox->currentItem() == SrcV1);

	Kid3App::s_genCfg.m_exportFormatIdx = m_formatComboBox->currentItem();
	Kid3App::s_genCfg.m_exportFormatNames[Kid3App::s_genCfg.m_exportFormatIdx] = m_formatComboBox->currentText();
	Kid3App::s_genCfg.m_exportFormatHeaders[Kid3App::s_genCfg.m_exportFormatIdx] = m_headerLineEdit->text();
	Kid3App::s_genCfg.m_exportFormatTracks[Kid3App::s_genCfg.m_exportFormatIdx] = m_trackLineEdit->text();
	Kid3App::s_genCfg.m_exportFormatTrailers[Kid3App::s_genCfg.m_exportFormatIdx] = m_trailerLineEdit->text();

	Kid3App::s_genCfg.m_exportWindowWidth = size().width();
	Kid3App::s_genCfg.m_exportWindowHeight = size().height();
}

/**
 * Show help.
 */
void ExportDialog::showHelp()
{
	Kid3App::displayHelp("export");
}
