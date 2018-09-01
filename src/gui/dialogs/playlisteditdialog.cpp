/**
 * \file playlisteditdialog.cpp
 * Edit playlist dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 05 Aug 2018
 *
 * Copyright (C) 2018  Urs Fleisch
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

#include "playlisteditdialog.h"
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>
#include <QCloseEvent>
#include "contexthelp.h"
#include "playlistmodel.h"
#include "proxyitemselectionmodel.h"
#include "playlistview.h"

/**
 * Constructor.
 * @param model playlist model
 * @param selModel selection model of associated file proxy model
 * @param parent parent widget
 */
PlaylistEditDialog::PlaylistEditDialog(PlaylistModel* model,
                                       QItemSelectionModel* selModel,
                                       QWidget* parent)
  : QDialog(parent), m_playlistModel(model)
{
  setObjectName(QLatin1String("PlaylistEditDialog"));
  setModal(false);
  setSizeGripEnabled(true);
  setAttribute(Qt::WA_DeleteOnClose);

  QVBoxLayout* vlayout = new QVBoxLayout(this);
  QListView* playlist = new PlaylistView;
  playlist->setModel(m_playlistModel);
  playlist->setSelectionMode(QAbstractItemView::ExtendedSelection);
  playlist->setSelectionBehavior(QAbstractItemView::SelectRows);
  playlist->setSelectionModel(new ProxyItemSelectionModel(m_playlistModel,
                                                          selModel, this));
  playlist->setAcceptDrops(true);
  playlist->setDragEnabled(true);
  playlist->setDragDropMode(QAbstractItemView::DragDrop);
  playlist->setDragDropOverwriteMode(false);
  playlist->setDefaultDropAction(Qt::MoveAction);
  playlist->setDropIndicatorShown(true);
  playlist->viewport()->setAcceptDrops(true);

  vlayout->addWidget(playlist);
  m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Help |
                                     QDialogButtonBox::Save |
                                     QDialogButtonBox::Cancel);
  connect(m_buttonBox, SIGNAL(helpRequested()), this, SLOT(showHelp()));
  connect(m_buttonBox, SIGNAL(accepted()), m_playlistModel, SLOT(save()));
  connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  vlayout->addWidget(m_buttonBox);

  connect(m_playlistModel, SIGNAL(modifiedChanged(bool)),
          this, SLOT(setModified(bool)));
  setModified(false);
}

/**
 * Destructor.
 */
PlaylistEditDialog::~PlaylistEditDialog()
{
  // Force rereading the file on the next Kid3Application::playlistModel().
  m_playlistModel->setPlaylistFile(QString());
}

/**
 * Show help.
 */
void PlaylistEditDialog::showHelp()
{
  ContextHelp::displayHelp(QLatin1String("edit-playlist"));
}

void PlaylistEditDialog::setModified(bool modified)
{
  setWindowCaption();
  m_buttonBox->button(QDialogButtonBox::Save)->setEnabled(modified);
}

/**
 * Set window caption.
 */
void PlaylistEditDialog::setWindowCaption()
{
  QString title = tr("Playlist");
  QString fileName = m_playlistModel->playlistFileName();
  if (!fileName.isEmpty()) {
    title += QLatin1String(" - ");
    title += fileName;
    if (m_playlistModel->modified()) {
      title += tr(" [modified]");
    }
  }
  setWindowTitle(title);
}

void PlaylistEditDialog::closeEvent(QCloseEvent* event)
{
  if (m_playlistModel->modified()) {
    int answer = QMessageBox::warning(
          this, tr("Warning"),
          tr("A playlist has been modified.\n"
             "Do you want to save it?"),
          QMessageBox::Yes | QMessageBox::Default,
          QMessageBox::No,
          QMessageBox::Cancel | QMessageBox::Escape);
    if (answer == QMessageBox::Yes) {
      m_playlistModel->save();
    }
    if (answer != QMessageBox::Yes && answer != QMessageBox::No) {
      event->ignore();
      return;
    }
  }
  QDialog::closeEvent(event);
}
