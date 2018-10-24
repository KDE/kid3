/**
 * \file useractionsconfig.cpp
 * User actions configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 29 Jun 2013
 *
 * Copyright (C) 2013-2018  Urs Fleisch
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

#include "useractionsconfig.h"
#include <QStringList>
#include <cstdlib>
#include "isettings.h"
#include "config.h"

int UserActionsConfig::s_index = -1;

/**
 * Constructor.
 */
UserActionsConfig::UserActionsConfig()
  : StoredConfig<UserActionsConfig>(QLatin1String("MenuCommands"))
{
}

/**
 * Persist configuration.
 *
 * @param config configuration
 */
void UserActionsConfig::writeToConfig(ISettings* config) const
{
  config->beginGroup(m_group);
  int cmdNr = 1;
  for (auto it = m_contextMenuCommands.constBegin();
       it != m_contextMenuCommands.constEnd();
       ++it) {
    config->setValue(QString(QLatin1String("Command%1")).arg(cmdNr++),
                     QVariant((*it).toStringList()));
  }
  // delete entries which are no longer used
  for (;;) {
    QStringList strList =
        config->value(QString(QLatin1String("Command%1")).arg(cmdNr),
                      QStringList()).toStringList();
    if (strList.empty()) {
      break;
    }
    config->remove(QString(QLatin1String("Command%1")).arg(cmdNr));
    ++cmdNr;
  }
  config->endGroup();
}

/**
 * Read persisted configuration.
 *
 * @param config configuration
 */
void UserActionsConfig::readFromConfig(ISettings* config)
{
  config->beginGroup(m_group);
  m_contextMenuCommands.clear();
  int cmdNr = 1;
  for (;;) {
    QStringList strList =
        config->value(QString(QLatin1String("Command%1")).arg(cmdNr),
                      QStringList()).toStringList();
    if (strList.empty()) {
      break;
    }
    if (strList.size() > 1 &&
        strList.at(1) == QLatin1String(
          "%{browser} http://images.google.com/images?q=%u{artist}%20%u{album}")) {
      strList[1] = QLatin1String(
        "%{browser} http://www.google.com/search?tbm=isch&q=%u{artist}%20%u{album}");
    }
    m_contextMenuCommands.push_back(UserActionsConfig::MenuCommand(strList));
    ++cmdNr;
  }
  config->endGroup();

  setDefaultUserActions(cmdNr != 1);
}

/**
 * Set default user actions.
 * @param upgradeOnly if true only upgrade configuration with new actions
 */
void UserActionsConfig::setDefaultUserActions(bool upgradeOnly)
{
  if (!upgradeOnly) {
    m_contextMenuCommands.clear();
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(QString(), QLatin1String("@separator")));
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(QLatin1String("Album Art"),
                                     QLatin1String("@beginmenu")));
  }
#ifdef HAVE_QML
  if (!upgradeOnly || ConfigStore::getConfigVersion() < 2) {
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(
            QLatin1String("Resize Album Art"),
            QLatin1String("@qml %{qmlpath}/script/ResizeAlbumArt.qml"),
            false, true));
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(
            QLatin1String("Extract Album Art"),
            QLatin1String("@qml %{qmlpath}/script/ExtractAlbumArt.qml"),
            false, true));
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(
            QLatin1String("Embed Album Art"),
            QLatin1String("@qml %{qmlpath}/script/EmbedAlbumArt.qml"),
            false, true));
  }
#endif
  if (!upgradeOnly) {
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(
            QLatin1String("Google Images"),
            QLatin1String("%{browser} http://www.google.com/"
                          "search?tbm=isch&q=%u{artist}%20%u{album}")));
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(
            QLatin1String("Amazon"),
            QLatin1String("%{browser} http://www.amazon.com/s?search-alias=aps&"
                          "field-keywords=%u{artist}+%u{album}")));
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(QString(), QLatin1String("@endmenu")));
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(QLatin1String("Lyrics"),
                                     QLatin1String("@beginmenu")));
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(
            QLatin1String("LyricWiki"),
            QLatin1String("%{browser} http://lyricwiki.org/%u{artist}:%u{title}")));
  }
#ifdef HAVE_QML
  if (!upgradeOnly || ConfigStore::getConfigVersion() < 2) {
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(
            QLatin1String("Embed Lyrics"),
            QLatin1String("@qml %{qmlpath}/script/EmbedLyrics.qml"),
            false, true));
  }
#endif
  if (!upgradeOnly) {
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(
            QLatin1String("Lyrics.com"),
            QLatin1String("%{browser} http://www.lyrics.com/lyrics/"
                          "%u{artist}+%u{title}")));
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(
            QLatin1String("AZLyrics"),
            QLatin1String("%{browser} http://search.azlyrics.com/"
                          "search.php?q=%u{artist}+%u{title}")));
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(
            QLatin1String("Dark Lyrics"),
            QLatin1String("%{browser} http://www.darklyrics.com/"
                          "search?q=%u{album}")));
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(
            QLatin1String("SongLyrics"),
            QLatin1String("%{browser} http://www.songlyrics.com/index.php"
                          "?section=search&searchW=%u{artist}+%u{title}")));
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(
            QLatin1String("LyricsMode"),
            QLatin1String("%{browser} http://www.lyricsmode.com/search.php"
                          "?search=%u{artist}+%u{title}")));
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(
            QLatin1String("MP3 Lyrics"),
            QLatin1String("%{browser} http://mp3lyrics.com/Search/Advanced/"
                          "?Track=%u{title}&Artist=%u{artist}")));
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(QString(), QLatin1String("@endmenu")));
  }
#ifdef HAVE_QML
  if (!upgradeOnly || ConfigStore::getConfigVersion() < 2) {
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(
            QLatin1String("QML Console"),
            QLatin1String("@qmlview %{qmlpath}/script/QmlConsole.qml"),
            false, true));
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(
            QLatin1String("ReplayGain to SoundCheck"),
            QLatin1String("@qml %{qmlpath}/script/ReplayGain2SoundCheck.qml"),
            false, true));
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(
            QLatin1String("Text Encoding ID3v1"),
            QLatin1String("@qml %{qmlpath}/script/ShowTextEncodingV1.qml"),
            false, true));
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(
            QLatin1String("Export CSV"),
            QLatin1String("@qml %{qmlpath}/script/ExportCsv.qml"),
            false, true));
  } else if (upgradeOnly && ConfigStore::getConfigVersion() == 2) {
    // Remove default argument from "Export CSV", a file selector is now used.
    int exportCsvIdx = m_contextMenuCommands.indexOf(
          UserActionsConfig::MenuCommand(
            QLatin1String("Export CSV"),
            QLatin1String("@qml %{qmlpath}/script/ExportCsv.qml "
                          "%{directory}/export.csv"), false, true));
    if (exportCsvIdx != -1) {
      m_contextMenuCommands[exportCsvIdx].setCommand(
            QLatin1String("@qml %{qmlpath}/script/ExportCsv.qml"));
    }
  }
  if (!upgradeOnly || ConfigStore::getConfigVersion() < 3) {
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(
            QLatin1String("Import CSV"),
            QLatin1String("@qml %{qmlpath}/script/ImportCsv.qml"),
            false, true));
  }
  if (!upgradeOnly || ConfigStore::getConfigVersion() < 4) {
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(
            QLatin1String("Export Playlist Folder"),
            QLatin1String("@qml %{qmlpath}/script/ExportPlaylist.qml"),
            false, true));
  }
#endif
}

void UserActionsConfig::setContextMenuCommands(
    const QList<MenuCommand>& contextMenuCommands)
{
  if (m_contextMenuCommands != contextMenuCommands) {
    m_contextMenuCommands = contextMenuCommands;
    emit contextMenuCommandsChanged();
  }
}

QVariantList UserActionsConfig::contextMenuCommandVariantList() const
{
  QVariantList lst;
  for (auto it = m_contextMenuCommands.constBegin();
       it != m_contextMenuCommands.constEnd();
       ++it) {
    lst.append(it->toStringList());
  }
  return lst;
}

void UserActionsConfig::setContextMenuCommandVariantList(const QVariantList& lst)
{
  QList<MenuCommand> commands;
  for (auto it = lst.constBegin(); it != lst.constEnd(); ++it) {
    commands.append(MenuCommand(it->toStringList()));
  }
  setContextMenuCommands(commands);
}


/**
 * Constructor.
 *
 * @param name display name
 * @param cmd  command string with argument codes
 * @param confirm true if confirmation required
 * @param showOutput true if output of command shall be shown
 */
UserActionsConfig::MenuCommand::MenuCommand(
    const QString& name, const QString& cmd, bool confirm, bool showOutput)
  : m_name(name), m_cmd(cmd), m_confirm(confirm), m_showOutput(showOutput)
{
}

/**
 * Constructor.
 *
 * @param strList string list with encoded command
 */
UserActionsConfig::MenuCommand::MenuCommand(const QStringList& strList)
{
  if (strList.size() == 3) {
    bool ok;
    uint flags = strList[2].toUInt(&ok);
    if (ok) {
      m_confirm = (flags & 1) != 0;
      m_showOutput = (flags & 2) != 0;
      m_name = strList[0];
      m_cmd = strList[1];
    } else {
      m_confirm = false;
      m_showOutput = false;
    }
  }
}

/**
 * Encode into string list.
 *
 * @return string list with encoded command.
 */
QStringList UserActionsConfig::MenuCommand::toStringList() const {
  QStringList strList;
  strList.push_back(m_name);
  strList.push_back(m_cmd);
  uint flags = (m_confirm ? 1 : 0) | (m_showOutput ? 2 : 0);
  strList.push_back(QString::number(flags));
  return strList;
}
