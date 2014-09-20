/**
 * \file kid3qmlapplication.h
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

#ifndef KID3QMLAPPLICATION_H
#define KID3QMLAPPLICATION_H

#include "kid3application.h"
#include <QList>
#include <QUrl>
#include "iframeeditor.h"

class QmlImageProvider;
class FrameObjectModel;

/**
 * Kid3 application with QML support functions.
 */
class Kid3QmlApplication : public Kid3Application, public IFrameEditor {
  Q_OBJECT
  /** ID to get cover art image. */
  Q_PROPERTY(QString coverArtImageId READ coverArtImageId
             NOTIFY coverArtImageIdChanged)
public:
  /**
   * Constructor.
   * @param platformTools platform tools
   * @param parent parent object
   */
  explicit Kid3QmlApplication(ICorePlatformTools* platformTools,
                              QObject* parent = 0);

  /**
   * Destructor.
   */
  virtual ~Kid3QmlApplication();

  // IFrameEditor implementation

  /**
   * Let user edit a frame and then update the fields
   * when the edits are accepted.
   * frameEdited() is emitted when the edit dialog is closed with the edited
   * frame as a parameter if it was accepted.
   *
   * @param frame frame to edit
   * @param taggedFile tagged file where frame has to be set
   */
  virtual void editFrameOfTaggedFile(const Frame* frame,
                                     TaggedFile* taggedFile);

  /**
   * Let user select a frame type.
   * frameSelected() is emitted when the edit dialog is closed with the selected
   * frame as a parameter if a frame is selected.
   *
   * @param frame is filled with the selected frame
   * @param taggedFile tagged file for which frame has to be selected
   */
  virtual void selectFrame(Frame* frame, const TaggedFile* taggedFile);

  /**
   * Return object which emits frameSelected(), frameEdited() signals.
   *
   * @return object which emits signals.
   */
  virtual QObject* qobject();

  // End of IFrameEditor implementation

  /**
   * Get ID to get cover art image.
   * @return ID for cover art image.
   */
  QString coverArtImageId() const { return m_coverArtImageId; }

  /**
   * Set the image provider.
   * @param imageProvider image provider
   */
  void setImageProvider(QmlImageProvider* imageProvider) {
    m_imageProvider = imageProvider;
  }

  /**
   * Get the numbers of the selected rows in a list suitable for scripting.
   * @return list with row numbers.
   */
  Q_INVOKABLE QVariantList getFileSelectionRows();

  /**
   * Set the file selection from a list of model indexes.
   * @param indexes list of model indexes suitable for scripting
   */
  Q_INVOKABLE void setFileSelectionIndexes(const QVariantList& indexes);

  /**
   * Convert a list of URLs to a list of local file paths.
   * @param urls file URLs
   * @return list with local file paths.
   */
  Q_INVOKABLE static QStringList toStringList(const QList<QUrl>& urls);

  /**
   * Convert a variant list containing model indexes to a list of persistent
   * model indexes.
   * @param lst variant list with model indexes
   * @return persistent model index list.
   */
  Q_INVOKABLE static QList<QPersistentModelIndex> toPersistentModelIndexList(
      const QVariantList& lst);

  /**
   * Convert an integer to a tag version.
   * @param nr tag mask (0=none, 1, 2, 3=1 and 2)
   */
  Q_INVOKABLE static TrackData::TagVersion toTagVersion(int nr);

  /**
   * Get data for @a roleName and @a row from @a model.
   * @param model model
   * @param row model row
   * @param roleName role name as used in scripting languages
   * @param parent optional parent model index
   * @return model data.
   */
  Q_INVOKABLE static QVariant getRoleData(
      QAbstractItemModel* model, int row, const QByteArray& roleName,
      QModelIndex parent = QModelIndex());

  /**
   * Set data for @a roleName and @a row in @a model.
   * @param model model
   * @param row model row
   * @param roleName role name as used in scripting languages
   * @param value model data
   * @param parent optional parent model index
   * @return true if ok.
   */
  Q_INVOKABLE static bool setRoleData(QAbstractItemModel* model, int row,
      const QByteArray& roleName, const QVariant& value,
      QModelIndex parent = QModelIndex());

  /**
   * Get property values as a string.
   * @param obj object to inspect
   * @return string containing property values.
   */
  Q_INVOKABLE static QString properties(QObject* obj);

  /** Get batch import configuration. */
  Q_INVOKABLE static QObject* batchImportConfig();

  /** Get filter configuration. */
  Q_INVOKABLE static QObject* filterConfig();

  /** Get file name formate configuration. */
  Q_INVOKABLE static QObject* filenameFormatConfig();

  /** Get tag format configuration. */
  Q_INVOKABLE static QObject* tagFormatConfig();

  /** Get import configuration. */
  Q_INVOKABLE static QObject* importConfig();

  /** Get export configuration. */
  Q_INVOKABLE static QObject* exportConfig();

  /** Get tag configuration. */
  Q_INVOKABLE static QObject* tagConfig();

  /** Get file configuration. */
  Q_INVOKABLE static QObject* fileConfig();

  /** Get rename directory configuration. */
  Q_INVOKABLE static QObject* renDirConfig();

  /** Get number tracks configuration. */
  Q_INVOKABLE static QObject* numberTracksConfig();

  /** Get user actions configuration. */
  Q_INVOKABLE static QObject* userActionsConfig();

  /** Get GUI configuration. */
  Q_INVOKABLE static QObject* guiConfig();

  /** Get network configuration. */
  Q_INVOKABLE static QObject* networkConfig();

  /** Get playlist configuration. */
  Q_INVOKABLE static QObject* playlistConfig();

  /** Get find/replace configuration. */
  Q_INVOKABLE static QObject* findReplaceConfig();

  /** Get main window configuration. */
  Q_INVOKABLE static QObject* mainWindowConfig();

public slots:
  /**
   * Called when the frame selection dialog is closed.
   *
   * @param name name of selected frame, empty if canceled
   *
   * @see frameSelectionRequested()
   */
  void onFrameSelectionFinished(const QString& name);

  /**
   * Called when the frame edit dialog is closed.
   *
   * @param frame frame object model, null if canceled
   *
   * @see frameEditRequested()
   */
  void onFrameEditFinished(FrameObjectModel* frame);

signals:
  // IFrameEditor implementation

  /**
   * Emitted when the dialog to add and edit a frame is closed.
   * @param frame edited frame if dialog was accepted, else 0
   */
  void frameEdited(const Frame* frame);

  /**
   * Emitted when the dialog to select a frame is closed.
   * @param frame selected frame if dialog was accepted, else 0
   */
  void frameSelected(const Frame* frame);

  // End of IFrameEditor implementation

  /**
   * Emitted to request a frame selection from the frame editor.
   * When the frame selection is accepted or canceled,
   * onFrameSelectionFinished() shall be called.
   *
   * @param frameNames list of possible frame names
   */
  void frameSelectionRequested(const QStringList& frameNames);

  /**
   * Emitted to request a frame edit from the frame editor.
   * When the frame editing is finished, onFrameEditFinished() shall be called.
   *
   * @param frame frame object model
   */
  void frameEditRequested(FrameObjectModel* frame);

  /**
   * Emitted when the file selection is changed.
   * @see getFileSelectionRows()
   */
  void fileSelectionChanged();

  /**
   * Emitted when a new cover art image is available
   * @param id ID of image.
   */
  void coverArtImageIdChanged(const QString& id);

private slots:
  void onSingleFileChanged();

private:
  void setNextCoverArtImageId();

  Frame* m_selectFrame;
  TaggedFile* m_editFrameTaggedFile;
  FrameObjectModel* m_frameObjectModel;
  Frame m_editFrame;
  QmlImageProvider* m_imageProvider;
  QString m_coverArtImageId;
};

#endif // KID3QMLAPPLICATION_H
