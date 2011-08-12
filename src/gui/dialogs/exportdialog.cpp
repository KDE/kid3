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
#include <QLineEdit>
#include <QComboBox>
#include <QDir>
#include <QApplication>
#include <QClipboard>
#include <QUrl>
#include <QToolTip>
#include <QMessageBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "taggedfile.h"
#include "genres.h"
#include "configstore.h"
#include "contexthelp.h"
#include "textexporter.h"
#include "texttablemodel.h"
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

    QGroupBox* fmtbox = new QGroupBox(i18n("&Format"), this);
    if (fmtbox) {
      m_formatComboBox = new QComboBox(fmtbox);
      m_formatComboBox->setEditable(true);
      m_headerLineEdit = new QLineEdit(fmtbox);
      m_trackLineEdit = new QLineEdit(fmtbox);
      m_trailerLineEdit = new QLineEdit(fmtbox);
      QString formatToolTip = ImportTrackData::getFormatToolTip();
      m_headerLineEdit->setToolTip(formatToolTip);
      m_trackLineEdit->setToolTip(formatToolTip);
      m_trailerLineEdit->setToolTip(formatToolTip);
      QVBoxLayout* vbox = new QVBoxLayout;
      vbox->setMargin(2);
      vbox->addWidget(m_formatComboBox);
      vbox->addWidget(m_headerLineEdit);
      vbox->addWidget(m_trackLineEdit);
      vbox->addWidget(m_trailerLineEdit);
      fmtbox->setLayout(vbox);
      vlayout->addWidget(fmtbox);
      connect(m_formatComboBox, SIGNAL(activated(int)), this,
              SLOT(setFormatLineEdit(int)));
      connect(m_headerLineEdit, SIGNAL(returnPressed()), this, SLOT(showPreview()));
      connect(m_trackLineEdit, SIGNAL(returnPressed()), this, SLOT(showPreview()));
      connect(m_trailerLineEdit, SIGNAL(returnPressed()), this, SLOT(showPreview()));
    }

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
    QFileDialog::getSaveFileName(this, QString(), ConfigStore::s_genCfg.m_importDir
#if !defined Q_OS_WIN32 && !defined Q_OS_MAC
      , QString(), 0, QFileDialog::DontUseNativeDialog
#endif
      );
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
 * Set the format lineedits to the format selected in the combo box.
 *
 * @param index current index of the combo box
 */
void ExportDialog::setFormatLineEdit(int index)
{
  if (index < static_cast<int>(m_formatHeaders.size())) {
    m_headerLineEdit->setText(m_formatHeaders[index]);
    m_trackLineEdit->setText(m_formatTracks[index]);
    m_trailerLineEdit->setText(m_formatTrailers[index]);
  } else {
    m_headerLineEdit->clear();
    m_trackLineEdit->clear();
    m_trailerLineEdit->clear();
  }
  showPreview();
}

/**
 * Show exported text as preview in editor.
 */
void ExportDialog::showPreview()
{
  m_textExporter->updateText(m_headerLineEdit->text(),
                             m_trackLineEdit->text(),
                             m_trailerLineEdit->text());
  QString text(m_textExporter->getText());
  if (m_textTableModel->setText(text, !m_headerLineEdit->text().isEmpty())) {
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
  m_formatHeaders = ConfigStore::s_genCfg.m_exportFormatHeaders;
  m_formatTracks = ConfigStore::s_genCfg.m_exportFormatTracks;
  m_formatTrailers = ConfigStore::s_genCfg.m_exportFormatTrailers;
  m_formatComboBox->clear();
  m_formatComboBox->addItems(ConfigStore::s_genCfg.m_exportFormatNames);
  m_formatComboBox->setCurrentIndex(ConfigStore::s_genCfg.m_exportFormatIdx);
  setFormatLineEdit(ConfigStore::s_genCfg.m_exportFormatIdx);
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
  ConfigStore::s_genCfg.m_exportFormatIdx = m_formatComboBox->currentIndex();
  if (ConfigStore::s_genCfg.m_exportFormatIdx < static_cast<int>(ConfigStore::s_genCfg.m_exportFormatNames.size())) {
    ConfigStore::s_genCfg.m_exportFormatNames[ConfigStore::s_genCfg.m_exportFormatIdx] = m_formatComboBox->currentText();
    ConfigStore::s_genCfg.m_exportFormatHeaders[ConfigStore::s_genCfg.m_exportFormatIdx] = m_headerLineEdit->text();
    ConfigStore::s_genCfg.m_exportFormatTracks[ConfigStore::s_genCfg.m_exportFormatIdx] = m_trackLineEdit->text();
    ConfigStore::s_genCfg.m_exportFormatTrailers[ConfigStore::s_genCfg.m_exportFormatIdx] = m_trailerLineEdit->text();
  } else {
    ConfigStore::s_genCfg.m_exportFormatIdx = ConfigStore::s_genCfg.m_exportFormatNames.size();
    ConfigStore::s_genCfg.m_exportFormatNames.append(m_formatComboBox->currentText());
    ConfigStore::s_genCfg.m_exportFormatHeaders.append(m_headerLineEdit->text());
    ConfigStore::s_genCfg.m_exportFormatTracks.append(m_trackLineEdit->text());
    ConfigStore::s_genCfg.m_exportFormatTrailers.append(m_trailerLineEdit->text());
  }
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
