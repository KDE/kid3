/**
 * \file importselector.h
 * Import selector widget.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2008  Urs Fleisch
 *
 * This file is part of Kid3.
 *
 * Kid3 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Kid3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef IMPORTSELECTOR_H
#define IMPORTSELECTOR_H

#include "importtrackdata.h"
#include "importparser.h"
#include "importconfig.h"
#include <qwidget.h>
#include <qstring.h>
#include <qstringlist.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <QList>
#else
#include <qvaluelist.h>
#endif

class QPushButton;
class QComboBox;
class QLineEdit;
class QCheckBox;
class QSpinBox;
class ImportParser;
class FreedbDialog;
class TrackTypeDialog;
class MusicBrainzDialog;
class MusicBrainzReleaseDialog;
class DiscogsDialog;
class AmazonDialog;
class ImportTable;

/**
 * Import selector widget.
 */
class ImportSelector : public QWidget
{
Q_OBJECT

public:
	/**
	 * Constructor.
	 *
	 * @param parent        parent widget
	 * @param trackDataList track data to be filled with imported values,
	 *                      is passed with durations of files set
	 */
	ImportSelector(QWidget* parent, ImportTrackDataVector& trackDataList);

	/**
	 * Destructor.
	 */
	~ImportSelector();

	/**
	 * Select the import server.
	 *
	 * @param server import server
	 */
	void setImportServer(ImportConfig::ImportServer server);

	/**
	 * Clear dialog data.
	 */
	void clear();

	/**
	 * Look for album specific information (artist, album, year, genre) in
	 * a header.
	 *
	 * @param frames frames to put resulting values in,
	 *           fields which are not found are not touched.
	 *
	 * @return true if one or more field were found.
	 */
	bool parseHeader(FrameCollection& frames);

	/**
	 * Get next line as frames from imported file or clipboard.
	 *
	 * @param frames frames
	 * @param start true to start with the first line, false for all
	 *              other lines
	 *
	 * @return true if ok (result in st),
	 *         false if end of file reached.
	 */
	bool getNextTags(FrameCollection& frames, bool start);

	/**
	 * Get import destination.
	 *
	 * @return DestV1, DestV2 or DestV1V2 for ID3v1, ID3v2 or both.
	 */
	ImportConfig::ImportDestination getDestination();

	/**
	 * Set import destination.
	 *
	 * @param dest DestV1, DestV2 or DestV1V2 for ID3v1, ID3v2 or both
	 */
	void setDestination(ImportConfig::ImportDestination dest);

	/**
	 * Get list with track durations.
	 *
	 * @return list with track durations,
	 *         0 if no track durations found.
	 */
	TrackDurationList* getTrackDurations();

	/**
	 * Get time difference check configuration.
	 *
	 * @param enable  true if check is enabled
	 * @param maxDiff maximum allowed time difference
	 */ 
	void getTimeDifferenceCheck(bool& enable, int& maxDiff) const;

	/**
	 * Import from a file.
	 *
	 * @param fn file name
	 *
	 * @return true if ok.
	 */
	bool importFromFile(const QString& fn);

	/**
	 * Save the local settings to the configuration.
	 *
	 * @param width  window width
	 * @param height window height
	 */
	void saveConfig(int width, int height);

	/**
	 * List with line formats.
	 * The following codes are used before the () expressions.
	 * %s title (song)
	 * %l album
	 * %a artist
	 * %c comment
	 * %y year
	 * %t track
	 * %g genre
	 */
	static const char** s_lineFmtList;

	/**
	 * Get last directory used for import or export.
	 * @return import directory.
	 */
	static QString getImportDir() { return s_importDir; }

	/**
	 * Set last directory used for import or export.
	 * @param dir import directory
	 */
	static void setImportDir(const QString& dir) { s_importDir = dir; }

public slots:
	/**
	 * Called when the maximum time difference value is changed.
	 */
	void maxDiffChanged();

	/**
	 * Move a table row.
	 *
	 * The first parameter @a section is not used.
	 * @param fromIndex index of position moved from
	 * @param toIndex   index of position moved to
	 */
	void moveTableRow(int, int fromIndex, int toIndex);

	/**
	 * Let user select file, assign file contents to text and preview in
	 * table.
	 */
	void fromFile();

	/**
	 * Assign clipboard contents to text and preview in table.
	 */
	void fromClipboard();

	/**
	 * Import from server and preview in table.
	 */
	void fromServer();

	/**
	 * Import from freedb.org and preview in table.
	 */
	void fromFreedb();

	/**
	 * Import from TrackType.org and preview in table.
	 */
	void fromTrackType();

	/**
	 * Import from MusicBrainz and preview in table.
	 */
	void fromMusicBrainz();

	/**
	 * Import from MusicBrainz release database and preview in table.
	 */
	void fromMusicBrainzRelease();

	/**
	 * Import from www.discogs.com and preview in table.
	 */
	void fromDiscogs();

	/**
	 * Import from www.amazon.com and preview in table.
	 */
	void fromAmazon();

	/**
	 * Set the format lineedits to the format selected in the combo box.
	 *
	 * @param index current index of the combo box
	 */
	void setFormatLineEdit(int index);

	/**
	 * Show fields to import in text as preview in table.
	 */
	void showPreview();

	/**
	 * Match import data with length.
	 */
	void matchWithLength();

	/**
	 * Match import data with track number.
	 */
	void matchWithTrack();

	/**
	 * Match import data with title.
	 */
	void matchWithTitle();

private:
	enum TabColumn {
		LengthColumn, TrackColumn, TitleColumn, ArtistColumn,
		AlbumColumn, YearColumn, GenreColumn, CommentColumn, NumColumns
	};

	enum ImportSource {
		None, File, Clipboard
	};

	/**
	 * Update track data list with imported tags.
	 *
	 * @param impSrc import source
	 *
	 * @return true if tags were found.
	 */
	bool updateTrackData(ImportSource impSrc);

	/**
	 * Clear columns for additional (non-standard) tags.
	 */
	void clearAdditionalFrameColumns();

	/**
	 * Add columns for additional (non-standard) tags.
	 *
	 * @param frames frames
	 * @param row    current table row
	 */
	void addAdditionalFrameColumns(const FrameCollection& frames, int row);

	/**
	 * Set the format combo box and line edits from the configuration.
	 */
	void setFormatFromConfig();

	/** From File button */
	QPushButton* m_fileButton;
	/** From Clipboard button */
	QPushButton* m_clipButton;
	/** From Server button */
	QPushButton* m_serverButton;
	/** Match with Length button */
	QPushButton* m_lengthButton;
	/** Match with Track button */
	QPushButton* m_trackButton;
	/** Match with Title button */
	QPushButton* m_titleButton;
	/** Preview table */
	ImportTable* m_tab;
	/** contents of imported file/clipboard */
	QString m_text;
	/** combobox with import servers */
	QComboBox* m_serverComboBox;
	/** combobox with import destinations */
	QComboBox* m_destComboBox;
	/** combobox with import formats */
	QComboBox* m_formatComboBox;
	/** LineEdit for header regexp */
	QLineEdit* m_headerLineEdit;
	/** LineEdit for track regexp */
	QLineEdit* m_trackLineEdit;
	QCheckBox* m_mismatchCheckBox;
	QSpinBox* m_maxDiffSpinBox;
	/** header parser object */
	ImportParser* m_headerParser;
	/** track parser object */
	ImportParser* m_trackParser;
	/** header format regexps */
	QStringList m_formatHeaders;
	/** track format regexps */
	QStringList m_formatTracks;
	/** freedb.org import dialog */
	FreedbDialog* m_freedbDialog;
	/** TrackType.org import dialog */
	TrackTypeDialog* m_trackTypeDialog;
	/** MusicBrainz import dialog */
	MusicBrainzDialog* m_musicBrainzDialog;
	/** MusicBrainz release import dialog */
	MusicBrainzReleaseDialog* m_musicBrainzReleaseDialog;
	/** Discogs import dialog */
	DiscogsDialog* m_discogsDialog;
	/** Amazon import dialog */
	AmazonDialog* m_amazonDialog;
	/** import source */
	ImportSource m_importSource;
	/** track data */
	ImportTrackDataVector& m_trackDataVector;
	/** names for additional tag columns */
#if QT_VERSION >= 0x040000
	QList<QString> m_additonalColumnNames;
#else
	QValueList<QString> m_additonalColumnNames;
#endif

	/** Last directory used for import or export. */
	static QString s_importDir;
};

#endif
