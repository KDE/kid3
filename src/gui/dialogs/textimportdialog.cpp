/**
 * \file textimportdialog.cpp
 * Dialog to import from a text (file or clipboard).
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 19 Jun 2011
 *
 * Copyright (C) 2011-2024  Urs Fleisch
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

#include "textimportdialog.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QClipboard>
#include <QTextStream>
#include <QApplication>
#include "config.h"
#include "iplatformtools.h"
#include "textimporter.h"
#include "importparser.h"
#include "importconfig.h"
#include "contexthelp.h"
#include "formatlistedit.h"

/**
 * Constructor.
 *
 * @param platformTools platform tools
 * @param parent  parent widget
 * @param trackDataModel track data to be filled with imported values
 */
TextImportDialog::TextImportDialog(IPlatformTools* platformTools,
                                   QWidget* parent,
                                   TrackDataModel* trackDataModel)
  : QDialog(parent), m_platformTools(platformTools),
    m_textImporter(new TextImporter(trackDataModel))
{
  setObjectName(QLatin1String("TextImportDialog"));
  setWindowTitle(tr("Import from File/Clipboard"));
  setSizeGripEnabled(true);

  auto vboxLayout = new QVBoxLayout(this);

  QString formatToolTip = ImportParser::getFormatToolTip();
  m_formatListEdit = new FormatListEdit(
        {tr("Format:"), tr("Header:"), tr("Tracks:")},
        {QString(), formatToolTip, formatToolTip},
        this);
  vboxLayout->addWidget(m_formatListEdit);

  auto buttonLayout = new QHBoxLayout;
  auto helpButton = new QPushButton(tr("&Help"), this);
  helpButton->setAutoDefault(false);
  buttonLayout->addWidget(helpButton);
  connect(helpButton, &QAbstractButton::clicked, this, &TextImportDialog::showHelp);
  auto saveButton = new QPushButton(tr("&Save Settings"), this);
  saveButton->setAutoDefault(false);
  buttonLayout->addWidget(saveButton);
  connect(saveButton, &QAbstractButton::clicked, this, &TextImportDialog::saveConfig);
  buttonLayout->addStretch();
  auto fileButton = new QPushButton(tr("From F&ile..."), this);
  fileButton->setAutoDefault(false);
  buttonLayout->addWidget(fileButton);
  connect(fileButton, &QAbstractButton::clicked, this, &TextImportDialog::fromFile);
  auto clipButton = new QPushButton(tr("From Clip&board"), this);
  clipButton->setAutoDefault(false);
  buttonLayout->addWidget(clipButton);
  connect(clipButton, &QAbstractButton::clicked, this, &TextImportDialog::fromClipboard);
  auto closeButton = new QPushButton(tr("&Close"), this);
  closeButton->setAutoDefault(false);
  buttonLayout->addWidget(closeButton);
  connect(closeButton, &QAbstractButton::clicked, this, &QDialog::accept);
  vboxLayout->addLayout(buttonLayout);
}

/**
 * Destructor.
 */
TextImportDialog::~TextImportDialog()
{
  // Must not be inline because of forwared declared QScopedPointer.
}

/**
 * Clear dialog data.
 */
void TextImportDialog::clear()
{
  setFormatFromConfig();
}

/**
 * Set the format combo box and line edits from the configuration.
 */
void TextImportDialog::setFormatFromConfig()
{
  const ImportConfig& importCfg = ImportConfig::instance();
  m_formatListEdit->setFormats(
        {importCfg.importFormatNames(), importCfg.importFormatHeaders(),
         importCfg.importFormatTracks()},
        importCfg.importFormatIndex());
}

/**
 * Import from a file.
 *
 * @param fn file name
 *
 * @return true if ok.
 */
bool TextImportDialog::importFromFile(const QString& fn)
{
  if (!fn.isEmpty()) {
    QFile file(fn);
    if (file.open(QIODevice::ReadOnly)) {
      ImportConfig::instance().setImportDir(QFileInfo(file).dir().path());
      QTextStream stream(&file);
      if (QString text = stream.readAll(); !text.isNull() &&
          m_textImporter->updateTrackData(
            text,
            m_formatListEdit->getCurrentFormat(1),
            m_formatListEdit->getCurrentFormat(2))) {
        emit trackDataUpdated();
      }
      file.close();
      return true;
    }
  }
  return false;
}

/**
 * Let user select file, assign file contents to text and preview in
 * table.
 */
void TextImportDialog::fromFile()
{
  importFromFile(m_platformTools->getOpenFileName(this, QString(),
      ImportConfig::instance().importDir(), QString(), nullptr)
    );
}

/**
 * Assign clipboard contents to text and preview in table.
 */
void TextImportDialog::fromClipboard()
{
  QClipboard* cb = QApplication::clipboard();
  QString text = cb->text(QClipboard::Clipboard);
  if (text.isNull())
    text = cb->text(QClipboard::Selection);
  if (!text.isNull() &&
      m_textImporter->updateTrackData(
        text,
        m_formatListEdit->getCurrentFormat(1),
        m_formatListEdit->getCurrentFormat(2)))
    emit trackDataUpdated();
}

/**
 * Save the local settings to the configuration.
 */
void TextImportDialog::saveConfig()
{
  ImportConfig& importCfg = ImportConfig::instance();
  int idx;
  QList<QStringList> formats = m_formatListEdit->getFormats(&idx);
  importCfg.setImportFormatIndex(idx);
  importCfg.setImportFormatNames(formats.at(0));
  importCfg.setImportFormatHeaders(formats.at(1));
  importCfg.setImportFormatTracks(formats.at(2));

  setFormatFromConfig();
}

/**
 * Show help.
 */
void TextImportDialog::showHelp()
{
  ContextHelp::displayHelp(QLatin1String("import-text"));
}
