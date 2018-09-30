/**
 * \file browserdialog.cpp
 * Help browser.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 Jun 2009
 *
 * Copyright (C) 2003-2017  Urs Fleisch
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

#include "browserdialog.h"
#include <QTextBrowser>
#include <QLocale>
#include <QDir>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QStyle>
#include <QAction>
#include "config.h"
#include "loadtranslation.h"

/**
 * Constructor.
 *
 * @param parent parent widget
 * @param caption dialog title
 */
BrowserDialog::BrowserDialog(QWidget* parent, QString& caption)
  : QDialog(parent)
{
  setObjectName(QLatin1String("BrowserDialog"));
  setWindowTitle(caption);
  auto vlayout = new QVBoxLayout(this);

  QString docDir;
#ifdef CFG_DOCDIR
  docDir = QLatin1String(CFG_DOCDIR);
  Utils::prependApplicationDirPathIfRelative(docDir);
#endif

  QLocale locale;
  QStringList docPaths;
#ifndef Q_OS_WIN32
  const auto uiLangs = locale.uiLanguages();
  for (const QString& uiLang : uiLangs) {
    QString lang(uiLang.left(2));
    docPaths += QDir::currentPath() + QLatin1String("/kid3_") + lang + QLatin1String(".html");
    if (!docDir.isNull()) {
      docPaths += docDir + QLatin1String("/kid3_") + lang + QLatin1String(".html");
    }
  }
#endif
  QString lang(locale.name().left(2));
  if (!docDir.isNull()) {
    docPaths += docDir + QLatin1String("/kid3_") + lang + QLatin1String(".html");
    docPaths += docDir + QLatin1String("/kid3_en.html");
  }
  docPaths += QDir::currentPath() + QLatin1String("/kid3_") + lang + QLatin1String(".html");
  docPaths += QDir::currentPath() + QLatin1String("/kid3_en.html");
  for (auto it = docPaths.constBegin(); it != docPaths.constEnd(); ++it) {
    m_filename = *it;
    if (QFile::exists(m_filename)) break;
  }
  m_textBrowser = new QTextBrowser(this);
  m_textBrowser->setOpenExternalLinks(true);
  m_textBrowser->setSource(QUrl::fromLocalFile(m_filename));
  vlayout->addWidget(m_textBrowser);

  auto hlayout = new QHBoxLayout;
  QPushButton* backButton = new QPushButton(tr("&Back"), this);
  backButton->setEnabled(false);
  connect(backButton, &QAbstractButton::clicked, m_textBrowser, &QTextBrowser::backward);
  connect(m_textBrowser, &QTextBrowser::backwardAvailable, backButton, &QWidget::setEnabled);
  hlayout->addWidget(backButton);
  QPushButton* forwardButton = new QPushButton(tr("&Forward"), this);
  forwardButton->setEnabled(false);
  connect(forwardButton, &QAbstractButton::clicked, m_textBrowser, &QTextBrowser::forward);
  connect(m_textBrowser, &QTextBrowser::forwardAvailable, forwardButton, &QWidget::setEnabled);
  hlayout->addWidget(forwardButton);
  QLabel* findLabel = new QLabel(tr("&Find:"), this);
  hlayout->addWidget(findLabel);
  m_findLineEdit = new QLineEdit(this);
  m_findLineEdit->setFocus();
  findLabel->setBuddy(m_findLineEdit);
  connect(m_findLineEdit, &QLineEdit::returnPressed, this, &BrowserDialog::findNext);
  hlayout->addWidget(m_findLineEdit);
  auto findAction = new QAction(this);
  findAction->setShortcut(QKeySequence::Find);
  connect(findAction, &QAction::triggered,
          m_findLineEdit, static_cast<void (QWidget::*)()>(&QWidget::setFocus));
  m_findLineEdit->addAction(findAction);
  auto findPreviousAction = new QAction(this);
  findPreviousAction->setIcon(
        QIcon(style()->standardIcon(QStyle::SP_ArrowBack)));
  findPreviousAction->setText(tr("Find Previous"));
  findPreviousAction->setShortcut(QKeySequence::FindPrevious);
  connect(findPreviousAction, &QAction::triggered,
          this, &BrowserDialog::findPrevious);
  auto findPreviousButton = new QToolButton(this);
  findPreviousButton->setDefaultAction(findPreviousAction);
  hlayout->addWidget(findPreviousButton);
  auto findNextAction = new QAction(this);
  findNextAction->setIcon(
        QIcon(style()->standardIcon(QStyle::SP_ArrowForward)));
  findNextAction->setText(tr("Find Next"));
  findNextAction->setShortcut(QKeySequence::FindNext);
  connect(findNextAction, &QAction::triggered,
          this, &BrowserDialog::findNext);
  auto findNextButton = new QToolButton(this);
  findNextButton->setDefaultAction(findNextAction);
  hlayout->addWidget(findNextButton);
  hlayout->addStretch();
  QPushButton* closeButton = new QPushButton(tr("&Close"), this);
  closeButton->setAutoDefault(false);
  connect(closeButton, &QAbstractButton::clicked, this, &QDialog::accept);
  hlayout->addWidget(closeButton);
  vlayout->addLayout(hlayout);
  resize(500, 500);
}

/**
 * Destructor.
 */
BrowserDialog::~BrowserDialog()
{}

/**
 * Display help document at anchor.
 *
 * @param anchor anchor
 */
void BrowserDialog::goToAnchor(const QString& anchor)
{
  QUrl url = QUrl::fromLocalFile(m_filename);
  url.setFragment(anchor);
  m_textBrowser->setSource(url);
}

/**
 * Find previous occurrence of search text.
 */
void BrowserDialog::findPrevious()
{
  m_textBrowser->find(m_findLineEdit->text(), QTextDocument::FindBackward);
}

/**
 * Find next occurrence of search text.
 */
void BrowserDialog::findNext()
{
  m_textBrowser->find(m_findLineEdit->text());
}
