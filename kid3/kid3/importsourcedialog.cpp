/**
 * \file importsourcedialog.cpp
 * Generic dialog to import from an external source.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Oct 2006
 */

#include "config.h"
#ifdef CONFIG_USE_KDE
#include <klocale.h>
#else
#define i18n(s) tr(s)
#define I18N_NOOP(s) QT_TR_NOOP(s)
#endif

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qstatusbar.h>
#include <qregexp.h>
#if QT_VERSION >= 0x040000
#include <Q3HBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#else
#include <qhbox.h>
#endif
#include "importsourceconfig.h"
#include "importsourceclient.h"
#include "importsourcedialog.h"
#include "kid3.h"

/**
 * Constructor.
 *
 * @param parent  parent widget
 * @param caption dialog title
 * @param trackDataVector track data to be filled with imported values
 * @param client  client to use, this object takes ownership of it
 * @param props   constant dialog properties, must exist while dialog exists
 */
ImportSourceDialog::ImportSourceDialog(QWidget* parent, QString caption,
																			 ImportTrackDataVector& trackDataVector,
																			 ImportSourceClient* client,
																			 const Properties& props)
	: QDialog(parent, "importsource", true), m_trackDataVector(trackDataVector),
		m_serverComboBox(0), m_cgiLineEdit(0),
		m_client(client), m_props(props)
{
	setCaption(caption);

	QVBoxLayout* vlayout = new QVBoxLayout(this);
	if (!vlayout) {
		return ;
	}
	vlayout->setSpacing(6);
	vlayout->setMargin(6);

	QHBoxLayout* findLayout = new QHBoxLayout(vlayout);
	m_artistLineEdit = new QComboBox(this);
	m_albumLineEdit = new QComboBox(this);
	m_findButton = new QPushButton(i18n("&Find"), this);
	if (findLayout && m_artistLineEdit && m_albumLineEdit && m_findButton) {
		m_artistLineEdit->setEditable(true);
		m_artistLineEdit->setAutoCompletion(true);
		m_artistLineEdit->setDuplicatesEnabled(false);
		m_albumLineEdit->setEditable(true);
		m_albumLineEdit->setAutoCompletion(true);
		m_albumLineEdit->setDuplicatesEnabled(false);
		m_artistLineEdit->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
		m_albumLineEdit->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
		m_findButton->setDefault(true);
		findLayout->addWidget(m_artistLineEdit);
		findLayout->addWidget(m_albumLineEdit);
		findLayout->addWidget(m_findButton);
		connect(m_findButton, SIGNAL(clicked()), this, SLOT(slotFind()));
	}
	if (m_props.defaultServer) {
		QHBoxLayout* serverLayout = new QHBoxLayout(vlayout);
		QLabel* serverLabel = new QLabel(i18n("&Server:"), this);
		m_serverComboBox = new QComboBox(this);
		QLabel* cgiLabel = 0;
		if (m_props.defaultCgiPath) {
			cgiLabel = new QLabel(i18n("C&GI Path:"), this);
			m_cgiLineEdit = new QLineEdit(this);
		}
		if (serverLayout && serverLabel && m_serverComboBox) {
			if (m_props.serverList) {
				QStringList strList;
				for (const char** sl = m_props.serverList; *sl != 0; ++sl) {
					strList += *sl;
				}
				m_serverComboBox->QCM_addItems(strList);
			}
			m_serverComboBox->setEditable(true);
			serverLayout->addWidget(serverLabel);
			serverLayout->addWidget(m_serverComboBox);
			serverLabel->setBuddy(m_serverComboBox);
			if (cgiLabel && m_cgiLineEdit) {
				serverLayout->addWidget(cgiLabel);
				serverLayout->addWidget(m_cgiLineEdit);
				cgiLabel->setBuddy(m_cgiLineEdit);
			}
		}
	}
	m_albumListBox = new Q3ListBox(this);
	if (m_albumListBox) {
		vlayout->addWidget(m_albumListBox);
#if QT_VERSION >= 0x040000
		connect(m_albumListBox, SIGNAL(selectionChanged(Q3ListBoxItem*)),
				this, SLOT(requestTrackList(Q3ListBoxItem*)));
#else
		connect(m_albumListBox, SIGNAL(selectionChanged(QListBoxItem*)),
				this, SLOT(requestTrackList(QListBoxItem*)));
#endif
		connect(m_albumListBox, SIGNAL(selected(int)),
				this, SLOT(requestTrackList(int)));
	}

	QHBoxLayout* buttonLayout = new QHBoxLayout(vlayout);
	QPushButton* helpButton = m_props.helpAnchor ?
		new QPushButton(i18n("&Help"), this) : 0;
	QPushButton* saveButton = m_props.cfg ? new QPushButton(i18n("&Save Settings"), this) : 0;
	QPushButton* closeButton = new QPushButton(i18n("&Close"), this);
	if (buttonLayout && closeButton) {
		if (helpButton) {
			buttonLayout->addWidget(helpButton);
			connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
		}
		if (saveButton) {
			buttonLayout->addWidget(saveButton);
			connect(saveButton, SIGNAL(clicked()), this, SLOT(saveConfig()));
		}
		QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
																					 QSizePolicy::Minimum);
		buttonLayout->addItem(hspacer);
		buttonLayout->addWidget(closeButton);
		connect(closeButton, SIGNAL(clicked()), this, SLOT(accept()));
	}

	m_statusBar = new QStatusBar(this);
	if (m_statusBar) {
		vlayout->addWidget(m_statusBar);
		m_client->init(m_statusBar);
		connect(m_client, SIGNAL(findFinished(const QByteArray&)),
				this, SLOT(slotFindFinished(const QByteArray&)));
		connect(m_client, SIGNAL(albumFinished(const QByteArray&)),
				this, SLOT(slotAlbumFinished(const QByteArray&)));
	}
}

/**
 * Destructor.
 */
ImportSourceDialog::~ImportSourceDialog()
{
	m_client->disconnect();
	delete m_client;
}

/**
 * Clear dialog data.
 */
void ImportSourceDialog::clear()
{
	m_albumListBox->clear();
}

/**
 * Get string with server and port.
 *
 * @return "servername:port".
 */
QString ImportSourceDialog::getServer() const
{
	if (m_serverComboBox) {
		QString server(m_serverComboBox->currentText());
		if (server.isEmpty()) {
			server = m_props.defaultServer;
		}
		return server;
	} else {
		return QString::null;
	}
}

/**
 * Set string with server and port.
 *
 * @param srv "servername:port"
 */
void ImportSourceDialog::setServer(const QString& srv)
{
	if (m_serverComboBox) {
		m_serverComboBox->setCurrentText(srv);
	}
}

/**
 * Get string with CGI path.
 *
 * @return CGI path, e.g. "/~cddb/cddb.cgi".
 */
QString ImportSourceDialog::getCgiPath() const
{
	if (m_cgiLineEdit) {
		QString cgi(m_cgiLineEdit->text());
		if (cgi.isEmpty()) {
			cgi = m_props.defaultCgiPath;
		}
		return cgi;
	} else {
		return QString::null;
	}
}

/**
 * Set string with CGI path.
 *
 * @param cgi CGI path, e.g. "/~cddb/cddb.cgi".
 */
void ImportSourceDialog::setCgiPath(const QString& cgi)
{
	if (m_cgiLineEdit) {
		m_cgiLineEdit->setText(cgi);
	}
}

/**
 * Get the local configuration.
 *
 * @param cfg configuration
 */
void ImportSourceDialog::getImportSourceConfig(ImportSourceConfig* cfg) const
{
	cfg->m_server = getServer();
	cfg->m_cgiPath = getCgiPath();
	cfg->m_windowWidth = size().width();
	cfg->m_windowHeight = size().height();
}

/**
 * Save the local settings to the configuration.
 */
void ImportSourceDialog::saveConfig()
{
	if (m_props.cfg) {
		getImportSourceConfig(m_props.cfg);
	}
}

/**
 * Set a find string from artist and album information.
 *
 * @param artist artist
 * @param album  album
 */
void ImportSourceDialog::setArtistAlbum(const QString& artist, const QString& album)
{
	if (m_props.cfg) {
		setServer(m_props.cfg->m_server);
		setCgiPath(m_props.cfg->m_cgiPath);
		if (m_props.cfg->m_windowWidth > 0 && m_props.cfg->m_windowHeight > 0) {
			resize(m_props.cfg->m_windowWidth, m_props.cfg->m_windowHeight);
		}
	}

	if (!(artist.isEmpty() && album.isEmpty())) {
		m_artistLineEdit->setCurrentText(artist);
		m_albumLineEdit->setCurrentText(album);
		QLineEdit* lineEdit = m_artistLineEdit->lineEdit();
		if (lineEdit) {
			lineEdit->selectAll();
		}
		m_artistLineEdit->setFocus();
	}
}

/**
 * Query a search for a keyword from the server.
 */
void ImportSourceDialog::slotFind()
{
	ImportSourceConfig cfg;
	getImportSourceConfig(&cfg);
	m_client->find(&cfg, m_artistLineEdit->currentText(),
								 m_albumLineEdit->currentText());
}

/**
 * Process finished find request.
 *
 * @param searchStr search data received
 */
void ImportSourceDialog::slotFindFinished(const QByteArray& searchStr)
{
	parseFindResults(searchStr);
}

/**
 * Process finished album data.
 *
 * @param albumStr album track data received
 */
void ImportSourceDialog::slotAlbumFinished(const QByteArray& albumStr)
{
	parseAlbumResults(albumStr);
	emit trackDataUpdated();
}

/**
 * Request track list from server.
 *
 * @param li list box item containing an AlbumListItem
 */
void ImportSourceDialog::requestTrackList(Q3ListBoxItem* li)
{
	AlbumListItem* ali;
	if ((ali = dynamic_cast<AlbumListItem *>(li)) != 0) {
		ImportSourceConfig cfg;
		getImportSourceConfig(&cfg);
		m_client->getTrackList(&cfg, ali->getCategory(), ali->getId());
	}
}

/**
 * Request track list from server.
 *
 * @param index index of list box item containing an AlbumListItem
 */
void ImportSourceDialog::requestTrackList(int index)
{
	requestTrackList(m_albumListBox->item(index));
}

/**
 * Show help.
 */
void ImportSourceDialog::showHelp()
{
	if (m_props.helpAnchor) {
		Kid3App::displayHelp(m_props.helpAnchor);
	}
}
