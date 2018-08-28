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
#include "contexthelp.h"
#include "fileproxymodel.h"
#include "playlistmodel.h"
#include "proxyitemselectionmodel.h"
#include "playlistview.h"

/**
 * Constructor.
 * @param selModel selection model of associated file proxy model
 * @param parent parent widget
 */
PlaylistEditDialog::PlaylistEditDialog(QItemSelectionModel* selModel,
                                       QWidget* parent)
  : QDialog(parent)
{
  setObjectName(QLatin1String("PlaylistEditDialog"));
  setModal(false);
  setSizeGripEnabled(true);
  setAttribute(Qt::WA_DeleteOnClose);

  FileProxyModel* fsModel = qobject_cast<FileProxyModel*>(
        const_cast<QAbstractItemModel*>(selModel->model()));
  m_playlistModel = new PlaylistModel(fsModel, this);

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
                                     QDialogButtonBox::Close);
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
}

/**
 * Set playlist to edit.
 * @param path path to playlist file
 */
void PlaylistEditDialog::setPlaylistFile(const QString& path)
{
  m_playlistModel->setPlaylistFile(path);
  setWindowCaption();
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
