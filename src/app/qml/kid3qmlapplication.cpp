/**
 * \file kid3qmlapplication.cpp
 * Kid3 application with QML support functions.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 21 Jun 2014
 *
 * Copyright (C) 2014  Urs Fleisch
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

#include "kid3qmlapplication.h"
#include <QItemSelectionModel>
#include <QMetaProperty>
#include <QCoreApplication>
#include "coreplatformtools.h"
#include "fileproxymodel.h"
#include "qmlimageprovider.h"
#include "taggedfileselection.h"
#include "framelist.h"
#include "frameobjectmodel.h"
#include "batchimportconfig.h"
#include "filterconfig.h"
#include "formatconfig.h"
#include "importconfig.h"
#include "exportconfig.h"
#include "tagconfig.h"
#include "fileconfig.h"
#include "rendirconfig.h"
#include "numbertracksconfig.h"
#include "useractionsconfig.h"
#include "guiconfig.h"
#include "networkconfig.h"
#include "playlistconfig.h"
#include "serverimporterconfig.h"
#include "findreplaceconfig.h"
#include "mainwindowconfig.h"

/**
 * Constructor.
 * @param platformTools platform tools
 * @param parent parent object
 */
Kid3QmlApplication::Kid3QmlApplication(ICorePlatformTools* platformTools,
                                       QObject* parent) :
  Kid3Application(platformTools, parent), m_selectFrame(0),
  m_editFrameTaggedFile(0), m_frameObjectModel(0), m_imageProvider(0)
{
  connect(getFileSelectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SIGNAL(fileSelectionChanged()));
  connect(selectionInfo(), SIGNAL(singleFileChanged()),
          this, SLOT(onSingleFileChanged()));
  getFrameList()->setFrameEditor(this);
}

/**
 * Destructor.
 */
Kid3QmlApplication::~Kid3QmlApplication()
{
}

/**
 * Let user edit a frame and then update the fields
 * when the edits are accepted.
 * frameEdited() is emitted when the edit dialog is closed with the edited
 * frame as a parameter if it was accepted.
 *
 * @param frame frame to edit
 * @param taggedFile tagged file where frame has to be set
 */
void Kid3QmlApplication::editFrameOfTaggedFile(const Frame* frame,
                                               TaggedFile* taggedFile)
{
  if (!frame || !taggedFile) {
    emit frameEdited(0);
    return;
  }

  m_editFrame = *frame;
  m_editFrameTaggedFile = taggedFile;
  if (!m_frameObjectModel) {
    m_frameObjectModel = new FrameObjectModel(this);
  }
  m_frameObjectModel->setFrame(m_editFrame);
  emit frameEditRequested(m_frameObjectModel);
}

/**
 * Called when the frame edit dialog is closed.
 *
 * @param frame frame object model, null if canceled
 *
 * @see frameEditRequested()
 */
void Kid3QmlApplication::onFrameEditFinished(FrameObjectModel* frame)
{
  if (frame) {
    m_editFrame = frame->getFrame();
    if (m_editFrameTaggedFile->setFrameV2(m_editFrame)) {
      m_editFrameTaggedFile->markTag2Changed(m_editFrame.getType());
    }
    emit frameEdited(&m_editFrame);
  } else {
    emit frameEdited(0);
  }
}

/**
 * Let user select a frame type.
 * frameSelected() is emitted when the edit dialog is closed with the selected
 * frame as a parameter if a frame is selected.
 *
 * @param frame is filled with the selected frame
 * @param taggedFile tagged file for which frame has to be selected
 */
void Kid3QmlApplication::selectFrame(Frame* frame, const TaggedFile* taggedFile)
{
  if (taggedFile && frame) {
    QStringList frameNames = taggedFile->getFrameIds();
    m_selectFrame = frame;
    emit frameSelectionRequested(frameNames);
  }
}

/**
 * Called when the frame selection dialog is closed.
 *
 * @param name name of selected frame, empty if canceled
 */
void Kid3QmlApplication::onFrameSelectionFinished(const QString& name)
{
  if (!name.isEmpty()) {
    Frame::Type type = Frame::getTypeFromTranslatedName(name);
    *m_selectFrame = Frame(type, QLatin1String(""), name, -1);
    emit frameSelected(m_selectFrame);
  } else {
    emit frameSelected(0);
  }
}

/**
 * Return object which emits frameSelected(), frameEdited() signals.
 *
 * @return object which emits signals.
 */
QObject* Kid3QmlApplication::qobject()
{
  return this;
}

/**
 * Get the numbers of the selected rows in a list suitable for scripting.
 * @return list with row numbers.
 */
QVariantList Kid3QmlApplication::getFileSelectionRows()
{
  QItemSelectionModel* selModel = getFileSelectionModel();
  QVariantList rows;
  foreach (const QModelIndex& index, selModel->selectedRows()) {
    rows.append(index.row());
  }
  return rows;
}

/**
 * Set the file selection from a list of model indexes.
 * @param indexes list of model indexes suitable for scripting
 */
void Kid3QmlApplication::setFileSelectionIndexes(const QVariantList& indexes)
{
  QItemSelection selection;
  QModelIndex firstIndex;
  foreach (const QVariant& var, indexes) {
    QModelIndex index = var.toModelIndex();
    if (!firstIndex.isValid()) {
      firstIndex = index;
    }
    selection.select(index, index);
  }
  QItemSelectionModel* selModel = getFileSelectionModel();
  disconnect(selModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
             this, SIGNAL(fileSelectionChanged()));
  selModel->select(selection,
                   QItemSelectionModel::Clear | QItemSelectionModel::Select |
                   QItemSelectionModel::Rows);
  if (firstIndex.isValid()) {
    selModel->setCurrentIndex(firstIndex,
        QItemSelectionModel::Select | QItemSelectionModel::Rows);
  }
  connect(selModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SIGNAL(fileSelectionChanged()));
}

void Kid3QmlApplication::onSingleFileChanged()
{
  TaggedFileSelection* selection = selectionInfo();
  QByteArray picture = selection->getPicture();
  if (m_imageProvider && picture != m_imageProvider->getImageData()) {
    m_imageProvider->setImageData(picture);
    setNextCoverArtImageId();
    emit coverArtImageIdChanged(m_coverArtImageId);
  }
}

void Kid3QmlApplication::setNextCoverArtImageId() {
  static quint32 nr = 0;
  m_coverArtImageId = QString(QLatin1String("image://kid3/data/%1")).
      arg(nr++, 8, 16, QLatin1Char('0'));
}

QStringList Kid3QmlApplication::toStringList(const QList<QUrl>& urls)
{
  QStringList paths;
  foreach (const QUrl& url, urls) {
    paths.append(url.toLocalFile());
  }
  return paths;
}

QList<QPersistentModelIndex> Kid3QmlApplication::toPersistentModelIndexList(const QVariantList& lst)
{
  QList<QPersistentModelIndex> indexes;
  foreach (const QVariant& var, lst) {
    indexes.append(var.toModelIndex());
  }
  return indexes;
}

TrackData::TagVersion Kid3QmlApplication::toTagVersion(int nr)
{
  TrackData::TagVersion tagVersion = static_cast<TrackData::TagVersion>(nr);
  switch (tagVersion) {
  case TrackData::TagNone:
  case TrackData::TagV1:
  case TrackData::TagV2:
  case TrackData::TagV2V1:
    return tagVersion;
  }
  return TrackData::TagNone;
}

QVariant Kid3QmlApplication::getRoleData(
    QAbstractItemModel* model, int row, const QByteArray& roleName,
    QModelIndex parent)
{
  if (model) {
    QHash<int,QByteArray> roleHash = model->roleNames();
    for (QHash<int,QByteArray>::const_iterator it = roleHash.constBegin();
         it != roleHash.constEnd();
         ++it) {
      if (it.value() == roleName) {
        return model->index(row, 0, parent).data(it.key());
      }
    }
  }
  return QVariant();
}

bool Kid3QmlApplication::setRoleData(
    QAbstractItemModel* model, int row, const QByteArray& roleName,
    const QVariant& value, QModelIndex parent)
{
  if (model) {
    QHash<int,QByteArray> roleHash = model->roleNames();
    for (QHash<int,QByteArray>::const_iterator it = roleHash.constBegin();
         it != roleHash.constEnd();
         ++it) {
      if (it.value() == roleName) {
        return model->setData(model->index(row, 0, parent), value, it.key());
      }
    }
  }
  return false;
}

QString Kid3QmlApplication::properties(QObject* obj)
{
  QString str;
  const QMetaObject* meta;
  if (obj && (meta = obj->metaObject()) != 0) {
    str += QLatin1String("className: ");
    str += QString::fromLatin1(meta->className());
    for (int i = 0; i < meta->propertyCount(); i++) {
      QMetaProperty property = meta->property(i);
      const char* name = property.name();
      QVariant value = obj->property(name);
      str += QLatin1Char('\n');
      str += QString::fromLatin1(name);
      str += QLatin1String(": ");
      str += value.toString();
    }
  }
  return str;
}

QObject* Kid3QmlApplication::batchImportConfig()
{
  return &BatchImportConfig::instance();
}

QObject* Kid3QmlApplication::filterConfig()
{
  return &FilterConfig::instance();
}

QObject* Kid3QmlApplication::filenameFormatConfig()
{
  return &FilenameFormatConfig::instance();
}

QObject* Kid3QmlApplication::tagFormatConfig()
{
  return &TagFormatConfig::instance();
}

QObject* Kid3QmlApplication::importConfig()
{
  return &ImportConfig::instance();
}

QObject* Kid3QmlApplication::exportConfig()
{
  return &ExportConfig::instance();
}

QObject* Kid3QmlApplication::tagConfig()
{
  return &TagConfig::instance();
}

QObject* Kid3QmlApplication::fileConfig()
{
  return &FileConfig::instance();
}

QObject* Kid3QmlApplication::renDirConfig()
{
  return &RenDirConfig::instance();
}

QObject* Kid3QmlApplication::numberTracksConfig()
{
  return &NumberTracksConfig::instance();
}

QObject* Kid3QmlApplication::userActionsConfig()
{
  return &UserActionsConfig::instance();
}

QObject* Kid3QmlApplication::guiConfig()
{
  return &GuiConfig::instance();
}

QObject* Kid3QmlApplication::networkConfig()
{
  return &NetworkConfig::instance();
}

QObject* Kid3QmlApplication::playlistConfig()
{
  return &PlaylistConfig::instance();
}

QObject* Kid3QmlApplication::findReplaceConfig()
{
  return &FindReplaceConfig::instance();
}

QObject* Kid3QmlApplication::mainWindowConfig()
{
  return &MainWindowConfig::instance();
}
