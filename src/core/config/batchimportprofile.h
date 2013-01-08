/**
 * \file batchimportprofile.h
 * Profile containing a name list for source for batch import.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 2 Jan 2013
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

#ifndef BATCHIMPORTPROFILE_H
#define BATCHIMPORTPROFILE_H

#include <QString>
#include <QList>

/**
 * Profile containing a name list for source for batch import.
 */
class BatchImportProfile {
public:
  /** Events occuring during batch import. */
  enum ImportEventType {
    Started,
    SourceSelected,
    QueryingAlbumList,
    FetchingTrackList,
    TrackListReceived,
    FetchingCoverArt,
    CoverArtReceived,
    Finished,
    Aborted,
    Error
  };

  /**
   * Properties of a source used during batch import.
   */
  class Source {
  public:
    /**
     * Constructor.
     */
    Source() : m_accuracy(0),
      m_standardTags(false), m_additionalTags(false), m_coverArt(false) {
    }

    /**
     * Destructor.
     */
    ~Source() {}

    /**
     * Get name.
     * @return name.
     */
    QString getName() const { return m_name; }

    /**
     * Set name.
     * @param name name
     */
    void setName(const QString& name) { m_name = name; }

    /**
     * Get required accuracy.
     * An import will only be applied if at least the given accuracy is reached.
     * @return accuracy.
     */
    int getRequiredAccuracy() const { return m_accuracy; }

    /**
     * Set required accuracy.
     * @param accuracy accuracy
     */
    void setRequiredAccuracy(int accuracy) { m_accuracy = accuracy; }

    /**
     * Check if standard tags are fetched from this source.
     * @return true if standard tags are fetched.
     */
    bool standardTagsEnabled() const { return m_standardTags; }

    /**
     * Enable fetching of standard tags from this source.
     * @param enable true to fetch standard tags
     */
    void enableStandardTags(bool enable) { m_standardTags = enable; }

    /**
     * Check if additional tags are fetched from this source.
     * @return true if additional tags are fetched.
     */
    bool additionalTagsEnabled() const { return m_additionalTags; }

    /**
     * Enable fetching of additional tags from this source.
     * @param enable true to fetch additional tags
     */
    void enableAdditionalTags(bool enable) { m_additionalTags = enable; }

    /**
     * Check if cover art is fetched from this source.
     * @return true if cover art is fetched.
     */
    bool coverArtEnabled() const { return m_coverArt; }

    /**
     * Enable fetching of cover art from this source.
     * @param enable true to fetch cover art
     */
    void enableCoverArt(bool enable) { m_coverArt = enable; }

  private:
    QString m_name;
    int m_accuracy;
    bool m_standardTags;
    bool m_additionalTags;
    bool m_coverArt;
  };


  /**
   * Constructor.
   */
  BatchImportProfile();

  /**
   * Destructor.
   */
  ~BatchImportProfile();

 /**
  * Get name.
  * @return name.
  */
  QString getName() const { return m_name; }

  /**
   * Set name.
   * @param name name
   */
  void setName(const QString& name) { m_name = name; }

  /**
   * Set import sources used by this batch.
   * @param sources import sources
   */
  void setSources(const QList<Source>& sources) { m_sources = sources; }

  /**
   * Get import sources used by this batch.
   * @return sources.
   */
  const QList<Source>& getSources() const { return m_sources; }

  /**
   * Restore batch import sources from serialized string.
   * @param str string representation of import sources
   */
  void setSourcesFromString(const QString& str);

  /**
   * Serialize batch import sources as a string.
   * @return string representation of import sources.
   */
  QString getSourcesAsString() const;

private:
  QString m_name;
  QList<Source> m_sources;
};

#endif // BATCHIMPORTPROFILE_H
