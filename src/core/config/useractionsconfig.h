/**
 * \file useractionsconfig.h
 * User actions configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 29 Jun 2013
 *
 * Copyright (C) 2013-2024  Urs Fleisch
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

#pragma once

#include <QVariantList>
#include "generalconfig.h"
#include "kid3api.h"

/**
 * User actions configuration.
 */
class KID3_CORE_EXPORT UserActionsConfig : public StoredConfig<UserActionsConfig>
{
  Q_OBJECT
  /** list of context menu commands */
  Q_PROPERTY(QVariantList contextMenuCommands
             READ contextMenuCommandVariantList
             WRITE setContextMenuCommandVariantList
             NOTIFY contextMenuCommandsChanged)

public:
  /**
   * External command in context menu.
   */
  class MenuCommand {
  public:
    /**
     * Constructor.
     *
     * @param name display name
     * @param cmd  command string with argument codes
     * @param confirm true if confirmation required
     * @param showOutput true if output of command shall be shown
     */
    explicit MenuCommand(const QString& name = QString(),
                         const QString& cmd = QString(),
                         bool confirm = false, bool showOutput = false);

    /**
     * Constructor.
     *
     * @param strList string list with encoded command
     */
    explicit MenuCommand(const QStringList& strList);

    /**
     * Encode into string list.
     *
     * @return string list with encoded command.
     */
    QStringList toStringList() const;

    /**
     * Get the display name.
     * @return name.
     */
    const QString& getName() const { return m_name; }

    /**
     * Set the display name.
     * @param name display name
     */
    void setName(const QString& name) { m_name = name; }

    /**
     * Get the command string.
     * @return command string.
     */
    const QString& getCommand() const { return m_cmd; }

    /**
     * Set the command string.
     * @param cmd command string.
     */
    void setCommand(const QString& cmd) { m_cmd = cmd; }

    /**
     * Check if command must be confirmed.
     * @return true if command has to be confirmed.
     */
    bool mustBeConfirmed() const { return m_confirm; }

    /**
     * Set if command must be confirmed.
     * @param confirm true if command has to be confirmed
     */
    void setMustBeConfirmed(bool confirm) { m_confirm = confirm; }

    /**
     * Check if command output has to be shown.
     * @return true if command output has to be shown.
     */
    bool outputShown() const { return m_showOutput; }

    /**
     * Set if command output has to be shown.
     * @param showOutput true if command output has to be shown
     */
    void setOutputShown(bool showOutput) { m_showOutput = showOutput; }

    /**
     * Test for equality.
     * @param rhs right hand side
     * @return true if equal.
     */
    bool operator==(const MenuCommand& rhs) const {
      return m_name == rhs.m_name && m_cmd == rhs.m_cmd &&
          m_confirm == rhs.m_confirm && m_showOutput == rhs.m_showOutput;
    }

  private:
    QString m_name;
    QString m_cmd;
    bool m_confirm;
    bool m_showOutput;
  };

  /**
   * Constructor.
   */
  UserActionsConfig();

  /**
   * Destructor.
   */
  ~UserActionsConfig() override = default;

  /**
   * Persist configuration.
   *
   * @param config configuration
   */
  void writeToConfig(ISettings* config) const override;

  /**
   * Read persisted configuration.
   *
   * @param config configuration
   */
  void readFromConfig(ISettings* config) override;

  /** Get list of context menu commands. */
  QList<MenuCommand> contextMenuCommands() const {
    return m_contextMenuCommands;
  }

  /** Set list of context menu commands. */
  void setContextMenuCommands(const QList<MenuCommand>& contextMenuCommands);

  /** Get list of context menu commands as variant list. */
  QVariantList contextMenuCommandVariantList() const;

  /** Set list of context menu commands from variant list. */
  void setContextMenuCommandVariantList(const QVariantList& lst);

  /**
   * Set default user actions.
   * @param upgradeOnly if true only upgrade configuration with new actions
   */
  void setDefaultUserActions(bool upgradeOnly = false);

signals:
  /** Emitted when commands changed. */
  void contextMenuCommandsChanged();

private:
  friend UserActionsConfig& StoredConfig<UserActionsConfig>::instance();

  /** commands available in context menu */
  QList<MenuCommand> m_contextMenuCommands;

  /** Index in configuration storage */
  static int s_index;
};
