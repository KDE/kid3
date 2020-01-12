/**
 * \file kid3application.cpp
 * Kid3 application logic, independent of GUI.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 Jul 2011
 *
 * Copyright (C) 2011-2018  Urs Fleisch
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
#include <QItemSelectionModel>
#include <QTextCodec>
#include <QTextStream>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QCoreApplication>
#include <QPluginLoader>
#include <QElapsedTimer>
#include <QUrl>
#ifdef Q_OS_MAC
#include <CoreFoundation/CFURL.h>
#endif
#ifdef Q_OS_ANDROID
#include <QStandardPaths>
#endif
#if defined Q_OS_LINUX && !defined Q_OS_ANDROID
#include <malloc.h>
#endif
#ifdef HAVE_QTDBUS
#include <QDBusConnection>
#include <unistd.h>
#include "scriptinterface.h"
#endif
#include "filesystemmodel.h"
#include "icoreplatformtools.h"
#include "fileproxymodel.h"
#include "fileproxymodeliterator.h"
#include "filefilter.h"
#include "dirproxymodel.h"
#include "modeliterator.h"
#include "trackdatamodel.h"
#include "genremodel.h"
#include "frametablemodel.h"
#include "taggedfileselection.h"
#include "timeeventmodel.h"
#include "framelist.h"
#include "frameeditorobject.h"
#include "frameobjectmodel.h"
#include "playlistmodel.h"
#include "imagedataprovider.h"
#include "pictureframe.h"
#include "textimporter.h"
#include "importparser.h"
#include "textexporter.h"
#include "serverimporter.h"
#include "dirrenamer.h"
#include "saferename.h"
#include "configstore.h"
#include "formatconfig.h"
#include "tagconfig.h"
#include "fileconfig.h"
#include "importconfig.h"
#include "guiconfig.h"
#include "playlistconfig.h"
#include "isettings.h"
#include "playlistcreator.h"
#include "downloadclient.h"
#include "iframeeditor.h"
#include "batchimportprofile.h"
#include "batchimporter.h"
#include "batchimportconfig.h"
#include "iserverimporterfactory.h"
#include "iservertrackimporterfactory.h"
#include "itaggedfilefactory.h"
#include "iusercommandprocessor.h"
#ifdef Q_OS_ANDROID
#include "androidutils.h"
#endif
#include "importplugins.h"

namespace {

/**
 * Get the file name of the plugin from the plugin name.
 * @param pluginName name of the plugin
 * @return file name.
 */
QString pluginFileName(const QString& pluginName)
{
  QString fileName = pluginName.toLower();
#ifdef Q_OS_WIN32
#ifdef Q_CC_MSVC
  fileName += QLatin1String(".dll");
#else
  fileName = QLatin1String("lib") + fileName + QLatin1String(".dll");
#endif
#elif defined Q_OS_MAC
  fileName = QLatin1String("lib") + fileName + QLatin1String(".dylib");
#else
  fileName = QLatin1String("lib") + fileName + QLatin1String(".so");
#endif
  return fileName;
}

/**
 * Get text encoding from tag config as frame text encoding.
 * @return frame text encoding.
 */
Frame::TextEncoding frameTextEncodingFromConfig()
{
  Frame::TextEncoding encoding;
  switch (TagConfig::instance().textEncoding()) {
  case TagConfig::TE_UTF16:
    encoding = Frame::TE_UTF16;
    break;
  case TagConfig::TE_UTF8:
    encoding = Frame::TE_UTF8;
    break;
  case TagConfig::TE_ISO8859_1:
  default:
    encoding = Frame::TE_ISO8859_1;
  }
  return encoding;
}

/**
 * Extract file path, field name and index from frame name.
 *
 * @param frameName frame name with additional information for file, field and
 * index
 * @param dataFileName the path to a data file is returned here if available,
 * else null
 * @param fieldName the field name is returned here if available, else null
 * @param index the index is returned here if available, else 0
 */
void extractFileFieldIndex(
    QString& frameName, QString& dataFileName, QString& fieldName, int& index)
{
  dataFileName.clear();
  fieldName.clear();
  index = 0;
  int colonIndex = frameName.indexOf(QLatin1Char(':'));
  if (colonIndex != -1) {
    dataFileName = frameName.mid(colonIndex + 1);
    frameName.truncate(colonIndex);
  }
  int dotIndex = frameName.indexOf(QLatin1Char('.'));
  if (dotIndex != -1) {
    fieldName = frameName.mid(dotIndex + 1);
    frameName.truncate(dotIndex);
  }
  int bracketIndex = frameName.indexOf(QLatin1Char('['));
  if (bracketIndex != -1) {
    const int closingBracketIndex =
        frameName.indexOf(QLatin1Char(']'), bracketIndex + 1);
    if (closingBracketIndex > bracketIndex) {
      bool ok;
      index = frameName.midRef(
          bracketIndex + 1, closingBracketIndex - bracketIndex - 1).toInt(&ok);
      if (ok) {
        frameName.remove(bracketIndex, closingBracketIndex - bracketIndex + 1);
      }
    }
  }
}

}

/** Fallback for path to search for plugins */
QString Kid3Application::s_pluginsPathFallback;

/**
 * Constructor.
 * @param platformTools platform tools
 * @param parent parent object
 */
Kid3Application::Kid3Application(ICorePlatformTools* platformTools,
                                 QObject* parent) : QObject(parent),
  m_platformTools(platformTools),
  m_configStore(new ConfigStore(m_platformTools->applicationSettings())),
  m_fileSystemModel(new FileSystemModel(this)),
  m_fileProxyModel(new FileProxyModel(m_platformTools->iconProvider(), this)),
  m_fileProxyModelIterator(new FileProxyModelIterator(m_fileProxyModel)),
  m_dirProxyModel(new DirProxyModel(this)),
  m_fileSelectionModel(new QItemSelectionModel(m_fileProxyModel, this)),
  m_dirSelectionModel(new QItemSelectionModel(m_dirProxyModel, this)),
  m_trackDataModel(new TrackDataModel(m_platformTools->iconProvider(), this)),
  m_netMgr(new QNetworkAccessManager(this)),
  m_downloadClient(new DownloadClient(m_netMgr)),
  m_textExporter(new TextExporter(this)),
  m_tagSearcher(new TagSearcher(this)),
  m_dirRenamer(new DirRenamer(this)),
  m_batchImporter(new BatchImporter(m_netMgr)),
  m_player(nullptr),
  m_expressionFileFilter(nullptr),
  m_downloadImageDest(ImageForSelectedFiles),
  m_fileFilter(nullptr), m_filterPassed(0), m_filterTotal(0),
  m_batchImportProfile(nullptr), m_batchImportTagVersion(Frame::TagNone),
  m_editFrameTaggedFile(nullptr), m_addFrameTaggedFile(nullptr),
  m_frameEditor(nullptr), m_storedFrameEditor(nullptr),
  m_imageProvider(nullptr),
#ifdef Q_OS_ANDROID
  m_pendingIntentsChecked(false),
#endif
#ifdef HAVE_QTDBUS
  m_dbusEnabled(false),
#endif
  m_filtered(false), m_selectionOperationRunning(false)
{
  const TagConfig& tagCfg = TagConfig::instance();
  FOR_ALL_TAGS(tagNr) {
    bool id3v1 = tagNr == Frame::Tag_Id3v1;
    m_genreModel[tagNr] = new GenreModel(id3v1, this);
    m_framesModel[tagNr] = new FrameTableModel(
          id3v1, platformTools->iconProvider(), this);
    if (!id3v1) {
      m_framesModel[tagNr]->setFrameOrder(tagCfg.quickAccessFrameOrder());
      connect(&tagCfg, &TagConfig::quickAccessFrameOrderChanged,
              m_framesModel[tagNr], &FrameTableModel::setFrameOrder);
    }
    m_framesSelectionModel[tagNr] = new QItemSelectionModel(m_framesModel[tagNr], this);
    m_framelist[tagNr] = new FrameList(tagNr, m_framesModel[tagNr],
        m_framesSelectionModel[tagNr]);
    connect(m_framelist[tagNr], &FrameList::frameEdited,
            this, &Kid3Application::onFrameEdited);
    connect(m_framelist[tagNr], &FrameList::frameAdded,
            this, &Kid3Application::onTag2FrameAdded);
    m_tagContext[tagNr] = new Kid3ApplicationTagContext(this, tagNr);
  }
  m_selection = new TaggedFileSelection(m_framesModel, this);
  setObjectName(QLatin1String("Kid3Application"));
  m_fileSystemModel->setReadOnly(false);
  const FileConfig& fileCfg = FileConfig::instance();
  m_fileSystemModel->setSortIgnoringPunctuation(fileCfg.sortIgnoringPunctuation());
  m_fileProxyModel->setSourceModel(m_fileSystemModel);
  m_dirProxyModel->setSourceModel(m_fileSystemModel);
  connect(m_fileSelectionModel,
          &QItemSelectionModel::selectionChanged,
          this, &Kid3Application::fileSelected);
  connect(m_fileSelectionModel,
          &QItemSelectionModel::selectionChanged,
          this, &Kid3Application::fileSelectionChanged);
  connect(m_fileProxyModel, &FileProxyModel::modifiedChanged,
          this, &Kid3Application::modifiedChanged);

  connect(m_selection, &TaggedFileSelection::singleFileChanged,
          this, &Kid3Application::updateCoverArtImageId);
  connect(m_selection, &TaggedFileSelection::fileNameModified,
          this, &Kid3Application::selectedFilesUpdated);

  initPlugins();
  m_batchImporter->setImporters(m_importers, m_trackDataModel);

#ifdef Q_OS_ANDROID
  new AndroidUtils(this);
  // Make sure that configuration changes are saved for the Android app.
  // Old style connect syntax is used to avoid a dependency to QGuiApplication.
  connect(qApp, SIGNAL(applicationStateChanged(Qt::ApplicationState)),
          this, SLOT(onApplicationStateChanged(Qt::ApplicationState)));
  QObject::connect(AndroidUtils::instance(), &AndroidUtils::filePathReceived,
                   this, [this](const QString& path) {
    dropLocalFiles({path}, false);
  });
#endif
}

/**
 * Destructor.
 */
Kid3Application::~Kid3Application()
{
#ifdef Q_OS_MAC
  // If a song is played, then stopped and Kid3 is terminated, it will crash in
  // the QMediaPlayer destructor (Dispatch queue: com.apple.main-thread,
  // objc_msgSend() selector name: setRate). Avoid calling the destructor by
  // setting the QMediaPlayer's parent to null. See also:
  // https://qt-project.org/forums/viewthread/29651
  if (m_player) {
    m_player->setParent(0);
  }
#endif
}

/**
 * Save config when suspended, check intents when activated.
 * @param state application state
 */
void Kid3Application::onApplicationStateChanged(Qt::ApplicationState state)
{
#ifdef Q_OS_ANDROID
  if (state == Qt::ApplicationSuspended) {
    saveConfig();
  } else if (state == Qt::ApplicationActive) {
    // When the app becomes active for the first time,
    // check if it was launched with an intent.
    if (!m_pendingIntentsChecked) {
      m_pendingIntentsChecked = true;
      AndroidUtils::instance()->checkPendingIntents();
    }
  }
#else
  Q_UNUSED(state)
#endif
}

#ifdef HAVE_QTDBUS
/**
 * Activate the D-Bus interface.
 * This method shall be called only once at initialization.
 */
void Kid3Application::activateDbusInterface()
{
  if (QDBusConnection::sessionBus().isConnected()) {
    QString serviceName(QLatin1String("net.sourceforge.kid3"));
    QDBusConnection::sessionBus().registerService(serviceName);
    // For the case of multiple Kid3 instances running, register also a service
    // with the PID appended. On KDE such a service is already registered but
    // the call to registerService() seems to succeed nevertheless.
    serviceName += QLatin1Char('-');
    serviceName += QString::number(::getpid());
    QDBusConnection::sessionBus().registerService(serviceName);
    new ScriptInterface(this);
    if (QDBusConnection::sessionBus().registerObject(QLatin1String("/Kid3"), this)) {
      m_dbusEnabled = true;
    } else {
      qWarning("Registering D-Bus object failed");
    }
  } else {
    qWarning("Cannot connect to the D-BUS session bus.");
  }
}
#endif

/**
 * Load and initialize plugins depending on configuration.
 */
void Kid3Application::initPlugins()
{
  // Load plugins, set information about plugins in configuration.
  ImportConfig& importCfg = ImportConfig::instance();
  TagConfig& tagCfg = TagConfig::instance();
  importCfg.clearAvailablePlugins();
  tagCfg.clearAvailablePlugins();
  const auto plugins = loadPlugins();
  for (QObject* plugin : plugins) {
    checkPlugin(plugin);
  }
  // Order the meta data plugins as configured.
  QStringList pluginOrder = tagCfg.pluginOrder();
  if (!pluginOrder.isEmpty()) {
    QList<ITaggedFileFactory*> orderedFactories;
    for (int i = 0; i < pluginOrder.size(); ++i) {
      orderedFactories.append(nullptr); // clazy:exclude=reserve-candidates
    }
    const auto factories = FileProxyModel::taggedFileFactories();
    for (ITaggedFileFactory* factory : factories) {
      int idx = pluginOrder.indexOf(factory->name());
      if (idx >= 0) {
        orderedFactories[idx] = factory;
      } else {
        orderedFactories.append(factory); // clazy:exclude=reserve-candidates
      }
    }
    orderedFactories.removeAll(nullptr);
    FileProxyModel::taggedFileFactories().swap(orderedFactories);
  }
}

/**
 * Find directory containing plugins.
 * @param pluginsDir the plugin directory is returned here
 * @return true if found.
 */
bool Kid3Application::findPluginsDirectory(QDir& pluginsDir) {
  // First check if we are running from the build directory to load the
  // plugins from there.
  pluginsDir.setPath(QCoreApplication::applicationDirPath());
  QString dirName = pluginsDir.dirName();
#ifdef Q_OS_WIN
  QString buildType;
  if (dirName.compare(QLatin1String("debug"), Qt::CaseInsensitive) == 0 ||
      dirName.compare(QLatin1String("release"), Qt::CaseInsensitive) == 0) {
    buildType = dirName;
    pluginsDir.cdUp();
    dirName = pluginsDir.dirName();
  }
#endif
  bool pluginsDirFound = pluginsDir.cd(QLatin1String(
      (dirName == QLatin1String("qt") || dirName == QLatin1String("kde") ||
       dirName == QLatin1String("cli") || dirName == QLatin1String("qml"))
      ? "../../plugins"
      : dirName == QLatin1String("test")
        ? "../plugins"
        : CFG_PLUGINSDIR));
#ifdef Q_OS_MAC
  if (!pluginsDirFound) {
    pluginsDirFound = pluginsDir.cd(QLatin1String("../../../../../plugins"));
  }
#endif
#ifdef Q_OS_WIN
  if (pluginsDirFound && !buildType.isEmpty()) {
    pluginsDir.cd(buildType);
  }
#endif
  return pluginsDirFound;
}

/**
 * Set fallback path for directory containing plugins.
 * @param path path to be searched for plugins if they are not found at the
 * standard location relative to the application directory
 */
void Kid3Application::setPluginsPathFallback(const QString& path)
{
  s_pluginsPathFallback = path;
}

/**
 * Load plugins.
 * @return list of plugin instances.
 */
QObjectList Kid3Application::loadPlugins()
{
  QObjectList plugins = QPluginLoader::staticInstances();

  QDir pluginsDir;
  bool pluginsDirFound = findPluginsDirectory(pluginsDir);
  if (!pluginsDirFound && !s_pluginsPathFallback.isEmpty()) {
    pluginsDir.setPath(s_pluginsPathFallback);
    pluginsDirFound = true;
  }
  if (pluginsDirFound) {
    ImportConfig& importCfg = ImportConfig::instance();
    TagConfig& tagCfg = TagConfig::instance();

    // Construct a set of disabled plugin file names
    QMap<QString, QString> disabledImportPluginFileNames;
    const QStringList disabledPlugins = importCfg.disabledPlugins();
    for (const QString& pluginName : disabledPlugins) {
      disabledImportPluginFileNames.insert(pluginFileName(pluginName),
                                           pluginName);
    }
    QMap<QString, QString> disabledTagPluginFileNames;
    const QStringList disabledTagPlugins = tagCfg.disabledPlugins();
    for (const QString& pluginName : disabledTagPlugins) {
      disabledTagPluginFileNames.insert(pluginFileName(pluginName),
                                        pluginName);
    }

    QStringList availablePlugins = importCfg.availablePlugins();
    QStringList availableTagPlugins = tagCfg.availablePlugins();
    const auto fileNames = pluginsDir.entryList(QDir::Files);
    for (const QString& fileName : fileNames) {
      if (disabledImportPluginFileNames.contains(fileName)) {
        availablePlugins.append(
              disabledImportPluginFileNames.value(fileName));
        continue;
      }
      if (disabledTagPluginFileNames.contains(fileName)) {
        availableTagPlugins.append(
              disabledTagPluginFileNames.value(fileName));
        continue;
      }
      QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
      QObject* plugin = loader.instance();
      if (plugin) {
        QString name(plugin->objectName());
        if (disabledPlugins.contains(name)) {
          availablePlugins.append(name);
          loader.unload();
        } else if (disabledTagPlugins.contains(name)) {
          availableTagPlugins.append(name);
          loader.unload();
        } else {
          plugins.append(plugin);
        }
      }
    }
    importCfg.setAvailablePlugins(availablePlugins);
    tagCfg.setAvailablePlugins(availableTagPlugins);
  }
  return plugins;
}

/**
 * Check type of a loaded plugin and register it.
 * @param plugin instance returned by plugin loader
 */
void Kid3Application::checkPlugin(QObject* plugin)
{
  if (IServerImporterFactory* importerFactory =
      qobject_cast<IServerImporterFactory*>(plugin)) {
    ImportConfig& importCfg = ImportConfig::instance();
    QStringList availablePlugins = importCfg.availablePlugins();
    availablePlugins.append(plugin->objectName());
    importCfg.setAvailablePlugins(availablePlugins);
    if (!importCfg.disabledPlugins().contains(plugin->objectName())) {
      const auto keys = importerFactory->serverImporterKeys();
      for (const QString& key : keys) {
        m_importers.append(importerFactory->createServerImporter(
                             key, m_netMgr, m_trackDataModel));
      }
    }
  }
  if (IServerTrackImporterFactory* importerFactory =
      qobject_cast<IServerTrackImporterFactory*>(plugin)) {
    ImportConfig& importCfg = ImportConfig::instance();
    QStringList availablePlugins = importCfg.availablePlugins();
    availablePlugins.append(plugin->objectName());
    importCfg.setAvailablePlugins(availablePlugins);
    if (!importCfg.disabledPlugins().contains(plugin->objectName())) {
      const auto keys = importerFactory->serverTrackImporterKeys();
      for (const QString& key : keys) {
        m_trackImporters.append(importerFactory->createServerTrackImporter(
                             key, m_netMgr, m_trackDataModel));
      }
    }
  }
  if (ITaggedFileFactory* taggedFileFactory =
      qobject_cast<ITaggedFileFactory*>(plugin)) {
    TagConfig& tagCfg = TagConfig::instance();
    QStringList availablePlugins = tagCfg.availablePlugins();
    availablePlugins.append(plugin->objectName());
    tagCfg.setAvailablePlugins(availablePlugins);
    if (!tagCfg.disabledPlugins().contains(plugin->objectName())) {
      int features = tagCfg.taggedFileFeatures();
      const auto keys = taggedFileFactory->taggedFileKeys();
      for (const QString& key : keys) {
        taggedFileFactory->initialize(key);
        features |= taggedFileFactory->taggedFileFeatures(key);
      }
      tagCfg.setTaggedFileFeatures(features);
      FileProxyModel::taggedFileFactories().append(taggedFileFactory);
    }
  }
  if (IUserCommandProcessor* userCommandProcessor =
      qobject_cast<IUserCommandProcessor*>(plugin)) {
    ImportConfig& importCfg = ImportConfig::instance();
    QStringList availablePlugins = importCfg.availablePlugins();
    availablePlugins.append(plugin->objectName());
    importCfg.setAvailablePlugins(availablePlugins);
    if (!importCfg.disabledPlugins().contains(plugin->objectName())) {
      m_userCommandProcessors.append(userCommandProcessor);
    }
  }
}

/**
 * Get names of available server track importers.
 * @return list of server track importer names.
 */
QStringList Kid3Application::getServerImporterNames() const
{
  QStringList names;
  const auto importers = m_importers;
  for (const ServerImporter* importer : importers) {
    names.append(QString::fromLatin1(importer->name()));
  }
  return names;
}

/**
 * Get audio player.
 * @return audio player.
 */
QObject* Kid3Application::getAudioPlayer()
{
  if (!m_player) {
#ifdef HAVE_QTDBUS
    m_player = m_platformTools->createAudioPlayer(this, m_dbusEnabled);
#else
    m_player = m_platformTools->createAudioPlayer(this, false);
#endif
  }
#ifdef HAVE_QTDBUS
  if (m_dbusEnabled) {
    activateMprisInterface();
  }
#endif
  return m_player;
}

/**
 * Delete audio player.
 */
void Kid3Application::deleteAudioPlayer() {
  if (m_player) {
    QMetaObject::invokeMethod(m_player, "stop");
#ifdef HAVE_QTDBUS
    if (m_dbusEnabled) {
      deactivateMprisInterface();
    }
#endif
    delete m_player;
    m_player = nullptr;
  }
}

#ifdef HAVE_QTDBUS
/**
 * Activate the MPRIS D-Bus Interface if not already active.
 */
void Kid3Application::activateMprisInterface()
{
  if (!m_mprisServiceName.isEmpty() || !m_player)
    return;

  if (QDBusConnection::sessionBus().isConnected()) {
    m_mprisServiceName = QLatin1String("org.mpris.MediaPlayer2.kid3");
    bool ok = QDBusConnection::sessionBus().registerService(m_mprisServiceName);
    if (!ok) {
      // If another instance of Kid3 is already running register a service
      // with ".instancePID" appended, see
      // https://specifications.freedesktop.org/mpris-spec/latest/
      m_mprisServiceName += QLatin1String(".instance");
      m_mprisServiceName += QString::number(::getpid());
      ok = QDBusConnection::sessionBus().registerService(m_mprisServiceName);
    }
    if (ok) {
      if (!QDBusConnection::sessionBus().registerObject(
            QLatin1String("/org/mpris/MediaPlayer2"), m_player)) {
        qWarning("Registering D-Bus MPRIS object failed");
      }
    } else {
      m_mprisServiceName.clear();
      qWarning("Registering D-Bus MPRIS service failed");
    }
  } else {
    qWarning("Cannot connect to the D-BUS session bus.");
  }
}

/**
 * Deactivate the MPRIS D-Bus Interface if it is active.
 */
void Kid3Application::deactivateMprisInterface()
{
  if (m_mprisServiceName.isEmpty())
    return;

  if (QDBusConnection::sessionBus().isConnected()) {
    QDBusConnection::sessionBus().unregisterObject(
          QLatin1String("/org/mpris/MediaPlayer2"));
    if (QDBusConnection::sessionBus().unregisterService(m_mprisServiceName)) {
      m_mprisServiceName.clear();
    } else {
      qWarning("Unregistering D-Bus MPRIS service failed");
    }
  } else {
    qWarning("Cannot connect to the D-BUS session bus.");
  }
}
#endif

/**
 * Get settings.
 * @return settings.
 */
ISettings* Kid3Application::getSettings() const
{
  return m_platformTools->applicationSettings();
}

/**
 * Apply configuration changes.
 */
void Kid3Application::applyChangedConfiguration()
{
  saveConfig();
  const FileConfig& fileCfg = FileConfig::instance();
  FOR_ALL_TAGS(tagNr) {
    if (!TagConfig::instance().markTruncations()) {
      m_framesModel[tagNr]->markRows(0);
    }
    if (!fileCfg.markChanges()) {
      m_framesModel[tagNr]->markChangedFrames(0);
    }
    m_genreModel[tagNr]->init();
  }
  notifyConfigurationChange();
  quint64 oldQuickAccessFrames = FrameCollection::getQuickAccessFrames();
  if (TagConfig::instance().quickAccessFrames() != oldQuickAccessFrames) {
    FrameCollection::setQuickAccessFrames(
          TagConfig::instance().quickAccessFrames());
    emit selectedFilesUpdated();
  }

  QStringList nameFilters(m_platformTools->getNameFilterPatterns(
                            fileCfg.nameFilter()).split(QLatin1Char(' ')));
  m_fileProxyModel->setNameFilters(nameFilters);
  m_fileProxyModel->setFolderFilters(fileCfg.includeFolders(),
                                     fileCfg.excludeFolders());

  QDir::Filters oldFilter = m_fileSystemModel->filter();
  QDir::Filters filter = oldFilter;
  if (fileCfg.showHiddenFiles()) {
    filter |= QDir::Hidden;
  } else {
    filter &= ~QDir::Hidden;
  }
  if (filter != oldFilter) {
    m_fileSystemModel->setFilter(filter);
  }
}

/**
 * Save settings to the configuration.
 */
void Kid3Application::saveConfig()
{
  if (FileConfig::instance().loadLastOpenedFile()) {
    FileConfig::instance().setLastOpenedFile(
        m_fileProxyModel->filePath(currentOrRootIndex()));
  }
  m_configStore->writeToConfig();
  getSettings()->sync();
}

/**
 * Read settings from the configuration.
 */
void Kid3Application::readConfig()
{
  if (FileConfig::instance().nameFilter().isEmpty()) {
    setAllFilesFileFilter();
  }
  notifyConfigurationChange();
  FrameCollection::setQuickAccessFrames(
        TagConfig::instance().quickAccessFrames());
}

/**
 * Open directory.
 * When finished directoryOpened() is emitted, also if false is returned.
 *
 * @param paths file or directory paths, if multiple paths are given, the
 * common directory is opened and the files are selected
 * @param fileCheck if true, only open directory if paths exist
 *
 * @return true if ok.
 */
bool Kid3Application::openDirectory(const QStringList& paths, bool fileCheck)
{
#ifdef Q_OS_ANDROID
  const QStringList musicLocations =
      QStandardPaths::standardLocations(QStandardPaths::MusicLocation).mid(0, 1);
  const QStringList pathList = paths.isEmpty() ? musicLocations : paths;
#else
  const QStringList pathList(paths);
#endif
  bool ok = true;
  QStringList filePaths;
  QStringList dirComponents;
  for (const QString& path : pathList) {
    if (!path.isEmpty()) {
      QFileInfo fileInfo(path);
      if (fileCheck && !fileInfo.exists()) {
        ok = false;
        break;
      }
      QString dirPath;
      if (!fileInfo.isDir()) {
        dirPath = fileInfo.absolutePath();
        if (fileInfo.isFile()) {
          filePaths.append(fileInfo.absoluteFilePath());
        }
      } else {
        dirPath = QDir(path).absolutePath();
      }
      QStringList dirPathComponents = dirPath.split(QDir::separator());
      if (dirComponents.isEmpty()) {
        dirComponents = dirPathComponents;
      } else {
        // Reduce dirPath to common prefix.
        auto dirIt = dirComponents.begin();
        auto dirPathIt = dirPathComponents.constBegin();
        while (dirIt != dirComponents.end() &&
               dirPathIt != dirPathComponents.constEnd() &&
               *dirIt == *dirPathIt) {
          ++dirIt;
          ++dirPathIt;
        }
        dirComponents.erase(dirIt, dirComponents.end());
      }
    }
  }
  QString dir;
  if (ok) {
    dir = dirComponents.join(QDir::separator());
    if (dir.isEmpty() && !filePaths.isEmpty()) {
      dir = QDir::rootPath();
    }
    ok = !dir.isEmpty();
  }
  QModelIndex rootIndex;
  QModelIndexList fileIndexes;
  if (ok) {
    const FileConfig& fileCfg = FileConfig::instance();
    QStringList nameFilters(m_platformTools->getNameFilterPatterns(
                              fileCfg.nameFilter()).split(QLatin1Char(' ')));
    m_fileProxyModel->setNameFilters(nameFilters);
    m_fileProxyModel->setFolderFilters(fileCfg.includeFolders(),
                                       fileCfg.excludeFolders());
    QDir::Filters filter = QDir::AllEntries | QDir::AllDirs;
    if (fileCfg.showHiddenFiles()) {
      filter |= QDir::Hidden;
    }
    m_fileSystemModel->setFilter(filter);
    rootIndex = m_fileSystemModel->setRootPath(dir);
    fileIndexes.reserve(filePaths.size());
    const auto constFilePaths = filePaths;
    for (const QString& filePath : constFilePaths) {
      fileIndexes.append(m_fileSystemModel->index(filePath));
    }
    ok = rootIndex.isValid();
  }
  if (ok) {
    setFiltered(false);
    m_dirName = dir;
    emit dirNameChanged(m_dirName);
    QModelIndex oldRootIndex = m_fileProxyModelRootIndex;
    m_fileProxyModelRootIndex = m_fileProxyModel->mapFromSource(rootIndex);
    m_fileProxyModelFileIndexes.clear();
    const auto constFileIndexes = fileIndexes;
    for (const QModelIndex& fileIndex : constFileIndexes) {
      m_fileProxyModelFileIndexes.append(
            m_fileProxyModel->mapFromSource(fileIndex));
    }
    if (m_fileProxyModelRootIndex != oldRootIndex) {
      connect(m_fileProxyModel, &FileProxyModel::sortingFinished,
              this, &Kid3Application::onDirectoryLoaded);
    } else {
      QTimer::singleShot(0, this, &Kid3Application::onDirectoryOpened);
    }
  }
  if (!ok) {
    QTimer::singleShot(0, this, &Kid3Application::onDirectoryOpened);
  }
  return ok;
}

/**
 * Update selection and emit signals when directory is opened.
 */
void Kid3Application::onDirectoryOpened()
{
  QModelIndex fsRoot = m_fileProxyModel->mapToSource(m_fileProxyModelRootIndex);
  m_dirProxyModelRootIndex = m_dirProxyModel->mapFromSource(fsRoot);

  emit fileRootIndexChanged(m_fileProxyModelRootIndex);
  emit dirRootIndexChanged(m_dirProxyModelRootIndex);

  if (m_fileProxyModelRootIndex.isValid()) {
    m_fileSelectionModel->clearSelection();
    if (!m_fileProxyModelFileIndexes.isEmpty()) {
      const auto fileIndexes = m_fileProxyModelFileIndexes;
      for (const QPersistentModelIndex& fileIndex : fileIndexes) {
        m_fileSelectionModel->select(fileIndex,
            QItemSelectionModel::Select | QItemSelectionModel::Rows);
      }
#if QT_VERSION >= 0x050600
      m_fileSelectionModel->setCurrentIndex(m_fileProxyModelFileIndexes.constFirst(),
                                            QItemSelectionModel::NoUpdate);
#else
      m_fileSelectionModel->setCurrentIndex(m_fileProxyModelFileIndexes.first(),
                                            QItemSelectionModel::NoUpdate);
#endif
    } else {
      m_fileSelectionModel->setCurrentIndex(m_fileProxyModelRootIndex,
          QItemSelectionModel::Clear | QItemSelectionModel::Current |
          QItemSelectionModel::Rows);
    }
  }

  emit directoryOpened();

  if (m_dirUpIndex.isValid()) {
    m_dirSelectionModel->setCurrentIndex(m_dirUpIndex,
        QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
    m_dirUpIndex = QPersistentModelIndex();
  }
}

/**
 * Called when the gatherer thread has finished to load the directory.
 */
void Kid3Application::onDirectoryLoaded()
{
  disconnect(m_fileProxyModel, &FileProxyModel::sortingFinished,
             this, &Kid3Application::onDirectoryLoaded);
  onDirectoryOpened();
}

/**
 * Unload all tags.
 * The tags of all files which are not modified or selected are freed to
 * reclaim their memory.
 */
void Kid3Application::unloadAllTags()
{
  TaggedFileIterator it(m_fileProxyModelRootIndex);
  while (it.hasNext()) {
    TaggedFile* taggedFile = it.next();
    if (taggedFile->isTagInformationRead() && !taggedFile->isChanged() &&
        !m_fileSelectionModel->isSelected(taggedFile->getIndex())) {
      taggedFile->clearTags(false);
      taggedFile->closeFileHandle();
    }
  }
#if defined Q_OS_LINUX && !defined Q_OS_ANDROID
  if (::malloc_trim(0)) {
    qDebug("Memory released by malloc_trim()");
  }
#endif
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
 * longRunningOperationProgress() is emitted while saving files.
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
  QString operationName = tr("Saving folder...");
  bool aborted = false;
  emit longRunningOperationProgress(operationName, -1, totalFiles, &aborted);

  TaggedFileIterator it(m_fileProxyModelRootIndex);
  while (it.hasNext()) {
    TaggedFile* taggedFile = it.next();
    QString fileName = taggedFile->getFilename();
    if (taggedFile->isFilenameChanged() &&
        Utils::replaceIllegalFileNameCharacters(fileName)) {
      taggedFile->setFilename(fileName);
    }
    bool renamed = false;
    if (taggedFile->isChanged() &&
        !taggedFile->writeTags(false, &renamed,
                               FileConfig::instance().preserveTime())) {
      QDir dir(taggedFile->getDirname());
      if (dir.exists(fileName) && taggedFile->isFilenameChanged()) {
        // File is renamed to a file name which already exists.
        // Try another file name ending with a number.
        QString baseName = fileName;
        QString ext;
        int dotPos = baseName.lastIndexOf(QLatin1Char('.'));
        if (dotPos != -1) {
          ext = baseName.mid(dotPos);
          baseName.truncate(dotPos);
        }
        baseName.append('(');
        ext.prepend(')');
        bool ok = false;
        for (int nr = 1; nr < 100; ++nr) {
          QString newName = baseName + QString::number(nr) + ext;
          if (!dir.exists(newName)) {
            taggedFile->setFilename(newName);
            ok = taggedFile->writeTags(false, &renamed,
                                       FileConfig::instance().preserveTime());
            break;
          }
        }
        if (ok) {
          continue;
        } else {
          taggedFile->setFilename(fileName);
        }
      }
      QString errorMsg = taggedFile->getAbsFilename();
      errorFiles.push_back(errorMsg);
    }
    ++numFiles;
    emit longRunningOperationProgress(operationName, numFiles, totalFiles,
                                      &aborted);
    if (aborted) {
      break;
    }
  }
  if (totalFiles == 0) {
    // To signal that operation is finished.
    ++totalFiles;
  }
  emit longRunningOperationProgress(operationName, totalFiles, totalFiles,
                                    &aborted);

  return errorFiles;
}

/**
 * Update tags of selected files to contain contents of frame models.
 */
void Kid3Application::frameModelsToTags()
{
  if (!m_currentSelection.isEmpty()) {
    FOR_ALL_TAGS(tagNr) {
      FrameCollection frames(m_framesModel[tagNr]->getEnabledFrames());
      for (auto it = m_currentSelection.constBegin();
           it != m_currentSelection.constEnd();
           ++it) {
        if (TaggedFile* taggedFile = FileProxyModel::getTaggedFileOfIndex(*it)) {
          taggedFile->setFrames(tagNr, frames);
        }
      }
    }
  }
}

/**
 * Update frame models to contain contents of selected files.
 * The properties starting with "selection" will be set by this method.
 */
void Kid3Application::tagsToFrameModels()
{
  QList<QPersistentModelIndex> indexes;
  const auto selectedIndexes = m_fileSelectionModel->selectedRows();
  indexes.reserve(selectedIndexes.size());
  for (const QModelIndex& index : selectedIndexes) {
    indexes.append(QPersistentModelIndex(index));
  }

  if (addTaggedFilesToSelection(indexes, true)) {
    m_currentSelection.swap(indexes);
  }
}

/**
 * Update frame models to contain contents of item selection.
 * The properties starting with "selection" will be set by this method.
 * @param selected item selection
 */
void Kid3Application::selectedTagsToFrameModels(const QItemSelection& selected)
{
  QList<QPersistentModelIndex> indexes;
  const auto selectedIndexes = selected.indexes();
  for (const QModelIndex& index : selectedIndexes) {
    if (index.column() == 0) {
      indexes.append(QPersistentModelIndex(index));
    }
  }

  if (addTaggedFilesToSelection(indexes, m_currentSelection.isEmpty())) {
    m_currentSelection.append(indexes);
  }
}

/**
 * Update frame models to contain contents of selected files.
 * @param indexes tagged file indexes
 * @param startSelection true if a new selection is started, false to add to
 * the existing selection
 * @return true if ok, false if selection operation is already running.
 */
bool Kid3Application::addTaggedFilesToSelection(
    const QList<QPersistentModelIndex>& indexes, bool startSelection)
{
  // It would crash if this is called while a long running selection operation
  // is in progress.
  if (m_selectionOperationRunning)
    return false;

  m_selectionOperationRunning = true;

  if (startSelection) {
    m_selection->beginAddTaggedFiles();
  }

  QElapsedTimer timer;
  timer.start();
  QString operationName = tr("Selection");
  int longRunningTotal = 0;
  int done = 0;
  bool aborted = false;
  for (auto it = indexes.constBegin(); it != indexes.constEnd(); ++it, ++done) {
    if (TaggedFile* taggedFile = FileProxyModel::getTaggedFileOfIndex(*it)) {
      m_selection->addTaggedFile(taggedFile);
      if (!longRunningTotal) {
        if (timer.elapsed() >= 3000) {
          longRunningTotal = indexes.size();
          emit longRunningOperationProgress(operationName, -1, longRunningTotal,
                                            &aborted);
        }
      } else {
        emit longRunningOperationProgress(operationName, done, longRunningTotal,
                                          &aborted);
        if (aborted) {
          break;
        }
      }
    }
  }
  if (longRunningTotal) {
    emit longRunningOperationProgress(operationName, longRunningTotal,
                                      longRunningTotal, &aborted);
  }

  m_selection->endAddTaggedFiles();

  if (TaggedFile* taggedFile = m_selection->singleFile()) {
    FOR_ALL_TAGS(tagNr) {
      m_framelist[tagNr]->setTaggedFile(taggedFile);
    }
  }
  m_selection->clearUnusedFrames();
  m_selectionOperationRunning = false;
  return true;
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
  }
  if (!it.hasNoSelection()) {
    emit selectedFilesUpdated();
  }
}

/**
 * Set filter state.
 *
 * @param val true if list is filtered
 */
void Kid3Application::setFiltered(bool val)
{
  if (m_filtered != val) {
    m_filtered = val;
    emit filteredChanged(m_filtered);
  }
}

/**
 * Import.
 *
 * @param tagMask tag mask
 * @param path    path of file, "clipboard" for import from clipboard
 * @param fmtIdx  index of format
 *
 * @return true if ok.
 */
bool Kid3Application::importTags(Frame::TagVersion tagMask,
                                 const QString& path, int fmtIdx)
{
  const ImportConfig& importCfg = ImportConfig::instance();
  filesToTrackDataModel(importCfg.importDest());
  QString text;
  if (path == QLatin1String("clipboard")) {
    text = m_platformTools->readFromClipboard();
  } else {
    QFile file(path);
    if (file.open(QIODevice::ReadOnly)) {
      text = QTextStream(&file).readAll();
      file.close();
    }
  }
  if (!text.isNull() &&
      fmtIdx < importCfg.importFormatHeaders().size()) {
    TextImporter(getTrackDataModel()).updateTrackData(
      text,
      importCfg.importFormatHeaders().at(fmtIdx),
      importCfg.importFormatTracks().at(fmtIdx));
    trackDataModelToFiles(tagMask);
    return true;
  }
  return false;
}

/**
 * Import from tags.
 *
 * @param tagMask tag mask
 * @param source format to get source text from tags
 * @param extraction regular expression with frame names and captures to
 * extract from source text
 */
void Kid3Application::importFromTags(Frame::TagVersion tagMask,
                                     const QString& source,
                                     const QString& extraction)
{
  ImportTrackDataVector trackDataVector;
  filesToTrackData(tagMask, trackDataVector);
  TextImporter::importFromTags(source, extraction, trackDataVector);
  getTrackDataModel()->setTrackData(trackDataVector);
  trackDataModelToFiles(tagMask);
}

/**
 * Import from tags on selected files.
 *
 * @param tagMask tag mask
 * @param source format to get source text from tags
 * @param extraction regular expression with frame names and captures to
 * extract from source text
 *
 * @return extracted values for "%{__return}(.+)", empty if not used.
 */
QStringList Kid3Application::importFromTagsToSelection(Frame::TagVersion tagMask,
                                                       const QString& source,
                                                       const QString& extraction)
{
  emit fileSelectionUpdateRequested();
  SelectedTaggedFileIterator it(getRootIndex(),
                                getFileSelectionModel(),
                                true);
  ImportParser parser;
  parser.setFormat(extraction);
  while (it.hasNext()) {
    TaggedFile* taggedFile = it.next();
    taggedFile = FileProxyModel::readTagsFromTaggedFile(taggedFile);
    ImportTrackData trackData(*taggedFile, tagMask);
    TextImporter::importFromTags(source, parser, trackData);
    taggedFile->setFrames(Frame::tagNumberFromMask(tagMask), trackData);
  }
  emit selectedFilesUpdated();
  return parser.getReturnValues();
}

/**
 * Export.
 *
 * @param tagVersion tag version
 * @param path   path of file, "clipboard" for export to clipboard
 * @param fmtIdx index of format
 *
 * @return true if ok.
 */
bool Kid3Application::exportTags(Frame::TagVersion tagVersion,
                                 const QString& path, int fmtIdx)
{
  ImportTrackDataVector trackDataVector;
  filesToTrackData(tagVersion, trackDataVector);
  m_textExporter->setTrackData(trackDataVector);
  m_textExporter->updateTextUsingConfig(fmtIdx);
  if (path == QLatin1String("clipboard")) {
    return m_platformTools->writeToClipboard(m_textExporter->getText());
  } else {
    return m_textExporter->exportToFile(path);
  }
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
  bool noSelection = !cfg.onlySelectedFiles() || !selectModel ||
                     !selectModel->hasSelection();
  bool ok = true;
  QModelIndex rootIndex;

  if (cfg.location() == PlaylistConfig::PL_CurrentDirectory) {
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
            selectedDirPrefix = QLatin1String("");
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
            selectedDirPrefix = QLatin1String("");
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
 * Write empty playlist.
 * @param cfg playlist configuration to use
 * @param fileName file name for playlist
 * @return true if ok.
 */
bool Kid3Application::writeEmptyPlaylist(const PlaylistConfig& cfg,
                                         const QString& fileName)
{
  QString path = getDirPath();
  PlaylistCreator plCtr(path, cfg);
  if (!path.endsWith(QLatin1Char('/'))) {
    path += QLatin1Char('/');
  }
  path += fileName;
  QString ext = cfg.fileExtensionForFormat();
  if (!path.endsWith(ext)) {
    path += ext;
  }
  return plCtr.write(path, QList<QPersistentModelIndex>());
}

/**
 * Write playlist using current playlist configuration.
 *
 * @return true if ok.
 */
bool Kid3Application::writePlaylist()
{
  return writePlaylist(PlaylistConfig::instance());
}

/**
 * Get items of a playlist.
 * @param path path to playlist file
 * @return list of absolute paths to playlist items.
 */
QStringList Kid3Application::getPlaylistItems(const QString& path)
{
  return playlistModel(path)->pathsInPlaylist();
}

/**
 * Set items of a playlist.
 * @param path path to playlist file
 * @param items list of absolute paths to playlist items
 * @return true if ok, false if not all @a items were found and added or
 *         saving failed.
 */
bool Kid3Application::setPlaylistItems(const QString& path,
                                       const QStringList& items)
{
  PlaylistModel* model = playlistModel(path);
  if (model->setPathsInPlaylist(items)) {
    return model->save();
  }
  return false;
}

/**
 * Get playlist model for a play list file.
 * @param path path to playlist file
 * @return playlist model.
 */
PlaylistModel* Kid3Application::playlistModel(const QString& path)
{
  // Create an absolute path with a value which does not depend on the file's
  // existence or whether the path given is relative or absolute.
  QString absPath;
  if (!path.isEmpty()) {
    QFileInfo fileInfo(path);
    absPath = fileInfo.absoluteDir().filePath(fileInfo.fileName());
  }

  PlaylistModel* model = m_playlistModels.value(absPath);
  if (!model) {
    model = new PlaylistModel(m_fileProxyModel, this);
    m_playlistModels.insert(absPath, model);
  }
  model->setPlaylistFile(absPath);
  return model;
}

/**
 * Check if any playlist model has unsaved modifications.
 * @return true if there is a modified playlist model.
 */
bool Kid3Application::hasModifiedPlaylistModel() const
{
  for (auto it = m_playlistModels.constBegin();
       it != m_playlistModels.constEnd();
       ++it) {
    if ((*it)->isModified()) {
      return true;
    }
  }
  return false;
}

/**
 * Save all modified playlist models.
 */
void Kid3Application::saveModifiedPlaylistModels()
{
  for (auto it = m_playlistModels.begin(); it != m_playlistModels.end(); ++it) { // clazy:exclude=detaching-member
    if ((*it)->isModified()) {
      (*it)->save();
    }
  }
}

/**
 * Set track data with tagged files of directory.
 *
 * @param tagVersion tag version
 * @param trackDataList is filled with track data
 */
void Kid3Application::filesToTrackData(Frame::TagVersion tagVersion,
                                       ImportTrackDataVector& trackDataList)
{
  TaggedFileOfDirectoryIterator it(currentOrRootIndex());
  while (it.hasNext()) {
    TaggedFile* taggedFile = it.next();
    taggedFile = FileProxyModel::readTagsFromTaggedFile(taggedFile);
    trackDataList.push_back(ImportTrackData(*taggedFile, tagVersion));
  }
}

/**
 * Set track data model with tagged files of directory.
 *
 * @param tagVersion tag version
 */
void Kid3Application::filesToTrackDataModel(Frame::TagVersion tagVersion)
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
void Kid3Application::trackDataModelToFiles(Frame::TagVersion tagVersion)
{
  ImportTrackDataVector trackDataList(getTrackDataModel()->getTrackData());
  auto it = trackDataList.begin();
  FrameFilter flt;
  Frame::TagNumber fltTagNr = Frame::tagNumberFromMask(tagVersion);
  if (fltTagNr < Frame::Tag_NumValues) {
    flt = frameModel(fltTagNr)->getEnabledFrameFilter(true);
  }
  TaggedFileOfDirectoryIterator tfit(currentOrRootIndex());
  while (tfit.hasNext()) {
    TaggedFile* taggedFile = tfit.next();
    taggedFile->readTags(false);
    if (it != trackDataList.end()) {
      it->removeDisabledFrames(flt);
      formatFramesIfEnabled(*it);
      FOR_TAGS_IN_MASK(tagNr, tagVersion) {
        if (tagNr == Frame::Tag_Id3v1) {
          taggedFile->setFrames(tagNr, *it, false);
        } else {
          FrameCollection oldFrames;
          taggedFile->getAllFrames(tagNr, oldFrames);
          it->markChangedFrames(oldFrames);
          taggedFile->setFrames(tagNr, *it, true);
        }
      }
      ++it;
    } else {
      break;
    }
  }

  if ((tagVersion & (1 << Frame::Tag_Picture)) &&
      flt.isEnabled(Frame::FT_Picture) &&
      !trackDataList.getCoverArtUrl().isEmpty()) {
    downloadImage(trackDataList.getCoverArtUrl(), ImageForImportTrackData);
  }

  if (getFileSelectionModel()->hasSelection()) {
    emit selectedFilesUpdated();
  }
}

/**
 * Download an image file.
 *
 * @param url  URL of image
 * @param dest specifies affected files
 */
void Kid3Application::downloadImage(const QUrl& url, DownloadImageDestination dest)
{
  QUrl imgurl(DownloadClient::getImageUrl(url));
  if (!imgurl.isEmpty()) {
    m_downloadImageDest = dest;
    m_downloadClient->startDownload(imgurl);
  }
}

/**
 * Download an image file.
 *
 * @param url URL of image
 * @param allFilesInDir true to add the image to all files in the directory
 */
void Kid3Application::downloadImage(const QString& url, bool allFilesInDir)
{
  QUrl imgurl(url);
  downloadImage(imgurl, allFilesInDir
                ? ImageForAllFilesInDirectory : ImageForSelectedFiles);
}

/**
 * Perform a batch import for the selected directories.
 * @param profile batch import profile
 * @param tagVersion import destination tag versions
 */
void Kid3Application::batchImport(const BatchImportProfile& profile,
                                  Frame::TagVersion tagVersion)
{
  m_batchImportProfile = &profile;
  m_batchImportTagVersion = tagVersion;
  m_batchImportAlbums.clear();
  m_batchImportTrackDataList.clear();
  m_lastProcessedDirName.clear();
  m_batchImporter->clearAborted();
  m_batchImporter->emitReportImportEvent(BatchImporter::ReadingDirectory,
                                         QString());
  // If no directories are selected, process files of the current directory.
  QList<QPersistentModelIndex> indexes;
  const auto selectedIndexes = m_fileSelectionModel->selectedRows();
  for (const QModelIndex& index : selectedIndexes) {
    if (m_fileProxyModel->isDir(index)) {
      indexes.append(index);
    }
  }
  if (indexes.isEmpty()) {
    indexes.append(m_fileProxyModelRootIndex);
  }

  connect(m_fileProxyModelIterator, &FileProxyModelIterator::nextReady,
          this, &Kid3Application::batchImportNextFile);
  m_fileProxyModelIterator->start(indexes);
}

/**
 * Perform a batch import for the selected directories.
 * @param profileName batch import profile name
 * @param tagVersion import destination tag versions
 * @return true if profile with @a profileName found.
 */
bool Kid3Application::batchImport(const QString& profileName,
                                  Frame::TagVersion tagVersion)
{
  if (!m_namedBatchImportProfile) {
    m_namedBatchImportProfile.reset(new BatchImportProfile);
  }
  if (BatchImportConfig::instance().getProfileByName(
        profileName, *m_namedBatchImportProfile)) {
    batchImport(*m_namedBatchImportProfile, tagVersion);
    return true;
  }
  return false;
}

/**
 * Apply single file to batch import.
 *
 * @param index index of file in file proxy model
 */
void Kid3Application::batchImportNextFile(const QPersistentModelIndex& index)
{
  bool terminated = !index.isValid();
  if (!terminated) {
    if (TaggedFile* taggedFile = FileProxyModel::getTaggedFileOfIndex(index)) {
      taggedFile = FileProxyModel::readTagsFromTaggedFile(taggedFile);
      if (taggedFile->getDirname() != m_lastProcessedDirName) {
        m_lastProcessedDirName = taggedFile->getDirname();
        if (!m_batchImportTrackDataList.isEmpty()) {
          m_batchImportAlbums.append(m_batchImportTrackDataList);
        }
        m_batchImportTrackDataList.clear();
        if (m_batchImporter->isAborted()) {
          terminated = true;
        }
      }
      m_batchImportTrackDataList.append(ImportTrackData(*taggedFile,
                                                      m_batchImportTagVersion));
    }
  }
  if (terminated) {
    m_fileProxyModelIterator->abort();
    disconnect(m_fileProxyModelIterator,
               &FileProxyModelIterator::nextReady,
               this, &Kid3Application::batchImportNextFile);
    if (!m_batchImporter->isAborted()) {
      if (!m_batchImportTrackDataList.isEmpty()) {
        m_batchImportAlbums.append(m_batchImportTrackDataList);
      }
      Frame::TagNumber fltTagNr =
          Frame::tagNumberFromMask(m_batchImportTagVersion);
      if (fltTagNr < Frame::Tag_NumValues) {
        m_batchImporter->setFrameFilter(
              frameModel(fltTagNr)->getEnabledFrameFilter(true));
      }
      m_batchImporter->start(m_batchImportAlbums, *m_batchImportProfile,
                             m_batchImportTagVersion);
    }
  }
}

/**
 * Format frames if format while editing is switched on.
 *
 * @param frames frames
 */
void Kid3Application::formatFramesIfEnabled(FrameCollection& frames) const
{
  TagFormatConfig::instance().formatFramesIfEnabled(frames);
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
    if (!dirname.endsWith(QLatin1Char('/'))) dirname += QLatin1Char('/');
    return dirname;
  } else if (TaggedFile* taggedFile =
             FileProxyModel::getTaggedFileOfIndex(index)) {
    return taggedFile->getAbsFilename();
  }
  return QLatin1String("");
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
    FilenameFormatConfig::instance().formatString(fn);
    taggedFile->setFilename(fn);
  }
  emit selectedFilesUpdated();
}

/**
 * Apply tag format.
 */
void Kid3Application::applyTagFormat()
{
  emit fileSelectionUpdateRequested();
  FrameCollection frames;
  FrameFilter flt[Frame::Tag_NumValues];
  FOR_ALL_TAGS(tagNr) {
    flt[tagNr] = frameModel(tagNr)->getEnabledFrameFilter(true);
  }
  SelectedTaggedFileIterator it(getRootIndex(),
                                getFileSelectionModel(),
                                true);
  while (it.hasNext()) {
    TaggedFile* taggedFile = it.next();
    taggedFile->readTags(false);
    FOR_ALL_TAGS(tagNr) {
      taggedFile->getAllFrames(tagNr, frames);
      frames.removeDisabledFrames(flt[tagNr]);
      TagFormatConfig::instance().formatFrames(frames);
      taggedFile->setFrames(tagNr, frames);
    }
  }
  emit selectedFilesUpdated();
}

/**
 * Apply text encoding.
 * Set the text encoding selected in the settings Tags/ID3v2/Text encoding
 * for all selected files which have an ID3v2 tag.
 */
void Kid3Application::applyTextEncoding()
{
  emit fileSelectionUpdateRequested();
  Frame::TextEncoding encoding = frameTextEncodingFromConfig();
  FrameCollection frames;
  SelectedTaggedFileIterator it(getRootIndex(),
                                getFileSelectionModel(),
                                true);
  while (it.hasNext()) {
    TaggedFile* taggedFile = it.next();
    taggedFile->readTags(false);
    taggedFile->getAllFrames(Frame::Tag_Id3v2, frames);
    for (auto frameIt = frames.begin(); frameIt != frames.end(); ++frameIt) {
      auto& frame = const_cast<Frame&>(*frameIt);
      Frame::TextEncoding enc = encoding;
      if (taggedFile->getTagFormat(Frame::Tag_Id3v2) == QLatin1String("ID3v2.3.0")) {
        // TagLib sets the ID3v2.3.0 frame containing the date internally with
        // ISO-8859-1, so the encoding cannot be set for such frames.
        if (taggedFile->taggedFileKey() == QLatin1String("TaglibMetadata") &&
            frame.getType() == Frame::FT_Date &&
            enc != Frame::TE_ISO8859_1)
          continue;
        // Only ISO-8859-1 and UTF16 are allowed for ID3v2.3.0.
        if (enc != Frame::TE_ISO8859_1)
          enc = Frame::TE_UTF16;
      }
      Frame::FieldList& fields = frame.fieldList();
      for (auto fieldIt = fields.begin(); fieldIt != fields.end(); ++fieldIt) {
        if (fieldIt->m_id == Frame::ID_TextEnc &&
            fieldIt->m_value.toInt() != enc) {
          fieldIt->m_value = enc;
          frame.setValueChanged();
        }
      }
    }
    taggedFile->setFrames(Frame::Tag_Id3v2, frames);
  }
  emit selectedFilesUpdated();
}

/**
 * Copy tags into copy buffer.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 */
void Kid3Application::copyTags(Frame::TagVersion tagMask)
{
  Frame::TagNumber tagNr = Frame::tagNumberFromMask(tagMask);
  if (tagNr >= Frame::Tag_NumValues)
    return;

  emit fileSelectionUpdateRequested();
  m_copyTags = frameModel(tagNr)->frames().copyEnabledFrames(
    frameModel(tagNr)->getEnabledFrameFilter(true));
}

/**
 * Paste from copy buffer to tags.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 */
void Kid3Application::pasteTags(Frame::TagVersion tagMask)
{
  Frame::TagNumber tagNr = Frame::tagNumberFromMask(tagMask);
  if (tagNr >= Frame::Tag_NumValues)
    return;

  emit fileSelectionUpdateRequested();
  FrameCollection frames(m_copyTags.copyEnabledFrames(
                         frameModel(tagNr)->getEnabledFrameFilter(true)));
  formatFramesIfEnabled(frames);
  SelectedTaggedFileIterator it(getRootIndex(),
                                getFileSelectionModel(),
                                false);
  while (it.hasNext()) {
    it.next()->setFrames(tagNr, frames, false);
  }
  emit selectedFilesUpdated();
}

/**
 * Set tag from other tag.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 */
void Kid3Application::copyToOtherTag(Frame::TagVersion tagMask)
{
  Frame::TagNumber dstTagNr = Frame::tagNumberFromMask(tagMask);
  if (dstTagNr >= Frame::Tag_NumValues)
    return;

  Frame::TagNumber srcTagNr = dstTagNr == Frame::Tag_2
      ? Frame::Tag_1 : Frame::Tag_2;
  copyTag(srcTagNr, dstTagNr);
}

/**
 * Copy from a tag to another tag.
 * @param srcTagNr source tag number
 * @param dstTagNr destination tag number
 */
void Kid3Application::copyTag(Frame::TagNumber srcTagNr, Frame::TagNumber dstTagNr)
{
  emit fileSelectionUpdateRequested();
  FrameCollection frames;
  FrameFilter flt(frameModel(dstTagNr)->getEnabledFrameFilter(true));
  SelectedTaggedFileIterator it(getRootIndex(),
                                getFileSelectionModel(),
                                false);
  while (it.hasNext()) {
    TaggedFile* taggedFile = it.next();
    taggedFile->getAllFrames(srcTagNr, frames);
    frames.removeDisabledFrames(flt);
    frames.setIndexesInvalid();
    formatFramesIfEnabled(frames);
    taggedFile->setFrames(dstTagNr, frames, false);
  }
  emit selectedFilesUpdated();
}

/**
 * Remove tags in selected files.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 */
void Kid3Application::removeTags(Frame::TagVersion tagMask)
{
  Frame::TagNumber tagNr = Frame::tagNumberFromMask(tagMask);
  if (tagNr >= Frame::Tag_NumValues)
    return;

  emit fileSelectionUpdateRequested();
  FrameFilter flt(frameModel(tagNr)->getEnabledFrameFilter(true));
  SelectedTaggedFileIterator it(getRootIndex(),
                                getFileSelectionModel(),
                                false);
  while (it.hasNext()) {
    it.next()->deleteFrames(tagNr, flt);
  }
  emit selectedFilesUpdated();
}

/**
 * Set tags according to filename.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 */
void Kid3Application::getTagsFromFilename(Frame::TagVersion tagMask)
{
  Frame::TagNumber tagNr = Frame::tagNumberFromMask(tagMask);
  if (tagNr >= Frame::Tag_NumValues)
    return;

  emit fileSelectionUpdateRequested();
  FrameCollection frames;
  QItemSelectionModel* selectModel = getFileSelectionModel();
  SelectedTaggedFileIterator it(getRootIndex(),
                                selectModel,
                                false);
  FrameFilter flt(frameModel(tagNr)->getEnabledFrameFilter(true));
  while (it.hasNext()) {
    TaggedFile* taggedFile = it.next();
    taggedFile->getAllFrames(tagNr, frames);
    taggedFile->getTagsFromFilename(
          frames, FileConfig::instance().fromFilenameFormat());
    frames.removeDisabledFrames(flt);
    formatFramesIfEnabled(frames);
    taggedFile->setFrames(tagNr, frames);
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
void Kid3Application::getFilenameFromTags(Frame::TagVersion tagVersion)
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
      taggedFile->setFilenameFormattedIfEnabled(
        trackData.formatFilenameFromTags(FileConfig::instance().toFilenameFormat()));
    }
  }
  emit selectedFilesUpdated();
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
      m_fileSelectionModel->selectedRows());
  if (selItems.size() != 1)
    return nullptr;

  return FileProxyModel::getTaggedFileOfIndex(selItems.first());
}

/**
 * Update the stored current selection with the list of all selected items.
 */
void Kid3Application::updateCurrentSelection()
{
  m_currentSelection.clear();
  const auto selectedIndexes = m_fileSelectionModel->selectedRows();
  for (const QModelIndex& index : selectedIndexes) {
    m_currentSelection.append(QPersistentModelIndex(index));
  }
}

/**
 * Edit selected frame.
 * @param tagNr tag number
 */
void Kid3Application::editFrame(Frame::TagNumber tagNr)
{
  FrameList* framelist = m_framelist[tagNr];
  emit fileSelectionUpdateRequested();
  m_editFrameTaggedFile = getSelectedFile();
  if (const Frame* selectedFrame = frameModel(tagNr)->getFrameOfIndex(
        getFramesSelectionModel(tagNr)->currentIndex())) {
    if (m_editFrameTaggedFile) {
      framelist->setTaggedFile(m_editFrameTaggedFile);
      framelist->setFrame(*selectedFrame);
      if (selectedFrame->getIndex() != -1) {
        framelist->editFrame();
      } else {
        // Edit a frame which does not exist, switch to add mode.
        m_addFrameTaggedFile = m_editFrameTaggedFile;
        m_editFrameTaggedFile = nullptr;
        framelist->addAndEditFrame();
      }
    } else {
      // multiple files selected
      // Get the first selected file by using a temporary iterator.
      TaggedFile* firstFile = SelectedTaggedFileIterator(
            getRootIndex(), getFileSelectionModel(), false).peekNext();
      if (firstFile) {
        framelist->setTaggedFile(firstFile);
        m_editFrameName = framelist->getSelectedName();
        if (!m_editFrameName.isEmpty()) {
          framelist->setFrame(*selectedFrame);
          framelist->addFrameFieldList();
          framelist->editFrame();
        }
      }
    }
  }
}

/**
 * Called when a frame is edited.
 * @param frame edited frame, 0 if canceled
 */
void Kid3Application::onFrameEdited(const Frame* frame)
{
  auto framelist = qobject_cast<FrameList*>(sender());
  if (!framelist || !frame)
    return;

  Frame::TagNumber tagNr = framelist->tagNumber();
  if (m_editFrameTaggedFile) {
    emit frameModified(m_editFrameTaggedFile, tagNr);
  } else {
    framelist->setFrame(*frame);

    // Start a new iteration because the file selection model can be
    // changed by editFrameOfTaggedFile(), e.g. when a file is exported
    // from a picture frame.
    SelectedTaggedFileIterator tfit(getRootIndex(),
                                    getFileSelectionModel(),
                                    false);
    while (tfit.hasNext()) {
      TaggedFile* currentFile = tfit.next();
      FrameCollection frames;
      currentFile->getAllFrames(tagNr, frames);
      for (auto it = frames.cbegin(); it != frames.cend(); ++it) {
        if (it->getName() == m_editFrameName) {
          currentFile->deleteFrame(tagNr, *it);
          break;
        }
      }
      framelist->setTaggedFile(currentFile);
      framelist->pasteFrame();
    }
    emit selectedFilesUpdated();
    framelist->selectByName(frame->getName());
  }
}

/**
 * Delete selected frame.
 * @param tagNr tag number
 * @param frameName name of frame to delete, empty to delete selected frame
 * @param index 0 for first frame with @a frameName, 1 for second, etc.
 */
void Kid3Application::deleteFrame(Frame::TagNumber tagNr,
                                  const QString& frameName, int index)
{
  FrameList* framelist = m_framelist[tagNr];
  emit fileSelectionUpdateRequested();
  TaggedFile* taggedFile = getSelectedFile();
  if (taggedFile && frameName.isEmpty()) {
    // delete selected frame from single file
    if (!framelist->deleteFrame()) {
      // frame not found
      return;
    }
    emit frameModified(taggedFile, tagNr);
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
        framelist->setTaggedFile(taggedFile);
        name = frameName.isEmpty() ? framelist->getSelectedName() : frameName;
      }
      FrameCollection frames;
      currentFile->getAllFrames(tagNr, frames);
      int currentIndex = 0;
      for (auto it = frames.cbegin(); it != frames.cend(); ++it) {
        if (it->getName() == name) {
          if (currentIndex == index) {
            currentFile->deleteFrame(tagNr, *it);
            break;
          } else {
            ++currentIndex;
          }
        }
      }
    }
    framelist->saveCursor();
    emit selectedFilesUpdated();
    framelist->restoreCursor();
  }
}

/**
 * Select a frame type and add such a frame to frame list.
 * @param tagNr tag number
 * @param frame frame to add, if 0 the user has to select and edit the frame
 * @param edit if true and a frame is set, the user can edit the frame before
 * it is added
 */
void Kid3Application::addFrame(Frame::TagNumber tagNr,
                               const Frame* frame, bool edit)
{
  if (tagNr >= Frame::Tag_NumValues)
    return;

  FrameList* framelist = m_framelist[tagNr];
  emit fileSelectionUpdateRequested();
  TaggedFile* currentFile = nullptr;
  m_addFrameTaggedFile = getSelectedFile();
  if (m_addFrameTaggedFile) {
    currentFile = m_addFrameTaggedFile;
  } else {
    // multiple files selected
    SelectedTaggedFileIterator tfit(getRootIndex(),
                                    getFileSelectionModel(),
                                    false);
    if (tfit.hasNext()) {
      currentFile = tfit.next();
      framelist->setTaggedFile(currentFile);
    }
  }

  if (currentFile) {
    if (edit) {
      if (frame) {
        framelist->setFrame(*frame);
        framelist->addAndEditFrame();
      } else {
        framelist->selectAddAndEditFrame();
      }
    } else {
      framelist->setFrame(*frame);
      onFrameAdded(framelist->pasteFrame() ? &framelist->getFrame()
                                           : nullptr, tagNr);
    }
  }
}

/**
 * Called when a frame is added.
 * @param frame edited frame, 0 if canceled
 * @param tagNr tag number used if slot is not invoked by framelist signal
 */
void Kid3Application::onFrameAdded(const Frame* frame, Frame::TagNumber tagNr)
{
  if (!frame)
    return;

  auto framelist = qobject_cast<FrameList*>(sender());
  if (!framelist) {
    framelist = m_framelist[tagNr];
  }
  if (m_addFrameTaggedFile) {
    emit frameModified(m_addFrameTaggedFile, tagNr);
    if (framelist->isPictureFrame()) {
      // update preview picture
      emit selectedFilesUpdated();
    }
  } else {
    // multiple files selected
    bool firstFile = true;
    int frameId = -1;
    framelist->setFrame(*frame);

    SelectedTaggedFileIterator tfit(getRootIndex(),
                                    getFileSelectionModel(),
                                    false);
    while (tfit.hasNext()) {
      TaggedFile* currentFile = tfit.next();
      if (firstFile) {
        firstFile = false;
        m_addFrameTaggedFile = currentFile;
        framelist->setTaggedFile(currentFile);
        frameId = framelist->getSelectedId();
      } else {
        framelist->setTaggedFile(currentFile);
        framelist->pasteFrame();
      }
    }
    framelist->setTaggedFile(m_addFrameTaggedFile);
    if (frameId != -1) {
      framelist->setSelectedId(frameId);
    }
    emit selectedFilesUpdated();
    framelist->selectByName(frame->getName());
  }
}

/**
 * Called by framelist when a frame is added.
 * Same as onFrameAdded() with default argument, provided for functor-based
 * connections.
 * @param frame added frame, 0 if canceled
 */
void Kid3Application::onTag2FrameAdded(const Frame* frame)
{
  onFrameAdded(frame, Frame::Tag_2);
}

/**
 * Select a frame type and add such a frame to the frame list.
 * @param tagNr tag number
 */
void Kid3Application::selectAndAddFrame(Frame::TagNumber tagNr)
{
  addFrame(tagNr, nullptr, true);
}

/**
 * Edit a picture frame if one exists or add a new one.
 */
void Kid3Application::editOrAddPicture()
{
  if (m_framelist[Frame::Tag_Picture]->selectByName(QLatin1String("Picture"))) {
    editFrame(Frame::Tag_Picture);
  } else {
    PictureFrame frame;
    PictureFrame::setTextEncoding(frame, frameTextEncodingFromConfig());
    addFrame(Frame::Tag_Picture, &frame, true);
  }
}

/**
 * Open directory or add pictures on drop.
 *
 * @param paths paths of directories or files in directory
 * @param isInternal true if this is an internal drop
 */
void Kid3Application::dropLocalFiles(const QStringList& paths, bool isInternal)
{
  QStringList filePaths;
  QStringList picturePaths;
  for (QString txt : paths) {
    int lfPos = txt.indexOf(QLatin1Char('\n'));
    if (lfPos > 0 && lfPos < static_cast<int>(txt.length()) - 1) {
      txt.truncate(lfPos + 1);
    }
    QString dir = txt.trimmed();
    if (!dir.isEmpty()) {
      if (dir.endsWith(QLatin1String(".jpg"), Qt::CaseInsensitive) ||
          dir.endsWith(QLatin1String(".jpeg"), Qt::CaseInsensitive) ||
          dir.endsWith(QLatin1String(".png"), Qt::CaseInsensitive)) {
        picturePaths.append(dir); // clazy:exclude=reserve-candidates
      } else {
        filePaths.append(dir); // clazy:exclude=reserve-candidates
      }
    }
  }
  if (!filePaths.isEmpty() && !isInternal) {
    resetFileFilterIfNotMatching(filePaths);
    emit fileSelectionUpdateRequested();
    emit confirmedOpenDirectoryRequested(filePaths);
  } else if (!picturePaths.isEmpty()) {
    const auto constPicturePaths = picturePaths;
    for (const QString& picturePath : constPicturePaths) {
      PictureFrame frame;
      if (PictureFrame::setDataFromFile(frame, picturePath)) {
        QString fileName(picturePath);
        int slashPos = fileName.lastIndexOf(QLatin1Char('/'));
        if (slashPos != -1) {
          fileName = fileName.mid(slashPos + 1);
        }
        PictureFrame::setMimeTypeFromFileName(frame, fileName);
        PictureFrame::setDescription(frame, fileName);
        PictureFrame::setTextEncoding(frame, frameTextEncodingFromConfig());
        addFrame(Frame::Tag_Picture, &frame);
        emit selectedFilesUpdated();
      }
    }
  }
}

/**
 * Open directory or add pictures on drop.
 *
 * @param paths paths of directories or files in directory
 */
void Kid3Application::openDrop(const QStringList& paths)
{
  dropLocalFiles(paths, false);
}

/**
 * Handle drop of URLs.
 *
 * @param urlList picture, tagged file and folder URLs to handle (if local)
 * @param isInternal true if this is an internal drop
 */
void Kid3Application::dropUrls(const QList<QUrl>& urlList, bool isInternal)
{
  QList<QUrl> urls(urlList);
#ifdef Q_OS_MAC
  // workaround for https://bugreports.qt-project.org/browse/QTBUG-40449
  for (auto it = urls.begin(); it != urls.end(); ++it) {
    if (it->host().isEmpty() &&
        it->path().startsWith(QLatin1String("/.file/id="))) {
      *it = QUrl::fromCFURL(CFURLCreateFilePathURL(NULL, it->toCFURL(), NULL));
    }
  }
#endif
  if (urls.isEmpty())
    return;
  if (urls.first().isLocalFile()) {
    QStringList localFiles;
    for (auto it = urls.constBegin(); it != urls.constEnd(); ++it) {
      localFiles.append(it->toLocalFile());
    }
    dropLocalFiles(localFiles, isInternal);
  } else {
    dropUrl(urls.first());
  }
}

/**
 * Handle drop of URLs.
 *
 * @param urlList picture, tagged file and folder URLs to handle (if local)
 */
void Kid3Application::openDropUrls(const QList<QUrl>& urlList)
{
  dropUrls(urlList, false);
}

/**
 * Add picture on drop.
 *
 * @param frame dropped picture frame
 */
void Kid3Application::dropImage(Frame* frame)
{
  PictureFrame::setTextEncoding(*frame, frameTextEncodingFromConfig());
  addFrame(Frame::Tag_Picture, frame);
  emit selectedFilesUpdated();
}

/**
 * Handle URL on drop.
 *
 * @param url dropped URL.
 */
void Kid3Application::dropUrl(const QUrl& url)
{
  downloadImage(url, Kid3Application::ImageForSelectedFiles);
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
  // An empty mime type is accepted to allow downloads via FTP.
  if (mimeType.startsWith(QLatin1String("image")) ||
      mimeType.isEmpty()) {
    PictureFrame frame(data, url, PictureFrame::PT_CoverFront, mimeType,
                       frameTextEncodingFromConfig());
    if (getDownloadImageDestination() == ImageForAllFilesInDirectory) {
      TaggedFileOfDirectoryIterator it(currentOrRootIndex());
      while (it.hasNext()) {
        TaggedFile* taggedFile = it.next();
        taggedFile->readTags(false);
        taggedFile->addFrame(Frame::Tag_Picture, frame);
      }
    } else if (getDownloadImageDestination() == ImageForImportTrackData) {
      const ImportTrackDataVector& trackDataVector(
            getTrackDataModel()->trackData());
      for (auto it = trackDataVector.constBegin();
           it != trackDataVector.constEnd();
           ++it) {
        TaggedFile* taggedFile;
        if (it->isEnabled() && (taggedFile = it->getTaggedFile()) != nullptr) {
          taggedFile->readTags(false);
          taggedFile->addFrame(Frame::Tag_Picture, frame);
        }
      }
    } else {
      addFrame(Frame::Tag_Picture, &frame);
    }
    emit selectedFilesUpdated();
  }
}

/**
 * Set the first file as the current file.
 *
 * @param select true to select the file
 * @param onlyTaggedFiles only consider tagged files
 *
 * @return true if a file exists.
 */
bool Kid3Application::firstFile(bool select, bool onlyTaggedFiles)
{
  m_fileSelectionModel->setCurrentIndex(getRootIndex(),
                                        QItemSelectionModel::NoUpdate);
  return nextFile(select, onlyTaggedFiles);
}

/**
 * Set the next file as the current file.
 *
 * @param select true to select the file
 * @param onlyTaggedFiles only consider tagged files
 *
 * @return true if a next file exists.
 */
bool Kid3Application::nextFile(bool select, bool onlyTaggedFiles)
{
  QModelIndex next(m_fileSelectionModel->currentIndex()), current;
  do {
    current = next;
    next = QModelIndex();
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
  } while (onlyTaggedFiles && !FileProxyModel::getTaggedFileOfIndex(next));
  if (!next.isValid())
    return false;
  m_fileSelectionModel->setCurrentIndex(next,
    select ? QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows
           : QItemSelectionModel::Current);
  return true;
}

/**
 * Set the previous file as the current file.
 *
 * @param select true to select the file
 * @param onlyTaggedFiles only consider tagged files
 *
 * @return true if a previous file exists.
 */
bool Kid3Application::previousFile(bool select, bool onlyTaggedFiles)
{
  QModelIndex previous(m_fileSelectionModel->currentIndex()), current;
  do {
    current = previous;
    previous = QModelIndex();
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
    if (previous == getRootIndex()) {
      return false;
    }
  } while (onlyTaggedFiles && !FileProxyModel::getTaggedFileOfIndex(previous));
  if (!previous.isValid())
    return false;
  m_fileSelectionModel->setCurrentIndex(previous,
    select ? QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows
           : QItemSelectionModel::Current);
  return true;
}

/**
 * Select or deselect the current file.
 *
 * @param select true to select the file, false to deselect it
 *
 * @return true if a current file exists.
 */
bool Kid3Application::selectCurrentFile(bool select)
{
  QModelIndex currentIdx(m_fileSelectionModel->currentIndex());
  if (!currentIdx.isValid() || currentIdx == getRootIndex())
    return false;

  m_fileSelectionModel->setCurrentIndex(currentIdx,
    (select ? QItemSelectionModel::Select : QItemSelectionModel::Deselect) |
    QItemSelectionModel::Rows);
  return true;
}

/**
 * Select all files.
 */
void Kid3Application::selectAllFiles()
{
  QItemSelection selection;
  ModelIterator it(m_fileProxyModelRootIndex);
  while (it.hasNext()) {
    selection.append(QItemSelectionRange(it.next()));
  }
  m_fileSelectionModel->select(selection,
      QItemSelectionModel::Select | QItemSelectionModel::Rows);
}

/**
 * Deselect all files.
 */
void Kid3Application::deselectAllFiles()
{
  m_fileSelectionModel->clearSelection();
}

/**
 * Select all files in the current directory.
 */
void Kid3Application::selectAllInDirectory()
{
  QModelIndex parent = m_fileSelectionModel->currentIndex();
  if (parent.isValid()) {
    if (!m_fileProxyModel->hasChildren(parent)) {
      parent = parent.parent();
    }
    QItemSelection selection;
    for (int row = 0; row < m_fileProxyModel->rowCount(parent); ++row) {
      QModelIndex index = m_fileProxyModel->index(row, 0, parent);
      if (!m_fileProxyModel->hasChildren(index)) {
        selection.append(QItemSelectionRange(index)); // clazy:exclude=reserve-candidates
      }
    }
    m_fileSelectionModel->select(selection,
                     QItemSelectionModel::Select | QItemSelectionModel::Rows);
  }
}

/**
 * Set a specific file as the current file.
 *
 * @param filePath path to file
 * @param select true to select the file
 *
 * @return true if file exists.
 */
bool Kid3Application::selectFile(const QString& filePath, bool select)
{
  QModelIndex index = m_fileProxyModel->index(filePath);
  if (!index.isValid())
    return false;

  m_fileSelectionModel->setCurrentIndex(index,
    select ? QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows
           : QItemSelectionModel::Current);
  return true;
}

/**
 * Get paths to all selected files.
 * @param onlyTaggedFiles only consider tagged files
 * @return list of absolute file paths.
 */
QStringList Kid3Application::getSelectedFilePaths(bool onlyTaggedFiles) const
{
  QStringList files;
  const QModelIndexList selItems = m_fileSelectionModel->selectedRows();
  if (onlyTaggedFiles) {
    for (const QModelIndex& index : selItems) {
      if (TaggedFile* taggedFile = FileProxyModel::getTaggedFileOfIndex(index))
      {
        files.append(taggedFile->getAbsFilename());
      }
    }
  } else {
    files.reserve(selItems.size());
    for (const QModelIndex& index : selItems) {
      files.append(m_fileProxyModel->filePath(index));
    }
  }
  return files;
}

/**
 * Fetch entries of directory if not already fetched.
 * This works like FileList::expand(), but without expanding tree view
 * items and independent of the GUI. The processing is done in the background
 * by FileSystemModel, so the fetched items are not immediately available
 * after calling this method.
 *
 * @param index index of directory item
 */
void Kid3Application::fetchDirectory(const QModelIndex& index)
{
  if (index.isValid() && m_fileProxyModel->canFetchMore(index)) {
    m_fileProxyModel->fetchMore(index);
  }
}

/**
 * Fetch entries of directory and toggle expanded state if GUI available.
 * @param index index of directory item
 */
void Kid3Application::expandDirectory(const QModelIndex& index)
{
  fetchDirectory(index);
  emit toggleExpandedRequested(index);
}

/**
 * Expand the whole file list if GUI available.
 * expandFileListFinished() is emitted when finished.
 */
void Kid3Application::requestExpandFileList()
{
  emit expandFileListRequested();
}

/**
 * Called when operation for requestExpandFileList() is finished.
 */
void Kid3Application::notifyExpandFileListFinished()
{
  emit expandFileListFinished();
}

/**
 * Process change of selection.
 * The GUI is signaled to update the current selection and the controls.
 * @param selected selected items
 * @param deselected deselected items
 */
void Kid3Application::fileSelected(const QItemSelection& selected,
                                   const QItemSelection& deselected)
{
  emit fileSelectionUpdateRequested();
  emit selectedFilesChanged(selected, deselected);
}

/**
 * Search in tags for a given text.
 * @param params search parameters
 */
void Kid3Application::findText(const TagSearcher::Parameters& params)
{
  m_tagSearcher->setModel(m_fileProxyModel);
  m_tagSearcher->setRootIndex(m_fileProxyModelRootIndex);
  m_tagSearcher->find(params);
}

/**
 * Replace found text.
 * @param params search parameters
 */
void Kid3Application::replaceText(const TagSearcher::Parameters& params)
{
  m_tagSearcher->setModel(m_fileProxyModel);
  m_tagSearcher->setRootIndex(m_fileProxyModelRootIndex);
  m_tagSearcher->replace(params);
}

/**
 * Replace all occurrences.
 * @param params search parameters
 */
void Kid3Application::replaceAll(const TagSearcher::Parameters& params)
{
  m_tagSearcher->setModel(m_fileProxyModel);
  m_tagSearcher->setRootIndex(m_fileProxyModelRootIndex);
  m_tagSearcher->replaceAll(params);
}

/**
 * Schedule actions to rename a directory.
 * When finished renameActionsScheduled() is emitted.
 */
void Kid3Application::scheduleRenameActions()
{
  m_dirRenamer->clearActions();
  m_dirRenamer->clearAborted();
  // If directories are selected, rename them, else process files of the
  // current directory.
  QList<QPersistentModelIndex> indexes;
  const auto selectedIndexes = m_fileSelectionModel->selectedRows();
  for (const QModelIndex& index : selectedIndexes) {
    if (m_fileProxyModel->isDir(index)) {
      indexes.append(index);
    }
  }
  if (indexes.isEmpty()) {
    indexes.append(m_fileProxyModelRootIndex);
  }

  connect(m_fileProxyModelIterator, &FileProxyModelIterator::nextReady,
          this, &Kid3Application::scheduleNextRenameAction);
  m_fileProxyModelIterator->start(indexes);
}

/**
 * Schedule rename action for a file.
 *
 * @param index index of file in file proxy model
 */
void Kid3Application::scheduleNextRenameAction(const QPersistentModelIndex& index)
{
  bool terminated = !index.isValid();
  if (!terminated) {
    if (TaggedFile* taggedFile = FileProxyModel::getTaggedFileOfIndex(index)) {
      taggedFile = FileProxyModel::readTagsFromTaggedFile(taggedFile);
      m_dirRenamer->scheduleAction(taggedFile);
      if (m_dirRenamer->isAborted()) {
        terminated = true;
      }
    }
  }
  if (terminated) {
    m_fileProxyModelIterator->abort();
    disconnect(m_fileProxyModelIterator, &FileProxyModelIterator::nextReady,
               this, &Kid3Application::scheduleNextRenameAction);
    m_dirRenamer->endScheduleActions();
    emit renameActionsScheduled();
  }
}

/**
 * Open directory after resetting the file system model.
 * When finished directoryOpened() is emitted, also if false is returned.
 *
 * @param paths file or directory paths, if multiple paths are given, the
 * common directory is opened and the files are selected, if empty, the
 * currently open directory is reopened
 *
 * @return true if ok.
 */
bool Kid3Application::openDirectoryAfterReset(const QStringList& paths)
{
  // Clear the selection.
  m_selection->beginAddTaggedFiles();
  m_selection->endAddTaggedFiles();
  QStringList dirs(paths);
  if (dirs.isEmpty()) {
    dirs.append(m_fileSystemModel->rootPath());
  }
  m_fileSystemModel->clear();
  return openDirectory(dirs);
}

/**
 * Apply file filter after the file system model has been reset.
 */
void Kid3Application::applyFilterAfterReset()
{
  disconnect(this, &Kid3Application::directoryOpened,
             this, &Kid3Application::applyFilterAfterReset);
  proceedApplyingFilter();
}

/**
 * Apply a file filter.
 *
 * @param fileFilter filter to apply.
 */
void Kid3Application::applyFilter(FileFilter& fileFilter)
{
  m_fileFilter = &fileFilter;
  /*
   * When a lot of files are filtered out,
   * QSortFilterProxyModel::invalidateFilter() is extremely slow (probably
   * depending on the source model). In this case, I measured
   * 3s for 3000 files, 8s for 5000 files, 54s for 10000 files, and too long
   * to wait for more files. If such a case is detected, the file system model
   * is recreated in order to avoid calling invalidateFilter().
   */
  if (m_filterTotal - m_filterPassed > 4000) {
    connect(this, &Kid3Application::directoryOpened,
            this, &Kid3Application::applyFilterAfterReset);
    openDirectoryAfterReset();
  } else {
    m_fileProxyModel->disableFilteringOutIndexes();
    proceedApplyingFilter();
  }
}

/**
 * Second stage for applyFilter().
 */
void Kid3Application::proceedApplyingFilter()
{
  const bool justClearingFilter =
      m_fileFilter->isEmptyFilterExpression() && isFiltered();
  setFiltered(false);
  m_fileFilter->clearAborted();
  m_filterPassed = 0;
  m_filterTotal = 0;
  emit fileFiltered(FileFilter::Started, QString(),
                    m_filterPassed, m_filterTotal);

  m_lastProcessedDirName.clear();
  if (!justClearingFilter) {
    connect(m_fileProxyModelIterator, &FileProxyModelIterator::nextReady,
            this, &Kid3Application::filterNextFile);
    m_fileProxyModelIterator->start(m_fileProxyModelRootIndex);
  } else {
    emit fileFiltered(FileFilter::Finished, QString(),
                      m_filterPassed, m_filterTotal);
  }
}

/**
 * Apply single file to file filter.
 *
 * @param index index of file in file proxy model
 */
void Kid3Application::filterNextFile(const QPersistentModelIndex& index)
{
  if (!m_fileFilter)
    return;

  bool terminated = !index.isValid();
  if (!terminated) {
    if (TaggedFile* taggedFile = FileProxyModel::getTaggedFileOfIndex(index)) {
      bool tagInfoRead = taggedFile->isTagInformationRead();
      taggedFile = FileProxyModel::readTagsFromTaggedFile(taggedFile);
      if (taggedFile->getDirname() != m_lastProcessedDirName) {
        m_lastProcessedDirName = taggedFile->getDirname();
        emit fileFiltered(FileFilter::Directory, m_lastProcessedDirName,
                          m_filterPassed, m_filterTotal);
      }
      bool ok;
      bool pass = m_fileFilter->filter(*taggedFile, &ok);
      if (ok) {
        ++m_filterTotal;
        if (pass) {
          ++m_filterPassed;
        }
        emit fileFiltered(
              pass ? FileFilter::FilePassed : FileFilter::FileFilteredOut,
              taggedFile->getFilename(), m_filterPassed, m_filterTotal);
        if (!pass)
          m_fileProxyModel->filterOutIndex(taggedFile->getIndex());
      } else {
        emit fileFiltered(FileFilter::ParseError, QString(),
                          m_filterPassed, m_filterTotal);
        terminated = true;
      }

      // Free resources if tag was not read before filtering
      if (!pass && !tagInfoRead) {
        taggedFile->clearTags(false);
      }

      if (m_fileFilter->isAborted()) {
        terminated = true;
        emit fileFiltered(FileFilter::Aborted, QString(),
                          m_filterPassed, m_filterTotal);
      }
    }
  }
  if (terminated) {
    if (!m_fileFilter->isAborted()) {
      emit fileFiltered(FileFilter::Finished, QString(),
                        m_filterPassed, m_filterTotal);
    }

    m_fileProxyModelIterator->abort();
    m_fileProxyModel->applyFilteringOutIndexes();
    setFiltered(!m_fileFilter->isEmptyFilterExpression());

    disconnect(m_fileProxyModelIterator, &FileProxyModelIterator::nextReady,
               this, &Kid3Application::filterNextFile);
  }
}

/**
 * Apply a file filter.
 *
 * @param expression filter expression
 */
void Kid3Application::applyFilter(const QString& expression)
{
  if (!m_expressionFileFilter) {
    m_expressionFileFilter = new FileFilter(this);
  }
  m_expressionFileFilter->clearAborted();
  m_expressionFileFilter->setFilterExpression(expression);
  m_expressionFileFilter->initParser();
  applyFilter(*m_expressionFileFilter);
}

/**
 * Abort expression file filter.
 */
void Kid3Application::abortFilter()
{
  if (m_expressionFileFilter) {
    m_expressionFileFilter->abort();
  }
}

/**
 * Perform rename actions and change application directory afterwards if it
 * was renamed.
 *
 * @return error messages, null string if no error occurred.
 */
QString Kid3Application::performRenameActions()
{
  QString errorMsg;
  m_dirRenamer->setDirName(getDirName());
  m_dirRenamer->performActions(&errorMsg);
  if (m_dirRenamer->getDirName() != getDirName()) {
    openDirectory({m_dirRenamer->getDirName()});
  }
  return errorMsg;
}

/**
 * Reset the file system model and then try to perform the rename actions.
 * On Windows, renaming directories fails when they have a subdirectory which
 * is open in the file system model. This method can be used to retry in such
 * a situation.
 */
void Kid3Application::tryRenameActionsAfterReset()
{
  connect(this, &Kid3Application::directoryOpened,
          this, &Kid3Application::performRenameActionsAfterReset);
  openDirectoryAfterReset();
}

/**
 * Perform rename actions after the file system model has been reset.
 */
void Kid3Application::performRenameActionsAfterReset()
{
  disconnect(this, &Kid3Application::directoryOpened,
             this, &Kid3Application::performRenameActionsAfterReset);
  performRenameActions();
}

/**
 * Reset the file system model and then try to rename a file.
 * On Windows, renaming directories fails when they have a subdirectory which
 * is open in the file system model. This method can be used to retry in such
 * a situation.
 *
 * @param oldName old file name
 * @param newName new file name
 */
void Kid3Application::tryRenameAfterReset(const QString& oldName,
                                          const QString& newName)
{
  m_renameAfterResetOldName = oldName;
  m_renameAfterResetNewName = newName;
  connect(this, &Kid3Application::directoryOpened,
          this, &Kid3Application::renameAfterReset);
  openDirectoryAfterReset();
}

/**
 * Rename after the file system model has been reset.
 */
void Kid3Application::renameAfterReset()
{
  disconnect(this, &Kid3Application::directoryOpened, this, &Kid3Application::renameAfterReset);
  if (!m_renameAfterResetOldName.isEmpty() &&
      !m_renameAfterResetNewName.isEmpty()) {
    Utils::safeRename(m_renameAfterResetOldName, m_renameAfterResetNewName);
    m_renameAfterResetOldName.clear();
    m_renameAfterResetNewName.clear();
  }
}

/**
 * Set the directory name from the tags.
 * The directory must not have modified files.
 * renameActionsScheduled() is emitted when the rename actions have been
 * scheduled. Then performRenameActions() has to be called to effectively
 * rename the directory.
 *
 * @param tagMask tag mask
 * @param format  directory name format
 * @param create  true to create, false to rename
 *
 * @return true if ok.
 */
bool Kid3Application::renameDirectory(Frame::TagVersion tagMask,
                                     const QString& format, bool create)
{
  TaggedFile* taggedFile =
    TaggedFileOfDirectoryIterator::first(currentOrRootIndex());
  if (!isModified() && taggedFile) {
    m_dirRenamer->setTagVersion(tagMask);
    m_dirRenamer->setFormat(format);
    m_dirRenamer->setAction(create);
    scheduleRenameActions();
    return true;
  }
  return false;
}

/**
 * Check modification state.
 *
 * @return true if a file is modified.
 */
bool Kid3Application::isModified() const
{
  return m_fileProxyModel->isModified();
}

/**
 * Number tracks in selected files of directory.
 *
 * @param nr start number
 * @param total total number of tracks, used if >0
 * @param tagVersion determines on which tags the numbers are set
 * @param options options for numbering operation
 */
void Kid3Application::numberTracks(int nr, int total,
                                   Frame::TagVersion tagVersion,
                                   NumberTrackOptions options)
{
  QString lastDirName;
  bool totalEnabled = TagConfig::instance().enableTotalNumberOfTracks();
  bool directoryMode = true;
  int startNr = nr;
  emit fileSelectionUpdateRequested();
  int numDigits = TagConfig::instance().trackNumberDigits();
  if (numDigits < 1 || numDigits > 5)
    numDigits = 1;

  // If directories are selected, number their files, else process the selected
  // files of the current directory.
  AbstractTaggedFileIterator* it =
      new TaggedFileOfSelectedDirectoriesIterator(getFileSelectionModel());
  if (!it->hasNext()) {
    delete it;
    it = new SelectedTaggedFileOfDirectoryIterator(
               currentOrRootIndex(),
               getFileSelectionModel(),
               true);
    directoryMode = false;
  }
  while (it->hasNext()) {
    TaggedFile* taggedFile = it->next();
    taggedFile->readTags(false);
    if (options & NumberTracksResetCounterForEachDirectory) {
      QString dirName = taggedFile->getDirname();
      if (lastDirName != dirName) {
        nr = startNr;
        if (totalEnabled && directoryMode) {
          total = taggedFile->getTotalNumberOfTracksInDir();
        }
        lastDirName = dirName;
      }
    }
    FOR_TAGS_IN_MASK(tagNr, tagVersion) {
      if (tagNr == Frame::Tag_Id3v1) {
        if (options & NumberTracksEnabled) {
          QString value;
          value.setNum(nr);
          Frame frame;
          if (taggedFile->getFrame(tagNr, Frame::FT_Track, frame)) {
            frame.setValueIfChanged(value);
            if (frame.isValueChanged()) {
              taggedFile->setFrame(tagNr, frame);
            }
          } else {
            frame.setValue(value);
            frame.setExtendedType(Frame::ExtendedType(Frame::FT_Track));
            taggedFile->setFrame(tagNr, frame);
          }
        }
      } else {
        // For tag 2 the frame is written, so that we have control over the
        // format and the total number of tracks, and it is possible to change
        // the format even if the numbers stay the same.
        FrameCollection frames;
        taggedFile->getAllFrames(tagNr, frames);
        Frame frame(Frame::FT_Track, QLatin1String(""), QLatin1String(""), -1);
        auto frameIt = frames.find(frame);
        QString value;
        if (options & NumberTracksEnabled) {
          if (total > 0) {
            value.sprintf("%0*d/%0*d", numDigits, nr, numDigits, total);
          } else {
            value.sprintf("%0*d", numDigits, nr);
          }
          if (frameIt != frames.end()) {
            frame = *frameIt;
            frame.setValueIfChanged(value);
            if (frame.isValueChanged()) {
              taggedFile->setFrame(tagNr, frame);
            }
          } else {
            frame.setValue(value);
            frame.setExtendedType(Frame::ExtendedType(Frame::FT_Track));
            taggedFile->setFrame(tagNr, frame);
          }
        } else {
          // If track numbering is not enabled, just reformat the current value.
          if (frameIt != frames.end()) {
            frame = *frameIt;
            int currentTotal;
            int currentNr = TaggedFile::splitNumberAndTotal(frame.getValue(),
                                                            &currentTotal);
            // Set the total if enabled.
            if (totalEnabled && total > 0) {
              currentTotal = total;
            }
            if (currentTotal > 0) {
              value.sprintf("%0*d/%0*d", numDigits, currentNr, numDigits,
                            currentTotal);
            } else {
              value.sprintf("%0*d", numDigits, currentNr);
            }
            frame.setValueIfChanged(value);
            if (frame.isValueChanged()) {
              taggedFile->setFrame(tagNr, frame);
            }
          }
        }
      }
    }
    ++nr;
  }
  emit selectedFilesUpdated();
  delete it;
}

/**
 * Play audio file.
 */
void Kid3Application::playAudio()
{
  QObject* player = getAudioPlayer();
  if (!player)
    return;

  QStringList files;
  int fileNr = 0;
  QModelIndexList selectedRows = m_fileSelectionModel->selectedRows();
  if (selectedRows.size() > 1) {
    // play only the selected files if more than one is selected
    SelectedTaggedFileIterator it(m_fileProxyModelRootIndex,
                                  m_fileSelectionModel,
                                  false);
    while (it.hasNext()) {
      files.append(it.next()->getAbsFilename());
    }
  } else {
    if (selectedRows.size() == 1) {
      // If a playlist file is selected, play the files in the playlist.
      QModelIndex index = selectedRows.first();
      index = index.sibling(index.row(), 0);
      QString path = m_fileProxyModel->filePath(index);
      bool isPlaylist = false;
      PlaylistConfig::formatFromFileExtension(path, &isPlaylist);
      if (isPlaylist) {
        files = playlistModel(path)->pathsInPlaylist();
      }
    }
    if (files.isEmpty()) {
      // play all files if none or only one is selected
      int idx = 0;
      ModelIterator it(m_fileProxyModelRootIndex);
      while (it.hasNext()) {
        QModelIndex index = it.next();
        if (TaggedFile* taggedFile = FileProxyModel::getTaggedFileOfIndex(index)) {
          files.append(taggedFile->getAbsFilename());
          if (m_fileSelectionModel->isSelected(index)) {
            fileNr = idx;
          }
          ++idx;
        }
      }
    }
  }
  emit aboutToPlayAudio();
  QMetaObject::invokeMethod(player, "setFiles",
                            Q_ARG(QStringList, files), Q_ARG(int, fileNr));
}

/**
 * Show play tool bar.
 */
void Kid3Application::showAudioPlayer()
{
  emit aboutToPlayAudio();
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
 * Create a filter string for the file dialog.
 * The filter string contains entries for all supported types.
 *
 * @return filter string.
 */
QString Kid3Application::createFilterString() const
{
  return m_platformTools->fileDialogNameFilter(
        FileProxyModel::createNameFilters());
}

/**
 * Remove the file filter if necessary to open the files.
 * @param filePaths paths to files or directories
 */
void Kid3Application::resetFileFilterIfNotMatching(const QStringList& filePaths)
{
  QStringList nameFilters(m_platformTools->getNameFilterPatterns(
              FileConfig::instance().nameFilter()).split(QLatin1Char(' ')));
  if (!nameFilters.isEmpty() && nameFilters.first() != QLatin1String("*")) {
    for (const QString& filePath : filePaths) {
      if (!QDir::match(nameFilters, filePath) &&
          !QFileInfo(filePath).isDir()) {
        setAllFilesFileFilter();
        break;
      }
    }
  }
}

/**
 * Set file name filter for "All Files (*)".
 */
void Kid3Application::setAllFilesFileFilter() {
  FileConfig::instance().setNameFilter(
        m_platformTools->fileDialogNameFilter(
          QList<QPair<QString, QString> >()
          << qMakePair(tr("All Files"), QString(QLatin1Char('*')))));
}

/**
 * Notify the tagged file factories about the changed configuration.
 */
void Kid3Application::notifyConfigurationChange()
{
  const auto factories = FileProxyModel::taggedFileFactories();
  for (ITaggedFileFactory* factory : factories) {
    const auto keys = factory->taggedFileKeys();
    for (const QString& key : keys) {
      factory->notifyConfigurationChange(key);
    }
  }
}

/**
 * Convert ID3v2.3 to ID3v2.4 tags.
 */
void Kid3Application::convertToId3v24()
{
  emit fileSelectionUpdateRequested();
  SelectedTaggedFileIterator it(getRootIndex(),
                                getFileSelectionModel(),
                                false);
  while (it.hasNext()) {
    TaggedFile* taggedFile = it.next();
    taggedFile->readTags(false);
    if (taggedFile->hasTag(Frame::Tag_Id3v2) && !taggedFile->isChanged()) {
      QString tagFmt = taggedFile->getTagFormat(Frame::Tag_Id3v2);
      if (tagFmt.length() >= 7 && tagFmt.startsWith(QLatin1String("ID3v2.")) &&
          tagFmt[6] < QLatin1Char('4')) {
        if ((taggedFile->taggedFileFeatures() &
             (TaggedFile::TF_ID3v23 | TaggedFile::TF_ID3v24)) ==
              TaggedFile::TF_ID3v23) {
          FrameCollection frames;
          taggedFile->getAllFrames(Frame::Tag_Id3v2, frames);
          FrameFilter flt;
          flt.enableAll();
          taggedFile->deleteFrames(Frame::Tag_Id3v2, flt);

          // The file has to be reread to write ID3v2.4 tags
          taggedFile = FileProxyModel::readWithId3V24(taggedFile);

          // Restore the frames
          FrameFilter frameFlt;
          frameFlt.enableAll();
          taggedFile->setFrames(Frame::Tag_Id3v2,
                                frames.copyEnabledFrames(frameFlt), false);
        }

        // Write the file with ID3v2.4 tags
        bool renamed;
        int storedFeatures = taggedFile->activeTaggedFileFeatures();
        taggedFile->setActiveTaggedFileFeatures(TaggedFile::TF_ID3v24);
        taggedFile->writeTags(true, &renamed,
                              FileConfig::instance().preserveTime());
        taggedFile->setActiveTaggedFileFeatures(storedFeatures);
        taggedFile->readTags(true);
      }
    }
  }
  emit selectedFilesUpdated();
}

/**
 * Convert ID3v2.4 to ID3v2.3 tags.
 */
void Kid3Application::convertToId3v23()
{
  emit fileSelectionUpdateRequested();
  SelectedTaggedFileIterator it(getRootIndex(),
                                getFileSelectionModel(),
                                false);
  while (it.hasNext()) {
    TaggedFile* taggedFile = it.next();
    taggedFile->readTags(false);
    if (taggedFile->hasTag(Frame::Tag_Id3v2) && !taggedFile->isChanged()) {
      QString tagFmt = taggedFile->getTagFormat(Frame::Tag_Id3v2);
      QString ext = taggedFile->getFileExtension();
      if (tagFmt.length() >= 7 && tagFmt.startsWith(QLatin1String("ID3v2.")) &&
          tagFmt[6] > QLatin1Char('3') &&
          (ext == QLatin1String(".mp3") || ext == QLatin1String(".mp2") ||
           ext == QLatin1String(".aac") || ext == QLatin1String(".wav") ||
           ext == QLatin1String(".dsf"))) {
        if (!(taggedFile->taggedFileFeatures() & TaggedFile::TF_ID3v23)) {
          FrameCollection frames;
          taggedFile->getAllFrames(Frame::Tag_Id3v2, frames);
          FrameFilter flt;
          flt.enableAll();
          taggedFile->deleteFrames(Frame::Tag_Id3v2, flt);

          // The file has to be reread to write ID3v2.3 tags
          taggedFile = FileProxyModel::readWithId3V23(taggedFile);

          // Restore the frames
          FrameFilter frameFlt;
          frameFlt.enableAll();
          taggedFile->setFrames(Frame::Tag_Id3v2,
                                frames.copyEnabledFrames(frameFlt), false);
        }

        // Write the file with ID3v2.3 tags
        bool renamed;
        int storedFeatures = taggedFile->activeTaggedFileFeatures();
        taggedFile->setActiveTaggedFileFeatures(TaggedFile::TF_ID3v23);
        taggedFile->writeTags(true, &renamed,
                              FileConfig::instance().preserveTime());
        taggedFile->setActiveTaggedFileFeatures(storedFeatures);
        taggedFile->readTags(true);
      }
    }
  }
  emit selectedFilesUpdated();
}

/**
 * Get value of frame.
 * To get binary data like a picture, the name of a file to write can be
 * added after the @a name, e.g. "Picture:/path/to/file".
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 * @param name    name of frame (e.g. "Artist")
 */
QString Kid3Application::getFrame(Frame::TagVersion tagMask,
                                  const QString& name) const
{
  QString frameName(name);
  QString dataFileName, fieldName;
  int index = 0;
  Frame::ExtendedType explicitType;
  if (frameName.startsWith(QLatin1Char('!'))) {
    frameName.remove(0, 1);
    explicitType = Frame::ExtendedType(Frame::FT_Other, frameName);
  }
  extractFileFieldIndex(frameName, dataFileName, fieldName, index);
  Frame::TagNumber tagNr = Frame::tagNumberFromMask(tagMask);
  if (tagNr >= Frame::Tag_NumValues)
    return QString();

  FrameTableModel* ft = m_framesModel[tagNr];
  const FrameCollection& frames = ft->frames();
  auto it = explicitType.getType() == Frame::FT_UnknownFrame
      ? frames.findByName(frameName, index)
      : frames.findByExtendedType(explicitType, index);
  if (it != frames.cend()) {
    if (!dataFileName.isEmpty()) {
      bool isSylt = it->getInternalName().startsWith(QLatin1String("SYLT"));
      if (isSylt ||
          it->getInternalName().startsWith(QLatin1String("ETCO"))) {
        QFile file(dataFileName);
        if (file.open(QIODevice::WriteOnly)) {
          TimeEventModel timeEventModel;
          if (isSylt) {
            timeEventModel.setType(TimeEventModel::SynchronizedLyrics);
            timeEventModel.fromSyltFrame(it->getFieldList());
          } else {
            timeEventModel.setType(TimeEventModel::EventTimingCodes);
            timeEventModel.fromEtcoFrame(it->getFieldList());
          }
          QTextStream stream(&file);
          QString codecName = FileConfig::instance().textEncoding();
          if (codecName != QLatin1String("System")) {
            stream.setCodec(codecName.toLatin1());
          }
          timeEventModel.toLrcFile(stream, frames.getTitle(),
                                   frames.getArtist(), frames.getAlbum());
          file.close();
        }
      } else {
        PictureFrame::writeDataToFile(*it, dataFileName);
      }
    }
    if (!fieldName.isEmpty()) {
      if (fieldName == QLatin1String("selected")) {
        const int frameIndex = it->getIndex();
        const int row = frameIndex >= 0
            ? ft->getRowWithFrameIndex(frameIndex)
            : std::distance(frames.cbegin(), it);
        if (row != -1) {
          return QLatin1String(ft->index(row, FrameTableModel::CI_Enable)
                               .data(Qt::CheckStateRole).toInt() == Qt::Checked
                               ? "1" : "0");
        }
        return QString();
      } else {
        return Frame::getField(*it, fieldName).toString();
      }
    }
    return it->getValue();
  } else {
    return QString();
  }
}

/**
 * Get names and values of all frames.
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 *
 * @return map containing frame values.
 */
QVariantMap Kid3Application::getAllFrames(Frame::TagVersion tagMask) const
{
  QVariantMap map;
  Frame::TagNumber tagNr = Frame::tagNumberFromMask(tagMask);
  if (tagNr >= Frame::Tag_NumValues)
    return QVariantMap();

  FrameTableModel* ft = m_framesModel[tagNr];
  const FrameCollection& frames = ft->frames();
  for (auto it = frames.cbegin(); it != frames.cend(); ++it) {
    QString name(it->getName());
    int nlPos = name.indexOf(QLatin1Char('\n'));
    if (nlPos > 0) {
      // probably "TXXX - User defined text information\nDescription" or
      // "WXXX - User defined URL link\nDescription"
      name = name.mid(nlPos + 1);
    } else if (name.midRef(4, 3) == QLatin1String(" - ")) {
      // probably "ID3-ID - Description"
      name = name.left(4);
    }
    map.insert(name, it->getValue());
  }
  return map;
}

/**
 * Set value of frame.
 * For tag 2 (@a tagMask 2), if no frame with @a name exists, a new frame
 * is added, if @a value is empty, the frame is deleted.
 * To add binary data like a picture, a file can be added after the
 * @a name, e.g. "Picture:/path/to/file".
 *
 * @param tagMask tag bit (1 for tag 1, 2 for tag 2)
 * @param name    name of frame (e.g. "Artist")
 * @param value   value of frame
 *
 * @return true if ok.
 */
bool Kid3Application::setFrame(Frame::TagVersion tagMask,
                               const QString& name, const QString& value)
{
  Frame::TagNumber tagNr = Frame::tagNumberFromMask(tagMask);
  if (tagNr >= Frame::Tag_NumValues)
    return false;

  FrameTableModel* ft = m_framesModel[tagNr];
  if (name == QLatin1String("*.selected")) {
    ft->setAllCheckStates(!value.isEmpty() && value != QLatin1String("0")
                                           && value != QLatin1String("false"));
    return true;
  }

  QString frameName(name);
  QString dataFileName, fieldName;
  int index = 0;
  Frame::ExtendedType explicitType;
  if (frameName.startsWith(QLatin1Char('!'))) {
    frameName.remove(0, 1);
    explicitType = Frame::ExtendedType(Frame::FT_Other, frameName);
  }
  extractFileFieldIndex(frameName, dataFileName, fieldName, index);
  FrameCollection frames(ft->frames());
  auto it = explicitType.getType() == Frame::FT_UnknownFrame
      ? frames.findByName(frameName, index)
      : frames.findByExtendedType(explicitType, index);
  if (it != frames.end()) {
    QString frmName(it->getName());
    bool isPicture, isGeob, isSylt = false;
    if (!dataFileName.isEmpty() &&
        (tagMask & (Frame::TagV2 | Frame::TagV3)) != 0 &&
        ((isPicture = (it->getType() == Frame::FT_Picture)) ||
         (isGeob = frmName.startsWith(QLatin1String("GEOB"))) ||
         (isSylt = frmName.startsWith(QLatin1String("SYLT"))) ||
         frmName.startsWith(QLatin1String("ETCO")))) {
      if (isPicture) {
        deleteFrame(tagNr, frmName, index);
        PictureFrame frame;
        PictureFrame::setDescription(frame, value);
        PictureFrame::setDataFromFile(frame, dataFileName);
        PictureFrame::setMimeTypeFromFileName(frame, dataFileName);
        PictureFrame::setTextEncoding(frame, frameTextEncodingFromConfig());
        addFrame(tagNr, &frame);
      } else if (isGeob) {
        Frame frame(*it);
        deleteFrame(tagNr, frmName, index);
        Frame::setField(frame, Frame::ID_MimeType,
                        PictureFrame::getMimeTypeForFile(dataFileName));
        Frame::setField(frame, Frame::ID_Filename,
                        QFileInfo(dataFileName).fileName());
        Frame::setField(frame, Frame::ID_Description, value);
        PictureFrame::setDataFromFile(frame, dataFileName);
        addFrame(tagNr, &frame);
      } else {
        QFile file(dataFileName);
        if (file.open(QIODevice::ReadOnly)) {
          QTextStream stream(&file);
          Frame frame(*it);
          Frame::setField(frame, Frame::ID_Description, value);
          deleteFrame(tagNr, frmName, index);
          TimeEventModel timeEventModel;
          if (isSylt) {
            timeEventModel.setType(TimeEventModel::SynchronizedLyrics);
            timeEventModel.fromLrcFile(stream);
            timeEventModel.toSyltFrame(frame.fieldList());
          } else {
            timeEventModel.setType(TimeEventModel::EventTimingCodes);
            timeEventModel.fromLrcFile(stream);
            timeEventModel.toEtcoFrame(frame.fieldList());
          }
          file.close();
          addFrame(tagNr, &frame);
        }
      }
    } else if (value.isEmpty() && fieldName.isEmpty() &&
               (tagMask & (Frame::TagV2 | Frame::TagV3)) != 0) {
      deleteFrame(tagNr, frmName, index);
    } else {
      auto& frame = const_cast<Frame&>(*it);
      if (fieldName.isEmpty()) {
        frame.setValueIfChanged(value);
      } else {
        if (fieldName == QLatin1String("selected")) {
          const int frameIndex = frame.getIndex();
          const int row = frameIndex >= 0
              ? ft->getRowWithFrameIndex(frameIndex)
              : std::distance(frames.cbegin(), it);
          if (row != -1) {
            ft->setData(ft->index(row, FrameTableModel::CI_Enable),
                        !value.isEmpty() && value != QLatin1String("0")
                                         && value != QLatin1String("false")
                        ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
            return true;
          }
        } else {
          TaggedFile* taggedFile = getSelectedFile();
          if (taggedFile && Frame::setField(frame, fieldName, value)) {
            taggedFile->setFrame(tagNr, frame);
          }
        }
      }
      ft->transferFrames(frames);
      ft->selectChangedFrames();
      emit fileSelectionUpdateRequested();
      emit selectedFilesUpdated();
    }
    return true;
  } else if (tagMask & (Frame::TagV2 | Frame::TagV3)) {
    Frame frame(explicitType.getType() == Frame::FT_UnknownFrame
                ? Frame::ExtendedType(frameName) : explicitType, value, -1);
    QString frmName(frame.getInternalName());
    bool isPicture, isGeob, isSylt = false;
    if (!dataFileName.isEmpty() &&
        ((isPicture = (frame.getType() == Frame::FT_Picture)) ||
         (isGeob = frmName.startsWith(QLatin1String("GEOB"))) ||
         (isSylt = frmName.startsWith(QLatin1String("SYLT"))) ||
         frmName.startsWith(QLatin1String("ETCO")))) {
      if (isPicture) {
        PictureFrame::setFields(frame);
        PictureFrame::setDescription(frame, value);
        PictureFrame::setDataFromFile(frame, dataFileName);
        PictureFrame::setMimeTypeFromFileName(frame, dataFileName);
        PictureFrame::setTextEncoding(frame, frameTextEncodingFromConfig());
      } else if (isGeob) {
        PictureFrame::setGeobFields(
              frame, Frame::TE_ISO8859_1,
              PictureFrame::getMimeTypeForFile(dataFileName),
              QFileInfo(dataFileName).fileName(), value);
        PictureFrame::setDataFromFile(frame, dataFileName);
      } else {
        QFile file(dataFileName);
        if (file.open(QIODevice::ReadOnly)) {
          Frame::Field field;
          Frame::FieldList& fields = frame.fieldList();
          fields.clear();
          field.m_id = Frame::ID_Description;
          field.m_value = value;
          fields.append(field);
          field.m_id = Frame::ID_Data;
          field.m_value = QVariant(QVariant::List);
          fields.append(field);
          QTextStream stream(&file);
          TimeEventModel timeEventModel;
          if (isSylt) {
            timeEventModel.setType(TimeEventModel::SynchronizedLyrics);
            timeEventModel.fromLrcFile(stream);
            timeEventModel.toSyltFrame(frame.fieldList());
          } else {
            timeEventModel.setType(TimeEventModel::EventTimingCodes);
            timeEventModel.fromLrcFile(stream);
            timeEventModel.toEtcoFrame(frame.fieldList());
          }
          file.close();
        }
      }
    } else if (value.isEmpty()) {
      // Do not add an empty frame
      return false;
    }
    if (!fieldName.isEmpty()) {
      if (TaggedFile* taggedFile = getSelectedFile()) {
        frame.setValue(QString());
        taggedFile->addFieldList(tagNr, frame);
        if (!Frame::setField(frame, fieldName, value)) {
          return false;
        }
      }
    }
    addFrame(tagNr, &frame);
    return true;
  }
  return false;
}

/**
 * Get data from picture frame.
 * @return picture data, empty if not found.
 */
QByteArray Kid3Application::getPictureData() const
{
  QByteArray data;
  const FrameCollection& frames = m_framesModel[Frame::Tag_Picture]->frames();
  auto it = frames.findByExtendedType(
        Frame::ExtendedType(Frame::FT_Picture));
  if (it != frames.cend()) {
    PictureFrame::getData(*it, data);
  }
  return data;
}

/**
 * Set data in picture frame.
 * @param data picture data
 */
void Kid3Application::setPictureData(const QByteArray& data)
{
  const FrameCollection& frames = m_framesModel[Frame::Tag_Picture]->frames();
  auto it = frames.findByExtendedType(
        Frame::ExtendedType(Frame::FT_Picture));
  PictureFrame frame;
  if (it != frames.cend()) {
    frame = PictureFrame(*it);
    deleteFrame(Frame::Tag_Picture, QLatin1String("Picture"));
  }
  if (!data.isEmpty()) {
    PictureFrame::setData(frame, data);
    PictureFrame::setTextEncoding(frame, frameTextEncodingFromConfig());
    addFrame(Frame::Tag_Picture, &frame);
  }
}

/**
 * Close the file handle of a tagged file.
 * @param filePath path to file
 */
void Kid3Application::closeFileHandle(const QString& filePath)
{
 QModelIndex index = m_fileProxyModel->index(filePath);
 if (index.isValid()) {
   if (TaggedFile* taggedFile = FileProxyModel::getTaggedFileOfIndex(index)) {
     taggedFile->closeFileHandle();
   }
 }
}

/**
 * Set a frame editor object to act as the frame editor.
 * @param frameEditor frame editor object, null to disable
 */
void Kid3Application::setFrameEditor(FrameEditorObject* frameEditor)
{
  if (m_frameEditor != frameEditor) {
    IFrameEditor* editor;
    bool storeCurrentEditor = false;
    if (frameEditor) {
      if (!m_frameEditor) {
        storeCurrentEditor = true;
      }
      editor = frameEditor;
    } else {
      editor = m_storedFrameEditor;
    }
    FOR_ALL_TAGS(tagNr) {
      if (tagNr != Frame::Tag_Id3v1) {
        FrameList* framelist = m_framelist[tagNr];
        if (storeCurrentEditor) {
          m_storedFrameEditor = framelist->frameEditor();
          storeCurrentEditor = false;
        }
        framelist->setFrameEditor(editor);
      }
    }
    m_frameEditor = frameEditor;
    emit frameEditorChanged();
  }
}

/**
 * Remove frame editor.
 * Has to be called in the destructor of the frame editor to avoid a dangling
 * pointer to a deleted object.
 * @param frameEditor frame editor
 */
void Kid3Application::removeFrameEditor(IFrameEditor* frameEditor)
{
  if (m_storedFrameEditor == frameEditor) {
    m_storedFrameEditor = nullptr;
  }
  if (m_frameEditor == frameEditor) {
    setFrameEditor(nullptr);
  }
}

/**
 * Get the numbers of the selected rows in a list suitable for scripting.
 * @return list with row numbers.
 */
QVariantList Kid3Application::getFileSelectionRows()
{
  QVariantList rows;
  const auto indexes = m_fileSelectionModel->selectedRows();
  rows.reserve(indexes.size());
  for (const QModelIndex& index : indexes) {
    rows.append(index.row());
  }
  return rows;
}

/**
 * Set the file selection from a list of model indexes.
 * @param indexes list of model indexes suitable for scripting
 */
void Kid3Application::setFileSelectionIndexes(const QVariantList& indexes)
{
  QItemSelection selection;
  QModelIndex firstIndex;
  for (const QVariant& var : indexes) {
    QModelIndex index = var.toModelIndex();
    if (!firstIndex.isValid()) {
      firstIndex = index;
    }
    selection.select(index, index);
  }
  disconnect(m_fileSelectionModel,
             &QItemSelectionModel::selectionChanged,
             this, &Kid3Application::fileSelectionChanged);
  m_fileSelectionModel->select(selection,
                   QItemSelectionModel::Clear | QItemSelectionModel::Select |
                   QItemSelectionModel::Rows);
  if (firstIndex.isValid()) {
    m_fileSelectionModel->setCurrentIndex(firstIndex,
        QItemSelectionModel::Select | QItemSelectionModel::Rows);
  }
  connect(m_fileSelectionModel,
          &QItemSelectionModel::selectionChanged,
          this, &Kid3Application::fileSelectionChanged);
}

/**
 * Set the image provider.
 * @param imageProvider image provider
 */
void Kid3Application::setImageProvider(ImageDataProvider* imageProvider) {
  m_imageProvider = imageProvider;
}

/**
 * If an image provider is used, update its picture and change the
 * coverArtImageId property if the picture of the selection changed.
 * This can be used to change a QML image.
 */
void Kid3Application::updateCoverArtImageId()
{
  // Only perform expensive picture operations if the signal is used
  // (when using a QML image provider).
  if (m_imageProvider &&
      receivers(SIGNAL(coverArtImageIdChanged(QString))) > 0) {
    setCoverArtImageData(m_selection->getPicture());
  }
}

/**
 * Set picture data for image provider.
 * @param picture picture data
 */
void Kid3Application::setCoverArtImageData(const QByteArray& picture)
{
  if (picture != m_imageProvider->getImageData()) {
    m_imageProvider->setImageData(picture);
    setNextCoverArtImageId();
    emit coverArtImageIdChanged(m_coverArtImageId);
  }
}

/**
 * Set the coverArtImageId property to a new value.
 * This can be used to trigger an update of QML images.
 */
void Kid3Application::setNextCoverArtImageId() {
  static quint32 nr = 0;
  m_coverArtImageId = QString(QLatin1String("image://kid3/data/%1"))
      .arg(nr++, 8, 16, QLatin1Char('0'));
}

/**
 * Open a file select dialog to get a file name.
 * For script support, is only supported when a GUI is available.
 * @param caption dialog caption
 * @param dir working directory
 * @param filter file type filter
 * @param saveFile true to open a save file dialog
 * @return selected file, empty if canceled.
 */
QString Kid3Application::selectFileName(const QString& caption, const QString& dir,
                                        const QString& filter, bool saveFile)
{
  return saveFile
      ? m_platformTools->getSaveFileName(nullptr, caption, dir, filter, nullptr)
      : m_platformTools->getOpenFileName(nullptr, caption, dir, filter, nullptr);
}

/**
 * Open a file select dialog to get a directory name.
 * For script support, is only supported when a GUI is available.
 * @param caption dialog caption
 * @param dir working directory
 * @return selected directory, empty if canceled.
 */
QString Kid3Application::selectDirName(const QString& caption,
                                       const QString& dir)
{
  return m_platformTools->getExistingDirectory(nullptr, caption, dir);
}
