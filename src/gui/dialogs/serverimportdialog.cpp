/**
 * \file serverimportdialog.cpp
 * Generic dialog to import from a server.
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

#include "serverimportdialog.h"
#include <QLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QStatusBar>
#include <QRegExp>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListView>
#include "serverimporter.h"
#include "serverimporterconfig.h"
#include "contexthelp.h"
#include "trackdata.h"
#include "qtcompatmac.h"

/**
 * Constructor.
 *
 * @param parent  parent widget
 */
ServerImportDialog::ServerImportDialog(QWidget* parent) : QDialog(parent),
    m_serverComboBox(0), m_cgiLineEdit(0), m_additionalTagsCheckBox(0),
    m_coverArtCheckBox(0), m_source(0)
{
  setObjectName("ServerImportDialog");

  QVBoxLayout* vlayout = new QVBoxLayout(this);
  vlayout->setSpacing(6);
  vlayout->setMargin(6);

  QHBoxLayout* findLayout = new QHBoxLayout;
  m_artistLineEdit = new QComboBox(this);
  m_albumLineEdit = new QComboBox(this);
  m_findButton = new QPushButton(i18n("&Find"), this);
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
  vlayout->addLayout(findLayout);

  QHBoxLayout* serverLayout = new QHBoxLayout;
  m_serverLabel = new QLabel(i18n("&Server:"), this);
  m_serverComboBox = new QComboBox(this);
  m_serverComboBox->setEditable(true);
  m_cgiLabel = new QLabel(i18n("C&GI Path:"), this);
  m_cgiLineEdit = new QLineEdit(this);
  serverLayout->addWidget(m_serverLabel);
  serverLayout->addWidget(m_serverComboBox);
  m_serverLabel->setBuddy(m_serverComboBox);
  serverLayout->addWidget(m_cgiLabel);
  serverLayout->addWidget(m_cgiLineEdit);
  m_cgiLabel->setBuddy(m_cgiLineEdit);
  vlayout->addLayout(serverLayout);

  QHBoxLayout* hlayout = new QHBoxLayout;
  m_additionalTagsCheckBox = new QCheckBox(i18n("&Additional Tags"), this);
  m_coverArtCheckBox = new QCheckBox(i18n("C&over Art"), this);
  hlayout->addWidget(m_additionalTagsCheckBox);
  hlayout->addWidget(m_coverArtCheckBox);
  vlayout->addLayout(hlayout);

  m_albumListBox = new QListView(this);
  m_albumListBox->setEditTriggers(QAbstractItemView::NoEditTriggers);
  vlayout->addWidget(m_albumListBox);
  connect(m_albumListBox, SIGNAL(activated(QModelIndex)),
      this, SLOT(requestTrackList(QModelIndex)));

  QHBoxLayout* buttonLayout = new QHBoxLayout;
  m_helpButton = new QPushButton(i18n("&Help"), this);
  m_saveButton = new QPushButton(i18n("&Save Settings"), this);
  QPushButton* closeButton = new QPushButton(i18n("&Close"), this);
  buttonLayout->addWidget(m_helpButton);
  connect(m_helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
  buttonLayout->addWidget(m_saveButton);
  connect(m_saveButton, SIGNAL(clicked()), this, SLOT(saveConfig()));
  QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
                                         QSizePolicy::Minimum);
  buttonLayout->addItem(hspacer);
  buttonLayout->addWidget(closeButton);
  connect(closeButton, SIGNAL(clicked()), this, SLOT(accept()));
  vlayout->addLayout(buttonLayout);

  m_statusBar = new QStatusBar(this);
  vlayout->addWidget(m_statusBar);
  showStatusMessage(i18n("Ready."));
}

/**
 * Destructor.
 */
ServerImportDialog::~ServerImportDialog()
{
}

/**
 * Set importer to be used.
 *
 * @param source  import source to use
 */
void ServerImportDialog::setImportSource(ServerImporter* source)
{
  if (m_source) {
    disconnect(m_source, SIGNAL(progress(const QString&, int, int)),
        this, SLOT(showStatusMessage(const QString&)));
    disconnect(m_source, SIGNAL(findFinished(const QByteArray&)),
        this, SLOT(slotFindFinished(const QByteArray&)));
    disconnect(m_source, SIGNAL(albumFinished(const QByteArray&)),
        this, SLOT(slotAlbumFinished(const QByteArray&)));
  }
  m_source = source;

  if (m_source) {
    connect(m_source, SIGNAL(progress(const QString&, int, int)),
        this, SLOT(showStatusMessage(const QString&)));
    connect(m_source, SIGNAL(findFinished(const QByteArray&)),
        this, SLOT(slotFindFinished(const QByteArray&)));
    connect(m_source, SIGNAL(albumFinished(const QByteArray&)),
        this, SLOT(slotAlbumFinished(const QByteArray&)));

    setWindowTitle(QCM_translate(m_source->name()));
    if (m_source->defaultServer()) {
      m_serverLabel->show();
      m_serverComboBox->show();
      if (m_source->defaultCgiPath()) {
        m_cgiLabel->show();
        m_cgiLineEdit->show();
      } else {
        m_cgiLabel->hide();
        m_cgiLineEdit->hide();
      }
      if (m_source->serverList()) {
        QStringList strList;
        for (const char** sl = m_source->serverList(); *sl != 0; ++sl) {
          strList += *sl;
        }
        m_serverComboBox->clear();
        m_serverComboBox->addItems(strList);
      }
    } else {
      m_serverLabel->hide();
      m_serverComboBox->hide();
      m_cgiLabel->hide();
      m_cgiLineEdit->hide();
    }
    if (m_source->additionalTags()) {
      m_additionalTagsCheckBox->show();
      m_coverArtCheckBox->show();
    } else {
      m_additionalTagsCheckBox->hide();
      m_coverArtCheckBox->hide();
    }

    m_albumListBox->setModel(m_source->getAlbumListModel());

    if (m_source->helpAnchor()) {
      m_helpButton->show();
    } else {
      m_helpButton->hide();
    }
    if (m_source->config()) {
      m_saveButton->show();
    } else {
      m_saveButton->hide();
    }
  }
}

/**
 * Display message in status bar.
 *
 * @param msg status message
 */
void ServerImportDialog::showStatusMessage(const QString& msg)
{
  m_statusBar->showMessage(msg);
}

/**
 * Get string with server and port.
 *
 * @return "servername:port".
 */
QString ServerImportDialog::getServer() const
{
  if (m_serverComboBox) {
    QString server(m_serverComboBox->currentText());
    if (server.isEmpty() && m_source) {
      server = m_source->defaultServer();
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
void ServerImportDialog::setServer(const QString& srv)
{
  if (m_serverComboBox) {
    int idx = m_serverComboBox->findText(srv);
    if (idx >= 0) {
      m_serverComboBox->setCurrentIndex(idx);
    } else {
      m_serverComboBox->addItem(srv);
      m_serverComboBox->setCurrentIndex(m_serverComboBox->count() - 1);
    }
  }
}

/**
 * Get string with CGI path.
 *
 * @return CGI path, e.g. "/~cddb/cddb.cgi".
 */
QString ServerImportDialog::getCgiPath() const
{
  if (m_cgiLineEdit) {
    QString cgi(m_cgiLineEdit->text());
    if (cgi.isEmpty() && m_source) {
      cgi = m_source->defaultCgiPath();
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
void ServerImportDialog::setCgiPath(const QString& cgi)
{
  if (m_cgiLineEdit) {
    m_cgiLineEdit->setText(cgi);
  }
}

/**
 * Get additional tags option.
 *
 * @return true if additional tags are enabled.
 */
bool ServerImportDialog::getAdditionalTags() const
{
  return m_additionalTagsCheckBox ?
    m_additionalTagsCheckBox->checkState() == Qt::Checked
    : false;
}

/**
 * Set additional tags option.
 *
 * @param enable true if additional tags are enabled
 */
void ServerImportDialog::setAdditionalTags(bool enable)
{
  if (m_additionalTagsCheckBox) {
    m_additionalTagsCheckBox->setCheckState(
      enable ? Qt::Checked : Qt::Unchecked);
  }
}

/**
 * Get cover art option.
 *
 * @return true if cover art are enabled.
 */
bool ServerImportDialog::getCoverArt() const
{
  return m_coverArtCheckBox ?
    m_coverArtCheckBox->checkState() == Qt::Checked
    : false;
}

/**
 * Set cover art option.
 *
 * @param enable true if cover art are enabled
 */
void ServerImportDialog::setCoverArt(bool enable)
{
  if (m_coverArtCheckBox) {
    m_coverArtCheckBox->setCheckState(
      enable ? Qt::Checked : Qt::Unchecked);
  }
}

/**
 * Get the local configuration.
 *
 * @param cfg configuration
 */
void ServerImportDialog::getImportSourceConfig(ServerImporterConfig* cfg) const
{
  cfg->m_server = getServer();
  cfg->m_cgiPath = getCgiPath();
  cfg->m_additionalTags = getAdditionalTags();
  cfg->m_coverArt = getCoverArt();
  cfg->m_windowWidth = size().width();
  cfg->m_windowHeight = size().height();
}

/**
 * Save the local settings to the configuration.
 */
void ServerImportDialog::saveConfig()
{
  if (m_source && m_source->config()) {
    getImportSourceConfig(m_source->config());
  }
}

/**
 * Set a find string from artist and album information.
 *
 * @param artist artist
 * @param album  album
 */
void ServerImportDialog::setArtistAlbum(const QString& artist, const QString& album)
{
  if (m_source && m_source->config()) {
    ServerImporterConfig* cf = m_source->config();
    setServer(cf->m_server);
    setCgiPath(cf->m_cgiPath);
    setAdditionalTags(cf->m_additionalTags);
    setCoverArt(cf->m_coverArt);
    if (cf->m_windowWidth > 0 && cf->m_windowHeight > 0) {
      resize(cf->m_windowWidth, cf->m_windowHeight);
    }
  }

  if (!(artist.isEmpty() && album.isEmpty())) {
    int idx = m_artistLineEdit->findText(artist);
    if (idx >= 0) {
      m_artistLineEdit->setCurrentIndex(idx);
    } else {
      m_artistLineEdit->addItem(artist);
      m_artistLineEdit->setCurrentIndex(m_artistLineEdit->count() - 1);
    }
    idx = m_albumLineEdit->findText(album);
    if (idx >= 0) {
      m_albumLineEdit->setCurrentIndex(idx);
    } else {
      m_albumLineEdit->addItem(album);
      m_albumLineEdit->setCurrentIndex(m_albumLineEdit->count() - 1);
    }
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
void ServerImportDialog::slotFind()
{
  ServerImporterConfig cfg;
  getImportSourceConfig(&cfg);
  if (m_source)
    m_source->find(&cfg, m_artistLineEdit->currentText(),
                   m_albumLineEdit->currentText());
}

/**
 * Process finished find request.
 *
 * @param searchStr search data received
 */
void ServerImportDialog::slotFindFinished(const QByteArray& searchStr)
{
  if (m_source)
    m_source->parseFindResults(searchStr);
  m_albumListBox->setFocus();
}

/**
 * Process finished album data.
 *
 * @param albumStr album track data received
 */
void ServerImportDialog::slotAlbumFinished(const QByteArray& albumStr)
{
  if (m_source) {
    m_source->setAdditionalTags(getAdditionalTags());
    m_source->setCoverArt(getCoverArt());
    m_source->parseAlbumResults(albumStr);
  }
  emit trackDataUpdated();
}

/**
 * Request track list from server.
 *
 * @param li standard item containing an AlbumListItem
 */
void ServerImportDialog::requestTrackList(QStandardItem* li)
{
  if (AlbumListItem* ali = dynamic_cast<AlbumListItem*>(li)) {
    ServerImporterConfig cfg;
    getImportSourceConfig(&cfg);
    if (m_source)
      m_source->getTrackList(&cfg, ali->getCategory(), ali->getId());
  }
}

/**
 * Request track list from server.
 *
 * @param index model index of list containing an AlbumListItem
 */
void ServerImportDialog::requestTrackList(const QModelIndex& index)
{
  if (m_source)
    requestTrackList(m_source->getAlbumListModel()->itemFromIndex(index));
}

/**
 * Show help.
 */
void ServerImportDialog::showHelp()
{
  if (m_source && m_source->helpAnchor()) {
    ContextHelp::displayHelp(m_source->helpAnchor());
  }
}
