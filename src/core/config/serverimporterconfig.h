/**
 * \file serverimporterconfig.h
 * Configuration for server import.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 09 Oct 2006
 *
 * Copyright (C) 2006-2007  Urs Fleisch
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

#include "generalconfig.h"
#include <QString>
#include "kid3api.h"

/**
 * Freedb configuration.
 */
class KID3_CORE_EXPORT ServerImporterConfig : public GeneralConfig {
  Q_OBJECT
  /** server */
  Q_PROPERTY(QString server READ server WRITE setServer NOTIFY serverChanged)
  /** CGI path used for access */
  Q_PROPERTY(QString cgiPath READ cgiPath WRITE setCgiPath NOTIFY cgiPathChanged)
  /** window geometry */
  Q_PROPERTY(QByteArray windowGeometry READ windowGeometry WRITE setWindowGeometry NOTIFY windowGeometryChanged)
  /** true if CgiPath configuration is used */
  Q_PROPERTY(bool cgiPathUsed READ cgiPathUsed WRITE setCgiPathUsed NOTIFY cgiPathUsedChanged)
  /** true if additional tags configuration is used */
  Q_PROPERTY(bool additionalTagsUsed READ additionalTagsUsed WRITE setAdditionalTagsUsed NOTIFY additionalTagsUsedChanged)
  /** standard tags imported */
  Q_PROPERTY(bool standardTags READ standardTags WRITE setStandardTags NOTIFY standardTagsChanged)
  /** additional tags imported */
  Q_PROPERTY(bool additionalTags READ additionalTags WRITE setAdditionalTags NOTIFY additionalTagsChanged)
  /** cover art imported */
  Q_PROPERTY(bool coverArt READ coverArt WRITE setCoverArt NOTIFY coverArtChanged)

public:
  /**
   * Constructor.
   * Set default configuration.
   *
   * @param grp         configuration group
   */
  explicit ServerImporterConfig(const QString& grp);

  /**
   * Constructor.
   * Used to create temporary configuration.
   */
  ServerImporterConfig();

  /**
   * Destructor.
   */
  virtual ~ServerImporterConfig() override = default;

  /**
   * Persist configuration.
   *
   * @param config KDE configuration
   */
  virtual void writeToConfig(ISettings* config) const override;

  /**
   * Read persisted configuration.
   *
   * @param config KDE configuration
   */
  virtual void readFromConfig(ISettings* config) override;

  /** Get server. */
  QString server() const { return m_server; }

  /** Set server. */
  void setServer(const QString& server);

  /** Get CGI path used for access. */
  QString cgiPath() const { return m_cgiPath; }

  /** Set CGI path used for access. */
  void setCgiPath(const QString& cgiPath);

  /** Get window geometry. */
  QByteArray windowGeometry() const { return m_windowGeometry; }

  /** Set window geometry. */
  void setWindowGeometry(const QByteArray& windowGeometry);

  /** Check if CgiPath configuration is used. */
  bool cgiPathUsed() const { return m_cgiPathUsed; }

  /** Set if CgiPath configuration is used. */
  void setCgiPathUsed(bool cgiPathUsed);

  /** Check if additional tags configuration is used. */
  bool additionalTagsUsed() const { return m_additionalTagsUsed; }

  /** Set if additional tags configuration is used. */
  void setAdditionalTagsUsed(bool additionalTagsUsed);

  /** Check if standard tags are imported. */
  bool standardTags() const { return m_standardTags; }

  /** Set if standard tags are imported. */
  void setStandardTags(bool standardTags);

  /** Check if additional tags are imported. */
  bool additionalTags() const { return m_additionalTags; }

  /** Set if additional tags are imported. */
  void setAdditionalTags(bool additionalTags);

  /** Check if cover art is imported. */
  bool coverArt() const { return m_coverArt; }

  /** Set if cover art is imported. */
  void setCoverArt(bool coverArt);

signals:
  /** Emitted when @a server changed. */
  void serverChanged(const QString& server);

  /** Emitted when @a cgiPath changed. */
  void cgiPathChanged(const QString& cgiPath);

  /** Emitted when @a windowGeometry changed. */
  void windowGeometryChanged(const QByteArray& windowGeometry);

  /** Emitted when @a cgiPathUsed changed. */
  void cgiPathUsedChanged(bool cgiPathUsed);

  /** Emitted when @a additionalTagsUsed changed. */
  void additionalTagsUsedChanged(bool additionalTagsUsed);

  /** Emitted when @a standardTags changed. */
  void standardTagsChanged(bool standardTags);

  /** Emitted when @a additionalTags changed. */
  void additionalTagsChanged(bool additionalTags);

  /** Emitted when @a coverArt changed. */
  void coverArtChanged(bool coverArt);

private:
  QString m_server;
  QString m_cgiPath;
  QByteArray m_windowGeometry;
  bool m_cgiPathUsed;
  bool m_additionalTagsUsed;
  bool m_standardTags;
  bool m_additionalTags;
  bool m_coverArt;
};
