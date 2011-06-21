/**
 * \file serverimporter.h
 * Generic baseclass to import from a server.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Oct 2006
 *
 * Copyright (C) 2006-2011  Urs Fleisch
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

#ifndef SERVERIMPORTER_H
#define SERVERIMPORTER_H

#include "importclient.h"
#include <QString>
#include <QStandardItem>

class QStandardItemModel;
class ServerImporterConfig;
class ImportClient;
class TrackDataModel;

/**
 * Generic baseclass to import from an external source.
 */
class ServerImporter : public ImportClient
{
	Q_OBJECT

public:
	/**
	 * Constructor.
	 *
	 * @param parent  parent object
	 * @param trackDataModel track data to be filled with imported values
	 */
	ServerImporter(QObject* parent,
								 TrackDataModel* trackDataModel);

	/**
	 * Destructor.
	 */
	virtual ~ServerImporter();

	/**
	 * Name of import source.
	 * @return name.
	 */
	virtual QString name() const = 0;

	/** NULL-terminated array of server strings, 0 if not used */
	virtual const char** serverList() const;

	/** default server, 0 to disable */
	virtual const char* defaultServer() const;

	/** default CGI path, 0 to disable */
	virtual const char* defaultCgiPath() const;

	/** anchor to online help, 0 to disable */
	virtual const char* helpAnchor() const;

	/** configuration, 0 if not used */
	virtual ServerImporterConfig* config() const;

	/** additional tags option, false if not used */
	virtual bool additionalTags() const;

	/**
	 * Parse result of find request and populate m_albumListBox with results.
	 * This method has to be reimplemented for the specific result data.
	 *
	 * @param searchStr search data received
	 */
	virtual void parseFindResults(const QByteArray& searchStr) = 0;

	/**
	 * Parse result of album request and populate m_trackDataModel with results.
	 * This method has to be reimplemented for the specific result data.
	 *
	 * @param albumStr album data received
	 */
	virtual void parseAlbumResults(const QByteArray& albumStr) = 0;

	/**
	 * Get model with album list.
	 *
	 * @return album list item model.
	 */
	QStandardItemModel* getAlbumListModel() const { return m_albumListModel; }

	/**
	 * Clear model data.
	 */
	void clear();

	/**
	 * Get additional tags option.
	 *
	 * @return true if additional tags are enabled.
	 */
	bool getAdditionalTags() const { return m_additionalTagsEnabled; }

	/**
	 * Set additional tags option.
	 *
	 * @param enable true if additional tags are enabled
	 */
	void setAdditionalTags(bool enable) { m_additionalTagsEnabled = enable; }

	/**
	 * Get cover art option.
	 *
	 * @return true if cover art are enabled.
	 */
	bool getCoverArt() const { return m_coverArtEnabled; }

	/**
	 * Set cover art option.
	 *
	 * @param enable true if cover art are enabled
	 */
	void setCoverArt(bool enable) { m_coverArtEnabled = enable; }

	/**
	 * Replace HTML entities in a string.
	 *
	 * @param str string with HTML entities (e.g. &quot;)
	 *
	 * @return string with replaced HTML entities.
	 */
	static QString replaceHtmlEntities(QString str);

	/**
	 * Replace HTML entities and remove HTML tags.
	 *
	 * @param str string containing HTML
	 *
	 * @return clean up string
	 */
	static QString removeHtml(QString str);

protected:
	QStandardItemModel* m_albumListModel; /**< albums to select */
	TrackDataModel* m_trackDataModel; /**< model with tracks to import */

private:
	bool m_additionalTagsEnabled;
	bool m_coverArtEnabled;
};

/**
 * QStandardItem subclass for album list.
 */
class AlbumListItem : public QStandardItem {
public:
	/**
	 * Constructor.
	 * @param text    title
	 * @param cat     category
	 * @param idStr   ID
	 */
	AlbumListItem(const QString& text,
				  const QString& cat, const QString& idStr) : 
		QStandardItem(text) {
		setData(cat, Qt::UserRole + 1);
		setData(idStr, Qt::UserRole + 2);
	}

	/**
	 * Get category.
	 * @return category.
	 */
	QString getCategory() const { return data(Qt::UserRole + 1).toString(); }

	/**
	 * Get ID.
	 * @return ID.
	 */
	QString getId() const { return data(Qt::UserRole + 2).toString(); }
};

#endif
