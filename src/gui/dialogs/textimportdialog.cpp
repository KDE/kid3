/**
 * \file textimportdialog.cpp
 * Dialog to import from a text (file or clipboard).
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 19 Jun 2011
 *
 * Copyright (C) 2011-2013  Urs Fleisch
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
                                   TrackDataModel* trackDataModel) :
  QDialog(parent), m_platformTools(platformTools),
  m_textImporter(new TextImporter(trackDataModel))
{
  setObjectName(QLatin1String("TextImportDialog"));
  setWindowTitle(tr("Import from File/Clipboard"));
  setSizeGripEnabled(true);

  QVBoxLayout* vboxLayout = new QVBoxLayout(this);

  QString formatToolTip = ImportParser::getFormatToolTip();
  m_formatListEdit = new FormatListEdit(
        QStringList() << tr("Format:")
                      << tr("Header:")
                      << tr("Tracks:"),
        QStringList() << QString()
                      << formatToolTip
                      << formatToolTip,
        this);
  vboxLayout->addWidget(m_formatListEdit);

  QHBoxLayout* buttonLayout = new QHBoxLayout;
  QPushButton* helpButton = new QPushButton(tr("&Help"), this);
  helpButton->setAutoDefault(false);
  buttonLayout->addWidget(helpButton);
  connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
  QPushButton* saveButton = new QPushButton(tr("&Save Settings"), this);
  saveButton->setAutoDefault(false);
  buttonLayout->addWidget(saveButton);
  connect(saveButton, SIGNAL(clicked()), this, SLOT(saveConfig()));
  buttonLayout->addStretch();
  QPushButton* fileButton = new QPushButton(tr("From F&ile..."), this);
  fileButton->setAutoDefault(false);
  buttonLayout->addWidget(fileButton);
  connect(fileButton, SIGNAL(clicked()), this, SLOT(fromFile()));
  QPushButton* clipButton = new QPushButton(tr("From Clip&board"), this);
  clipButton->setAutoDefault(false);
  buttonLayout->addWidget(clipButton);
  connect(clipButton, SIGNAL(clicked()), this, SLOT(fromClipboard()));
  QPushButton* closeButton = new QPushButton(tr("&Close"), this);
  closeButton->setAutoDefault(false);
  buttonLayout->addWidget(closeButton);
  connect(closeButton, SIGNAL(clicked()), this, SLOT(accept()));
  vboxLayout->addLayout(buttonLayout);
}

/**
 * Destructor.
 */
TextImportDialog::~TextImportDialog()
{
  delete m_textImporter;
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
  m_formatListEdit->setFormats(
        QList<QStringList>() << ImportConfig::instance().m_importFormatNames
                             << ImportConfig::instance().m_importFormatHeaders
                             << ImportConfig::instance().m_importFormatTracks,
        ImportConfig::instance().m_importFormatIdx);
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
      ImportConfig::instance().m_importDir = QFileInfo(file).dir().path();
      QTextStream stream(&file);
      QString text = stream.readAll();
      if (!text.isNull() &&
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
      ImportConfig::instance().m_importDir, QString(), 0)
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
  QList<QStringList> formats = m_formatListEdit->getFormats(
        &ImportConfig::instance().m_importFormatIdx);
  ImportConfig::instance().m_importFormatNames = formats.at(0);
  ImportConfig::instance().m_importFormatHeaders = formats.at(1);
  ImportConfig::instance().m_importFormatTracks = formats.at(2);

  setFormatFromConfig();
}

/**
 * Show help.
 */
void TextImportDialog::showHelp()
{
  ContextHelp::displayHelp(QLatin1String("import-text"));
}
