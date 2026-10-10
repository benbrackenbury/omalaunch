#pragma once

#include <QFileSystemWatcher>
#include <QHash>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QObject>
#include <QSet>
#include <QString>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>
#include <QVector>

#include "missionlistmodel.h"
#include "snapshot.h"

class Backend : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool darkMode READ darkMode WRITE setDarkMode NOTIFY darkModeChanged)
    Q_PROPERTY(qreal textScale READ textScale WRITE setTextScale NOTIFY textScaleChanged)
    Q_PROPERTY(QString themeBackground READ themeBackground NOTIFY themeColorsChanged)
    Q_PROPERTY(QString themeForeground READ themeForeground NOTIFY themeColorsChanged)
    Q_PROPERTY(QString themeAccent READ themeAccent NOTIFY themeColorsChanged)
    Q_PROPERTY(QString themeSelection READ themeSelection NOTIFY themeColorsChanged)
    Q_PROPERTY(QString themeMuted READ themeMuted NOTIFY themeColorsChanged)
    Q_PROPERTY(QString themeSurface READ themeSurface NOTIFY themeColorsChanged)
    Q_PROPERTY(QString themeDanger READ themeDanger NOTIFY themeColorsChanged)
    Q_PROPERTY(QString themeSuccess READ themeSuccess NOTIFY themeColorsChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
    Q_PROPERTY(QString lastUpdated READ lastUpdated NOTIFY lastUpdatedChanged)
    Q_PROPERTY(MissionListModel *missions READ missions CONSTANT)
    Q_PROPERTY(MissionListModel *starshipMissions READ starshipMissions CONSTANT)
    Q_PROPERTY(MissionListModel *searchMissions READ searchMissions CONSTANT)
    Q_PROPERTY(QVariantMap stats READ stats NOTIFY snapshotChanged)
    Q_PROPERTY(QVariantList roadClosures READ roadClosures NOTIFY snapshotChanged)
    Q_PROPERTY(QVariantList otherLaunches READ otherLaunches NOTIFY snapshotChanged)
    Q_PROPERTY(QVariantMap selectedMission READ selectedMission NOTIFY selectedChanged)
    Q_PROPERTY(QVariantList selectedTimeline READ selectedTimeline NOTIFY selectedChanged)
    Q_PROPERTY(QVariantList selectedParagraphs READ selectedParagraphs NOTIFY selectedChanged)
    Q_PROPERTY(QVariantList selectedWebcasts READ selectedWebcasts NOTIFY selectedChanged)
    Q_PROPERTY(QVariantList telemetry READ telemetry NOTIFY telemetryChanged)
    Q_PROPERTY(QString snapshotUrl READ snapshotUrl WRITE setSnapshotUrl NOTIFY settingsChanged)
    Q_PROPERTY(bool useUtc READ useUtc WRITE setUseUtc NOTIFY settingsChanged)
    Q_PROPERTY(bool use24Hour READ use24Hour WRITE setUse24Hour NOTIFY settingsChanged)
    Q_PROPERTY(bool showOtherLaunches READ showOtherLaunches WRITE setShowOtherLaunches NOTIFY settingsChanged)
    Q_PROPERTY(QString typeFilter READ typeFilter WRITE setTypeFilter NOTIFY settingsChanged)
    Q_PROPERTY(QString searchQuery READ searchQuery WRITE setSearchQuery NOTIFY settingsChanged)

public:
    explicit Backend(QObject *parent = nullptr);

    bool darkMode() const { return m_darkMode; }
    void setDarkMode(bool darkMode);
    qreal textScale() const { return m_textScale; }
    void setTextScale(qreal textScale);
    QString themeBackground() const { return m_themeBackground; }
    QString themeForeground() const { return m_themeForeground; }
    QString themeAccent() const { return m_themeAccent; }
    QString themeSelection() const { return m_themeSelection; }
    QString themeMuted() const { return m_themeMuted; }
    QString themeSurface() const { return m_themeSurface; }
    QString themeDanger() const { return m_themeDanger; }
    QString themeSuccess() const { return m_themeSuccess; }

    bool loading() const { return m_loading; }
    QString error() const { return m_error; }
    QString lastUpdated() const { return m_lastUpdated; }
    MissionListModel *missions() const { return m_missions; }
    MissionListModel *starshipMissions() const { return m_starship; }
    MissionListModel *searchMissions() const { return m_search; }
    QVariantMap stats() const { return m_stats; }
    QVariantList roadClosures() const { return m_closures; }
    QVariantList otherLaunches() const { return m_other; }
    QVariantMap selectedMission() const { return m_selected; }
    QVariantList selectedTimeline() const { return m_selectedTimeline; }
    QVariantList selectedParagraphs() const { return m_selectedParagraphs; }
    QVariantList selectedWebcasts() const { return m_selectedWebcasts; }
    QVariantList telemetry() const { return m_telemetry; }

    QString snapshotUrl() const { return m_snapshotUrl; }
    void setSnapshotUrl(const QString &url);
    bool useUtc() const { return m_useUtc; }
    void setUseUtc(bool value);
    bool use24Hour() const { return m_use24h; }
    void setUse24Hour(bool value);
    bool showOtherLaunches() const { return m_showOther; }
    void setShowOtherLaunches(bool value);
    QString typeFilter() const { return m_typeFilter; }
    void setTypeFilter(const QString &type);
    QString searchQuery() const { return m_searchQuery; }
    void setSearchQuery(const QString &query);

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void openMission(const QString &slug);
    Q_INVOKABLE void closeMission();
    Q_INVOKABLE void toggleFollow(const QString &slug);
    Q_INVOKABLE void openUrl(const QString &url) const;
    Q_INVOKABLE QString countdown(qint64 netMs, bool hasNet) const;
    Q_INVOKABLE QVariantMap windowGeometry() const;
    Q_INVOKABLE void saveWindowGeometry(int x, int y, int width, int height, bool maximized);
    Q_INVOKABLE void startTelemetry();
    Q_INVOKABLE void stopTelemetry();

signals:
    void darkModeChanged();
    void textScaleChanged();
    void themeColorsChanged();
    void loadingChanged();
    void errorChanged();
    void lastUpdatedChanged();
    void snapshotChanged();
    void selectedChanged();
    void telemetryChanged();
    void settingsChanged();

private:
    void loadOmarchyTheme();
    void watchOmarchyTheme();
    void loadSettings();
    void applyDisplaySettings();
    void loadCache();
    void saveCache(const QByteArray &bytes) const;
    QString cachePath() const;
    void ingestSnapshot(const QJsonObject &root, const QDateTime &updatedAt);
    void applyFollowed();
    void fetchTelemetry();

    bool m_darkMode = true;
    qreal m_textScale = 1.0;
    QString m_themeBackground;
    QString m_themeForeground;
    QString m_themeAccent;
    QString m_themeSelection;
    QString m_themeMuted;
    QString m_themeSurface;
    QString m_themeDanger;
    QString m_themeSuccess;
    QFileSystemWatcher m_themeWatcher;

    MissionListModel *m_missions = nullptr;
    MissionListModel *m_starship = nullptr;
    MissionListModel *m_search = nullptr;
    QNetworkAccessManager m_nam;
    QTimer m_statusTimer;
    QTimer m_telemetryTimer;
    QVector<MissionRecord> m_records;
    QHash<QString, QJsonObject> m_details;
    QSet<QString> m_followed;
    QVariantMap m_stats;
    QVariantList m_closures;
    QVariantList m_other;
    QVariantMap m_selected;
    QVariantList m_selectedTimeline;
    QVariantList m_selectedParagraphs;
    QVariantList m_selectedWebcasts;
    QVariantList m_telemetry;
    QString m_snapshotUrl;
    QString m_error;
    QString m_lastUpdated;
    QString m_typeFilter;
    QString m_searchQuery;
    bool m_loading = false;
    bool m_useUtc = false;
    bool m_use24h = true;
    bool m_showOther = false;
};
