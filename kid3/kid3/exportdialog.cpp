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
#include <klocale.h>
#include <kconfig.h>
#include <kfiledialog.h>
#else
#define i18n(s) tr(s)
#define I18N_NOOP(s) QT_TR_NOOP(s)
#include <qfiledialog.h>
#endif

#include "taggedfile.h"
#include "genres.h"
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qstring.h>
#include <qtextedit.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qdir.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qurl.h>

/**
 * Constructor.
 *
 * @param parent        parent widget
 */
ExportDialog::ExportDialog(QWidget* parent) :
	QDialog(parent, "export", true),
	m_windowWidth(-1), m_windowHeight(-1)
{
	setCaption(i18n("Export"));

	QVBoxLayout* vlayout = new QVBoxLayout(this, 6, 6, "vlayout");
	if (vlayout) {
		m_edit = new QTextEdit(this);
		if (m_edit) {
			m_edit->setTextFormat(Qt::PlainText);
			vlayout->addWidget(m_edit);
		}

		QGroupBox* fmtbox = new QGroupBox(4, Qt::Vertical, i18n("&Format"), this);
		if (fmtbox) {
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
				m_srcComboBox->insertItem("ID3v1", SrcV1);
				m_srcComboBox->insertItem("ID3v2", SrcV2);
				srcLabel->setBuddy(m_srcComboBox);
				butlayout->addWidget(m_srcComboBox);
				connect(m_srcComboBox, SIGNAL(activated(int)),
								this, SIGNAL(exportDataRequested(int)));
			}
		}

		QHBoxLayout* hlayout = new QHBoxLayout(vlayout, 6, "hlayout");
		if (hlayout) {
			QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
																						 QSizePolicy::Minimum);
			hlayout->addItem(hspacer);

			QPushButton* okButton = new QPushButton(i18n("&OK"), this);
			if (okButton) {
				okButton->setAutoDefault(false);
				hlayout->addWidget(okButton);
				connect(okButton, SIGNAL(clicked()), this, SLOT(saveWindowSizeAndClose()));
			}
			QPushButton* cancelButton = new QPushButton(i18n("&Cancel"), this);
			if (cancelButton) {
				cancelButton->setAutoDefault(false);
				hlayout->addWidget(cancelButton);
				connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
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
 * Set export format.
 *
 * @param names    export format names list
 * @param headers  export format headers
 * @param tracks   export format tracks
 * @param trailers export format trailers
 * @param idx      selected index
 */
void ExportDialog::setExportFormat(const QStringList& names,
								   const QStringList& headers,
								   const QStringList& tracks,
								   const QStringList& trailers,
								   int idx)
{
	m_formatHeaders = headers;
	m_formatTracks = tracks;
	m_formatTrailers = trailers;
	m_formatComboBox->clear();
	m_formatComboBox->insertStringList(names);
	m_formatComboBox->setCurrentItem(idx);
	setFormatLineEdit(idx);
}

/**
 * Get export format.
 *
 * @param name    export format name
 * @param header  export format header
 * @param track   export format track
 * @param trailer export format trailer
 *
 * @return index of current selection.
 */
int ExportDialog::getExportFormat(QString& name,
																	QString& header,
																	QString& track,
																	QString& trailer) const
{
	name = m_formatComboBox->currentText();
	header = m_headerLineEdit->text();
	track = m_trackLineEdit->text();
	trailer = m_trailerLineEdit->text();
	return m_formatComboBox->currentItem();
}

/**
 * Set ID3v1 or ID3v2 tags as export source.
 *
 * @param v1 true to set ID3v1, false for ID3v2
 */
void ExportDialog::setSrcV1(bool v1)
{
	m_srcComboBox->setCurrentItem(v1 ? SrcV1 : SrcV2);
}

/**
 * Get export source.
 *
 * @return true if ID3v1 is source,
 *         false if ID3v2.
 */
bool ExportDialog::getSrcV1() const
{
	return m_srcComboBox->currentItem() == SrcV1;
}

/**
 * Set size of window.
 *
 * @param width  width
 * @param height height
 */
void ExportDialog::setWindowSize(int width, int height)
{
	m_windowWidth = width;
	m_windowHeight = height;
	if (width > 0 && height > 0) {
		resize(width, height);
	}
}

/**
 * Get size of window.
 *
 * @param width  the width is returned here
 * @param height the height is returned here
 */
void ExportDialog::getWindowSize(int& width, int& height) const
{
	width = m_windowWidth;
	height = m_windowHeight;
}

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
		if (file.open(IO_WriteOnly)) {
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
 * %s title (song)
 * %l album
 * %a artist
 * %c comment
 * %y year
 * %t track, two digits, i.e. leading zero if < 10
 * %T track, without leading zeroes
 * %g genre
 * %f filename
 * %p path to file
 * %u URL of file
 * %d duration in minutes:seconds
 * %D duration in seconds
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
	QString fmt(format);
	if (!fmt.isEmpty()) {

		const int numTagCodes = 14;
		const QChar tagCode[numTagCodes] = {
	    's', 'l', 'a', 'c', 'y', 't', 'T', 'g', 'f', 'p', 'u', 'd', 'D', 'n'};
		QString tagStr[numTagCodes];

		QString year, track;
		year.sprintf("%d", trackData.year);
		track.sprintf("%02d", trackData.track);
		QString filename(trackData.getAbsFilename());
		int sepPos = filename.findRev('/');
		if (sepPos < 0) {
			sepPos = filename.findRev(QDir::separator());
		}
		if (sepPos >= 0) {
			filename.remove(0, sepPos + 1);
		}

		tagStr[0] = trackData.title;
		tagStr[1] = trackData.album;
		tagStr[2] = trackData.artist;
		tagStr[3] = trackData.comment;
		tagStr[4] = year;
		tagStr[5] = track;
		tagStr[6] = QString::number(trackData.track);
		tagStr[7] = Genres::getName(trackData.genre);
		if (tagStr[7].isEmpty()) {
			tagStr[7] = trackData.genreStr;
		}
		tagStr[8] = filename;
		tagStr[9] = trackData.getAbsFilename();
		QUrl url;
		url.setFileName(tagStr[9]);
		url.setProtocol("file");
		tagStr[10] = url.toString(true);
		tagStr[11] = TaggedFile::formatTime(trackData.getFileDuration());
		tagStr[12] = QString::number(trackData.getFileDuration());
		tagStr[13] = QString::number(numTracks);

		const int numEscCodes = 8;
		const QChar escCode[numEscCodes] = {
			'n', 't', 'r', '\\', 'a', 'b', 'f', 'v'};
		const char escChar[numEscCodes] = {
			'\n', '\t', '\r', '\\', '\a', '\b', '\f', '\v'};

		int pos;
		for (pos = 0; pos < static_cast<int>(fmt.length());) {
			pos = fmt.find('\\', pos);
			if (pos == -1) break;
			++pos;
			for (int k = 0;; ++k) {
				if (k >= numEscCodes) {
					// invalid code at pos
					++pos;
					break;
				}
				if (fmt[pos] == escCode[k]) {
					// code found, replace it
					fmt.replace(pos - 1, 2, escChar[k]);
					break;
				}
			}
		}

		for (pos = 0; pos < static_cast<int>(fmt.length());) {
			pos = fmt.find('%', pos);
			if (pos == -1) break;

			for (int k = 0;; ++k) {
				if (k >= numTagCodes) {
					// invalid code at pos
					++pos;
					break;
				}
				if (fmt[pos + 1] == tagCode[k]) {
					// code found, replace it
					fmt.replace(pos, 2, tagStr[k]);
					pos += tagStr[k].length();
					break;
				}
			}
		}
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
 * Save the size of the window and close it.
 */
void ExportDialog::saveWindowSizeAndClose()
{
	m_windowWidth = size().width();
	m_windowHeight = size().height();
	accept();
}
