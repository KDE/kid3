/**
 * \file rendirdialog.cpp
 * Rename directory dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 21 Mar 2004
 *
 * Copyright (C) 2004-2018  Urs Fleisch
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

#include "rendirdialog.h"
#include <QLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QDir>
#include <QApplication>
#include <QTextEdit>
#include <QCursor>
#include <QVBoxLayout>
#include <QFormLayout>
#include "taggedfile.h"
#include "frame.h"
#include "trackdata.h"
#include "contexthelp.h"
#include "rendirconfig.h"
#include "dirrenamer.h"
#include "stringlisteditdialog.h"

/**
 * Constructor.
 *
 * @param parent parent widget
 * @param dirRenamer directory renamer
 */
RenDirDialog::RenDirDialog(QWidget* parent, DirRenamer* dirRenamer)
  : QWizard(parent), m_taggedFile(nullptr), m_dirRenamer(dirRenamer)
{
  setObjectName(QLatin1String("RenDirDialog"));
  setModal(true);
  setWindowTitle(tr("Rename Directory"));
  setSizeGripEnabled(true);

  auto mainPage = new QWizardPage;

  auto mainLayout = new QVBoxLayout(mainPage);
  setupMainPage(mainPage, mainLayout);
  mainPage->setTitle(tr("Format"));
  addPage(mainPage);

  auto previewPage = new QWizardPage;
  setupPreviewPage(previewPage);
  previewPage->setTitle(tr("Preview"));
  addPage(previewPage);

  setOptions(HaveHelpButton | HaveCustomButton1);
  setButtonText(CustomButton1, tr("&Save Settings"));
  connect(this, &QWizard::helpRequested, this, &RenDirDialog::showHelp);
  connect(this, &QWizard::customButtonClicked, this, &RenDirDialog::saveConfig);
  connect(this, &QWizard::currentIdChanged, this, &RenDirDialog::pageChanged);
}

/**
 * Set up the main wizard page.
 *
 * @param page    widget
 * @param vlayout layout
 */
void RenDirDialog::setupMainPage(QWidget* page, QVBoxLayout* vlayout)
{
  if (!page || !vlayout) {
    return;
  }

  auto actionLayout = new QFormLayout;
  actionLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
  m_actionComboBox = new QComboBox(page);
  m_tagversionComboBox = new QComboBox(page);
  m_actionComboBox->insertItem(ActionRename, tr("Rename Directory"));
  m_actionComboBox->insertItem(ActionCreate, tr("Create Directory"));
  actionLayout->addRow(tr("&Action:"), m_actionComboBox);
  connect(m_actionComboBox, static_cast<void (QComboBox::*)(int)>(
            &QComboBox::activated), this, &RenDirDialog::slotUpdateNewDirname);
  const QList<QPair<Frame::TagVersion, QString> > tagVersions =
      Frame::availableTagVersions();
  for (auto it = tagVersions.constBegin(); it != tagVersions.constEnd(); ++it) {
    m_tagversionComboBox->addItem(it->second, it->first);
  }
  actionLayout->addRow(tr("&Source:"), m_tagversionComboBox);
  connect(m_tagversionComboBox, static_cast<void (QComboBox::*)(int)>(
            &QComboBox::activated), this, &RenDirDialog::slotUpdateNewDirname);

  auto formatLayout = new QHBoxLayout;
  m_formatComboBox = new QComboBox(page);
  m_formatComboBox->setEditable(true);
  const RenDirConfig& renDirCfg = RenDirConfig::instance();
  m_formats = renDirCfg.dirFormats();
  m_format = renDirCfg.dirFormat();
  setFormats();
  formatLayout->addWidget(m_formatComboBox, 1);
  auto editFormatsButton = new QPushButton(tr("&Edit..."));
  connect(editFormatsButton, &QPushButton::clicked,
          this, &RenDirDialog::editFormats);
  formatLayout->addWidget(editFormatsButton);
  auto formatLabel = new QLabel(tr("&Format:"));
  formatLabel->setBuddy(m_formatComboBox);
  actionLayout->addRow(formatLabel, formatLayout);
  m_tagversionComboBox->setCurrentIndex(
        m_tagversionComboBox->findData(renDirCfg.renDirSource()));
  connect(m_formatComboBox, static_cast<void (QComboBox::*)(int)>(
            &QComboBox::activated), this, &RenDirDialog::slotUpdateNewDirname);
  connect(m_formatComboBox, &QComboBox::editTextChanged,
          this, &RenDirDialog::slotUpdateNewDirname);

  m_currentDirLabel = new QLabel(page);
  m_newDirLabel = new QLabel(page);
  actionLayout->addRow(tr("From:"), m_currentDirLabel);
  actionLayout->addRow(tr("To:"), m_newDirLabel);

  vlayout->addLayout(actionLayout);
}

/**
 * Set up the preview wizard page.
 *
 * @param page widget
 */
void RenDirDialog::setupPreviewPage(QWidget* page)
{
  auto vlayout = new QVBoxLayout(page);
  m_edit = new QTextEdit(page);
  m_edit->setReadOnly(true);
  m_edit->setAcceptRichText(false);
  vlayout->addWidget(m_edit);
}

/**
 * Start dialog.
 *
 * @param taggedFile file to use for rename preview
 * @param dirName    if taggedFile is 0, the directory can be set here
 */
void RenDirDialog::startDialog(TaggedFile* taggedFile, const QString& dirName)
{
  m_taggedFile = taggedFile;
  if (m_taggedFile) {
    slotUpdateNewDirname();
  } else {
    m_currentDirLabel->setText(dirName);
    m_newDirLabel->clear();
  }
  restart();
}

/**
 * Set new directory name.
 *
 * @param dir new directory name
 */
void RenDirDialog::setNewDirname(const QString& dir)
{
  m_newDirLabel->setText(dir);
}

/**
 * Get new directory name.
 *
 * @return new directory name.
 */
QString RenDirDialog::getNewDirname() const
{
  return m_newDirLabel->text();
}

/**
 * Set configuration from dialog in directory renamer.
 */
void RenDirDialog::setDirRenamerConfiguration() {
  m_dirRenamer->setTagVersion(Frame::tagVersionCast(
    m_tagversionComboBox->itemData(m_tagversionComboBox->currentIndex()).toInt()));
  m_dirRenamer->setAction(m_actionComboBox->currentIndex() == ActionCreate);
  m_format = m_formatComboBox->currentText();
  m_dirRenamer->setFormat(m_format);
}

/**
 * Set new directory name according to current settings.
 */
void RenDirDialog::slotUpdateNewDirname()
{
  if (m_taggedFile) {
    setDirRenamerConfiguration();
    QString currentDirname;
    QString newDirname(m_dirRenamer->generateNewDirname(m_taggedFile,
                                                        &currentDirname));
    m_currentDirLabel->setText(currentDirname);
    setNewDirname(newDirname);
  }
}

/**
 * Save the local settings to the configuration.
 */
void RenDirDialog::saveConfig()
{
  RenDirConfig& renDirCfg = RenDirConfig::instance();
  m_format = m_formatComboBox->currentText();
  setFormats();
  renDirCfg.setDirFormats(m_formats);
  renDirCfg.setDirFormat(m_format);
  renDirCfg.setRenDirSource(Frame::tagVersionCast(
    m_tagversionComboBox->itemData(m_tagversionComboBox->currentIndex()).toInt()));
}

/**
 * Show help.
 */
void RenDirDialog::showHelp()
{
  ContextHelp::displayHelp(QLatin1String("rename-directory"));
}

/**
 * Request action scheduling and then accept dialog.
 */
void RenDirDialog::requestActionSchedulingAndAccept()
{
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  setDirRenamerConfiguration();
  emit actionSchedulingRequested();
  QApplication::restoreOverrideCursor();
  accept();
}

/**
 * Clear action preview.
 */
void RenDirDialog::clearPreview()
{
  if (m_edit) {
    m_edit->clear();
    m_edit->setLineWrapMode(QTextEdit::NoWrap);
  }
}

/**
 * Display action preview.
 *
 * @param actionStrs description of action
 */
void RenDirDialog::displayActionPreview(const QStringList& actionStrs)
{
  QString str = actionStrs.at(0);
#if QT_VERSION >= 0x050b00
  int width = fontMetrics().horizontalAdvance(str) + 8;
#else
  int width = fontMetrics().width(str) + 8;
#endif
#if QT_VERSION >= 0x050a00
  if (m_edit->tabStopDistance() < width) {
    m_edit->setTabStopDistance(width);
  }
#else
  if (m_edit->tabStopWidth() < width) {
    m_edit->setTabStopWidth(width);
  }
#endif
  if (actionStrs.size() > 1) {
    str += QLatin1Char('\t');
    str += actionStrs.at(1);
  }
  if (actionStrs.size() > 2) {
    str += QLatin1String("\n\t");
    str += actionStrs.at(2);
  }
  m_edit->append(str);
}

/**
 * Wizard page changed.
 */
void RenDirDialog::pageChanged()
{
  if (currentId() == 1) {
    clearPreview();
    setDirRenamerConfiguration();
    emit actionSchedulingRequested();
  }
}

/**
 * Open dialog to edit formats.
 */
void RenDirDialog::editFormats()
{
  setFormats();
  StringListEditDialog dialog(m_formats, tr("Directory Name from Tag"), this);
  if (dialog.exec() == QDialog::Accepted) {
    m_formats = dialog.stringList();
    setFormats();
  }
}

/**
 * Set items of format combo box from configuration.
 */
void RenDirDialog::setFormats()
{
  int idx = m_formats.indexOf(m_format);
  if (idx == -1) {
    m_formats.append(m_format);
    idx = m_formats.size() - 1;
  }
  m_formatComboBox->blockSignals(true);
  if (!m_formats.isEmpty()) {
    m_formatComboBox->clear();
    m_formatComboBox->addItems(m_formats);
  }
  m_formatComboBox->setCurrentIndex(idx);
  m_formatComboBox->blockSignals(false);
}

/**
 * Called when the wizard is canceled.
 */
void RenDirDialog::reject()
{
  m_dirRenamer->abort();
  QWizard::reject();
}
