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
#include <klocale.h>
#include <kfiledialog.h>
#else
#define i18n(s) tr(s)
#define I18N_NOOP(s) QT_TR_NOOP(s)
#include <qfiledialog.h>
#endif
#include <qlayout.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qtable.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qbitarray.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include "genres.h"
#include "standardtags.h"
#include "importparser.h"
#include "freedbdialog.h"
#include "freedbconfig.h"
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
class ImportTable: public QTable {
public:
	/**
	 * Constructor.
	 * @param parent parent widget
	 * @param name   Qt name
	 */
	ImportTable(QWidget* parent = 0, const char* name = 0) :
		QTable(parent, name) {}
	/**
	 * Constructor.
	 * @param numRows number of rows
	 * @param parent  parent widget
	 * @param name    Qt name
	 */
	ImportTable(int numRows, int numCols, QWidget* parent = 0, const char* name = 0) :
		QTable(numRows, numCols, parent, name) {} 
	/**
	 * Clear marked rows.
	 */
	void clearMarks() { m_markedRows.fill(false); }
	/**
	 * Mark a row.
	 * This first cell of such a row will have a red background.
	 * @param row number of row
	 */
	void markRow(unsigned row) {
		if (m_markedRows.size() <= row) m_markedRows.resize(row + 1);
		m_markedRows.setBit(row);
	}
	/**
	 * Check if a row is marked.
	 * @param row number of row
	 * @return true if row is marked.
	 */
	bool isRowMarked(unsigned row) const {
		return m_markedRows.size() > row && m_markedRows.testBit(row);
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
#if QT_VERSION >= 300
	virtual void paintCell(QPainter* p, int row, int col, const QRect& cr, bool selected, const QColorGroup& cg) {
		if (col == 0 && isRowMarked(row)) {
			QColorGroup g(cg);
			g.setColor(QColorGroup::Base, QColor("red"));
			QTable::paintCell(p, row, col, cr, selected, g);
		} else {
			QTable::paintCell(p, row, col, cr, selected, cg);
		}
	}
#else
	virtual void paintCell(QPainter* p, int row, int col, const QRect& cr, bool selected) {
		if (col == 0 && isRowMarked(row)) {
			p->setBackgroundColor(QColor("red"));
		}
		QTable::paintCell(p, row, col, cr, selected);
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
 * @param name          Qt name
 * @param f             window flags
 */
ImportSelector::ImportSelector(
	QWidget *parent, ImportTrackDataVector& trackDataList,
	const char *name, WFlags f) :
	QVBox(parent, name, f),
	m_trackDataVector(trackDataList)
{
	freedbDialog = 0;
	freedbCfg = 0;
#ifdef HAVE_TUNEPIMP
	m_musicBrainzDialog = 0;
	m_musicBrainzCfg = 0;
#endif
	importSource = None;
	header_parser = new ImportParser();
	track_parser = new ImportParser();
	setSpacing(6);
	setMargin(6);
	tab = new ImportTable(0, NumColumns, this);
#if QT_VERSION >= 300
	tab->setReadOnly(true);
	tab->setFocusStyle(QTable::FollowStyle);
#endif
	tab->setRowMovingEnabled(true);
	tab->setSelectionMode(QTable::NoSelection);
	QHeader* hHeader = tab->horizontalHeader();
	QHeader* vHeader = tab->verticalHeader();
	hHeader->setLabel(LengthColumn, i18n("Length"));
	hHeader->setLabel(TrackColumn, i18n("Track"));
	hHeader->setLabel(TitleColumn, i18n("Title"));
	hHeader->setLabel(ArtistColumn, i18n("Artist"));
	hHeader->setLabel(AlbumColumn, i18n("Album"));
	hHeader->setLabel(YearColumn, i18n("Year"));
	hHeader->setLabel(GenreColumn, i18n("Genre"));
	hHeader->setLabel(CommentColumn, i18n("Comment"));
	tab->adjustColumn(TrackColumn);
	tab->adjustColumn(YearColumn);

#if QT_VERSION >= 300
	QGroupBox *fmtbox = new QGroupBox(3, Qt::Vertical, i18n("Format"), this);
	formatComboBox = new QComboBox(false, fmtbox, "formatComboBox");
	formatComboBox->setEditable(true);
	headerLineEdit = new QLineEdit(fmtbox);
	trackLineEdit = new QLineEdit(fmtbox);
	connect(formatComboBox, SIGNAL(activated(int)), this, SLOT(setFormatLineEdit(int)));
#endif

	QWidget *butbox = new QWidget(this);
	QHBoxLayout *butlayout = new QHBoxLayout(butbox);
	butlayout->setSpacing(6);
	butlayout->setMargin(6);
	fileButton = new QPushButton(i18n("From F&ile"), butbox);
	butlayout->addWidget(fileButton);
	clipButton = new QPushButton(i18n("From Clip&board"), butbox);
	butlayout->addWidget(clipButton);
	freedbButton = new QPushButton(i18n("From &freedb.org"), butbox);
	butlayout->addWidget(freedbButton);
#ifdef HAVE_TUNEPIMP
	m_musicBrainzButton = new QPushButton(i18n("From &MusicBrainz"), butbox);
	butlayout->addWidget(m_musicBrainzButton);
#endif
	QSpacerItem *butspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
	                                       QSizePolicy::Minimum);
	butlayout->addItem(butspacer);
	QLabel *destLabel = new QLabel(butbox, "destLabel");
	destLabel->setText(i18n("D&estination:"));
	butlayout->addWidget(destLabel);
	destComboBox = new QComboBox(false, butbox, "destComboBox");
	destComboBox->insertItem("ID3v1", DestV1);
	destComboBox->insertItem("ID3v2", DestV2);
	destLabel->setBuddy(destComboBox);
	butlayout->addWidget(destComboBox);

	QWidget* diffbox = new QWidget(this);
	QHBoxLayout* checkLayout = new QHBoxLayout(diffbox);
	if (checkLayout) {
		mismatchCheckBox = new QCheckBox(diffbox);
		mismatchCheckBox->setText(
			i18n("Check maximum allowable time &difference (sec):"));
		maxDiffSpinBox = new QSpinBox(diffbox);
		maxDiffSpinBox->setMaxValue(9999);
		if (mismatchCheckBox && maxDiffSpinBox) {
			checkLayout->addSpacing(margin() * 2);
			checkLayout->addWidget(mismatchCheckBox);
			checkLayout->addWidget(maxDiffSpinBox);
			checkLayout->addStretch();
		}
	}

	connect(fileButton, SIGNAL(clicked()), this, SLOT(fromFile()));
	connect(clipButton, SIGNAL(clicked()), this, SLOT(fromClipboard()));
	connect(freedbButton, SIGNAL(clicked()), this, SLOT(fromFreedb()));
#ifdef HAVE_TUNEPIMP
	connect(m_musicBrainzButton, SIGNAL(clicked()), this, SLOT(fromMusicBrainz()));
#endif
	connect(vHeader, SIGNAL(indexChange(int, int, int)), this, SLOT(moveTableRow(int, int, int)));
	connect(mismatchCheckBox, SIGNAL(toggled(bool)), this, SLOT(showPreview()));
	connect(maxDiffSpinBox, SIGNAL(valueChanged(int)), this, SLOT(maxDiffChanged()));
}

/**
 * Destructor.
 */
ImportSelector::~ImportSelector()
{
	delete header_parser;
	delete track_parser;
	if (freedbDialog) {
		freedbDialog->disconnect();
		delete freedbDialog;
		freedbDialog = 0;
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
 * Look for album specific information (artist, album, year, genre) in
 * a header (e.g. in a freedb header).
 *
 * @param st standard tags to put resulting values in,
 *           fields which are not found are not touched.
 *
 * @return true if one or more field were found.
 */
bool ImportSelector::parseHeader(StandardTags &st)
{
	int pos = 0;
	header_parser->setFormat(
		importSource == Freedb ? "freedb_header" :
#if QT_VERSION >= 300
		headerLineEdit->text()
#else
		/* dummy format to indicate a header format */
		"%y(+\\d)"
#endif
		);
	return header_parser->getNextTags(text, st, pos);
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
		QFileDialog::getOpenFileName(QString::null, QString::null, this);
#endif
	if (!fn.isEmpty()) {
		QFile file(fn);
		if (file.open(IO_ReadOnly)) {
			QTextStream stream(&file);
			text = stream.read();
			if (!text.isNull()) {
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
	QClipboard *cb = QApplication::clipboard();
#if QT_VERSION >= 0x030100
	text = cb->text(QClipboard::Clipboard);
	if (!text.isNull() && updateTrackData(Clipboard)) {
		showPreview();
	} else {
		text = cb->text(QClipboard::Selection);
		if (!text.isNull()) {
			updateTrackData(Clipboard);
			showPreview();
		}
	}
#else
	text = cb->text();
	if (!text.isNull()) {
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
	if (!freedbDialog) {
		freedbDialog = new FreedbDialog(this);
		if (!freedbDialog) {
			return;
		}
		if (freedbCfg) {
			freedbDialog->setFreedbConfig(freedbCfg);
		}
		connect(freedbDialog, SIGNAL(albumDataReceived(QString)),
				this, SLOT(freedbAlbumDataReceived(QString)));
	}
	(void)freedbDialog->exec();
}

/**
 * Called when freedb.org album data is received.
 *
 * @param txt text containing album data from freedb.org
 */
void ImportSelector::freedbAlbumDataReceived(QString txt)
{
	text = txt;
	updateTrackData(Freedb);
	showPreview();
}

/**
 * Set the format lineedits to the format selected in the combo box.
 *
 * @param index current index of the combo box
 */
void ImportSelector::setFormatLineEdit(int index)
{
#if QT_VERSION >= 300
	headerLineEdit->setText(formatHeaders[index]);
	trackLineEdit->setText(formatTracks[index]);
#endif
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
	importSource = impSrc;
	(void)parseHeader(st_hdr);

	StandardTags st(st_hdr);
	bool start = true;
#if QT_VERSION >= 300
	ImportTrackDataVector::iterator
#else
	ImportTrackDataVector::Iterator
#endif
		it = m_trackDataVector.begin();
	bool atTrackDataListEnd = (it == m_trackDataVector.end());
	while (getNextTags(st, start)) {
		start = false;
		if (atTrackDataListEnd) {
			ImportTrackData trackData;
			trackData.setStandardTags(st);
#if QT_VERSION >= 300
			m_trackDataVector.push_back(trackData);
#else
			m_trackDataVector.append(trackData);
#endif
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
#if QT_VERSION >= 300
			it = m_trackDataVector.erase(it);
#else
			it = m_trackDataVector.remove(it);
#endif
		} else {
			(*it).setStandardTags(st);
			(*it).setImportDuration(0);
			++it;
		}
		atTrackDataListEnd = (it == m_trackDataVector.end());
	}

	if (!start) {
		/* start is false => tags were found */
		QValueList<int>* trackDuration = getTrackDurations();
		if (trackDuration) {
			it = m_trackDataVector.begin();
			for (
#if QT_VERSION >= 300
				 QValueList<int>::const_iterator
#else
				 QValueList<int>::ConstIterator
#endif
					 tdit = trackDuration->begin();
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
	tab->setNumRows(0);
	int row = 0;
	QHeader* vHeader = tab->verticalHeader();
	for (
#if QT_VERSION >= 300
		 ImportTrackDataVector::const_iterator
#else
		 ImportTrackDataVector::ConstIterator
#endif
			 it = m_trackDataVector.begin();
			 it != m_trackDataVector.end();
			 ++it) {
		tab->setNumRows(row + 1);
		int fileDuration = (*it).getFileDuration();
		if (fileDuration != 0) {
			vHeader->setLabel(row, TaggedFile::formatTime(fileDuration));
		}
		int importDuration = (*it).getImportDuration();
		if (importDuration != 0)
			tab->setText(row, LengthColumn, TaggedFile::formatTime(importDuration));
		if ((*it).track != -1) {
			QString trackStr;
			trackStr.setNum((*it).track);
			tab->setText(row, TrackColumn, trackStr);
		}
		if (!(*it).title.isNull())
			tab->setText(row, TitleColumn, (*it).title);
		if (!(*it).artist.isNull())
			tab->setText(row, ArtistColumn, (*it).artist);
		if (!(*it).album.isNull())
			tab->setText(row, AlbumColumn, (*it).album);
		if ((*it).year != -1) {
			QString yearStr;
			yearStr.setNum((*it).year);
			tab->setText(row, YearColumn, yearStr);
		}
		if ((*it).genre != -1) {
			QString genreStr(Genres::getName((*it).genre));
			tab->setText(row, GenreColumn, genreStr);
		}
		if (!(*it).comment.isNull())
			tab->setText(row, CommentColumn, (*it).comment);
		++row;
	}

	// make time difference check
	tab->clearMarks();
	bool diffCheckEnable;
	int maxDiff;
	getTimeDifferenceCheck(diffCheckEnable, maxDiff);
	if (diffCheckEnable) {
		row = 0;
		for (
#if QT_VERSION >= 300
			ImportTrackDataVector::const_iterator
#else
			ImportTrackDataVector::ConstIterator
#endif
				 it = m_trackDataVector.begin();
				 it != m_trackDataVector.end();
				 ++it) {
			int fileDuration = (*it).getFileDuration();
			int importDuration = (*it).getImportDuration();
			if (fileDuration != 0 && importDuration != 0) {
				int diff = fileDuration > importDuration ?
					fileDuration - importDuration : importDuration - fileDuration;
				if (diff > maxDiff) {
					tab->markRow(row);
				}
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
bool ImportSelector::getNextTags(StandardTags &st, bool start)
{
	static int pos = 0;
	if (start || pos == 0) {
		pos = 0;
		track_parser->setFormat(
			importSource == Freedb ? "freedb_tracks" :
#if QT_VERSION >= 300
			trackLineEdit->text()
#else
			/* dummy format to indicate a track format */
			"%t(\\d+)"
#endif
			, true);
	}
	return track_parser->getNextTags(text, st, pos);
}

/**
 * Set ID3v1 or ID3v2 tags as import destination.
 *
 * @param dest DestV1 or DestV2 for ID3v1 or ID3v2
 */
void ImportSelector::setDestination(ImportSelector::Destination dest)
{
	destComboBox->setCurrentItem((int)dest);
}

/**
 * Get import destination.
 *
 * @return DestV1 or DestV2 for ID3v1 or ID3v2.
 */
ImportSelector::Destination ImportSelector::getDestination()
{
	return (Destination)destComboBox->currentItem();
}

/**
 * Set import format regexp.
 *
 * @param names   import format names list
 * @param headers import format header regexps
 * @param tracks  import format track regexps
 * @param idx     selected index
 */
void ImportSelector::setImportFormat(const QStringList &names,
									 const QStringList &headers,
									 const QStringList &tracks,
									 int idx)
{
#if QT_VERSION >= 300
	formatHeaders = headers;
	formatTracks = tracks;
	formatComboBox->clear();
	formatComboBox->insertStringList(names);
	formatComboBox->setCurrentItem(idx);
	setFormatLineEdit(idx);
#endif
}

/**
 * Get import format regexp.
 *
 * @param name   import format name
 * @param header import format header regexp
 * @param track  import format track regexp
 *
 * @return index of current selection.
 */
int ImportSelector::getImportFormat(QString &name,
									QString &header,
									QString &track) const
{
#if QT_VERSION >= 300
	name = formatComboBox->currentText();
	header = headerLineEdit->text();
	track = trackLineEdit->text();
	return formatComboBox->currentItem();
#else
	return 0;
#endif
}

/**
 * Set freedb.org configuration.
 *
 * @param cfg freedb configuration.
 */
void ImportSelector::setFreedbConfig(const FreedbConfig *cfg)
{
	freedbCfg = cfg;
}

/**
 * Get freedb.org configuration.
 *
 * @param cfg freedb configuration.
 */
void ImportSelector::getFreedbConfig(FreedbConfig *cfg) const
{
	if (freedbDialog) {
		freedbDialog->getFreedbConfig(cfg);
	} else {
		// freedb dialog does not exist => copy configuration which was set
		if (freedbCfg && (freedbCfg != cfg)) {
			*cfg = *freedbCfg;
		}
	}
}

/**
 * Get list with track durations.
 *
 * @return list with track durations,
 *         0 if no track durations found.
 */
QValueList<int>* ImportSelector::getTrackDurations()
{
	QValueList<int>* lst = 0;
	if (header_parser && ((lst = header_parser->getTrackDurations()) != 0) &&
#if QT_VERSION >= 300
		(lst->size() > 0)
#else
		(lst->count() > 0)
#endif
		) {
		return lst;
	} else if (track_parser && ((lst = track_parser->getTrackDurations()) != 0) &&
#if QT_VERSION >= 300
		(lst->size() > 0)
#else
		(lst->count() > 0)
#endif
	   ) {
		return lst;
	} else {
		return 0;
	}
}

/**
 * Set time difference check configuration.
 *
 * @param enable  true to enable check
 * @param maxDiff maximum allowable time difference
 */ 
void ImportSelector::setTimeDifferenceCheck(bool enable, int maxDiff)
{
	mismatchCheckBox->setChecked(enable);
	maxDiffSpinBox->setValue(maxDiff);
}

/**
 * Get time difference check configuration.
 *
 * @param enable  true if check is enabled
 * @param maxDiff maximum allowed time difference
 */ 
void ImportSelector::getTimeDifferenceCheck(bool& enable, int& maxDiff) const
{
	enable = mismatchCheckBox->isChecked();
	maxDiff = maxDiffSpinBox->value();
}

/**
 * Called when the maximum time difference value is changed.
 */
void ImportSelector::maxDiffChanged() {
	if (mismatchCheckBox->isChecked()) {
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
	if (toIndex > fromIndex && toIndex > 0) {
		--toIndex;
	}
#if QT_VERSION >= 300
	int numTracks = static_cast<int>(m_trackDataVector.size());
#else
	int numTracks = static_cast<int>(m_trackDataVector.count());
#endif
	if (fromIndex < numTracks && toIndex < numTracks) {
		// swap elements but keep file durations
		ImportTrackData fromData(m_trackDataVector[fromIndex]);
		ImportTrackData toData(m_trackDataVector[toIndex]);
		int fromDuration = fromData.getFileDuration();
		fromData.setFileDuration(toData.getFileDuration());
		toData.setFileDuration(fromDuration);
		m_trackDataVector[fromIndex] = toData;
		m_trackDataVector[toIndex] = fromData;

		// redisplay the table
		showPreview();
	}
}

/**
 * Import from freedb.org and preview in table.
 */
void ImportSelector::fromMusicBrainz()
{
#ifdef HAVE_TUNEPIMP
	if (!m_musicBrainzDialog) {
		m_musicBrainzDialog = new MusicBrainzDialog(this, m_trackDataVector);
		if (!m_musicBrainzDialog) {
			return;
		}
		if (m_musicBrainzCfg) {
			m_musicBrainzDialog->setMusicBrainzConfig(m_musicBrainzCfg);
		}
		connect(m_musicBrainzDialog, SIGNAL(trackDataUpdated()),
						this, SLOT(showPreview()));
	}
	(void)m_musicBrainzDialog->exec();
#endif
}

#ifdef HAVE_TUNEPIMP
/**
 * Set MusicBrainz configuration.
 *
 * @param cfg MusicBrainz configuration.
 */
void ImportSelector::setMusicBrainzConfig(const MusicBrainzConfig* cfg)
{
	m_musicBrainzCfg = cfg;
}
#else
void ImportSelector::setMusicBrainzConfig(const MusicBrainzConfig*) {}
#endif

#ifdef HAVE_TUNEPIMP
/**
 * Get MusicBrainz configuration.
 *
 * @param cfg MusicBrainz configuration.
 */
void ImportSelector::getMusicBrainzConfig(MusicBrainzConfig* cfg) const
{
	if (m_musicBrainzDialog) {
		m_musicBrainzDialog->getMusicBrainzConfig(cfg);
	} else {
		// MusicBrainz dialog does not exist => copy configuration which was set
		if (m_musicBrainzCfg && (m_musicBrainzCfg != cfg)) {
			*cfg = *m_musicBrainzCfg;
		}
	}
}
#else
void ImportSelector::getMusicBrainzConfig(MusicBrainzConfig*) const {}
#endif
