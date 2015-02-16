/**
 * \file qmlprocess.h
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

#ifndef QMLPROCESS_H
#define QMLPROCESS_H

#include <QObject>
#include "kid3api.h"
#if QT_VERSION < 0x050000
#include <QDeclarativeView>
#endif

class Kid3Application;
#if QT_VERSION >= 0x050000
class QQuickView;
class QQmlEngine;
#else
class QDeclarativeEngine;
class QQuickCloseEvent;
#endif

/**
 * Starter for QML scripts.
 */
class KID3_CORE_EXPORT QmlProcess : public QObject {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param app application context
   * @param parent parent object
   */
  explicit QmlProcess(Kid3Application* app, QObject* parent = 0);

  /**
   * Destructor.
   */
  virtual ~QmlProcess();

  /**
   * Start a QML script.
   *
   * @param program virtual program, e.g. "qmlview"
   * @param arguments arguments to pass to script
   * @param showOutput true to enable output in output viewer
   * @return true if program and arguments are suitable for QML script.
   */
  bool startQml(const QString& program, const QStringList& arguments,
                bool showOutput);

signals:
  /**
   * Emitted when output is enabled and a QML message is generated.
   * @param msg message from QML, error or console output
   */
  void qmlOutput(const QString& msg);

private slots:
  void onQmlViewClosing();
  void onQmlViewFinished();
  void onQmlEngineQuit();
  void onEngineFinished();

private:
  void onEngineReady();

#if QT_VERSION >= 0x050000
  static void messageHandler(QtMsgType type, const QMessageLogContext& context,
                             const QString& msg);
#else
  static void messageHandler(QtMsgType type, const char* msg);
#endif

  Kid3Application* m_app;
#if QT_VERSION >= 0x050000
  QQuickView* m_qmlView;
  QQmlEngine* m_qmlEngine;
#else
  QDeclarativeView* m_qmlView;
  QDeclarativeEngine* m_qmlEngine;
#endif
  bool m_showOutput;

  static QmlProcess* s_messageHandlerInstance;
};

#if QT_VERSION < 0x050000
/**
 * QDeclarativeView with a closing signal.
 */
class QmlView : public QDeclarativeView {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param parent parent widget
   */
  explicit QmlView(QWidget* parent = 0);

  /**
   * Destructor.
   */
  virtual ~QmlView();

signals:
  /**
   * Emitted when window is closed.
   * @param ev close event, always 0, just for compatibility with Qt 5
   */
  void closing(QQuickCloseEvent* ev);

protected:
  /**
   * Handle close event.
   * @param ev close event
   */
  virtual void closeEvent(QCloseEvent* ev);
};
#endif

#endif // QMLPROCESS_H
