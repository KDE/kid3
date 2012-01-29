/**
 * \file exportdialog.cpp
 * Export dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 May 2006
 *
 * Copyright (C) 2006-2008  Urs Fleisch
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

#include "exportdialog.h"
#include "config.h"
#ifdef CONFIG_USE_KDE
#include <kfiledialog.h>
#else
#include <QFileDialog>
#endif

#include <QLayout>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QString>
#include <QTextEdit>
#include <QTableView>
#include <QComboBox>
#include <QDir>
#include <QApplication>
#include <QClipboard>
#include <QUrl>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "taggedfile.h"
#include "genres.h"
#include "configstore.h"
#include "contexthelp.h"
#include "textexporter.h"
#include "texttablemodel.h"
#include "formatlistedit.h"
#include "qtcompatmac.h"

/**
 * Constructor.
 *
 * @param parent       parent widget
 * @param textExporter text exporter to use
 */
ExportDialog::ExportDialog(QWidget* parent, TextExporter* textExporter) :
  QDialog(parent),
  m_textExporter(textExporter), m_textTableModel(new TextTableModel(this))
{
  setObjectName("ExportDialog");
  setModal(true);
  setWindowTitle(i18n("Export"));
  setSizeGripEnabled(true);

  QVBoxLayout* vlayout = new QVBoxLayout(this);
  if (vlayout) {
    vlayout->setMargin(6);
    vlayout->setSpacing(6);
    m_edit = new QTextEdit(this);
    if (m_edit) {
      m_edit->setAcceptRichText(false);
      vlayout->addWidget(m_edit);
    }
    m_table = new QTableView(this);
    m_table->setModel(m_textTableModel);
    m_table->hide();
    vlayout->addWidget(m_table);

    QString formatToolTip = ImportTrackData::getFormatToolTip();
    m_formatListEdit = new FormatListEdit(
          QStringList() << i18n("Format:")
                        << i18n("Header:")
                        << i18n("Tracks:")
                        << i18n("Footer:"),
          QStringList() << QString()
                        << formatToolTip
                        << formatToolTip
                        << formatToolTip,
          this);
    connect(m_formatListEdit, SIGNAL(formatChanged()),
            this, SLOT(showPreview()));
    vlayout->addWidget(m_formatListEdit);

    QHBoxLayout* butlayout = new QHBoxLayout;
    if (butlayout) {
      butlayout->setSpacing(6);
      m_fileButton = new QPushButton(i18n("To F&ile"), this);
      if (m_fileButton) {
        m_fileButton->setAutoDefault(false);
        butlayout->addWidget(m_fileButton);
        connect(m_fileButton, SIGNAL(clicked()), this, SLOT(slotToFile()));
      }
      m_clipButton = new QPushButton(i18n("To Clip&board"), this);
      if (m_clipButton) {
        m_clipButton->setAutoDefault(false);
        butlayout->addWidget(m_clipButton);
        connect(m_clipButton, SIGNAL(clicked()), this, SLOT(slotToClipboard()));
      }
      QSpacerItem* butspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
                                               QSizePolicy::Minimum);
      butlayout->addItem(butspacer);

      QLabel* srcLabel = new QLabel(i18n("&Source:"), this);
      butlayout->addWidget(srcLabel);
      m_srcComboBox = new QComboBox(this);
      if (m_srcComboBox) {
        m_srcComboBox->setEditable(false);
        m_srcComboBox->addItem(i18n("Tag 1"), TrackData::TagV1);
        m_srcComboBox->addItem(i18n("Tag 2"), TrackData::TagV2);
        srcLabel->setBuddy(m_srcComboBox);
        butlayout->addWidget(m_srcComboBox);
        connect(m_srcComboBox, SIGNAL(activated(int)),
                this, SLOT(onSrcComboBoxActivated(int)));
      }
      vlayout->addLayout(butlayout);
    }

    QHBoxLayout* hlayout = new QHBoxLayout;
    if (hlayout) {
      hlayout->setSpacing(6);
      QPushButton* helpButton = new QPushButton(i18n("&Help"), this);
      if (helpButton) {
        helpButton->setAutoDefault(false);
        hlayout->addWidget(helpButton);
        connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
      }
      QPushButton* saveButton = new QPushButton(i18n("&Save Settings"), this);
      if (saveButton) {
        saveButton->setAutoDefault(false);
        hlayout->addWidget(saveButton);
        connect(saveButton, SIGNAL(clicked()), this, SLOT(saveConfig()));
      }
      QSpacerItem* hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
                                             QSizePolicy::Minimum);
      hlayout->addItem(hspacer);

      QPushButton* closeButton = new QPushButton(i18n("&Close"), this);
      if (closeButton) {
        closeButton->setAutoDefault(false);
        hlayout->addWidget(closeButton);
        connect(closeButton, SIGNAL(clicked()), this, SLOT(accept()));
      }
      vlayout->addLayout(hlayout);
    }
  }
}

/**
 * Destructor.
 */
ExportDialog::~ExportDialog()
{
}

/**
 * Export to a file.
 */
void ExportDialog::slotToFile()
{
  QString fileName =
#ifdef CONFIG_USE_KDE
    KFileDialog::getSaveFileName(ConfigStore::s_genCfg.m_importDir,
                                 QString::null, this);
#else
    QFileDialog::getSaveFileName(this, QString(),
     ConfigStore::s_genCfg.m_importDir, QString(), 0,
     ConfigStore::s_miscCfg.m_dontUseNativeDialogs
     ? QFileDialog::DontUseNativeDialog : QFileDialog::Options(0));
#endif
  if (!fileName.isEmpty()) {
    if (!m_textExporter->exportToFile(fileName)) {
      QMessageBox::warning(
        0, i18n("File Error"),
        i18n("Error while writing file:\n") + fileName,
        QMessageBox::Ok, Qt::NoButton);
    }
  }
}

/**
 * Export to clipboard.
 */
void ExportDialog::slotToClipboard()
{
  QApplication::clipboard()->setText(
    m_textExporter->getText(), QClipboard::Clipboard);
}

/**
 * Show exported text as preview in editor.
 */
void ExportDialog::showPreview()
{
  m_textExporter->updateText(m_formatListEdit->getCurrentFormat(1),
                             m_formatListEdit->getCurrentFormat(2),
                             m_formatListEdit->getCurrentFormat(3));
  QString text(m_textExporter->getText());
  if (m_textTableModel->setText(
        text, !m_formatListEdit->getCurrentFormat(1).isEmpty())) {
    m_table->resizeColumnsToContents();
    m_table->show();
    m_edit->hide();
  } else {
    m_edit->setPlainText(text);
    m_table->hide();
    m_edit->show();
  }
}

/**
 * Set the format combo box and line edits from the configuration.
 */
void ExportDialog::setFormatFromConfig()
{
  m_formatListEdit->setFormats(
        QList<QStringList>() << ConfigStore::s_genCfg.m_exportFormatNames
                             << ConfigStore::s_genCfg.m_exportFormatHeaders
                             << ConfigStore::s_genCfg.m_exportFormatTracks
                             << ConfigStore::s_genCfg.m_exportFormatTrailers,
        ConfigStore::s_genCfg.m_exportFormatIdx);
}

/**
 * Read the local settings from the configuration.
 */
void ExportDialog::readConfig()
{
  m_srcComboBox->setCurrentIndex(
      m_srcComboBox->findData(ConfigStore::s_genCfg.m_exportSrcV1));

  setFormatFromConfig();

  if (ConfigStore::s_genCfg.m_exportWindowWidth > 0 &&
      ConfigStore::s_genCfg.m_exportWindowHeight > 0) {
    resize(ConfigStore::s_genCfg.m_exportWindowWidth,
           ConfigStore::s_genCfg.m_exportWindowHeight);
  }
}

/**
 * Save the local settings to the configuration.
 */
void ExportDialog::saveConfig()
{
  ConfigStore::s_genCfg.m_exportSrcV1 = TrackData::tagVersionCast(
    m_srcComboBox->itemData(m_srcComboBox->currentIndex()).toInt());
  QList<QStringList> formats = m_formatListEdit->getFormats(
        &ConfigStore::s_genCfg.m_exportFormatIdx);
  ConfigStore::s_genCfg.m_exportFormatNames = formats.at(0);
  ConfigStore::s_genCfg.m_exportFormatHeaders = formats.at(1);
  ConfigStore::s_genCfg.m_exportFormatTracks = formats.at(2);
  ConfigStore::s_genCfg.m_exportFormatTrailers = formats.at(3);
  ConfigStore::s_genCfg.m_exportWindowWidth = size().width();
  ConfigStore::s_genCfg.m_exportWindowHeight = size().height();

  setFormatFromConfig();
}

/**
 * Show help.
 */
void ExportDialog::showHelp()
{
  ContextHelp::displayHelp("export");
}

/**
 * Called when the source combo box selection is changed.
 * @param index combo box index
 */
void ExportDialog::onSrcComboBoxActivated(int index)
{
  m_textExporter->readTagsInTrackData(
        TrackData::tagVersionCast(m_srcComboBox->itemData(index).toInt()));
  showPreview();
}