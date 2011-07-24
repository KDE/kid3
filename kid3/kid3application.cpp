/**
 * \file kid3application.cpp
 * Kid3 application logic, independent of GUI.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 Jul 2011
 *
 * Copyright (C) 2011  Urs Fleisch
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

#include "kid3application.h"
#include <QFileSystemModel>
#include <QItemSelectionModel>
#include <QTextCodec>
#include <QUrl>
#include <QTextStream>
#include "config.h"
#ifdef CONFIG_USE_KDE
#include <kapplication.h>
#else
#include <QApplication>
#endif
#include "fileproxymodel.h"
#include "dirproxymodel.h"
#include "modeliterator.h"
#include "trackdatamodel.h"
#include "frametablemodel.h"
#include "framelist.h"
#include "pictureframe.h"
#include "textimporter.h"
#include "textexporter.h"
#include "dirrenamer.h"
#include "configstore.h"
#include "playlistcreator.h"
#include "downloadclient.h"
#include "iframeeditor.h"
#include "qtcompatmac.h"
#ifdef HAVE_ID3LIB
#include "mp3file.h"
#endif
#ifdef HAVE_VORBIS
#include "oggfile.hpp"
#endif
#ifdef HAVE_FLAC
#include "flacfile.hpp"
#endif
#ifdef HAVE_MP4V2
#include "m4afile.h"
#endif
#ifdef HAVE_TAGLIB
#include "taglibfile.h"
#endif

/** Current directory */
QString Kid3Application::s_dirName;

/**
 * Constructor.
 * @param parent parent object
 */
Kid3Application::Kid3Application(QObject* parent) : QObject(parent),
  m_fileSystemModel(new QFileSystemModel(this)),
  m_fileProxyModel(new FileProxyModel(this)),
  m_dirProxyModel(new DirProxyModel(this)),
  m_fileSelectionModel(new QItemSelectionModel(m_fileProxyModel, this)),
  m_trackDataModel(new TrackDataModel(this)),
  m_framesV1Model(new FrameTableModel(true, this)),
  m_framesV2Model(new FrameTableModel(false, this)),
  m_framesV1SelectionModel(new QItemSelectionModel(m_framesV1Model, this)),
  m_framesV2SelectionModel(new QItemSelectionModel(m_framesV2Model, this)),
  m_framelist(new FrameList(m_framesV2Model, m_framesV2SelectionModel)),
  m_configStore(new ConfigStore),
  m_downloadClient(new DownloadClient(this)),
  m_textExporter(new TextExporter(this)),
  m_dirRenamer(new DirRenamer(this)),
  m_downloadImageDest(ImageForSelectedFiles)
{
  m_fileProxyModel->setSourceModel(m_fileSystemModel);
  m_dirProxyModel->setSourceModel(m_fileSystemModel);

  connect(m_fileSelectionModel,
          SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(fileSelected()));

  initFileTypes();
  setModified(false);
  setFiltered(false);
  ConfigStore::s_fnFormatCfg.setAsFilenameFormatter();
}

/**
 * Destructor.
 */
Kid3Application::~Kid3Application()
{
  delete m_configStore;
}

/**
 * Init file types.
 */
void Kid3Application::initFileTypes()
{
#ifdef HAVE_ID3LIB
  TaggedFile::addResolver(new Mp3File::Resolver);
#endif
#ifdef HAVE_VORBIS
  TaggedFile::addResolver(new OggFile::Resolver);
#endif
#ifdef HAVE_FLAC
  TaggedFile::addResolver(new FlacFile::Resolver);
#endif
#ifdef HAVE_MP4V2
  TaggedFile::addResolver(new M4aFile::Resolver);
#endif
#ifdef HAVE_TAGLIB
  TagLibFile::staticInit();
  TaggedFile::addResolver(new TagLibFile::Resolver);
#endif
}

/**
 * Get settings.
 * @return settings.
 */
Kid3Settings* Kid3Application::getSettings() const
{
  return m_configStore->getSettings();
}

/**
 * Save settings to the configuration.
 */
void Kid3Application::saveConfig()
{
  m_configStore->writeToConfig();
  m_configStore->getSettings()->sync();
}

/**
 * Read settings from the configuration.
 */
void Kid3Application::readConfig()
{
  m_configStore->readFromConfig();
  if (ConfigStore::s_miscCfg.m_nameFilter.isEmpty()) {
    createFilterString(&ConfigStore::s_miscCfg.m_nameFilter);
  }
  setTextEncodings();
  if (ConfigStore::s_freedbCfg.m_server == "freedb2.org:80") {
    ConfigStore::s_freedbCfg.m_server = "www.gnudb.org:80"; // replace old default
  }
  if (ConfigStore::s_trackTypeCfg.m_server == "gnudb.gnudb.org:80") {
    ConfigStore::s_trackTypeCfg.m_server = "tracktype.org:80"; // replace default
  }
}

/**
 * Open directory.
 *
 * @param dir       directory or file path
 * @param fileCheck if true and dir in not directory, only open directory
 *                  if dir is a valid file path
 *
 * @return true if ok, directoryOpened() is emitted.
 */
bool Kid3Application::openDirectory(QString dir, bool fileCheck)
{
  if (dir.isEmpty()) {
    return false;
  }
  QFileInfo file(dir);
  QString filePath;
  if (!file.isDir()) {
    if (fileCheck && !file.isFile()) {
      return false;
    }
    dir = file.absolutePath();
    filePath = file.absoluteFilePath();
  } else {
    dir = QDir(dir).absolutePath();
  }

  QStringList nameFilters(ConfigStore::s_miscCfg.m_nameFilter.split(' '));
  m_fileProxyModel->setNameFilters(nameFilters);
  m_fileSystemModel->setFilter(QDir::AllEntries | QDir::AllDirs);
  QModelIndex rootIndex = m_fileSystemModel->setRootPath(dir);
  QModelIndex fileIndex = m_fileSystemModel->index(filePath);
  bool ok = rootIndex.isValid();
  if (ok) {
    setModified(false);
    setFiltered(false);
    setDirName(dir);
    m_fileProxyModelRootIndex = m_fileProxyModel->mapFromSource(rootIndex);
    emit directoryOpened(rootIndex, fileIndex);
  }
  return ok;
}

/**
 * Get directory path of opened directory.
 * @return directory path.
 */
QString Kid3Application::getDirPath() const
{
  return FileProxyModel::getPathIfIndexOfDir(m_fileProxyModelRootIndex);
}

/**
 * Get current index in file proxy model or root index if current index is
 * invalid.
 * @return current index, root index if not valid.
 */
QModelIndex Kid3Application::currentOrRootIndex() const
{
  QModelIndex index(m_fileSelectionModel->currentIndex());
  if (index.isValid())
    return index;
  else
    return m_fileProxyModelRootIndex;
}

/**
 * Save all changed files.
 * saveStarted() and saveProgress() are emitted while saving files.
 *
 * @return list of files with error, empty if ok.
 */
QStringList Kid3Application::saveDirectory()
{
  QStringList errorFiles;
  int numFiles = 0, totalFiles = 0;
  // Get number of files to be saved to display correct progressbar
  TaggedFileIterator countIt(m_fileProxyModelRootIndex);
  while (countIt.hasNext()) {
    if (countIt.next()->isChanged()) {
      ++totalFiles;
    }
  }
  emit saveStarted(totalFiles);

  TaggedFileIterator it(m_fileProxyModelRootIndex);
  while (it.hasNext()) {
    TaggedFile* taggedFile = it.next();
    bool renamed = false;
    if (!taggedFile->writeTags(false, &renamed,
                               ConfigStore::s_miscCfg.m_preserveTime)) {
      errorFiles.push_back(taggedFile->getFilename());
    }
    ++numFiles;
    emit saveProgress(numFiles);
  }

  return errorFiles;
}

/**
 * Revert file modifications.
 * Acts on selected files or all files if no file is selected.
 */
void Kid3Application::revertFileModifications()
{
  SelectedTaggedFileIterator it(getRootIndex(),
                                getFileSelectionModel(),
                                true);
  while (it.hasNext()) {
    TaggedFile* taggedFile = it.next();
    taggedFile->readTags(true);
    // update icon
    getFileProxyModel()->emitDataChanged(taggedFile->getIndex(),
                                         taggedFile->getIndex());
  }
  if (!it.hasNoSelection()) {
    emit selectedFilesUpdated();
  }
  else {
    emit fileModified();
  }
}

/**
 * Import.
 *
 * @param tagMask tag mask
 * @param path    path of file
 * @param fmtIdx  index of format
 *
 * @return true if ok.
 */
bool Kid3Application::importTags(TrackData::TagVersion tagMask,
                                 const QString& path, int fmtIdx)
{
  filesToTrackDataModel(ConfigStore::s_genCfg.m_importDest);
  QFile file(path);
  if (file.open(QIODevice::ReadOnly) &&
      fmtIdx < ConfigStore::s_genCfg.m_importFormatHeaders.size()) {
    TextImporter(getTrackDataModel()).updateTrackData(
      QTextStream(&file).readAll(),
      ConfigStore::s_genCfg.m_importFormatHeaders.at(fmtIdx),
      ConfigStore::s_genCfg.m_importFormatTracks.at(fmtIdx));
    file.close();
    trackDataModelToFiles(tagMask);
    return true;
  }
  return false;
}

/**
 * Export.
 *
 * @param tagVersion tag version
 * @param path   path of file
 * @param fmtIdx index of format
 *
 * @return true if ok.
 */
bool Kid3Application::exportTags(TrackData::TagVersion tagVersion,
                                 const QString& path, int fmtIdx)
{
  ImportTrackDataVector trackDataVector;
  filesToTrackData(tagVersion, trackDataVector);
  m_textExporter->setTrackData(trackDataVector);
  m_textExporter->updateTextUsingConfig(fmtIdx);
  return m_textExporter->exportToFile(path);
}

/**
 * Write playlist according to playlist configuration.
 *
 * @param cfg playlist configuration to use
 *
 * @return true if ok.
 */
bool Kid3Application::writePlaylist(const PlaylistConfig& cfg)
{
  PlaylistCreator plCtr(getDirPath(), cfg);
  QItemSelectionModel* selectModel = getFileSelectionModel();
  bool noSelection = !cfg.m_onlySelectedFiles || !selectModel ||
                     !selectModel->hasSelection();
  bool ok = true;
  QModelIndex rootIndex;

  if (cfg.m_location == PlaylistConfig::PL_CurrentDirectory) {
    // Get first child of parent of current index.
    rootIndex = currentOrRootIndex();
    if (rootIndex.model() && rootIndex.model()->rowCount(rootIndex) <= 0)
      rootIndex = rootIndex.parent();
    if (const QAbstractItemModel* model = rootIndex.model()) {
      for (int row = 0; row < model->rowCount(rootIndex); ++row) {
        QModelIndex index = model->index(row, 0, rootIndex);
        PlaylistCreator::Item plItem(index, plCtr);
        if (plItem.isFile() &&
            (noSelection || selectModel->isSelected(index))) {
          ok = plItem.add() && ok;
        }
      }
    }
  } else {
    QString selectedDirPrefix;
    rootIndex = getRootIndex();
    ModelIterator it(rootIndex);
    while (it.hasNext()) {
      QModelIndex index = it.next();
      PlaylistCreator::Item plItem(index, plCtr);
      bool inSelectedDir = false;
      if (plItem.isDir()) {
        if (!selectedDirPrefix.isEmpty()) {
          if (plItem.getDirName().startsWith(selectedDirPrefix)) {
            inSelectedDir = true;
          } else {
            selectedDirPrefix = "";
          }
        }
        if (inSelectedDir || noSelection || selectModel->isSelected(index)) {
          // if directory is selected, all its files are selected
          if (!inSelectedDir) {
            selectedDirPrefix = plItem.getDirName();
          }
        }
      } else if (plItem.isFile()) {
        QString dirName = plItem.getDirName();
        if (!selectedDirPrefix.isEmpty()) {
          if (dirName.startsWith(selectedDirPrefix)) {
            inSelectedDir = true;
          } else {
            selectedDirPrefix = "";
          }
        }
        if (inSelectedDir || noSelection || selectModel->isSelected(index)) {
          ok = plItem.add() && ok;
        }
      }
    }
  }

  ok = plCtr.write() && ok;
  return ok;
}

/**
 * Write playlist using current playlist configuration.
 *
 * @return true if ok.
 */
bool Kid3Application::writePlaylist()
{
  return writePlaylist(ConfigStore::s_playlistCfg);
}

/**
 * Set track data with tagged files of directory.
 *
 * @param tagVersion tag version
 * @param trackDataList is filled with track data
 */
void Kid3Application::filesToTrackData(TrackData::TagVersion tagVersion,
                                       ImportTrackDataVector& trackDataList)
{
  TaggedFileOfDirectoryIterator it(currentOrRootIndex());
  while (it.hasNext()) {
    TaggedFile* taggedFile = it.next();
    taggedFile->readTags(false);
#if defined HAVE_ID3LIB && defined HAVE_TAGLIB
    taggedFile = FileProxyModel::readWithTagLibIfId3V24(taggedFile);
#endif
    trackDataList.push_back(ImportTrackData(*taggedFile, tagVersion));
  }
}

/**
 * Set track data model with tagged files of directory.
 *
 * @param tagVersion tag version
 */
void Kid3Application::filesToTrackDataModel(TrackData::TagVersion tagVersion)
{
  ImportTrackDataVector trackDataList;
  filesToTrackData(tagVersion, trackDataList);
  getTrackDataModel()->setTrackData(trackDataList);
}

/**
 * Set tagged files of directory from track data model.
 *
 * @param tagVersion tags to set
 */
void Kid3Application::trackDataModelToFiles(TrackData::TagVersion tagVersion)
{
  ImportTrackDataVector trackDataList(getTrackDataModel()->getTrackData());
  ImportTrackDataVector::iterator it = trackDataList.begin();
  FrameFilter flt((tagVersion & TrackData::TagV1) ?
                  frameModelV1()->getEnabledFrameFilter(true) :
                  frameModelV2()->getEnabledFrameFilter(true));
  TaggedFileOfDirectoryIterator tfit(currentOrRootIndex());
  while (tfit.hasNext()) {
    TaggedFile* taggedFile = tfit.next();
    taggedFile->readTags(false);
    if (it != trackDataList.end()) {
      it->removeDisabledFrames(flt);
      formatFramesIfEnabled(*it);
      if (tagVersion & TrackData::TagV1) taggedFile->setFramesV1(*it, false);
      if (tagVersion & TrackData::TagV2) {
        FrameCollection oldFrames;
        taggedFile->getAllFramesV2(oldFrames);
        it->markChangedFrames(oldFrames);
        taggedFile->setFramesV2(*it, true);
      }
      ++it;
    } else {
      break;
    }
  }

  if ((tagVersion & TrackData::TagV2) && flt.isEnabled(Frame::FT_Picture) &&
      !trackDataList.getCoverArtUrl().isEmpty()) {
    downloadImage(trackDataList.getCoverArtUrl(), ImageForImportTrackData);
  }

  if (getFileSelectionModel()->hasSelection()) {
    emit selectedFilesUpdated();
  }
  else {
    emit fileModified();
  }
}

/**
 * Download an image file.
 *
 * @param url  URL of image
 * @param dest specifies affected files
 */
void Kid3Application::downloadImage(const QString& url, DownloadImageDestination dest)
{
  QString imgurl(getImageUrl(url));
  if (!imgurl.isEmpty()) {
    int hostPos = imgurl.indexOf("://");
    if (hostPos > 0) {
      int pathPos = imgurl.indexOf("/", hostPos + 3);
      if (pathPos > hostPos) {
        m_downloadImageDest = dest;
        m_downloadClient->startDownload(
          imgurl.mid(hostPos + 3, pathPos - hostPos - 3),
          imgurl.mid(pathPos));
      }
    }
  }
}

/**
 * Format a filename if format while editing is switched on.
 *
 * @param taggedFile file to modify
 */
void Kid3Application::formatFileNameIfEnabled(TaggedFile* taggedFile) const
{
  if (ConfigStore::s_fnFormatCfg.m_formatWhileEditing) {
    QString fn(taggedFile->getFilename());
    ConfigStore::s_fnFormatCfg.formatString(fn);
    taggedFile->setFilename(fn);
  }
}

/**
 * Format frames if format while editing is switched on.
 *
 * @param frames frames
 */
void Kid3Application::formatFramesIfEnabled(FrameCollection& frames) const
{
  if (ConfigStore::s_id3FormatCfg.m_formatWhileEditing) {
    ConfigStore::s_id3FormatCfg.formatFrames(frames);
  }
}

/**
 * Get name of selected file.
 *
 * @return absolute file name, ends with "/" if it is a directory.
 */
QString Kid3Application::getFileNameOfSelectedFile()
{
  QModelIndex index = getFileSelectionModel()->currentIndex();
  QString dirname = FileProxyModel::getPathIfIndexOfDir(index);
  if (!dirname.isNull()) {
    if (!dirname.endsWith('/')) dirname += '/';
    return dirname;
  } else if (TaggedFile* taggedFile =
             FileProxyModel::getTaggedFileOfIndex(index)) {
    return taggedFile->getAbsFilename();
  }
  return "";
}

/**
 * Set name of selected file.
 * Exactly one file has to be selected.
 *
 * @param name file name.
 */
void Kid3Application::setFileNameOfSelectedFile(const QString& name)
{
  if (TaggedFile* taggedFile = getSelectedFile()) {
    QFileInfo fi(name);
    taggedFile->setFilename(fi.fileName());
    emit selectedFilesUpdated();
  }
}

/**
 * Apply filename format.
 */
void Kid3Application::applyFilenameFormat()
{
  emit fileSelectionUpdateRequested();
  SelectedTaggedFileIterator it(getRootIndex(),
                                getFileSelectionModel(),
                                true);
  while (it.hasNext()) {
    TaggedFile* taggedFile = it.next();
    taggedFile->readTags(false);
    QString fn = taggedFile->getFilename();
    ConfigStore::s_fnFormatCfg.formatString(fn);
    taggedFile->setFilename(fn);
  }
  emit selectedFilesUpdated();
}

/**
 * Apply ID3 format.
 */
void Kid3Application::applyId3Format()
{
  emit fileSelectionUpdateRequested();
  FrameCollection frames;
  FrameFilter fltV1(frameModelV1()->getEnabledFrameFilter(true));
  FrameFilter fltV2(frameModelV2()->getEnabledFrameFilter(true));
  SelectedTaggedFileIterator it(getRootIndex(),
                                getFileSelectionModel(),
                                true);
  while (it.hasNext()) {
    TaggedFile* taggedFile = it.next();
    taggedFile->readTags(false);
    taggedFile->getAllFramesV1(frames);
    frames.removeDisabledFrames(fltV1);
    ConfigStore::s_id3FormatCfg.formatFrames(frames);
    taggedFile->setFramesV1(frames);
    taggedFile->getAllFramesV2(frames);
    frames.removeDisabledFrames(fltV2);
    ConfigStore::s_id3FormatCfg.formatFrames(frames);
    taggedFile->setFramesV2(frames);
  }
  emit selectedFilesUpdated();
}

/**
 * Copy tags 1 into copy buffer.
 */
void Kid3Application::copyTagsV1()
{
  emit fileSelectionUpdateRequested();
  m_copyTags = frameModelV1()->frames().copyEnabledFrames(
    frameModelV1()->getEnabledFrameFilter(true));
}

/**
 * Copy tags 2 into copy buffer.
 */
void Kid3Application::copyTagsV2()
{
  emit fileSelectionUpdateRequested();
  m_copyTags = frameModelV2()->frames().copyEnabledFrames(
    frameModelV2()->getEnabledFrameFilter(true));
}

/**
 * Paste from copy buffer to ID3v1 tags.
 */
void Kid3Application::pasteTagsV1()
{
  emit fileSelectionUpdateRequested();
  FrameCollection frames(m_copyTags.copyEnabledFrames(
                         frameModelV1()->getEnabledFrameFilter(true)));
  formatFramesIfEnabled(frames);
  SelectedTaggedFileIterator it(getRootIndex(),
                                getFileSelectionModel(),
                                false);
  while (it.hasNext()) {
    it.next()->setFramesV1(frames, false);
  }
  emit selectedFilesUpdated();
}

/**
 * Paste from copy buffer to ID3v2 tags.
 */
void Kid3Application::pasteTagsV2()
{
  emit fileSelectionUpdateRequested();
  FrameCollection frames(m_copyTags.copyEnabledFrames(
                         frameModelV2()->getEnabledFrameFilter(true)));
  formatFramesIfEnabled(frames);
  SelectedTaggedFileIterator it(getRootIndex(),
                                getFileSelectionModel(),
                                false);
  while (it.hasNext()) {
    it.next()->setFramesV2(frames, false);
  }
  emit selectedFilesUpdated();
}

/**
 * Copy ID3v1 tags to ID3v2 tags of selected files.
 */
void Kid3Application::copyV1ToV2()
{
  emit fileSelectionUpdateRequested();
  FrameCollection frames;
  FrameFilter flt(frameModelV2()->getEnabledFrameFilter(true));
  SelectedTaggedFileIterator it(getRootIndex(),
                                getFileSelectionModel(),
                                false);
  while (it.hasNext()) {
    TaggedFile* taggedFile = it.next();
    taggedFile->getAllFramesV1(frames);
    frames.removeDisabledFrames(flt);
    formatFramesIfEnabled(frames);
    taggedFile->setFramesV2(frames, false);
  }
  emit selectedFilesUpdated();
}

/**
 * Copy ID3v2 tags to ID3v1 tags of selected files.
 */
void Kid3Application::copyV2ToV1()
{
  emit fileSelectionUpdateRequested();
  FrameCollection frames;
  FrameFilter flt(frameModelV1()->getEnabledFrameFilter(true));
  SelectedTaggedFileIterator it(getRootIndex(),
                                getFileSelectionModel(),
                                false);
  while (it.hasNext()) {
    TaggedFile* taggedFile = it.next();
    taggedFile->getAllFramesV2(frames);
    frames.removeDisabledFrames(flt);
    formatFramesIfEnabled(frames);
    taggedFile->setFramesV1(frames, false);
  }
  emit selectedFilesUpdated();
}

/**
 * Remove ID3v1 tags in selected files.
 */
void Kid3Application::removeTagsV1()
{
  emit fileSelectionUpdateRequested();
  FrameFilter flt(frameModelV1()->getEnabledFrameFilter(true));
  SelectedTaggedFileIterator it(getRootIndex(),
                                getFileSelectionModel(),
                                false);
  while (it.hasNext()) {
    it.next()->deleteFramesV1(flt);
  }
  emit selectedFilesUpdated();
}

/**
 * Remove ID3v2 tags in selected files.
 */
void Kid3Application::removeTagsV2()
{
  emit fileSelectionUpdateRequested();
  FrameFilter flt(frameModelV2()->getEnabledFrameFilter(true));
  SelectedTaggedFileIterator it(getRootIndex(),
                                getFileSelectionModel(),
                                false);
  while (it.hasNext()) {
    it.next()->deleteFramesV2(flt);
  }
  emit selectedFilesUpdated();
}

/**
 * Set ID3v1 tags according to filename.
 * If a single file is selected the tags in the GUI controls
 * are set, else the tags in the multiple selected files.
 */
void Kid3Application::getTagsFromFilenameV1()
{
  emit fileSelectionUpdateRequested();
  FrameCollection frames;
  QItemSelectionModel* selectModel = getFileSelectionModel();
  SelectedTaggedFileIterator it(getRootIndex(),
                                selectModel,
                                false);
  FrameFilter flt(frameModelV1()->getEnabledFrameFilter(true));
  while (it.hasNext()) {
    TaggedFile* taggedFile = it.next();
    taggedFile->getAllFramesV1(frames);
    taggedFile->getTagsFromFilename(frames, m_filenameToTagsFormat);
    frames.removeDisabledFrames(flt);
    formatFramesIfEnabled(frames);
    taggedFile->setFramesV1(frames);
  }
  emit selectedFilesUpdated();
}

/**
 * Set ID3v2 tags according to filename.
 * If a single file is selected the tags in the GUI controls
 * are set, else the tags in the multiple selected files.
 */
void Kid3Application::getTagsFromFilenameV2()
{
  emit fileSelectionUpdateRequested();
  FrameCollection frames;
  QItemSelectionModel* selectModel = getFileSelectionModel();
  SelectedTaggedFileIterator it(getRootIndex(),
                                selectModel,
                                false);
  FrameFilter flt(frameModelV2()->getEnabledFrameFilter(true));
  while (it.hasNext()) {
    TaggedFile* taggedFile = it.next();
    taggedFile->getAllFramesV2(frames);
    taggedFile->getTagsFromFilename(frames, m_filenameToTagsFormat);
    frames.removeDisabledFrames(flt);
    formatFramesIfEnabled(frames);
    taggedFile->setFramesV2(frames);
  }
  emit selectedFilesUpdated();
}

/**
 * Set filename according to tags.
 * If a single file is selected the tags in the GUI controls
 * are used, else the tags in the multiple selected files.
 *
 * @param tagVersion tag version
 */
void Kid3Application::getFilenameFromTags(TrackData::TagVersion tagVersion)
{
  emit fileSelectionUpdateRequested();
  QItemSelectionModel* selectModel = getFileSelectionModel();
  SelectedTaggedFileIterator it(getRootIndex(),
                                selectModel,
                                false);
  while (it.hasNext()) {
    TaggedFile* taggedFile = it.next();
    TrackData trackData(*taggedFile, tagVersion);
    if (!trackData.isEmptyOrInactive()) {
      taggedFile->setFilename(
            trackData.formatFilenameFromTags(m_tagsToFilenameFormat));
      formatFileNameIfEnabled(taggedFile);
    }
  }
  emit selectedFilesUpdated();
}

/**
 * Set format used to generate filename from tags.
 * When changed, filenameToTagsFormatChanged() is emitted.
 * @param format format
 */
void Kid3Application::setFilenameToTagsFormat(const QString& format) {
  if (m_filenameToTagsFormat != format) {
    m_filenameToTagsFormat = format;
    emit filenameToTagsFormatChanged(format);
  }
}

/**
 * Set format used to generate tags from filename.
 * When changed, tagsToFilenameFormatChanged() is emitted.
 * @param format format
 */
void Kid3Application::setTagsToFilenameFormat(const QString& format) {
  if (m_tagsToFilenameFormat != format) {
    m_tagsToFilenameFormat = format;
    emit tagsToFilenameFormatChanged(format);
  }
}

/**
 * Get the selected file.
 *
 * @return the selected file,
 *         0 if not exactly one file is selected
 */
TaggedFile* Kid3Application::getSelectedFile()
{
  QModelIndexList selItems(
      m_fileSelectionModel->selectedIndexes());
  if (selItems.size() != 1)
    return 0;

  return FileProxyModel::getTaggedFileOfIndex(selItems.first());
}


/**
 * Edit selected frame.
 *
 * @param frameEditor editor for frame fields
 */
void Kid3Application::editFrame(IFrameEditor* frameEditor)
{
  emit fileSelectionUpdateRequested();
  TaggedFile* taggedFile = getSelectedFile();
  if (const Frame* selectedFrame = frameModelV2()->getFrameOfIndex(
        getFramesV2SelectionModel()->currentIndex())) {
    Frame frame(*selectedFrame);
    if (taggedFile && frameEditor->editFrameOfTaggedFile(&frame, taggedFile)) {
      emit frameModified(taggedFile);
    } else if (!taggedFile) {
      // multiple files selected
      bool firstFile = true;
      QString name;
      SelectedTaggedFileIterator tfit(getRootIndex(),
                                      getFileSelectionModel(),
                                      false);
      while (tfit.hasNext()) {
        TaggedFile* currentFile = tfit.next();
        if (firstFile) {
          firstFile = false;
          taggedFile = currentFile;
          m_framelist->setTaggedFile(taggedFile);
          name = m_framelist->getSelectedName();
          if (name.isEmpty() ||
              !frameEditor->editFrameOfTaggedFile(&frame, taggedFile)) {
            break;
          }
          m_framelist->setFrame(frame);
        }
        FrameCollection frames;
        currentFile->getAllFramesV2(frames);
        for (FrameCollection::const_iterator it = frames.begin();
             it != frames.end();
             ++it) {
          if (it->getName() == name) {
            currentFile->deleteFrameV2(*it);
            m_framelist->setTaggedFile(currentFile);
            m_framelist->pasteFrame();
            break;
          }
        }
      }
      emit frameModified(taggedFile);
    }
  }
}

/**
 * Delete selected frame.
 *
 * @param frameName name of frame to delete, empty to delete selected frame
 */
void Kid3Application::deleteFrame(const QString& frameName)
{
  emit fileSelectionUpdateRequested();
  TaggedFile* taggedFile = getSelectedFile();
  if (taggedFile && frameName.isEmpty()) {
    // delete selected frame from single file
    if (!m_framelist->deleteFrame()) {
      // frame not found
      return;
    }
  } else {
    // multiple files selected or frame name specified
    bool firstFile = true;
    QString name;
    SelectedTaggedFileIterator tfit(getRootIndex(),
                                    getFileSelectionModel(),
                                    false);
    while (tfit.hasNext()) {
      TaggedFile* currentFile = tfit.next();
      if (firstFile) {
        firstFile = false;
        taggedFile = currentFile;
        m_framelist->setTaggedFile(taggedFile);
        name = frameName.isEmpty() ? m_framelist->getSelectedName() :
          frameName;
      }
      FrameCollection frames;
      currentFile->getAllFramesV2(frames);
      for (FrameCollection::const_iterator it = frames.begin();
           it != frames.end();
           ++it) {
        if (it->getName() == name) {
          currentFile->deleteFrameV2(*it);
          break;
        }
      }
    }
  }
  emit frameModified(taggedFile);
}

/**
 * Let the user select and edit a frame type and then edit the frame.
 * Add the frame if the edits are accepted.
 *
 * @param frameEditor frame editor
 *
 * @return true if edits accepted.
 */
bool Kid3Application::selectAddAndEditFrame(IFrameEditor* frameEditor)
{
  if (TaggedFile* taggedFile = m_framelist->getTaggedFile()) {
    Frame frame;
    if (frameEditor->selectFrame(&frame, taggedFile)) {
      m_framelist->setFrame(frame);
      return m_framelist->addAndEditFrame(frameEditor);
    }
  }
  return false;
}

/**
 * Select a frame type and add such a frame to frame list.
 *
 * @param frame frame to add, if 0 the user has to select and edit the frame
 * @param frameEditor editor for frame fields, if not null and a frame
 * is set, the user can edit the frame before it is added
 */
void Kid3Application::addFrame(const Frame* frame, IFrameEditor* frameEditor)
{
  emit fileSelectionUpdateRequested();
  TaggedFile* taggedFile = getSelectedFile();
  if (taggedFile) {
    bool frameAdded;
    if (!frame) {
      frameAdded = selectAddAndEditFrame(frameEditor);
    } else if (frameEditor) {
      m_framelist->setFrame(*frame);
      frameAdded = m_framelist->addAndEditFrame(frameEditor);
    } else {
      m_framelist->setFrame(*frame);
      frameAdded = m_framelist->pasteFrame();
    }
    if (frameAdded) {
      emit frameModified(taggedFile);
      if (m_framelist->isPictureFrame()) {
        // update preview picture
        emit selectedFilesUpdated();
      }
    }
  } else {
    // multiple files selected
    bool firstFile = true;
    int frameId = -1;
    SelectedTaggedFileIterator tfit(getRootIndex(),
                                    getFileSelectionModel(),
                                    false);
    while (tfit.hasNext()) {
      TaggedFile* currentFile = tfit.next();
      if (firstFile) {
        firstFile = false;
        taggedFile = currentFile;
        m_framelist->setTaggedFile(currentFile);
        if (!frame) {
          if (selectAddAndEditFrame(frameEditor)) {
            frameId = m_framelist->getSelectedId();
          } else {
            break;
          }
        } else if (frameEditor) {
          m_framelist->setFrame(*frame);
          if (m_framelist->addAndEditFrame(frameEditor)) {
            frameId = m_framelist->getSelectedId();
          } else {
            break;
          }
        } else {
          m_framelist->setFrame(*frame);
          if (m_framelist->pasteFrame()) {
            frameId = m_framelist->getSelectedId();
          } else {
            break;
          }
        }
      } else {
        m_framelist->setTaggedFile(currentFile);
        m_framelist->pasteFrame();
      }
    }
    m_framelist->setTaggedFile(taggedFile);
    if (frameId != -1) {
      m_framelist->setSelectedId(frameId);
    }
    emit fileModified();
  }
}

/**
 * Edit a picture frame if one exists or add a new one.
 *
 * @param frameEditor editor for frame fields
 */
void Kid3Application::editOrAddPicture(IFrameEditor* frameEditor)
{
  if (m_framelist->selectByName("Picture")) {
    editFrame(frameEditor);
  } else {
    PictureFrame frame;
    addFrame(&frame, frameEditor);
  }
}

/**
 * Open directory on drop.
 *
 * @param txt URL of directory or file in directory
 */
void Kid3Application::openDrop(QString txt)
{
  int lfPos = txt.indexOf('\n');
  if (lfPos > 0 && lfPos < (int)txt.length() - 1) {
    txt.truncate(lfPos + 1);
  }
  QUrl url(txt);
  if (!url.path().isEmpty()) {
#if defined _WIN32 || defined WIN32
    QString dir = url.toString();
#else
    QString dir = url.path().trimmed();
#endif
    if (dir.endsWith(".jpg", Qt::CaseInsensitive) ||
        dir.endsWith(".jpeg", Qt::CaseInsensitive) ||
        dir.endsWith(".png", Qt::CaseInsensitive)) {
      PictureFrame frame;
      if (PictureFrame::setDataFromFile(frame, dir)) {
        QString fileName(dir);
        int slashPos = fileName.lastIndexOf('/');
        if (slashPos != -1) {
          fileName = fileName.mid(slashPos + 1);
        }
        PictureFrame::setMimeTypeFromFileName(frame, fileName);
        PictureFrame::setDescription(frame, fileName);
        addFrame(&frame);
        emit selectedFilesUpdated();
      }
    } else {
      emit fileSelectionUpdateRequested();
      emit confirmedOpenDirectoryRequested(dir);
    }
  }
}

/**
 * Add picture on drop.
 *
 * @param image dropped image.
 */
void Kid3Application::dropImage(const QImage& image)
{
  if (!image.isNull()) {
    PictureFrame frame;
    if (PictureFrame::setDataFromImage(frame, image)) {
      addFrame(&frame);
      emit selectedFilesUpdated();
    }
  }
}

/**
 * Handle URL on drop.
 *
 * @param txt dropped URL.
 */
void Kid3Application::dropUrl(const QString& txt)
{
  downloadImage(txt, Kid3Application::ImageForSelectedFiles);
}

/**
 * Add a downloaded image.
 *
 * @param data     HTTP response of download
 * @param mimeType MIME type of data
 * @param url      URL of downloaded data
 */
void Kid3Application::imageDownloaded(const QByteArray& data,
                              const QString& mimeType, const QString& url)
{
  if (mimeType.startsWith("image")) {
    PictureFrame frame(data, url, PictureFrame::PT_CoverFront, mimeType);
    if (getDownloadImageDestination() == ImageForAllFilesInDirectory) {
      TaggedFileOfDirectoryIterator it(currentOrRootIndex());
      while (it.hasNext()) {
        TaggedFile* taggedFile = it.next();
        taggedFile->readTags(false);
        taggedFile->addFrameV2(frame);
      }
    } else if (getDownloadImageDestination() == ImageForImportTrackData) {
      const ImportTrackDataVector& trackDataVector(
            getTrackDataModel()->trackData());
      for (ImportTrackDataVector::const_iterator it =
           trackDataVector.constBegin();
           it != trackDataVector.constEnd();
           ++it) {
        TaggedFile* taggedFile;
        if (it->isEnabled() && (taggedFile = it->getTaggedFile()) != 0) {
          taggedFile->readTags(false);
          taggedFile->addFrameV2(frame);
        }
      }
    } else {
      addFrame(&frame);
    }
    emit selectedFilesUpdated();
  }
}

/**
 * Select the first file.
 *
 * @return true if a file exists.
 */
bool Kid3Application::selectFirstFile()
{
  m_fileSelectionModel->setCurrentIndex(getRootIndex(),
                                        QItemSelectionModel::SelectCurrent);
  return selectNextFile();
}

/**
 * Select the next file.
 *
 * @return true if a next file exists.
 */
bool Kid3Application::selectNextFile()
{
  QModelIndex current(m_fileSelectionModel->currentIndex()), next;
  if (m_fileProxyModel->rowCount(current) > 0) {
    // to first child
    next = m_fileProxyModel->index(0, 0, current);
  } else {
    QModelIndex parent = current;
    while (!next.isValid() && parent.isValid()) {
      // to next sibling or next sibling of parent
      int row = parent.row();
      if (parent == getRootIndex()) {
        // do not move beyond root index
        return false;
      }
      parent = parent.parent();
      if (row + 1 < m_fileProxyModel->rowCount(parent)) {
        // to next sibling
        next = m_fileProxyModel->index(row + 1, 0, parent);
      }
    }
  }
  if (!next.isValid())
    return false;
  m_fileSelectionModel->setCurrentIndex(next,
                                        QItemSelectionModel::SelectCurrent);
  return true;
}

/**
 * Select the previous file.
 *
 * @return true if a previous file exists.
 */
bool Kid3Application::selectPreviousFile()
{
  QModelIndex current(m_fileSelectionModel->currentIndex()), previous;
  int row = current.row() - 1;
  if (row >= 0) {
    // to last leafnode of previous sibling
    previous = current.sibling(row, 0);
    row = m_fileProxyModel->rowCount(previous) - 1;
    while (row >= 0) {
      previous = m_fileProxyModel->index(row, 0, previous);
      row = m_fileProxyModel->rowCount(previous) - 1;
    }
  } else {
    // to parent
    previous = current.parent();
  }
  if (!previous.isValid() || previous == getRootIndex())
    return false;
  m_fileSelectionModel->setCurrentIndex(previous,
                                        QItemSelectionModel::SelectCurrent);
  return true;
}

/**
 * Process change of selection.
 * The GUI is signaled to update the current selection and the controls.
 */
void Kid3Application::fileSelected()
{
  emit fileSelectionUpdateRequested();
  emit selectedFilesUpdated();
}

/**
 * Schedule actions to rename a directory.
 */
void Kid3Application::scheduleRenameActions()
{
  getDirRenamer()->clearActions();
  TaggedFileIterator it(getRootIndex());
  while (it.hasNext()) {
    TaggedFile* taggedFile = it.next();
    taggedFile->readTags(false);
#if defined HAVE_ID3LIB && defined HAVE_TAGLIB
    taggedFile = FileProxyModel::readWithTagLibIfId3V24(taggedFile);
#endif
    getDirRenamer()->scheduleAction(taggedFile);
#ifdef CONFIG_USE_KDE
    kapp->processEvents();
#else
    qApp->processEvents();
#endif
    if (getDirRenamer()->getAbortFlag()) {
      break;
    }
  }
}

/**
 * Apply a file filter.
 *
 * @param fileFilter filter to apply.
 */
void Kid3Application::applyFilter(FileFilter& fileFilter)
{
  m_fileProxyModel->disableFilteringOutIndexes();
  setFiltered(false);
  fileFilter.clearAbortFlag();

  bool ok = true;
  unsigned numFiles = 0;
  TaggedFileIterator it(m_fileProxyModelRootIndex);
  while (it.hasNext()) {
    TaggedFile* taggedFile = it.next();

    taggedFile->readTags(false);
#if defined HAVE_ID3LIB && defined HAVE_TAGLIB
    taggedFile = FileProxyModel::readWithTagLibIfId3V24(taggedFile);
#endif
    bool pass = fileFilter.filter(*taggedFile, &ok);
    if (!ok) {
      emit fileFiltered(FileFilter::ParseError, QString());
      break;
    }
    emit fileFiltered(
          pass ? FileFilter::FilePassed : FileFilter::FileFilteredOut,
          taggedFile->getFilename());
    if (!pass)
      m_fileProxyModel->filterOutIndex(taggedFile->getIndex());

    if (++numFiles == 8) {
      numFiles = 0;
#ifdef CONFIG_USE_KDE
      kapp->processEvents();
#else
      qApp->processEvents();
#endif
      if (fileFilter.getAbortFlag())
        break;
    }
  }

  m_fileProxyModel->applyFilteringOutIndexes();
  setFiltered(!fileFilter.isEmptyFilterExpression());
  emit fileModified();
}

/**
 * Apply a file filter.
 *
 * @param expression filter expression
 */
void Kid3Application::applyFilter(const QString& expression)
{
  FileFilter filter;
  filter.setFilterExpression(expression);
  filter.initParser();
  applyFilter(filter);
}

/**
 * Set the directory name from the tags.
 * The directory must not have modified files.
 *
 * @param tagMask tag mask
 * @param format  directory name format
 * @param create  true to create, false to rename
 * @param errStr  if not 0, a string describing the error is returned here
 *
 * @return true if ok.
 */
bool Kid3Application::renameDirectory(TrackData::TagVersion tagMask,
                                     const QString& format,
                                     bool create, QString* errStr)
{
  bool ok = false;
  TaggedFile* taggedFile =
    TaggedFileOfDirectoryIterator::first(currentOrRootIndex());
  if (!isModified() && taggedFile) {
    m_dirRenamer->setTagVersion(tagMask);
    m_dirRenamer->setFormat(format);
    m_dirRenamer->setAction(create);
    scheduleRenameActions();
    openDirectory(getDirName());
    QString errorMsg;
    m_dirRenamer->performActions(&errorMsg);
    ok = errorMsg.isEmpty();
    if (errStr) {
      *errStr = errorMsg;
    }
  }
  return ok;
}

/**
 * Number tracks in selected files of directory.
 *
 * @param nr start number
 * @param total total number of tracks, used if >0
  * @param tagVersion determines on which tags the numbers are set
 */
void Kid3Application::numberTracks(int nr, int total,
                                   TrackData::TagVersion tagVersion)
{
  emit fileSelectionUpdateRequested();
  int numDigits = ConfigStore::s_miscCfg.m_trackNumberDigits;
  if (numDigits < 1 || numDigits > 5)
    numDigits = 1;

  SelectedTaggedFileOfDirectoryIterator it(
      currentOrRootIndex(),
      getFileSelectionModel(),
      true);
  while (it.hasNext()) {
    TaggedFile* taggedFile = it.next();
    taggedFile->readTags(false);
    if (tagVersion & TrackData::TagV1) {
      int oldnr = taggedFile->getTrackNumV1();
      if (nr != oldnr) {
        taggedFile->setTrackNumV1(nr);
      }
    }
    if (tagVersion & TrackData::TagV2) {
      // For tag 2 the frame is written, so that we have control over the
      // format and the total number of tracks, and it is possible to change
      // the format even if the numbers stay the same.
      QString value;
      if (total > 0) {
        value.sprintf("%0*d/%0*d", numDigits, nr, numDigits, total);
      } else {
        value.sprintf("%0*d", numDigits, nr);
      }
      FrameCollection frames;
      taggedFile->getAllFramesV2(frames);
      Frame frame(Frame::FT_Track, "", "", -1);
      FrameCollection::const_iterator it = frames.find(frame);
      if (it != frames.end()) {
        frame = *it;
        frame.setValueIfChanged(value);
        if (frame.isValueChanged()) {
          taggedFile->setFrameV2(frame);
        }
      } else {
        frame.setValue(value);
        frame.setInternalName(Frame::getNameFromType(Frame::FT_Track));
        taggedFile->setFrameV2(frame);
      }
    }
    ++nr;
  }
  emit selectedFilesUpdated();
}

/**
 * Get number of tracks in current directory.
 *
 * @return number of tracks, 0 if not found.
 */
int Kid3Application::getTotalNumberOfTracksInDir()
{
  if (TaggedFile* taggedFile = TaggedFileOfDirectoryIterator::first(
      currentOrRootIndex())) {
    return taggedFile->getTotalNumberOfTracksInDir();
  }
  return 0;
}

/**
 * Get the URL of an image file.
 * The input URL is transformed using the match picture URL table to
 * get the URL of an image file.
 *
 * @param url URL from image drag
 *
 * @return URL of image file, empty if no image URL found.
 */
QString Kid3Application::getImageUrl(const QString& url)
{
  QString imgurl;
  if (url.startsWith("http://")) {
    if (url.endsWith(".jpg", Qt::CaseInsensitive) ||
        url.endsWith(".jpeg", Qt::CaseInsensitive) ||
        url.endsWith(".png", Qt::CaseInsensitive)) {
      imgurl = url;
    }
    else {
      for (QMap<QString, QString>::ConstIterator it =
             ConfigStore::s_genCfg.m_matchPictureUrlMap.begin();
           it != ConfigStore::s_genCfg.m_matchPictureUrlMap.end();
           ++it) {
        QRegExp re(it.key());
        if (re.exactMatch(url)) {
          QString pictureUrl(url);
          imgurl = url;
          imgurl.replace(re, *it);
          if (imgurl.indexOf("%25") != -1) {
            // double URL encoded: first decode
            imgurl = QUrl::fromPercentEncoding(imgurl.toUtf8());
          }
          if (imgurl.indexOf("%2F") != -1) {
            // URL encoded: decode
            imgurl = QUrl::fromPercentEncoding(imgurl.toUtf8());
          }
          break;
        }
      }
    }
  }
  return imgurl;
}

#ifndef WIN32
/**
 * Get all combinations with lower- and uppercase characters.
 *
 * @param str original string
 *
 * @return string with all combinations, separated by spaces.
 */
static QString lowerUpperCaseCombinations(const QString& str)
{
  QString result;
  QString lc(str.toLower());
  QString uc(str.toUpper());

  // get a mask of all alphabetic characters in the string
  unsigned char numChars = 0, charMask = 0, posMask = 1;
  int numPos = lc.length();
  if (numPos > 8) numPos = 8;
  for (int pos = 0; pos < numPos; ++pos, posMask <<= 1) {
    if (lc[pos] >= 'a' && lc[pos] <= 'z') {
      charMask |= posMask;
      ++numChars;
    }
  }

  int numCombinations = 1 << numChars;
  for (int comb = 0; comb < numCombinations; ++comb) {
    posMask = 1;
    int combMask = 1;
    if (!result.isEmpty()) {
      result += ' ';
    }
    for (int pos = 0; pos < numPos; ++pos, posMask <<= 1) {
      if (charMask & posMask) {
        if (comb & combMask) {
          result += uc[pos];
        } else {
          result += lc[pos];
        }
        combMask <<= 1;
      } else {
        result += lc[pos];
      }
    }
  }

  return result;
}
#endif

/**
 * Create a filter string for the file dialog.
 * The filter string contains entries for all supported types.
 *
 * @param defaultNameFilter if not 0, return default name filter here
 *
 * @return filter string.
 */
QString Kid3Application::createFilterString(QString* defaultNameFilter) const
{
  QStringList extensions = TaggedFile::getSupportedFileExtensions();
  QString result, allCombinations;
  for (QStringList::const_iterator it = extensions.begin();
       it != extensions.end();
       ++it) {
    QString text = (*it).mid(1).toUpper();
    QString lowerExt = '*' + *it;
#ifdef WIN32
    const QString& combinations = lowerExt;
#else
    QString combinations = lowerUpperCaseCombinations(lowerExt);
#endif
    if (!allCombinations.isEmpty()) {
      allCombinations += ' ';
    }
    allCombinations += combinations;
#ifdef CONFIG_USE_KDE
    result += combinations;
    result += '|';
    result += text;
    result += " (";
    result += lowerExt;
    result += ")\n";
#else
    result += text;
    result += " (";
    result += combinations;
    result += ");;";
#endif
  }

#ifdef CONFIG_USE_KDE
  QString allExt = allCombinations;
  allExt += '|';
  allExt += i18n("All Supported Files");
  allExt += '\n';
  result = allExt + result + "*|" + i18n("All Files (*)");
#else
  QString allExt = i18n("All Supported Files");
  allExt += " (";
  allExt += allCombinations;
  allExt += ");;";
  result = allExt + result + i18n("All Files (*)");
#endif

  if (defaultNameFilter) {
    *defaultNameFilter = allCombinations;
  }

  return result;
}

/**
 * Set the ID3v1 and ID3v2 text encodings from the configuration.
 */
void Kid3Application::setTextEncodings()
{
#if defined HAVE_ID3LIB || defined HAVE_TAGLIB
  const QTextCodec* id3v1TextCodec =
    ConfigStore::s_miscCfg.m_textEncodingV1 != "ISO-8859-1" ?
    QTextCodec::codecForName(ConfigStore::s_miscCfg.m_textEncodingV1.toLatin1().data()) : 0;
#endif
#ifdef HAVE_ID3LIB
  Mp3File::setDefaultTextEncoding(
    static_cast<MiscConfig::TextEncoding>(ConfigStore::s_miscCfg.m_textEncoding));
  Mp3File::setTextCodecV1(id3v1TextCodec);
#endif
#ifdef HAVE_TAGLIB
  TagLibFile::setDefaultTextEncoding(
    static_cast<MiscConfig::TextEncoding>(ConfigStore::s_miscCfg.m_textEncoding));
  TagLibFile::setTextCodecV1(id3v1TextCodec);
#endif
}

/**
 * Convert ID3v2.3 to ID3v2.4 tags.
 */
void Kid3Application::convertToId3v24()
{
#ifdef HAVE_TAGLIB
  emit fileSelectionUpdateRequested();
  SelectedTaggedFileIterator it(getRootIndex(),
                                getFileSelectionModel(),
                                false);
  while (it.hasNext()) {
    TaggedFile* taggedFile = it.next();
    taggedFile->readTags(false);
    if (taggedFile->hasTagV2() && !taggedFile->isChanged()) {
      QString tagFmt = taggedFile->getTagFormatV2();
      if (tagFmt.length() >= 7 && tagFmt.startsWith("ID3v2.") && tagFmt[6] < '4') {
#ifdef HAVE_ID3LIB
        if (dynamic_cast<Mp3File*>(taggedFile) != 0) {
          FrameCollection frames;
          taggedFile->getAllFramesV2(frames);
          FrameFilter flt;
          flt.enableAll();
          taggedFile->deleteFramesV2(flt);

          // The file has to be read with TagLib to write ID3v2.4 tags
          taggedFile = FileProxyModel::readWithTagLib(taggedFile);

          // Restore the frames
          FrameFilter frameFlt;
          frameFlt.enableAll();
          taggedFile->setFramesV2(frames.copyEnabledFrames(frameFlt), false);
        }
#endif

        // Write the file with TagLib, it always writes ID3v2.4 tags
        bool renamed;
        taggedFile->writeTags(true, &renamed, ConfigStore::s_miscCfg.m_preserveTime);
        taggedFile->readTags(true);
      }
    }
  }
  emit selectedFilesUpdated();
#endif
}

/**
 * Convert ID3v2.4 to ID3v2.3 tags.
 */
void Kid3Application::convertToId3v23()
{
#if defined HAVE_TAGLIB && defined HAVE_ID3LIB
  emit fileSelectionUpdateRequested();
  SelectedTaggedFileIterator it(getRootIndex(),
                                getFileSelectionModel(),
                                false);
  while (it.hasNext()) {
    TaggedFile* taggedFile = it.next();
    taggedFile->readTags(false);
    if (taggedFile->hasTagV2() && !taggedFile->isChanged()) {
      QString tagFmt = taggedFile->getTagFormatV2();
      if (tagFmt.length() >= 7 && tagFmt.startsWith("ID3v2.") && tagFmt[6] > '3') {
        if (dynamic_cast<TagLibFile*>(taggedFile) != 0) {
          FrameCollection frames;
          taggedFile->getAllFramesV2(frames);
          FrameFilter flt;
          flt.enableAll();
          taggedFile->deleteFramesV2(flt);

          // The file has to be read with id3lib to write ID3v2.3 tags
          taggedFile = FileProxyModel::readWithId3Lib(taggedFile);

          // Restore the frames
          FrameFilter frameFlt;
          frameFlt.enableAll();
          taggedFile->setFramesV2(frames.copyEnabledFrames(frameFlt), false);
        }

        // Write the file with id3lib, it always writes ID3v2.3 tags
        bool renamed;
        taggedFile->writeTags(true, &renamed, ConfigStore::s_miscCfg.m_preserveTime);
        taggedFile->readTags(true);
      }
    }
  }
  emit selectedFilesUpdated();
#endif
}
