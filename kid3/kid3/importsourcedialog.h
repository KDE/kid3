/**
 * \file importsourcedialog.h
 * Generic dialog to import from an external source.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Oct 2006
 */

#ifndef IMPORTSOURCEDIALOG_H
#define IMPORTSOURCEDIALOG_H

#include "config.h"
#include "importtrackdata.h"
#include <qdialog.h>
#include <qlistbox.h>
#include <qstring.h>

class QLineEdit;
class QComboBox;
class QPushButton;
class QCheckBox;
class QListBox;
class QListBoxItem;
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
	virtual void parseFindResults(const QCString& searchStr) = 0;

	/**
	 * Parse result of album request and populate m_trackDataVector with results.
	 * This method has to be reimplemented for the specific result data.
	 *
	 * @param albumStr album data received
	 */
	virtual void parseAlbumResults(const QCString& albumStr) = 0;

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
	void slotFindFinished(QCString searchStr);

	/**
	 * Process finished album data.
	 *
	 * @param albumStr album track data received
	 */
	void slotAlbumFinished(QCString albumStr);

	/**
	 * Request track list from server.
	 *
	 * @param li list box item containing an AlbumListItem
	 */
	void requestTrackList(QListBoxItem* li);

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

signals:
	/**
	 * Emitted when the m_trackDataVector was updated with new imported data.
	 */
	void trackDataUpdated();

protected:
	QListBox* m_albumListBox;
	ImportTrackDataVector& m_trackDataVector;

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
	QStatusBar* m_statusBar;
	ImportSourceClient* m_client;
	const Properties& m_props;
};

/**
 * QListBoxItem subclass for album list.
 */
class AlbumListItem : public QListBoxText {
public:
	/**
	 * Constructor.
	 * @param listbox listbox
	 * @param text    title
	 * @param cat     category
	 * @param idStr   ID
	 */
	AlbumListItem(QListBox* listbox, const QString& text,
				  const QString& cat, const QString& idStr) : 
		QListBoxText(listbox, text), category(cat), id(idStr) {}
	/**
	 * Get category.
	 * @return category.
	 */
	QString getCategory() { return category; }
	/**
	 * Get ID.
	 * @return ID.
	 */
	QString getId() { return id; }
private:
	QString category;
	QString id;
};

#endif
