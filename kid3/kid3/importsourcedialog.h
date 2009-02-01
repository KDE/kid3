/**
 * \file importsourcedialog.h
 * Generic dialog to import from an external source.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Oct 2006
 *
 * Copyright (C) 2006-2009  Urs Fleisch
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

#ifndef IMPORTSOURCEDIALOG_H
#define IMPORTSOURCEDIALOG_H

#include "config.h"
#include "importtrackdata.h"
#include <qdialog.h>
#include <qstring.h>
#include "qtcompatmac.h"
#if QT_VERSION >= 0x040000
#include <QListWidget>
#else
#include <qlistbox.h>
#endif

class QLineEdit;
class QComboBox;
class QPushButton;
class QCheckBox;
class QStatusBar;
class ImportSourceConfig;
class ImportSourceClient;

/**
 * Generic dialog to import from an external source.
 */
class ImportSourceDialog : public QDialog
{
Q_OBJECT

public:
	/**
	 * Properties of dialog.
	 */
	struct Properties {
		const char** serverList;    /**< NULL-terminated array of server strings, 0 if not used */
		const char* defaultServer;  /**< default server, 0 to disable */
		const char* defaultCgiPath; /**< default CGI path, 0 to disable */
		const char* helpAnchor;     /**< anchor to online help, 0 to disable */
		ImportSourceConfig* cfg;    /**< configuration, 0 if not used */
		bool additionalTags;        /**< additional tags option, false if not used */
	};

	/**
	 * Constructor.
	 *
	 * @param parent  parent widget
	 * @param caption dialog title
	 * @param trackDataVector track data to be filled with imported values
	 * @param client  client to use, this object takes ownership of it
	 * @param props   constant dialog properties, must exist while dialog exists
	 */
	ImportSourceDialog(QWidget* parent, QString caption,
										 ImportTrackDataVector& trackDataVector,
										 ImportSourceClient* client,
										 const Properties& props);

	/**
	 * Destructor.
	 */
	virtual ~ImportSourceDialog();

	/**
	 * Parse result of find request and populate m_albumListBox with results.
	 * This method has to be reimplemented for the specific result data.
	 *
	 * @param searchStr search data received
	 */
	virtual void parseFindResults(const QByteArray& searchStr) = 0;

	/**
	 * Parse result of album request and populate m_trackDataVector with results.
	 * This method has to be reimplemented for the specific result data.
	 *
	 * @param albumStr album data received
	 */
	virtual void parseAlbumResults(const QByteArray& albumStr) = 0;

	/**
	 * Clear dialog data.
	 */
	void clear();

	/**
	 * Get string with server and port.
	 *
	 * @return "servername:port".
	 */
	QString getServer() const;

	/**
	 * Set string with server and port.
	 *
	 * @param srv "servername:port"
	 */
	void setServer(const QString& srv);

	/**
	 * Get string with CGI path.
	 *
	 * @return CGI path, e.g. "/~cddb/cddb.cgi".
	 */
	QString getCgiPath() const;

	/**
	 * Set string with CGI path.
	 *
	 * @param cgi CGI path, e.g. "/~cddb/cddb.cgi".
	 */
	void setCgiPath(const QString& cgi);

	/**
	 * Get additional tags option.
	 *
	 * @return true if additional tags are enabled.
	 */
	bool getAdditionalTags() const;

	/**
	 * Set additional tags option.
	 *
	 * @param enable true if additional tags are enabled
	 */
	void setAdditionalTags(bool enable);

	/**
	 * Get cover art option.
	 *
	 * @return true if cover art are enabled.
	 */
	bool getCoverArt() const;

	/**
	 * Set cover art option.
	 *
	 * @param enable true if cover art are enabled
	 */
	void setCoverArt(bool enable);

	/**
	 * Set a find string from artist and album information.
	 *
	 * @param artist artist
	 * @param album  album
	 */
	void setArtistAlbum(const QString& artist, const QString& album);

private slots:
	/**
	 * Query a search for a keyword from the server.
	 */
	void slotFind();

	/**
	 * Process finished find request.
	 *
	 * @param searchStr search data received
	 */
	void slotFindFinished(const QByteArray& searchStr);

	/**
	 * Process finished album data.
	 *
	 * @param albumStr album track data received
	 */
	void slotAlbumFinished(const QByteArray& albumStr);

	/**
	 * Request track list from server.
	 *
	 * @param li list box item containing an AlbumListItem
	 */
#if QT_VERSION >= 0x040000
	void requestTrackList(QListWidgetItem* li);
#else
	void requestTrackList(QListBoxItem* li);
#endif

	/**
	 * Request track list from server.
	 *
	 * @param index index of list box item containing an AlbumListItem
	 */
	void requestTrackList(int index);

	/**
	 * Save the local settings to the configuration.
	 */
	void saveConfig();

	/**
	 * Show help.
	 */
	void showHelp();

	/**
	 * Display message in status bar.
	 *
	 * @param msg status message
	 */
	void showStatusMessage(const QString& msg);

signals:
	/**
	 * Emitted when the m_trackDataVector was updated with new imported data.
	 */
	void trackDataUpdated();

protected:
#if QT_VERSION >= 0x040000
	QListWidget* m_albumListBox; /**< list box with albums to select */
#else
	QListBox* m_albumListBox; /**< list box with albums to select */
#endif
	ImportTrackDataVector& m_trackDataVector; /**< vector with tracks to import */

private:
	/**
	 * Get the local configuration.
	 *
	 * @param cfg configuration
	 */
	void getImportSourceConfig(ImportSourceConfig* cfg) const;

	QComboBox* m_artistLineEdit;
	QComboBox* m_albumLineEdit;
	QPushButton* m_findButton;
	QComboBox* m_serverComboBox;
	QLineEdit* m_cgiLineEdit;
	QCheckBox* m_additionalTagsCheckBox;
	QCheckBox* m_coverArtCheckBox;
	QStatusBar* m_statusBar;
	ImportSourceClient* m_client;
	const Properties& m_props;
};

/** The base class depends on the Qt version and is a list widget item. */
#if QT_VERSION >= 0x040000
typedef QListWidgetItem AlbumListItemBaseClass;
#else
typedef QListBoxText AlbumListItemBaseClass;
#endif

/**
 * QListBoxItem subclass for album list.
 */
class AlbumListItem : public AlbumListItemBaseClass {
public:
	/**
	 * Constructor.
	 * @param listbox listbox
	 * @param text    title
	 * @param cat     category
	 * @param idStr   ID
	 */
#if QT_VERSION >= 0x040000
	AlbumListItem(QListWidget* listbox, const QString& text,
				  const QString& cat, const QString& idStr) : 
		QListWidgetItem(text, listbox), m_category(cat), m_id(idStr) {}
#else
	AlbumListItem(QListBox* listbox, const QString& text,
				  const QString& cat, const QString& idStr) : 
		QListBoxText(listbox, text), m_category(cat), m_id(idStr) {}
#endif

	/**
	 * Get category.
	 * @return category.
	 */
	QString getCategory() const { return m_category; }

	/**
	 * Get ID.
	 * @return ID.
	 */
	QString getId() const { return m_id; }

private:
	QString m_category;
	QString m_id;
};

#endif
