/**
 * \file freedbdialog.h
 * freedb.org import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 3 Jan 2004
 */

#ifndef FREEDBDIALOG_H
#define FREEDBDIALOG_H

#include "config.h"
#include <qdialog.h>
#include <qstring.h>

class QLineEdit;
class QComboBox;
class QPushButton;
class QCheckBox;
class QListBox;
class QListBoxItem;
class QStatusBar;
class FreedbConfig;
class FreedbClient;

/**
 * freedb.org import dialog.
 */
class FreedbDialog : public QDialog
{
Q_OBJECT

public:
	/**
	 * Constructor.
	 *
	 * @param parent  parent widget
	 * @param caption dialog title
	 */
	FreedbDialog(QWidget *parent = 0, QString caption = QString::null);
	/**
	 * Destructor.
	 */
	~FreedbDialog();
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
	void setServer(const QString &srv);
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
	void setCgiPath(const QString &cgi);
	/**
	 * Get proxy.
	 *
	 * @param used is set to true if proxy is used
	 *
	 * @return proxy, e.g. "myproxy:8080".
	 */
	QString getProxy(bool *used) const;
	/**
	 * Set proxy.
	 *
	 * @param proxy proxy, e.g. "myproxy:8080"
	 * @param used is set to true if proxy is used
	 */
	void setProxy(const QString &proxy, bool used);
	/**
	 * Set freedb.org configuration.
	 *
	 * @param cfg freedb configuration.
	 */
	void setFreedbConfig(const FreedbConfig *cfg);
	/**
	 * Get freedb.org configuration.
	 *
	 * @param cfg freedb configuration.
	 */
	void getFreedbConfig(FreedbConfig *cfg) const;
	/**
	 * Set a find string from artist and album information.
	 *
	 * @param artist artist
	 * @param album  album
	 */
	void setArtistAlbum(const QString& artist, const QString& album);
private slots:
	/**
	 * Find keyword in freedb.
	 */
	void slotFind();
	/**
	 * Process finished find request.
	 *
	 * @param searchStr search data received
	 */
	void slotFindFinished(QString searchStr);
	/**
	 * Process finished album data.
	 *
	 * @param albumStr album track data received
	 */
	void slotAlbumFinished(QString albumStr);
	/**
	 * Request track list from freedb server.
	 *
	 * @param li list box item containing an AlbumListItem
	 */
	void requestTrackList(QListBoxItem *li);
	/**
	 * Request track list from freedb server.
	 *
	 * @param index index of list box item containing an AlbumListItem
	 */
	void requestTrackList(int index);
	/**
	 * Save the size of the window and close it.
	 */
	void saveWindowSizeAndClose();
signals:
	/**
	 * Emitted when album data is received.
	 * Parameter: text containing album data from freedb.org
	 */
	void albumDataReceived(QString);
private:
	QComboBox *findLineEdit;
	QPushButton *findButton;
	QComboBox *serverComboBox;
	QLineEdit *cgiLineEdit;
	QCheckBox *proxyCheckBox;
	QLineEdit *proxyLineEdit;
	QListBox *albumListBox;
	QStatusBar *statusBar;
	FreedbClient *client;
	int m_windowWidth;
	int m_windowHeight;
};

#endif
