/**
 * \file browsecoverartdialog.cpp
 * Browse cover art dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 11-Jan-2009
 *
 * Copyright (C) 2009-2013  Urs Fleisch
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

#include "browsecoverartdialog.h"
#include <QLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QString>
#include <QLineEdit>
#include <QMessageBox>
#include <QRegExp>
#include <QUrl>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCoreApplication>
#include "configstore.h"
#include "contexthelp.h"
#include "externalprocess.h"
#include "configtable.h"
#include "configtablemodel.h"
#include "formatlistedit.h"

/**
 * Get help text for supported format codes.
 *
 * @return help text.
 */
static QString getToolTip()
{
  QString str = QLatin1String("<table>\n");
  str += FrameFormatReplacer::getToolTip(true);

  str += QLatin1String("<tr><td>%ua...</td><td>%u{artist}...</td><td>");
  str += QCoreApplication::translate("@default", QT_TRANSLATE_NOOP("@default", "Encode as URL"));
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("</table>\n");
  return str;
}

/**
 * Constructor.
 *
 * @param parent parent widget
 */
BrowseCoverArtDialog::BrowseCoverArtDialog(QWidget* parent) :
  QDialog(parent), m_process(0)
{
  setObjectName(QLatin1String("BrowseCoverArtDialog"));
  setModal(true);
  setWindowTitle(tr("Browse Cover Art"));
  setSizeGripEnabled(true);

  QVBoxLayout* vlayout = new QVBoxLayout(this);

  m_edit = new QTextEdit(this);
  m_edit->setReadOnly(true);
  vlayout->addWidget(m_edit);

  QGroupBox* artistAlbumBox = new QGroupBox(tr("&Artist/Album"), this);
  m_artistLineEdit = new QLineEdit(artistAlbumBox);
  m_albumLineEdit = new QLineEdit(artistAlbumBox);
  QHBoxLayout* hbox = new QHBoxLayout;
  hbox->addWidget(m_artistLineEdit);
  hbox->addWidget(m_albumLineEdit);
  artistAlbumBox->setLayout(hbox);
  vlayout->addWidget(artistAlbumBox);
  connect(m_artistLineEdit, SIGNAL(returnPressed()),
          this, SLOT(showPreview()));
  connect(m_albumLineEdit, SIGNAL(returnPressed()),
          this, SLOT(showPreview()));

  QGroupBox* srcbox = new QGroupBox(tr("&Source"), this);
  m_formatListEdit = new FormatListEdit(
        QStringList() << tr("Source:")
                      << tr("URL:"),
        QStringList() << QString()
                      << getToolTip(),
        srcbox);

  QVBoxLayout* vbox = new QVBoxLayout;
  vbox->addWidget(m_formatListEdit);
  srcbox->setLayout(vbox);
  vlayout->addWidget(srcbox);
  connect(m_formatListEdit, SIGNAL(formatChanged()),
          this, SLOT(showPreview()));

  QGroupBox* tabbox = new QGroupBox(tr("&URL extraction"), this);
  m_matchUrlTableModel = new ConfigTableModel(tabbox);
  m_matchUrlTableModel->setLabels(
    QStringList() << tr("Match") << tr("Picture URL"));
  m_matchUrlTable = new ConfigTable(m_matchUrlTableModel, tabbox);
  m_matchUrlTable->setHorizontalResizeModes(
      m_matchUrlTableModel->getHorizontalResizeModes());
  QVBoxLayout* tablayout = new QVBoxLayout;
  tablayout->addWidget(m_matchUrlTable);
  tabbox->setLayout(tablayout);
  vlayout->addWidget(tabbox);

  QHBoxLayout* hlayout = new QHBoxLayout;
  QPushButton* helpButton = new QPushButton(tr("&Help"), this);
  helpButton->setAutoDefault(false);
  hlayout->addWidget(helpButton);
  connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));

  QPushButton* saveButton = new QPushButton(tr("&Save Settings"), this);
  saveButton->setAutoDefault(false);
  hlayout->addWidget(saveButton);
  connect(saveButton, SIGNAL(clicked()), this, SLOT(saveConfig()));

  QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
                                         QSizePolicy::Minimum);
  hlayout->addItem(hspacer);

  QPushButton* browseButton = new QPushButton(tr("&Browse"), this);
  QPushButton* cancelButton = new QPushButton(tr("&Cancel"), this);
  browseButton->setAutoDefault(false);
  cancelButton->setAutoDefault(false);
  hlayout->addWidget(browseButton);
  hlayout->addWidget(cancelButton);
  connect(browseButton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

  vlayout->addLayout(hlayout);
}

/**
 * Destructor.
 */
BrowseCoverArtDialog::~BrowseCoverArtDialog()
{
  delete m_process;
}

/**
 * Show browse command as preview.
 */
void BrowseCoverArtDialog::showPreview()
{
  m_frames.setArtist(m_artistLineEdit->text());
  m_frames.setAlbum(m_albumLineEdit->text());
  FrameFormatReplacer fmt(m_frames, m_formatListEdit->getCurrentFormat(1));
  fmt.replaceEscapedChars();
  fmt.replacePercentCodes(FormatReplacer::FSF_SupportUrlEncode);
  m_url = fmt.getString();
  QString txt(QLatin1String("<p><b>"));
  txt += tr("Click Browse to start");
  txt += QLatin1String("</b></p><p><tt>");
  txt += ConfigStore::s_networkCfg.m_browser;
  txt += QLatin1Char(' ');
  txt += m_url;
  txt += QLatin1String("</tt></p><p><b>");
  txt += tr("Then drag the picture from the browser to Kid3.");
  txt += QLatin1String("</b></p>");
  m_edit->clear();
  m_edit->append(txt);
}

/**
 * Set frames for which picture has to be found.
 *
 * @param frames track data
 */
void BrowseCoverArtDialog::setFrames(const FrameCollection& frames)
{
  m_frames = frames;

  m_artistLineEdit->setText(m_frames.getArtist());
  m_albumLineEdit->setText(m_frames.getAlbum());

  showPreview();
}

/**
 * Set the source combo box and line edits from the configuration.
 */
void BrowseCoverArtDialog::setSourceFromConfig()
{
  m_formatListEdit->setFormats(
        QList<QStringList>() << ConfigStore::s_genCfg.m_pictureSourceNames
                             << ConfigStore::s_genCfg.m_pictureSourceUrls,
        ConfigStore::s_genCfg.m_pictureSourceIdx);
}

/**
 * Read the local settings from the configuration.
 */
void BrowseCoverArtDialog::readConfig()
{
  setSourceFromConfig();
  m_matchUrlTableModel->setMap(ConfigStore::s_genCfg.m_matchPictureUrlMap);

  if (!ConfigStore::s_genCfg.m_browseCoverArtWindowGeometry.isEmpty()) {
    restoreGeometry(ConfigStore::s_genCfg.m_browseCoverArtWindowGeometry);
  }
}

/**
 * Save the local settings to the configuration.
 */
void BrowseCoverArtDialog::saveConfig()
{
  QList<QStringList> formats = m_formatListEdit->getFormats(
        &ConfigStore::s_genCfg.m_pictureSourceIdx);
  ConfigStore::s_genCfg.m_pictureSourceNames = formats.at(0);
  ConfigStore::s_genCfg.m_pictureSourceUrls = formats.at(1);
  ConfigStore::s_genCfg.m_matchPictureUrlMap = m_matchUrlTableModel->getMap();
  ConfigStore::s_genCfg.m_browseCoverArtWindowGeometry = saveGeometry();

  setSourceFromConfig();
}

/**
 * Show help.
 */
void BrowseCoverArtDialog::showHelp()
{
  ContextHelp::displayHelp(QLatin1String("browse_pictures"));
}

/**
 * Hide modal dialog, start browse command.
 */
void BrowseCoverArtDialog::accept()
{
  if (!m_process) {
    m_process = new ExternalProcess(this);
  }
  m_process->launchCommand(
    tr("Browse Cover Art"),
    QStringList() << ConfigStore::s_networkCfg.m_browser << m_url);
  QDialog::accept();
}
