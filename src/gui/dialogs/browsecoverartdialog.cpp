/**
 * \file browsecoverartdialog.cpp
 * Browse cover art dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 11-Jan-2009
 *
 * Copyright (C) 2009-2018  Urs Fleisch
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
#include <QUrl>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCoreApplication>
#include "networkconfig.h"
#include "importconfig.h"
#include "contexthelp.h"
#include "externalprocess.h"
#include "configtable.h"
#include "configtablemodel.h"
#include "formatlistedit.h"

namespace {

/**
 * Get help text for supported format codes.
 *
 * @return help text.
 */
QString getToolTip()
{
  QString str = QLatin1String("<table>\n");
  str += FrameFormatReplacer::getToolTip(true);

  str += QLatin1String("<tr><td>%ua...</td><td>%u{artist}...</td><td>");
  const char* const encodeAsUrlStr =
      QT_TRANSLATE_NOOP("@default", "Encode as URL");
  str += QCoreApplication::translate("@default", encodeAsUrlStr);
  str += QLatin1String("</td></tr>\n");

  str += QLatin1String("</table>\n");
  return str;
}

}

/**
 * Constructor.
 *
 * @param app application context
 * @param parent parent widget
 */
BrowseCoverArtDialog::BrowseCoverArtDialog(Kid3Application* app,
                                           QWidget* parent)
  : QDialog(parent), m_app(app)
{
  setObjectName(QLatin1String("BrowseCoverArtDialog"));
  setModal(true);
  setWindowTitle(tr("Browse Cover Art"));
  setSizeGripEnabled(true);

  auto vlayout = new QVBoxLayout(this);

  m_edit = new QTextEdit(this);
  m_edit->setReadOnly(true);
  vlayout->addWidget(m_edit);

  QGroupBox* artistAlbumBox = new QGroupBox(tr("&Artist/Album"), this);
  m_artistLineEdit = new QLineEdit(artistAlbumBox);
  m_albumLineEdit = new QLineEdit(artistAlbumBox);
  auto hbox = new QHBoxLayout;
  hbox->addWidget(m_artistLineEdit);
  hbox->addWidget(m_albumLineEdit);
  artistAlbumBox->setLayout(hbox);
  vlayout->addWidget(artistAlbumBox);
  connect(m_artistLineEdit, &QLineEdit::returnPressed,
          this, &BrowseCoverArtDialog::showPreview);
  connect(m_albumLineEdit, &QLineEdit::returnPressed,
          this, &BrowseCoverArtDialog::showPreview);

  QGroupBox* srcbox = new QGroupBox(tr("&Source"), this);
  m_formatListEdit = new FormatListEdit(
        {tr("Source:"), tr("URL:")},
        {QString(), getToolTip()},
        srcbox);

  auto vbox = new QVBoxLayout;
  vbox->addWidget(m_formatListEdit);
  srcbox->setLayout(vbox);
  vlayout->addWidget(srcbox);
  connect(m_formatListEdit, &FormatListEdit::formatChanged,
          this, &BrowseCoverArtDialog::showPreview);

  QGroupBox* tabbox = new QGroupBox(tr("&URL extraction"), this);
  m_matchUrlTableModel = new ConfigTableModel(tabbox);
  m_matchUrlTableModel->setLabels({tr("Match"), tr("Picture URL")});
  m_matchUrlTable = new ConfigTable(m_matchUrlTableModel, tabbox);
  m_matchUrlTable->setHorizontalResizeModes(
      m_matchUrlTableModel->getHorizontalResizeModes());
  auto tablayout = new QVBoxLayout;
  tablayout->addWidget(m_matchUrlTable);
  tabbox->setLayout(tablayout);
  vlayout->addWidget(tabbox);

  auto hlayout = new QHBoxLayout;
  QPushButton* helpButton = new QPushButton(tr("&Help"), this);
  helpButton->setAutoDefault(false);
  hlayout->addWidget(helpButton);
  connect(helpButton, &QAbstractButton::clicked, this, &BrowseCoverArtDialog::showHelp);

  QPushButton* saveButton = new QPushButton(tr("&Save Settings"), this);
  saveButton->setAutoDefault(false);
  hlayout->addWidget(saveButton);
  connect(saveButton, &QAbstractButton::clicked, this, &BrowseCoverArtDialog::saveConfig);

  auto hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
                                         QSizePolicy::Minimum);
  hlayout->addItem(hspacer);

  QPushButton* browseButton = new QPushButton(tr("&Browse"), this);
  QPushButton* cancelButton = new QPushButton(tr("&Cancel"), this);
  browseButton->setAutoDefault(false);
  browseButton->setDefault(true);
  cancelButton->setAutoDefault(false);
  hlayout->addWidget(browseButton);
  hlayout->addWidget(cancelButton);
  connect(browseButton, &QAbstractButton::clicked, this, &QDialog::accept);
  connect(cancelButton, &QAbstractButton::clicked, this, &QDialog::reject);

  vlayout->addLayout(hlayout);
}

/**
 * Destructor.
 */
BrowseCoverArtDialog::~BrowseCoverArtDialog()
{
  // Must not be inline because of forwared declared QScopedPointer.
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
  txt += NetworkConfig::instance().browser();
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
  const ImportConfig& importCfg = ImportConfig::instance();
  m_formatListEdit->setFormats(
        {importCfg.pictureSourceNames(),importCfg.pictureSourceUrls()},
        importCfg.pictureSourceIndex());
}

/**
 * Read the local settings from the configuration.
 */
void BrowseCoverArtDialog::readConfig()
{
  const ImportConfig& importCfg = ImportConfig::instance();
  setSourceFromConfig();
  m_matchUrlTableModel->setMap(importCfg.matchPictureUrlMap());

  if (!importCfg.browseCoverArtWindowGeometry().isEmpty()) {
    restoreGeometry(importCfg.browseCoverArtWindowGeometry());
  }
}

/**
 * Save the local settings to the configuration.
 */
void BrowseCoverArtDialog::saveConfig()
{
  ImportConfig& importCfg = ImportConfig::instance();
  int idx;
  QList<QStringList> formats = m_formatListEdit->getFormats(&idx);
  importCfg.setPictureSourceIndex(idx);
  importCfg.setPictureSourceNames(formats.at(0));
  importCfg.setPictureSourceUrls(formats.at(1));
  importCfg.setMatchPictureUrlMap(m_matchUrlTableModel->getMap());
  importCfg.setBrowseCoverArtWindowGeometry(saveGeometry());

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
    m_process.reset(new ExternalProcess(m_app, this));
  }
  m_process->launchCommand(
    tr("Browse Cover Art"),
    {NetworkConfig::instance().browser(), m_url});
  QDialog::accept();
}
