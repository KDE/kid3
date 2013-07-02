/**
 * \file useractionsconfig.cpp
 * User actions configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 29 Jun 2013
 *
 * Copyright (C) 2013  Urs Fleisch
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
#include <stdlib.h>

/**
 * Constructor.
 *
 * @param group configuration group
 */
UserActionsConfig::UserActionsConfig(const QString& group) :
  StoredConfig<UserActionsConfig>(group)
{
}

/**
 * Destructor.
 */
UserActionsConfig::~UserActionsConfig() {}

/**
 * Persist configuration.
 *
 * @param config configuration
 */
void UserActionsConfig::writeToConfig(ISettings* config) const
{
  config->beginGroup(m_group);
  int cmdNr = 1;
  for (QList<MenuCommand>::const_iterator
         it = m_contextMenuCommands.begin();
       it != m_contextMenuCommands.end();
       ++it) {
    config->setValue(QString(QLatin1String("Command%1")).arg(cmdNr++), QVariant((*it).toStringList()));
  }
  // delete entries which are no longer used
  for (;;) {
    QStringList strList = config->value(QString(QLatin1String("Command%1")).arg(cmdNr),
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
    QStringList strList = config->value(QString(QLatin1String("Command%1")).arg(cmdNr),
                                        QStringList()).toStringList();
    if (strList.empty()) {
      break;
    }
    m_contextMenuCommands.push_back(UserActionsConfig::MenuCommand(strList));
    ++cmdNr;
  }
  config->endGroup();

  if (cmdNr == 1) {
#ifdef Q_OS_WIN32
    QString prgDir = QString::fromLocal8Bit(::getenv("ProgramFiles"));
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(
        QLatin1String("Windows Media Player"),
        QLatin1Char('"') + prgDir + QLatin1String("\\Windows Media Player\\wmplayer.exe\" %{files}")));
#elif !defined Q_OS_MAC
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(QLatin1String("Amarok"), QLatin1String("amarok %{files}")));
#endif
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(QLatin1String("Google Images"), QLatin1String("%{browser} http://images.google.com/images?q=%u{artist}%20%u{album}")));
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(QLatin1String("Amazon"), QLatin1String("%{browser} http://www.amazon.com/s?search-alias=aps&field-keywords=%u{artist}+%u{album}")));
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(QLatin1String("LyricWiki"), QLatin1String("%{browser} http://lyricwiki.org/%u{artist}:%u{title}")));
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(QLatin1String("Lyrics.com"), QLatin1String("%{browser} http://www.lyrics.com/search.php?keyword=%u{artist}+%u{title}&what=all")));
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(QLatin1String("AZLyrics"), QLatin1String("%{browser} http://search.azlyrics.com/search.php?q=%u{artist}+%u{title}")));
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(QLatin1String("Dark Lyrics"), QLatin1String("%{browser} http://www.darklyrics.com/search?q=%u{album}")));
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(QLatin1String("Metro Lyrics"), QLatin1String("%{browser} http://www.metrolyrics.com/search.php?category=artisttitle&search=%u{artist}+%u{title}")));
    m_contextMenuCommands.push_back(
      UserActionsConfig::MenuCommand(QLatin1String("SongLyrics"), QLatin1String("%{browser} http://www.songlyrics.com/index.php?section=search&searchW=%u{artist}+%u{title}")));
  }
}


/**
 * Constructor.
 *
 * @param name display name
 * @param cmd  command string with argument codes
 * @param confirm true if confirmation required
 * @param showOutput true if output of command shall be shown
 */
UserActionsConfig::MenuCommand::MenuCommand(const QString& name, const QString& cmd,
                                     bool confirm, bool showOutput) :
  m_name(name), m_cmd(cmd), m_confirm(confirm), m_showOutput(showOutput)
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
