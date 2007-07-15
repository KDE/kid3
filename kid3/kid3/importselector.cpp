/**
 * \file importselector.cpp
 * Import selector widget.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 */

#include "config.h"
#ifdef CONFIG_USE_KDE
#include <kfiledialog.h>
#else
#include <qfiledialog.h>
#endif
#include <qlayout.h>
#include <qpushbutton.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qbitarray.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <QTableWidget>
#include <QHeaderView>
#include <QList>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#else
#include <qtable.h>
#include <qgroupbox.h>
#endif
#include "genres.h"
#include "standardtags.h"
#include "freedbdialog.h"
#include "tracktypedialog.h"
#include "musicbrainzreleasedialog.h"
#include "discogsdialog.h"
#include "kid3.h"
#include "taggedfile.h"
#include "importselector.h"
#ifdef HAVE_TUNEPIMP
#include "musicbrainzdialog.h"
#include "musicbrainzconfig.h"
#endif

/**
 * Table used for import data.
 * Subclassed to be able to change the cell colors.
 */
class ImportTable: public
#if QT_VERSION >= 0x040000
QTableWidget
#else
QTable
#endif
{
public:
	/**
	 * Constructor.
	 * @param parent parent widget
	 */
	ImportTable(QWidget* parent = 0) :
#if QT_VERSION >= 0x040000
		QTableWidget(parent)
#else
		QTable(parent)
#endif
		{}

	/**
	 * Constructor.
	 * @param numRows number of rows
	 * @param numCols number of columns
	 * @param parent  parent widget
	 */
	ImportTable(int numRows, int numCols, QWidget* parent = 0) :
#if QT_VERSION >= 0x040000
		QTableWidget(numRows, numCols, parent)
#else
		QTable(numRows, numCols, parent)
#endif
		{} 

	/**
	 * Clear marked rows.
	 */
	void clearMarks() {
		m_markedRows.fill(false);
#if QT_VERSION >= 0x040000
		for (int row = 0; row < rowCount(); ++row) {
			QTableWidgetItem* twi = item(row, 0);
#if QT_VERSION >= 0x040200
			if (twi) twi->setBackground(Qt::NoBrush);
#else
			if (twi) twi->setBackgroundColor(QColor());
#endif
		}
#endif
	}

	/**
	 * Mark a row.
	 * This first cell of such a row will have a red background.
	 * @param row number of row
	 */
	void markRow(unsigned row) {
		if (static_cast<unsigned>(m_markedRows.size()) <= row)
			m_markedRows.resize(row + 1);
		m_markedRows.setBit(row);
#if QT_VERSION >= 0x040200
		QTableWidgetItem* twi = item(row, 0);
		if (twi) twi->setBackground(Qt::red);
#elif QT_VERSION >= 0x040000
		QTableWidgetItem* twi = item(row, 0);
		if (twi) twi->setBackgroundColor(Qt::red);
#endif
	}

	/**
	 * Check if a row is marked.
	 * @param row number of row
	 * @return true if row is marked.
	 */
	bool isRowMarked(unsigned row) const {
		return static_cast<unsigned>(m_markedRows.size()) > row &&
			m_markedRows.testBit(row);
	}

protected:
	/**
	 * Called when a cell is painted.
	 * Paint the first cell of marked rows with red background.
	 * @param p painter
	 * @param row column
	 * @param col column
	 * @param cr  cell rectangle
	 * @param selected true if selected
	 * @param cg color group
	 */
#if QT_VERSION < 0x040000
	virtual void paintCell(QPainter* p, int row, int col, const QRect& cr, bool selected, const QColorGroup& cg) {
		if (col == 0 && isRowMarked(row)) {
			QColorGroup g(cg);
			g.setColor(QColorGroup::Base, QColor("red"));
			QTable::paintCell(p, row, col, cr, selected, g);
		} else {
			QTable::paintCell(p, row, col, cr, selected, cg);
		}
	}
#endif

private:
	QBitArray m_markedRows;
};


/**
 * Constructor.
 *
 * @param parent parent widget
 * @param trackDataList track data to be filled with imported values,
 *                      is passed with durations of files set
 */
ImportSelector::ImportSelector(
	QWidget* parent, ImportTrackDataVector& trackDataList) :
	QWidget(parent),
	m_trackDataVector(trackDataList)
{
	m_freedbDialog = 0;
	m_trackTypeDialog = 0;
	m_musicBrainzReleaseDialog = 0;
	m_discogsDialog = 0;
#ifdef HAVE_TUNEPIMP
	m_musicBrainzDialog = 0;
#endif
	m_importSource = None;
	m_headerParser = new ImportParser();
	m_trackParser = new ImportParser();
	QVBoxLayout* vboxLayout = new QVBoxLayout(this);
	vboxLayout->setSpacing(6);
	vboxLayout->setMargin(6);
#if QT_VERSION >= 0x040000
	m_tab = new ImportTable(0, NumColumns, this);
	m_tab->setSelectionMode(QAbstractItemView::NoSelection);
	m_tab->setHorizontalHeaderLabels(
		QStringList() << i18n("Length") << i18n("Track") << i18n("Title") <<
		i18n("Artist") << i18n("Album") << i18n("Year") << i18n("Genre") <<
		i18n("Comment"));
	m_tab->resizeColumnToContents(TrackColumn);
	m_tab->resizeColumnToContents(YearColumn);
	QHeaderView* vHeader = m_tab->verticalHeader();
	if (vHeader) {
		vHeader->setMovable(true);
		vHeader->setResizeMode(QHeaderView::ResizeToContents);
	}
#else
	m_tab = new ImportTable(0, NumColumns, this);
	m_tab->setReadOnly(true);
	m_tab->setFocusStyle(QTable::FollowStyle);
	m_tab->setRowMovingEnabled(true);
	m_tab->setSelectionMode(QTable::NoSelection);
	QHeader* hHeader = m_tab->horizontalHeader();
	QHeader* vHeader = m_tab->verticalHeader();
	hHeader->setLabel(LengthColumn, i18n("Length"));
	hHeader->setLabel(TrackColumn, i18n("Track"));
	hHeader->setLabel(TitleColumn, i18n("Title"));
	hHeader->setLabel(ArtistColumn, i18n("Artist"));
	hHeader->setLabel(AlbumColumn, i18n("Album"));
	hHeader->setLabel(YearColumn, i18n("Year"));
	hHeader->setLabel(GenreColumn, i18n("Genre"));
	hHeader->setLabel(CommentColumn, i18n("Comment"));
	m_tab->adjustColumn(TrackColumn);
	m_tab->adjustColumn(YearColumn);
#endif
	vboxLayout->addWidget(m_tab);

#if QT_VERSION >= 0x040000
	QGroupBox* fmtbox = new QGroupBox(i18n("Format"), this);
#else
	QGroupBox* fmtbox = new QGroupBox(3, Qt::Vertical, i18n("Format"), this);
#endif
	m_formatComboBox = new QComboBox(fmtbox);
	m_formatComboBox->setEditable(true);
	m_headerLineEdit = new QLineEdit(fmtbox);
	m_trackLineEdit = new QLineEdit(fmtbox);
	connect(m_formatComboBox, SIGNAL(activated(int)), this, SLOT(setFormatLineEdit(int)));
#if QT_VERSION >= 0x040000
	QVBoxLayout* vbox = new QVBoxLayout;
	vbox->setMargin(2);
	vbox->addWidget(m_formatComboBox);
	vbox->addWidget(m_headerLineEdit);
	vbox->addWidget(m_trackLineEdit);
	fmtbox->setLayout(vbox);
#endif
	vboxLayout->addWidget(fmtbox);

	QWidget* butbox = new QWidget(this);
#if QT_VERSION >= 0x040000
	QGridLayout* butlayout = new QGridLayout(butbox);
	butlayout->setMargin(0);
	butlayout->setSpacing(6);
#else
	QGridLayout* butlayout = new QGridLayout(butbox, 2, 8, 0, 6);
#endif
	m_fileButton = new QPushButton(i18n("From F&ile"), butbox);
	butlayout->addWidget(m_fileButton, 0, 0);
	m_clipButton = new QPushButton(i18n("From Clip&board"), butbox);
	butlayout->addWidget(m_clipButton, 0, 1);
	m_freedbButton = new QPushButton(i18n("&gnudb.org"), butbox);
	butlayout->addWidget(m_freedbButton, 0, 2);
	m_trackTypeButton = new QPushButton(i18n("&TrackType.org"), butbox);
	butlayout->addWidget(m_trackTypeButton, 0, 3);
	m_discogsButton = new QPushButton(i18n("Disco&gs"), butbox);
	butlayout->addWidget(m_discogsButton, 0, 4);
	QSpacerItem* butspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
	                                       QSizePolicy::Minimum);
	butlayout->addItem(butspacer, 0, 5);
	QLabel* destLabel = new QLabel(butbox);
	destLabel->setText(i18n("D&estination:"));
	butlayout->addWidget(destLabel, 0, 6);
	m_destComboBox = new QComboBox(butbox);
	m_destComboBox->setEditable(false);
	m_destComboBox->QCM_insertItem(DestV1, i18n("Tag 1"));
	m_destComboBox->QCM_insertItem(DestV2, i18n("Tag 2"));
	destLabel->setBuddy(m_destComboBox);
	butlayout->addWidget(m_destComboBox, 0, 7);

	m_musicBrainzReleaseButton = new QPushButton(i18n("From MusicBrainz &Release"), butbox);
#if QT_VERSION >= 0x040000
	butlayout->addWidget(m_musicBrainzReleaseButton, 1, 0, 1, 2);
#else
	butlayout->addMultiCellWidget(m_musicBrainzReleaseButton, 1, 1, 0, 1);
#endif
#ifdef HAVE_TUNEPIMP
	m_musicBrainzButton = new QPushButton(i18n("From &MusicBrainz Fingerprint"), butbox);
#if QT_VERSION >= 0x040000
	butlayout->addWidget(m_musicBrainzButton, 1, 2, 1, 3);
#else
	butlayout->addMultiCellWidget(m_musicBrainzButton, 1, 1, 2, 4);
#endif
#endif
	vboxLayout->addWidget(butbox);

	QWidget* matchBox = new QWidget(this);
	QHBoxLayout* matchLayout = new QHBoxLayout(matchBox);
	matchLayout->setMargin(0);
	matchLayout->setSpacing(6);
	m_mismatchCheckBox = new QCheckBox(
		i18n("Check maximum allowable time &difference (sec):"), matchBox);
	matchLayout->addWidget(m_mismatchCheckBox);
	m_maxDiffSpinBox = new QSpinBox(matchBox);
	m_maxDiffSpinBox->QCM_setMaximum(9999);
	matchLayout->addWidget(m_maxDiffSpinBox);
	QSpacerItem* matchSpacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
																						 QSizePolicy::Minimum);
	matchLayout->addItem(matchSpacer);
	QLabel* matchLabel = new QLabel(i18n("Match with:"), matchBox);
	matchLayout->addWidget(matchLabel);
	m_lengthButton = new QPushButton(i18n("&Length"), matchBox);
	matchLayout->addWidget(m_lengthButton);
	m_trackButton = new QPushButton(i18n("T&rack"), matchBox);
	matchLayout->addWidget(m_trackButton);
	m_titleButton = new QPushButton(i18n("&Title"), matchBox);
	matchLayout->addWidget(m_titleButton);
	vboxLayout->addWidget(matchBox);

	connect(m_fileButton, SIGNAL(clicked()), this, SLOT(fromFile()));
	connect(m_clipButton, SIGNAL(clicked()), this, SLOT(fromClipboard()));
	connect(m_freedbButton, SIGNAL(clicked()), this, SLOT(fromFreedb()));
	connect(m_trackTypeButton, SIGNAL(clicked()), this, SLOT(fromTrackType()));
	connect(m_musicBrainzReleaseButton, SIGNAL(clicked()), this, SLOT(fromMusicBrainzRelease()));
	connect(m_discogsButton, SIGNAL(clicked()), this, SLOT(fromDiscogs()));
#ifdef HAVE_TUNEPIMP
	connect(m_musicBrainzButton, SIGNAL(clicked()), this, SLOT(fromMusicBrainz()));
#endif
	connect(m_lengthButton, SIGNAL(clicked()), this, SLOT(matchWithLength()));
	connect(m_trackButton, SIGNAL(clicked()), this, SLOT(matchWithTrack()));
	connect(m_titleButton, SIGNAL(clicked()), this, SLOT(matchWithTitle()));
#if QT_VERSION >= 0x040000
	connect(vHeader, SIGNAL(sectionMoved(int, int, int)), this, SLOT(moveTableRow(int, int, int)));
#else
	connect(vHeader, SIGNAL(indexChange(int, int, int)), this, SLOT(moveTableRow(int, int, int)));
#endif
	connect(m_mismatchCheckBox, SIGNAL(toggled(bool)), this, SLOT(showPreview()));
	connect(m_maxDiffSpinBox, SIGNAL(valueChanged(int)), this, SLOT(maxDiffChanged()));
}

/**
 * Destructor.
 */
ImportSelector::~ImportSelector()
{
	delete m_headerParser;
	delete m_trackParser;
	if (m_freedbDialog) {
		m_freedbDialog->disconnect();
		delete m_freedbDialog;
		m_freedbDialog = 0;
	}
	if (m_trackTypeDialog) {
		m_trackTypeDialog->disconnect();
		delete m_trackTypeDialog;
		m_trackTypeDialog = 0;
	}
	if (m_musicBrainzReleaseDialog) {
		m_musicBrainzReleaseDialog->disconnect();
		delete m_musicBrainzReleaseDialog;
		m_musicBrainzReleaseDialog = 0;
	}
	if (m_discogsDialog) {
		m_discogsDialog->disconnect();
		delete m_discogsDialog;
		m_discogsDialog = 0;
	}
#ifdef HAVE_TUNEPIMP
	if (m_musicBrainzDialog) {
		m_musicBrainzDialog->disconnect();
		delete m_musicBrainzDialog;
		m_musicBrainzDialog = 0;
	}
#endif
}

/**
 * Clear dialog data.
 */
void ImportSelector::clear()
{
#if QT_VERSION >= 0x040000
	m_tab->setRowCount(0);
#else
	m_tab->setNumRows(0);
#endif
	m_destComboBox->QCM_setCurrentIndex(
		static_cast<int>(Kid3App::s_genCfg.m_importDestV1 ? DestV1 : DestV2));

	m_formatHeaders = Kid3App::s_genCfg.m_importFormatHeaders;
	m_formatTracks = Kid3App::s_genCfg.m_importFormatTracks;
	m_formatComboBox->clear();
	m_formatComboBox->QCM_addItems(Kid3App::s_genCfg.m_importFormatNames);
	m_formatComboBox->QCM_setCurrentIndex(Kid3App::s_genCfg.m_importFormatIdx);
	setFormatLineEdit(Kid3App::s_genCfg.m_importFormatIdx);

	m_mismatchCheckBox->setChecked(Kid3App::s_genCfg.m_enableTimeDifferenceCheck);
	m_maxDiffSpinBox->setValue(Kid3App::s_genCfg.m_maxTimeDifference);
}

/**
 * Look for album specific information (artist, album, year, genre) in
 * a header.
 *
 * @param st standard tags to put resulting values in,
 *           fields which are not found are not touched.
 *
 * @return true if one or more field were found.
 */
bool ImportSelector::parseHeader(StandardTags& st)
{
	int pos = 0;
	m_headerParser->setFormat(m_headerLineEdit->text());
	return m_headerParser->getNextTags(m_text, st, pos);
}

/**
 * Let user select file, assign file contents to text and preview in
 * table.
 */
void ImportSelector::fromFile()
{
	QString fn =
#ifdef CONFIG_USE_KDE
		KFileDialog::getOpenFileName(QString::null, QString::null, this);
#else
		QFileDialog::QCM_getOpenFileName(this);
#endif
	if (!fn.isEmpty()) {
		QFile file(fn);
		if (file.open(QCM_ReadOnly)) {
			QTextStream stream(&file);
			m_text = stream.QCM_readAll();
			if (!m_text.isNull()) {
				updateTrackData(File);
				showPreview();
			}
			file.close();
		}
	}
}

/**
 * Assign clipboard contents to text and preview in table.
 */
void ImportSelector::fromClipboard()
{
	QClipboard* cb = QApplication::clipboard();
#if QT_VERSION >= 0x030100
	m_text = cb->text(QClipboard::Clipboard);
	if (!m_text.isNull() && updateTrackData(Clipboard)) {
		showPreview();
	} else {
		m_text = cb->text(QClipboard::Selection);
		if (!m_text.isNull()) {
			updateTrackData(Clipboard);
			showPreview();
		}
	}
#else
	m_text = cb->text();
	if (!m_text.isNull()) {
		updateTrackData(Clipboard);
		showPreview();
	}
#endif
}

/**
 * Import from freedb.org and preview in table.
 */
void ImportSelector::fromFreedb()
{
	if (!m_freedbDialog) {
		m_freedbDialog = new FreedbDialog(this, m_trackDataVector);
		connect(m_freedbDialog, SIGNAL(trackDataUpdated()),
						this, SLOT(showPreview()));
	}
	if (m_freedbDialog) {
		m_freedbDialog->setArtistAlbum(m_trackDataVector.m_artist,
																	 m_trackDataVector.m_album);
		(void)m_freedbDialog->exec();
	}
}

/**
 * Import from TrackType.org and preview in table.
 */
void ImportSelector::fromTrackType()
{
	if (!m_trackTypeDialog) {
		m_trackTypeDialog = new TrackTypeDialog(this, m_trackDataVector);
		connect(m_trackTypeDialog, SIGNAL(trackDataUpdated()),
						this, SLOT(showPreview()));
	}
	if (m_trackTypeDialog) {
		m_trackTypeDialog->setArtistAlbum(m_trackDataVector.m_artist,
																	 m_trackDataVector.m_album);
		(void)m_trackTypeDialog->exec();
	}
}

/**
 * Import from MusicBrainz release database and preview in table.
 */
void ImportSelector::fromMusicBrainzRelease()
{
	if (!m_musicBrainzReleaseDialog) {
		m_musicBrainzReleaseDialog = new MusicBrainzReleaseDialog(this, m_trackDataVector);
		connect(m_musicBrainzReleaseDialog, SIGNAL(trackDataUpdated()),
						this, SLOT(showPreview()));
	}
	if (m_musicBrainzReleaseDialog) {
		m_musicBrainzReleaseDialog->setArtistAlbum(m_trackDataVector.m_artist,
																							 m_trackDataVector.m_album);
		(void)m_musicBrainzReleaseDialog->exec();
	}
}

/**
 * Import from www.discogs.com and preview in table.
 */
void ImportSelector::fromDiscogs()
{
	if (!m_discogsDialog) {
		m_discogsDialog = new DiscogsDialog(this, m_trackDataVector);
		connect(m_discogsDialog, SIGNAL(trackDataUpdated()),
						this, SLOT(showPreview()));
	}
	if (m_discogsDialog) {
		m_discogsDialog->setArtistAlbum(m_trackDataVector.m_artist,
																		m_trackDataVector.m_album);
		(void)m_discogsDialog->exec();
	}
}

/**
 * Set the format lineedits to the format selected in the combo box.
 *
 * @param index current index of the combo box
 */
void ImportSelector::setFormatLineEdit(int index)
{
	m_headerLineEdit->setText(m_formatHeaders[index]);
	m_trackLineEdit->setText(m_formatTracks[index]);
}

/**
 * Update track data list with imported tags.
 *
 * @param impSrc import source
 *
 * @return true if tags were found.
 */
bool ImportSelector::updateTrackData(ImportSource impSrc) {
	StandardTags st_hdr;
	st_hdr.setInactive();
	m_importSource = impSrc;
	(void)parseHeader(st_hdr);

	StandardTags st(st_hdr);
	bool start = true;
	ImportTrackDataVector::iterator it = m_trackDataVector.begin();
	bool atTrackDataListEnd = (it == m_trackDataVector.end());
	while (getNextTags(st, start)) {
		start = false;
		if (atTrackDataListEnd) {
			ImportTrackData trackData;
			trackData.setStandardTags(st);
			m_trackDataVector.push_back(trackData);
		} else {
			(*it).setStandardTags(st);
			++it;
			atTrackDataListEnd = (it == m_trackDataVector.end());
		}
		st = st_hdr;
	}
	st.setInactive();
	while (!atTrackDataListEnd) {
		if ((*it).getFileDuration() == 0) {
			it = m_trackDataVector.erase(it);
		} else {
			(*it).setStandardTags(st);
			(*it).setImportDuration(0);
			++it;
		}
		atTrackDataListEnd = (it == m_trackDataVector.end());
	}

	if (!start) {
		/* start is false => tags were found */
		TrackDurationList* trackDuration = getTrackDurations();
		if (trackDuration) {
			it = m_trackDataVector.begin();
			for (TrackDurationList::const_iterator tdit = trackDuration->begin();
					 tdit != trackDuration->end();
					 ++tdit) {
				if (it != m_trackDataVector.end()) {
					(*it).setImportDuration(*tdit);
					++it;
				} else {
					break;
				}
			}
		}
		return true;
	}
	return false;
}

/**
 * Show fields to import in text as preview in table.
 */
void ImportSelector::showPreview() {
#if QT_VERSION >= 0x040000
  m_tab->setRowCount(m_trackDataVector.size());
	int row = 0;
	QStringList vLabels;
	for (ImportTrackDataVector::const_iterator it = m_trackDataVector.begin();
			 it != m_trackDataVector.end();
			 ++it) {
		QTableWidgetItem* twi;
		int fileDuration = (*it).getFileDuration();
		if (fileDuration != 0) {
			vLabels.append(TaggedFile::formatTime(fileDuration));
		}
		int importDuration = (*it).getImportDuration();
		if (importDuration != 0) {
			QString lengthStr = TaggedFile::formatTime(importDuration);
			if ((twi = m_tab->item(row, LengthColumn)) != 0) {
				twi->setText(lengthStr);
			} else {
				twi = new QTableWidgetItem(lengthStr);
				twi->setFlags(twi->flags() & ~Qt::ItemIsEditable);
				m_tab->setItem(row, LengthColumn, twi);
			}
		}
		if ((*it).track != -1) {
			QString trackStr;
			trackStr.setNum((*it).track);
			if ((twi = m_tab->item(row, TrackColumn)) != 0) {
				twi->setText(trackStr);
			} else {
				twi = new QTableWidgetItem(trackStr);
				twi->setFlags(twi->flags() & ~Qt::ItemIsEditable);
				m_tab->setItem(row, TrackColumn, twi);
			}
		}
		if (!(*it).title.isNull()) {
			if ((twi = m_tab->item(row, TitleColumn)) != 0) {
				twi->setText((*it).title);
			} else {
				twi = new QTableWidgetItem((*it).title);
				twi->setFlags(twi->flags() & ~Qt::ItemIsEditable);
				m_tab->setItem(row, TitleColumn, twi);
			}
		}
		if (!(*it).artist.isNull()) {
			if ((twi = m_tab->item(row, ArtistColumn)) != 0) {
				twi->setText((*it).artist);
			} else {
				twi = new QTableWidgetItem((*it).artist);
				twi->setFlags(twi->flags() & ~Qt::ItemIsEditable);
				m_tab->setItem(row, ArtistColumn, twi);
			}
		}
		if (!(*it).album.isNull()) {
			if ((twi = m_tab->item(row, AlbumColumn)) != 0) {
				twi->setText((*it).album);
			} else {
				twi = new QTableWidgetItem((*it).album);
				twi->setFlags(twi->flags() & ~Qt::ItemIsEditable);
				m_tab->setItem(row, AlbumColumn, twi);
			}
		}
		if ((*it).year != -1) {
			QString yearStr;
			yearStr.setNum((*it).year);
			if ((twi = m_tab->item(row, YearColumn)) != 0) {
				twi->setText(yearStr);
			} else {
				twi = new QTableWidgetItem(yearStr);
				twi->setFlags(twi->flags() & ~Qt::ItemIsEditable);
				m_tab->setItem(row, YearColumn, twi);
			}
		}
		if (!(*it).genre.isNull()) {
			if ((twi = m_tab->item(row, GenreColumn)) != 0) {
				twi->setText((*it).genre);
			} else {
				twi = new QTableWidgetItem((*it).genre);
				twi->setFlags(twi->flags() & ~Qt::ItemIsEditable);
				m_tab->setItem(row, GenreColumn, twi);
			}
		}
		if (!(*it).comment.isNull()) {
			if ((twi = m_tab->item(row, CommentColumn)) != 0) {
				twi->setText((*it).comment);
			} else {
				twi = new QTableWidgetItem((*it).comment);
				twi->setFlags(twi->flags() & ~Qt::ItemIsEditable);
				m_tab->setItem(row, CommentColumn, twi);
			}
		}
		++row;
	}
	m_tab->setVerticalHeaderLabels(vLabels);
#else
	m_tab->setNumRows(0);
	int row = 0;
	QHeader* vHeader = m_tab->verticalHeader();
	for (ImportTrackDataVector::const_iterator it = m_trackDataVector.begin();
			 it != m_trackDataVector.end();
			 ++it) {
		m_tab->setNumRows(row + 1);
		int fileDuration = (*it).getFileDuration();
		if (fileDuration != 0) {
			vHeader->setLabel(row, TaggedFile::formatTime(fileDuration));
		}
		int importDuration = (*it).getImportDuration();
		if (importDuration != 0)
			m_tab->setText(row, LengthColumn, TaggedFile::formatTime(importDuration));
		if ((*it).track != -1) {
			QString trackStr;
			trackStr.setNum((*it).track);
			m_tab->setText(row, TrackColumn, trackStr);
		}
		if (!(*it).title.isNull())
			m_tab->setText(row, TitleColumn, (*it).title);
		if (!(*it).artist.isNull())
			m_tab->setText(row, ArtistColumn, (*it).artist);
		if (!(*it).album.isNull())
			m_tab->setText(row, AlbumColumn, (*it).album);
		if ((*it).year != -1) {
			QString yearStr;
			yearStr.setNum((*it).year);
			m_tab->setText(row, YearColumn, yearStr);
		}
		if (!(*it).genre.isNull())
			m_tab->setText(row, GenreColumn, (*it).genre);
		if (!(*it).comment.isNull())
			m_tab->setText(row, CommentColumn, (*it).comment);
		++row;
	}
#endif

	// make time difference check
	m_tab->clearMarks();
	bool diffCheckEnable;
	int maxDiff;
	getTimeDifferenceCheck(diffCheckEnable, maxDiff);
	if (diffCheckEnable) {
		bool tracksAvailable = false;
		row = 0;
		for (ImportTrackDataVector::const_iterator it = m_trackDataVector.begin();
				 it != m_trackDataVector.end();
				 ++it) {
			int fileDuration = (*it).getFileDuration();
			int importDuration = (*it).getImportDuration();
			if (fileDuration != 0 && importDuration != 0) {
				int diff = fileDuration > importDuration ?
					fileDuration - importDuration : importDuration - fileDuration;
				if (diff > maxDiff) {
					m_tab->markRow(row);
				}
				tracksAvailable = true;
			}
			if (tracksAvailable &&
					((fileDuration == 0 && importDuration != 0) ||
					 (fileDuration != 0 && importDuration == 0))) {
				m_tab->markRow(row);
			}
			++row;
		}
	}
}

/**
 * Get next line as standardtags from imported file or clipboard.
 *
 * @param st standard tags
 * @param start true to start with the first line, false for all
 *              other lines
 *
 * @return true if ok (result in st),
 *         false if end of file reached.
 */
bool ImportSelector::getNextTags(StandardTags& st, bool start)
{
	static int pos = 0;
	if (start || pos == 0) {
		pos = 0;
		m_trackParser->setFormat(m_trackLineEdit->text(), true);
	}
	return m_trackParser->getNextTags(m_text, st, pos);
}

/**
 * Get import destination.
 *
 * @return DestV1 or DestV2 for ID3v1 or ID3v2.
 */
ImportSelector::Destination ImportSelector::getDestination()
{
	return (Destination)m_destComboBox->QCM_currentIndex();
}

/**
 * Save the local settings to the configuration.
 */
void ImportSelector::saveConfig()
{
	Kid3App::s_genCfg.m_importDestV1 = 
		(m_destComboBox->QCM_currentIndex() == static_cast<int>(ImportSelector::DestV1));

	Kid3App::s_genCfg.m_importFormatIdx = m_formatComboBox->QCM_currentIndex();
	Kid3App::s_genCfg.m_importFormatNames[Kid3App::s_genCfg.m_importFormatIdx] = m_formatComboBox->currentText();
	Kid3App::s_genCfg.m_importFormatHeaders[Kid3App::s_genCfg.m_importFormatIdx] = m_headerLineEdit->text();
	Kid3App::s_genCfg.m_importFormatTracks[Kid3App::s_genCfg.m_importFormatIdx] = m_trackLineEdit->text();
	getTimeDifferenceCheck(Kid3App::s_genCfg.m_enableTimeDifferenceCheck,
												 Kid3App::s_genCfg.m_maxTimeDifference);

}

/**
 * Get list with track durations.
 *
 * @return list with track durations,
 *         0 if no track durations found.
 */
TrackDurationList* ImportSelector::getTrackDurations()
{
	TrackDurationList* lst = 0;
	if (m_headerParser && ((lst = m_headerParser->getTrackDurations()) != 0) &&
			(lst->size() > 0)) {
		return lst;
	} else if (m_trackParser && ((lst = m_trackParser->getTrackDurations()) != 0) &&
						 (lst->size() > 0)) {
		return lst;
	} else {
		return 0;
	}
}

/**
 * Get time difference check configuration.
 *
 * @param enable  true if check is enabled
 * @param maxDiff maximum allowed time difference
 */ 
void ImportSelector::getTimeDifferenceCheck(bool& enable, int& maxDiff) const
{
	enable = m_mismatchCheckBox->isChecked();
	maxDiff = m_maxDiffSpinBox->value();
}

/**
 * Called when the maximum time difference value is changed.
 */
void ImportSelector::maxDiffChanged() {
	if (m_mismatchCheckBox->isChecked()) {
		showPreview();
	}
}

/**
 * Move a table row.
 *
 * @param section not used
 * @param fromIndex index of position moved from
 * @param fromIndex index of position moved to
 */
void ImportSelector::moveTableRow(int, int fromIndex, int toIndex) {
#if QT_VERSION >= 0x040000
	QHeaderView* vHeader = m_tab->verticalHeader();
	if (vHeader) {
		// revert movement, but avoid recursion
		disconnect(vHeader, SIGNAL(sectionMoved(int, int, int)), 0, 0);
		vHeader->moveSection(toIndex, fromIndex);
		connect(vHeader, SIGNAL(sectionMoved(int, int, int)), this, SLOT(moveTableRow(int, int, int)));
	}
#else
	if (toIndex > fromIndex && toIndex > 0) {
		--toIndex;
	}
#endif
	int numTracks = static_cast<int>(m_trackDataVector.size());
	if (fromIndex < numTracks && toIndex < numTracks) {
		// swap elements but keep file durations and names
		ImportTrackData fromData(m_trackDataVector[fromIndex]);
		ImportTrackData toData(m_trackDataVector[toIndex]);
		m_trackDataVector[fromIndex].setStandardTags(toData);
		m_trackDataVector[toIndex].setStandardTags(fromData);
		m_trackDataVector[fromIndex].setImportDuration(toData.getImportDuration());
		m_trackDataVector[toIndex].setImportDuration(fromData.getImportDuration());
		// redisplay the table
		showPreview();
	}
}

/**
 * Import from MusicBrainz and preview in table.
 */
void ImportSelector::fromMusicBrainz()
{
#ifdef HAVE_TUNEPIMP
	if (!m_musicBrainzDialog) {
		m_musicBrainzDialog = new MusicBrainzDialog(this, m_trackDataVector);
		connect(m_musicBrainzDialog, SIGNAL(trackDataUpdated()),
						this, SLOT(showPreview()));
	}
	if (m_musicBrainzDialog) {
		m_musicBrainzDialog->initTable();
		(void)m_musicBrainzDialog->exec();
	}
#endif
}

/**
 * Match import data with length.
 */
void ImportSelector::matchWithLength()
{
	struct MatchData {
		int fileLen;      // length of file
		int importLen;    // length of import
		int assignedTo;   // number of file import is assigned to, -1 if not assigned
		int assignedFrom; // number of import assigned to file, -1 if not assigned
	};

	bool failed = false;
	unsigned numTracks = m_trackDataVector.size();
	if (numTracks > 0) {
		bool diffCheckEnable;
		int maxDiff;
		getTimeDifferenceCheck(diffCheckEnable, maxDiff);

		MatchData* md = new MatchData[numTracks];
		unsigned numFiles = 0, numImports = 0;
		unsigned i = 0;
		for (ImportTrackDataVector::const_iterator it = m_trackDataVector.begin();
				 it != m_trackDataVector.end();
				 ++it) {
			if (i >= numTracks) {
				break;
			}
			md[i].fileLen = (*it).getFileDuration();
			if (md[i].fileLen > 0) {
				++numFiles;
			}
			md[i].importLen = (*it).getImportDuration();
			if (md[i].importLen > 0) {
				++numImports;
			}
			md[i].assignedTo = -1;
			md[i].assignedFrom = -1;
			// If time difference checking is enabled and the time difference
			// is not larger then the allowed limit, do not reassign the track.
			if (diffCheckEnable) {
				if (md[i].fileLen != 0 && md[i].importLen != 0) {
					int diff = md[i].fileLen > md[i].importLen ?
						md[i].fileLen - md[i].importLen : md[i].importLen - md[i].fileLen;
					if (diff <= maxDiff) {
						md[i].assignedTo = i;
						md[i].assignedFrom = i;
					}
				}
			}
			++i;
		}

		if (numFiles <= numImports) {
			// more imports than files => first look through all imports
			for (i = 0; i < numTracks; ++i) {
				if (md[i].assignedFrom == -1) {
					int bestTrack = -1;
					int bestDiff = INT_MAX;
					// Find the unassigned import with the best difference
					for (unsigned comparedTrack = 0; comparedTrack < numTracks; ++comparedTrack) {
						if (md[comparedTrack].assignedTo == -1) {
							int comparedDiff = md[i].fileLen > md[comparedTrack].importLen ?
								md[i].fileLen - md[comparedTrack].importLen :
								md[comparedTrack].importLen - md[i].fileLen;
							if (comparedDiff < bestDiff) {
								bestDiff = comparedDiff;
								bestTrack = comparedTrack;
							}
						}
					}
					if (bestTrack >= 0 && bestTrack < static_cast<int>(numTracks)) {
						md[i].assignedFrom = bestTrack;
						md[bestTrack].assignedTo = i;
					} else {
						qDebug("No match for track %d", i);
						failed = true;
						break;
					}
				}
			}
		} else {
			// more files than imports => first look through all files
			for (i = 0; i < numTracks; ++i) {
				if (md[i].assignedTo == -1) {
					int bestTrack = -1;
					int bestDiff = INT_MAX;
					// Find the unassigned file with the best difference
					for (unsigned comparedTrack = 0; comparedTrack < numTracks; ++comparedTrack) {
						if (md[comparedTrack].assignedFrom == -1) {
							int comparedDiff = md[comparedTrack].fileLen > md[i].importLen ?
								md[comparedTrack].fileLen - md[i].importLen :
								md[i].importLen - md[comparedTrack].fileLen;
							if (comparedDiff < bestDiff) {
								bestDiff = comparedDiff;
								bestTrack = comparedTrack;
							}
						}
					}
					if (bestTrack >= 0 && bestTrack < static_cast<int>(numTracks)) {
						md[i].assignedTo = bestTrack;
						md[bestTrack].assignedFrom = i;
					} else {
						qDebug("No match for track %d", i);
						failed = true;
						break;
					}
				}
			}
		}

		if (!failed) {
			ImportTrackDataVector oldTrackDataVector(m_trackDataVector);
			for (i = 0; i < numTracks; ++i) {
				m_trackDataVector[i].setStandardTags(
					oldTrackDataVector[md[i].assignedFrom]);
				m_trackDataVector[i].setImportDuration(
					oldTrackDataVector[md[i].assignedFrom].getImportDuration());
			}
			showPreview();
		}

		delete [] md;
	}
}

/**
 * Match import data with track number.
 */
void ImportSelector::matchWithTrack()
{
	struct MatchData {
		int track;        // track number starting with 0
		int assignedTo;   // number of file import is assigned to, -1 if not assigned
		int assignedFrom; // number of import assigned to file, -1 if not assigned
	};

	bool failed = false;
	unsigned numTracks = m_trackDataVector.size();
	if (numTracks > 0) {
		MatchData* md = new MatchData[numTracks];

		// 1st pass: Get track data and keep correct assignments.
		unsigned i = 0;
		for (ImportTrackDataVector::const_iterator it = m_trackDataVector.begin();
				 it != m_trackDataVector.end();
				 ++it) {
			if (i >= numTracks) {
				break;
			}
			if ((*it).track > 0 && (*it).track <= static_cast<int>(numTracks)) {
				md[i].track = (*it).track - 1;
			} else {
				md[i].track = -1;
			}
			md[i].assignedTo = -1;
			md[i].assignedFrom = -1;
			if (md[i].track == static_cast<int>(i)) {
				md[i].assignedTo = i;
				md[i].assignedFrom = i;
			}
			++i;
		}

		// 2nd pass: Assign imported track numbers to unassigned tracks.
		for (i = 0; i < numTracks; ++i) {
			if (md[i].assignedTo == -1 &&
					md[i].track >= 0 && md[i].track < static_cast<int>(numTracks)) {
				if (md[md[i].track].assignedFrom == -1) {
					md[md[i].track].assignedFrom = i;
					md[i].assignedTo = md[i].track;
				}
			}
		}

		// 3rd pass: Assign remaining tracks.
		unsigned unassignedTrack = 0;
		for (i = 0; i < numTracks; ++i) {
			if (md[i].assignedFrom == -1) {
				while (unassignedTrack < numTracks) {
					if (md[unassignedTrack].assignedTo == -1) {
						md[i].assignedFrom = unassignedTrack;
						md[unassignedTrack++].assignedTo = i;
						break;
					}
					++unassignedTrack;
				}
				if (md[i].assignedFrom == -1) {
					qDebug("No track assigned to %d", i);
					failed = true;
				}
			}
		}

		if (!failed) {
			ImportTrackDataVector oldTrackDataVector(m_trackDataVector);
			for (i = 0; i < numTracks; ++i) {
				m_trackDataVector[i].setStandardTags(
					oldTrackDataVector[md[i].assignedFrom]);
				m_trackDataVector[i].setImportDuration(
					oldTrackDataVector[md[i].assignedFrom].getImportDuration());
			}
			showPreview();
		}

		delete [] md;
	}
}

/**
 * Match import data with title.
 */
void ImportSelector::matchWithTitle()
{
	struct MatchData {
		QStringList fileWords;  // words in file name
		QStringList titleWords; // words in title
		int assignedTo;   // number of file import is assigned to, -1 if not assigned
		int assignedFrom; // number of import assigned to file, -1 if not assigned
	};

	bool failed = false;
	unsigned numTracks = m_trackDataVector.size();
	if (numTracks > 0) {
		MatchData* md = new MatchData[numTracks];
		unsigned numFiles = 0, numImports = 0;
		QRegExp nonWordCharRegExp("\\W");
		QRegExp nonLetterSpaceRegExp("[^a-z ]");
		unsigned i = 0;
		for (ImportTrackDataVector::const_iterator it = m_trackDataVector.begin();
				 it != m_trackDataVector.end();
				 ++it) {
			if (i >= numTracks) {
				break;
			}
			QString fileName = (*it).getAbsFilename();
			if (!fileName.isEmpty()) {
				++numFiles;
				int startIndex = fileName.QCM_lastIndexOf(QDir::separator()) + 1;
				int endIndex = fileName.QCM_lastIndexOf('.');
				if (endIndex > startIndex) {
					fileName = fileName.mid(startIndex, endIndex - startIndex);
				} else {
					fileName = fileName.mid(startIndex);
				}
				md[i].fileWords = QCM_split(
					nonWordCharRegExp, fileName.QCM_toLower().
					replace(nonLetterSpaceRegExp, " "));
			}
			if (!(*it).title.isEmpty()) {
				++numImports;
				md[i].titleWords = QCM_split(
					nonWordCharRegExp, (*it).title.QCM_toLower().
					replace(nonLetterSpaceRegExp, " "));
			}
			md[i].assignedTo = -1;
			md[i].assignedFrom = -1;
			++i;
		}

		if (numFiles <= numImports) {
			// more imports than files => first look through all imports
			for (i = 0; i < numTracks; ++i) {
				if (md[i].assignedFrom == -1) {
					int bestTrack = -1;
					int bestMatch = -1;
					// Find the unassigned import with the best match
					for (unsigned comparedTrack = 0; comparedTrack < numTracks; ++comparedTrack) {
						if (md[comparedTrack].assignedTo == -1) {
							int comparedMatch = 0;
							for (QStringList::const_iterator fwit = md[i].fileWords.begin();
									 fwit != md[i].fileWords.end();
									 ++fwit) {
								if (md[comparedTrack].titleWords.contains(*fwit)) {
									++comparedMatch;
								}
							}
							if (comparedMatch > bestMatch) {
								bestMatch = comparedMatch;
								bestTrack = comparedTrack;
							}
						}
					}
					if (bestTrack >= 0 && bestTrack < static_cast<int>(numTracks)) {
						md[i].assignedFrom = bestTrack;
						md[bestTrack].assignedTo = i;
					} else {
						qDebug("No match for track %d", i);
						failed = true;
						break;
					}
				}
			}
		} else {
			// more files than imports => first look through all files
			for (i = 0; i < numTracks; ++i) {
				if (md[i].assignedTo == -1) {
					int bestTrack = -1;
					int bestMatch = -1;
					// Find the unassigned file with the best match
					for (unsigned comparedTrack = 0; comparedTrack < numTracks; ++comparedTrack) {
						if (md[comparedTrack].assignedFrom == -1) {
							int comparedMatch = 0;
							for (QStringList::const_iterator fwit = md[comparedTrack].fileWords.begin();
									 fwit != md[comparedTrack].fileWords.end();
									 ++fwit) {
								if (md[i].titleWords.contains(*fwit)) {
									++comparedMatch;
								}
							}
							if (comparedMatch > bestMatch) {
								bestMatch = comparedMatch;
								bestTrack = comparedTrack;
							}
						}
					}
					if (bestTrack >= 0 && bestTrack < static_cast<int>(numTracks)) {
						md[i].assignedTo = bestTrack;
						md[bestTrack].assignedFrom = i;
					} else {
						qDebug("No match for track %d", i);
						failed = true;
						break;
					}
				}
			}
		}
		if (!failed) {
			ImportTrackDataVector oldTrackDataVector(m_trackDataVector);
			for (i = 0; i < numTracks; ++i) {
				m_trackDataVector[i].setStandardTags(
					oldTrackDataVector[md[i].assignedFrom]);
				m_trackDataVector[i].setImportDuration(
					oldTrackDataVector[md[i].assignedFrom].getImportDuration());
			}
			showPreview();
		}

		delete [] md;
	}
}
