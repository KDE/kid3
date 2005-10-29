/**
 * \file freedbdialog.cpp
 * freedb.org import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 3 Jan 2004
 */

#include "config.h"
#ifdef CONFIG_USE_KDE
#include <klocale.h>
#else
#define i18n(s) tr(s)
#define I18N_NOOP(s) QT_TR_NOOP(s)
#endif

#include <qlayout.h>
#include <qhbox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlistbox.h>
#include <qlabel.h>
#include <qstatusbar.h>
#include "freedbconfig.h"
#include "freedbclient.h"
#include "freedbdialog.h"

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
	AlbumListItem(QListBox *listbox, const QString &text,
				  const QString &cat, const QString &idStr) : 
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


/**
 * Constructor.
 *
 * @param parent  parent widget
 * @param caption dialog title
 */
FreedbDialog::FreedbDialog(QWidget *parent, QString caption)
	: QDialog(parent, "freedb", true),
		m_windowWidth(0), m_windowHeight(0)
{
	if (caption.isNull()) {
		caption = "freedb.org";
	}
	setCaption(caption);

	QVBoxLayout *vlayout = new QVBoxLayout(this);
	if (!vlayout) {
		return ;
	}
	vlayout->setSpacing(6);
	vlayout->setMargin(6);

	QHBoxLayout *findLayout = new QHBoxLayout(vlayout);
	findLineEdit = new QLineEdit(this);
	findButton = new QPushButton(i18n("&Find"), this);
	QPushButton *closeButton = new QPushButton(i18n("&Close"), this);
	if (findLayout && findLineEdit && findButton) {
		findButton->setDefault(true);
		findLayout->addWidget(findLineEdit);
		findLayout->addWidget(findButton);
		connect(findButton, SIGNAL(clicked()), this, SLOT(slotFind()));
		findLayout->addWidget(closeButton);
		connect(closeButton, SIGNAL(clicked()), this, SLOT(saveWindowSizeAndClose()));
	}
	QHBoxLayout *serverLayout = new QHBoxLayout(vlayout);
	QLabel *serverLabel = new QLabel(i18n("&Server:"), this);
	serverComboBox = new QComboBox(this);
	QLabel *cgiLabel = new QLabel(i18n("C&GI Path:"), this);
	cgiLineEdit = new QLineEdit(this);
	if (serverLayout && serverLabel && serverComboBox &&
		cgiLabel && cgiLineEdit) {
		static const char *serverList[] = {
			"freedb.freedb.org:80",
			"at.freedb.org:80",
			"au.freedb.org:80",
			"ca.freedb.org:80",
			"ca2.freedb.org:80",
			"de.freedb.org:80",
			"de2.freedb.org:80",
			"es.freedb.org:80",
			"fi.freedb.org:80",
			"lu.freedb.org:80",
			"ru.freedb.org:80",
			"uk.freedb.org:80",
			"us.freedb.org:80",
			0                  // end of StrList
		};
		serverComboBox->insertStrList(serverList);
		serverComboBox->setEditable(true);
		serverLayout->addWidget(serverLabel);
		serverLayout->addWidget(serverComboBox);
		serverLayout->addWidget(cgiLabel);
		serverLayout->addWidget(cgiLineEdit);
		cgiLabel->setBuddy(cgiLineEdit);
		serverLabel->setBuddy(serverComboBox);
	}
	QHBoxLayout *proxyLayout = new QHBoxLayout(vlayout);
	proxyCheckBox = new QCheckBox(i18n("&Proxy:"), this);
	proxyLineEdit = new QLineEdit(this);
	if (proxyLayout && proxyCheckBox && proxyLineEdit) {
		proxyLayout->addWidget(proxyCheckBox);
		proxyLayout->addWidget(proxyLineEdit);
	}
	albumListBox = new QListBox(this);
	if (albumListBox) {
		vlayout->addWidget(albumListBox);
		connect(albumListBox, SIGNAL(selectionChanged(QListBoxItem*)),
				this, SLOT(requestTrackList(QListBoxItem*)));
		connect(albumListBox, SIGNAL(selected(int)),
				this, SLOT(requestTrackList(int)));
	}

	statusBar = new QStatusBar(this);
	if (statusBar) {
		vlayout->addWidget(statusBar);
		client = new FreedbClient(statusBar);
		connect(client, SIGNAL(findFinished(QString)),
				this, SLOT(slotFindFinished(QString)));
		connect(client, SIGNAL(albumFinished(QString)),
				this, SLOT(slotAlbumFinished(QString)));
	}
}

/**
 * Destructor.
 */
FreedbDialog::~FreedbDialog()
{
	client->disconnect();
	delete client;
}

/**
 * Get string with server and port.
 *
 * @return "servername:port".
 */
QString FreedbDialog::getServer() const
{
	QString server(serverComboBox->currentText());
	if (server.isEmpty()) {
		server = "freedb.freedb.org:80";
	}
	return server;
}

/**
 * Set string with server and port.
 *
 * @param srv "servername:port"
 */
void FreedbDialog::setServer(const QString &srv)
{
#if QT_VERSION >= 300
	serverComboBox->setCurrentText(srv);
#else
	serverComboBox->setEditText(srv);
#endif
}

/**
 * Get string with CGI path.
 *
 * @return CGI path, e.g. "/~cddb/cddb.cgi".
 */
QString FreedbDialog::getCgiPath() const
{
	QString cgi(cgiLineEdit->text());
	if (cgi.isEmpty()) {
		cgi = "/~cddb/cddb.cgi";
	}
	return cgi;
}

/**
 * Set string with CGI path.
 *
 * @param cgi CGI path, e.g. "/~cddb/cddb.cgi".
 */
void FreedbDialog::setCgiPath(const QString &cgi)
{
	cgiLineEdit->setText(cgi);
}

/**
 * Get proxy.
 *
 * @param used is set to true if proxy is used
 *
 * @return proxy, e.g. "myproxy:8080".
 */
QString FreedbDialog::getProxy(bool *used) const
{
	*used = proxyCheckBox->isChecked();
	return proxyLineEdit->text();
}

/**
 * Set proxy.
 *
 * @param proxy proxy, e.g. "myproxy:8080"
 * @param used is set to true if proxy is used
 */
void FreedbDialog::setProxy(const QString &proxy, bool used)
{
	proxyCheckBox->setChecked(used);
	proxyLineEdit->setText(proxy);
}

/**
 * Set freedb.org configuration.
 *
 * @param cfg freedb configuration.
 */
void FreedbDialog::setFreedbConfig(const FreedbConfig *cfg)
{
	setProxy(cfg->proxy, cfg->useProxy);
	setServer(cfg->server);
	setCgiPath(cfg->cgiPath);
	if (cfg->m_windowWidth > 0 && cfg->m_windowHeight > 0) {
		resize(cfg->m_windowWidth, cfg->m_windowHeight);
	}
}

/**
 * Get freedb.org configuration.
 *
 * @param cfg freedb configuration.
 */
void FreedbDialog::getFreedbConfig(FreedbConfig *cfg) const
{
	cfg->proxy = getProxy(&cfg->useProxy);
	cfg->server = getServer();
	cfg->cgiPath = getCgiPath();
	if (m_windowWidth > 0 && m_windowHeight > 0) {
		cfg->m_windowWidth = m_windowWidth;
		cfg->m_windowHeight = m_windowHeight;
	}
}

/**
 * Find keyword in freedb.
 */
void FreedbDialog::slotFind()
{
	FreedbConfig cfg;
	getFreedbConfig(&cfg);
	client->find(&cfg, findLineEdit->text());
}

/**
 * Process finished find request.
 *
 * @param searchStr search data received
 */
void FreedbDialog::slotFindFinished(QString searchStr)
{
/*
read line with the format "http://www.freedb.org/freedb_search_fmt.php?cat=category&id=id">albumartist</a>"
  if albumartist ">otherversion</font>" => other version
  else artist_album
*/
		int catStart, catEnd, idStart, idEnd, albumArtistStart, albumArtistEnd;
		static const char catStr[] =
			"http://www.freedb.org/freedb_search_fmt.php?cat=";
		static const char idStr[] = "&id=";
		static const char albumArtistStr[] = "\">";
		albumListBox->clear();
		albumArtistEnd = 0; // start search at begin
		while ((catStart = searchStr.find(catStr, albumArtistEnd)) != -1) {
			catStart += sizeof(catStr) - 1;
			catEnd = searchStr.find(idStr, catStart);
			if (catEnd == -1) break;
			idStart = catEnd + sizeof(idStr) - 1;
			idEnd = searchStr.find(albumArtistStr, idStart);
			if (idEnd == -1) break;
			albumArtistStart = idEnd + sizeof(albumArtistStr) - 1;
			if (searchStr.mid(albumArtistStart, 5) == "<font") {
				albumArtistStart = searchStr.find('>', albumArtistStart);
				if (albumArtistStart == -1) break;
				++albumArtistStart;
			}
			albumArtistEnd = searchStr.find('<', albumArtistStart);
			if (albumArtistEnd == -1) break;

			new AlbumListItem(
				albumListBox,
				searchStr.mid(albumArtistStart,
							  albumArtistEnd - albumArtistStart),
				searchStr.mid(catStart, catEnd - catStart),
				searchStr.mid(idStart, idEnd - idStart));
		}
}

/**
 * Process finished album data.
 *
 * @param albumStr album track data received
 */
void FreedbDialog::slotAlbumFinished(QString albumStr)
{
	emit albumDataReceived(albumStr);
}


/**
 * Request track list from freedb server.
 *
 * @param li list box item containing an AlbumListItem
 */
void FreedbDialog::requestTrackList(QListBoxItem *li)
{
	AlbumListItem *ali;
	if ((ali = dynamic_cast<AlbumListItem *>(li)) != 0) {
		FreedbConfig cfg;
		getFreedbConfig(&cfg);
		client->getTrackList(&cfg, ali->getCategory(), ali->getId());
	}
}

/**
 * Request track list from freedb server.
 *
 * @param index index of list box item containing an AlbumListItem
 */
void FreedbDialog::requestTrackList(int index)
{
	requestTrackList(albumListBox->item(index));
}

/**
 * Save the size of the window and close it.
 */
void FreedbDialog::saveWindowSizeAndClose()
{
	m_windowWidth = size().width();
	m_windowHeight = size().height();
	accept();
}
