/**
 * \file qmlcommandplugin.h
 * Starter for QML scripts.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 15 Feb 2015
 *
 * Copyright (C) 2015  Urs Fleisch
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

#include <QObject>
#include "iusercommandprocessor.h"

class Kid3Application;
class QQuickView;
class QQmlEngine;
class QQmlError;

/**
 * Starter for QML scripts.
 */
class KID3_PLUGIN_EXPORT QmlCommandPlugin :
    public QObject, public IUserCommandProcessor {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "net.sourceforge.kid3.IUserCommandProcessor")
  Q_INTERFACES(IUserCommandProcessor)
public:
  /**
   * Constructor.
   *
   * @param parent parent object
   */
  explicit QmlCommandPlugin(QObject* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~QmlCommandPlugin() override = default;

  /**
   * Get keys of available user commands.
   * @return list of keys, ["qml", "qmlview"].
   */
  virtual QStringList userCommandKeys() const override;

  /**
   * Initialize processor.
   * This method must be invoked before the first call to startUserCommand()
   * to set the application context.
   * @param app application context
   */
  virtual void initialize(Kid3Application* app) override;

  /**
   * Cleanup processor.
   * This method must be invoked to close and delete the QML resources.
   */
  virtual void cleanup() override;

  /**
   * Start a QML script.
   * @param key user command name, "qml" or "qmlview"
   * @param arguments arguments to pass to script
   * @param showOutput true to enable output in output viewer, using signal
   *                   commandOutput().
   * @return true if command is started.
   */
  virtual bool startUserCommand(
      const QString& key, const QStringList& arguments, bool showOutput) override;

  /**
   * Return object which emits commandOutput() signal.
   * @return this.
   */
  virtual QObject* qobject() override;

signals:
  /**
   * Emitted when output is enabled and a QML message is generated.
   * @param msg message from QML, error or console output
   */
  void commandOutput(const QString& msg);

private slots:
  void onEngineError(const QList<QQmlError>& errors);
  void onQmlViewClosing();
  void onQmlViewFinished();
  void onQmlEngineQuit();
  void onEngineFinished();

private:
  void setupQmlEngine(QQmlEngine* engine);
  void onEngineReady();

  static void messageHandler(QtMsgType type, const QMessageLogContext& context,
                             const QString& msg);

  Kid3Application* m_app;
  QQuickView* m_qmlView;
  QQmlEngine* m_qmlEngine;
  bool m_showOutput;

  static QmlCommandPlugin* s_messageHandlerInstance;
};
