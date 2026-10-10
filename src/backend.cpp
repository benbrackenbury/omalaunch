#include "backend.h"

#include "missionlistmodel.h"

#include <QColor>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkDiskCache>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QStandardPaths>
#include <QTextStream>
#include <QTimeZone>
#include <QUrl>

namespace {
const QString defaultSnapshotUrl =
    QStringLiteral("https://redstone-server-ben-brackenburys-projects.vercel.app/v1/snapshot");
const QString starshipTrackerUrl =
    QStringLiteral("https://sxcontent9668.azureedge.us/cms-assets/starship_tracker_public.json");

QString tomlValue(const QString &line, QString *key)
{
    const int equals = line.indexOf(QLatin1Char('='));
    if (equals < 0)
        return {};
    *key = line.left(equals).trimmed();
    QString value = line.mid(equals + 1).trimmed();
    if (value.size() >= 2
        && ((value.front() == QLatin1Char('"') && value.back() == QLatin1Char('"'))
            || (value.front() == QLatin1Char('\'') && value.back() == QLatin1Char('\''))))
        value = value.mid(1, value.size() - 2);
    return value;
}
}

Backend::Backend(QObject *parent)
    : QObject(parent)
    , m_missions(new MissionListModel(MissionListModel::Kind::All, this))
    , m_starship(new MissionListModel(MissionListModel::Kind::Starship, this))
    , m_search(new MissionListModel(MissionListModel::Kind::Search, this))
{
    auto *cache = new QNetworkDiskCache(this);
    cache->setCacheDirectory(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
                             + QStringLiteral("/http"));
    m_nam.setCache(cache);

    loadSettings();
    loadOmarchyTheme();
    watchOmarchyTheme();
    connect(&m_themeWatcher, &QFileSystemWatcher::fileChanged, this, [this] {
        loadOmarchyTheme();
        watchOmarchyTheme();
    });
    connect(&m_themeWatcher, &QFileSystemWatcher::directoryChanged, this, [this] {
        loadOmarchyTheme();
        watchOmarchyTheme();
    });

    m_statusTimer.setInterval(30000);
    connect(&m_statusTimer, &QTimer::timeout, this, [this] {
        m_missions->refreshStatus();
        m_starship->refreshStatus();
        m_search->refreshStatus();
    });
    m_statusTimer.start();

    m_telemetryTimer.setInterval(10000);
    connect(&m_telemetryTimer, &QTimer::timeout, this, &Backend::fetchTelemetry);

    loadCache();
    refresh();
}

void Backend::setDarkMode(bool darkMode)
{
    if (m_darkMode == darkMode)
        return;
    m_darkMode = darkMode;
    emit darkModeChanged();
    loadOmarchyTheme();
}

void Backend::setTextScale(qreal textScale)
{
    if (qFuzzyCompare(m_textScale, textScale))
        return;
    m_textScale = textScale;
    emit textScaleChanged();
}

void Backend::setSnapshotUrl(const QString &url)
{
    const QString trimmed = url.trimmed();
    if (trimmed.isEmpty() || trimmed == m_snapshotUrl)
        return;
    m_snapshotUrl = trimmed;
    QSettings().setValue(QStringLiteral("snapshotUrl"), m_snapshotUrl);
    emit settingsChanged();
    refresh();
}

void Backend::setUseUtc(bool value)
{
    if (m_useUtc == value)
        return;
    m_useUtc = value;
    QSettings().setValue(QStringLiteral("useUtc"), m_useUtc);
    applyDisplaySettings();
    emit settingsChanged();
}

void Backend::setUse24Hour(bool value)
{
    if (m_use24h == value)
        return;
    m_use24h = value;
    QSettings().setValue(QStringLiteral("use24Hour"), m_use24h);
    applyDisplaySettings();
    emit settingsChanged();
}

void Backend::setShowOtherLaunches(bool value)
{
    if (m_showOther == value)
        return;
    m_showOther = value;
    QSettings().setValue(QStringLiteral("showOtherLaunches"), m_showOther);
    emit settingsChanged();
}

void Backend::setTypeFilter(const QString &type)
{
    if (m_typeFilter == type)
        return;
    m_typeFilter = type;
    m_missions->setTypeFilter(type);
    emit settingsChanged();
}

void Backend::setSearchQuery(const QString &query)
{
    if (m_searchQuery == query)
        return;
    m_searchQuery = query;
    m_search->setQuery(query);
    emit settingsChanged();
}

void Backend::refresh()
{
    if (m_loading)
        return;
    m_loading = true;
    emit loadingChanged();

    QNetworkRequest request{QUrl(m_snapshotUrl)};
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork);
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("redstone-omarchy/0.1"));
    request.setTransferTimeout(30000);

    auto *reply = m_nam.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        reply->deleteLater();
        m_loading = false;
        emit loadingChanged();

        if (reply->error() != QNetworkReply::NoError) {
            m_error = reply->errorString();
            emit errorChanged();
            return;
        }

        const QByteArray body = reply->readAll();
        if (body.trimmed().startsWith('<')) {
            m_error = QStringLiteral("Server returned HTML instead of JSON.");
            emit errorChanged();
            return;
        }

        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
        if (!doc.isObject()) {
            m_error = parseError.errorString();
            emit errorChanged();
            return;
        }

        m_error.clear();
        emit errorChanged();
        saveCache(body);
        ingestSnapshot(doc.object(), QDateTime::currentDateTimeUtc());
    });
}

void Backend::openMission(const QString &slug)
{
    const MissionRecord *mission = m_missions->bySlug(slug);
    if (!mission)
        return;
    m_selected = missionToMap(*mission, m_followed.contains(slug), m_useUtc, m_use24h);
    const QJsonObject details = m_details.value(slug);
    m_selectedTimeline = parseTimeline(details);
    m_selectedParagraphs = parseParagraphs(details);
    m_selectedWebcasts = parseWebcasts(details);
    emit selectedChanged();
}

void Backend::closeMission()
{
    m_selected.clear();
    m_selectedTimeline.clear();
    m_selectedParagraphs.clear();
    m_selectedWebcasts.clear();
    emit selectedChanged();
}

void Backend::toggleFollow(const QString &slug)
{
    if (m_followed.contains(slug))
        m_followed.remove(slug);
    else
        m_followed.insert(slug);
    QSettings().setValue(QStringLiteral("followed"), QStringList(m_followed.begin(), m_followed.end()));
    applyFollowed();
    if (m_selected.value(QStringLiteral("slug")).toString() == slug) {
        m_selected.insert(QStringLiteral("followed"), m_followed.contains(slug));
        emit selectedChanged();
    }
}

void Backend::openUrl(const QString &url) const
{
    if (!url.isEmpty())
        QDesktopServices::openUrl(QUrl(url));
}

QString Backend::countdown(qint64 netMs, bool hasNet) const
{
    return countdownText(QDateTime::fromMSecsSinceEpoch(netMs, QTimeZone::UTC), hasNet,
                         QDateTime::currentDateTimeUtc());
}

QVariantMap Backend::windowGeometry() const
{
    return QSettings().value(QStringLiteral("window/geometry")).toMap();
}

void Backend::saveWindowGeometry(int x, int y, int width, int height, bool maximized)
{
    QVariantMap geo;
    geo.insert(QStringLiteral("x"), x);
    geo.insert(QStringLiteral("y"), y);
    geo.insert(QStringLiteral("width"), width);
    geo.insert(QStringLiteral("height"), height);
    geo.insert(QStringLiteral("maximized"), maximized);
    QSettings().setValue(QStringLiteral("window/geometry"), geo);
}

void Backend::startTelemetry()
{
    fetchTelemetry();
    m_telemetryTimer.start();
}

void Backend::stopTelemetry()
{
    m_telemetryTimer.stop();
}

void Backend::loadOmarchyTheme()
{
    m_themeBackground = m_darkMode ? QStringLiteral("#101010") : QStringLiteral("#ffffff");
    m_themeForeground = m_darkMode ? QStringLiteral("#eeeeee") : QStringLiteral("#222324");
    m_themeAccent = m_darkMode ? QStringLiteral("#5584aa") : QStringLiteral("#2077b2");
    m_themeSelection = m_darkMode ? QStringLiteral("#186a9a") : QStringLiteral("#2077b2");
    m_themeMuted = m_darkMode ? QStringLiteral("#888888") : QStringLiteral("#666666");
    m_themeSurface = m_darkMode ? QStringLiteral("#1a1a1a") : QStringLiteral("#f4f4f4");
    m_themeDanger = QStringLiteral("#fd6883");
    m_themeSuccess = QStringLiteral("#adda78");

    QString themeMode;
    const QString colorsPath = QDir::homePath()
        + QStringLiteral("/.local/state/omarchy/current/theme/colors.toml");
    QFile file(colorsPath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            const QString line = in.readLine().trimmed();
            if (line.isEmpty() || line.startsWith(QLatin1Char('#')))
                continue;
            QString key;
            const QString value = tomlValue(line, &key);
            if (key == QLatin1String("mode"))
                themeMode = value;
            else if (key == QLatin1String("background"))
                m_themeBackground = value;
            else if (key == QLatin1String("foreground"))
                m_themeForeground = value;
            else if (key == QLatin1String("accent"))
                m_themeAccent = value;
            else if (key == QLatin1String("selection"))
                m_themeSelection = value;
            else if (key == QLatin1String("muted"))
                m_themeMuted = value;
            else if (key == QLatin1String("lighter_background"))
                m_themeSurface = value;
            else if (key == QLatin1String("red"))
                m_themeDanger = value;
            else if (key == QLatin1String("green"))
                m_themeSuccess = value;
        }
    }

    bool themeModeKnown = false;
    bool themeIsDark = m_darkMode;
    if (themeMode == QLatin1String("dark")) {
        themeIsDark = true;
        themeModeKnown = true;
    } else if (themeMode == QLatin1String("light")) {
        themeIsDark = false;
        themeModeKnown = true;
    } else {
        const QColor background(m_themeBackground);
        if (background.isValid()) {
            const double luminance = 0.299 * background.redF()
                + 0.587 * background.greenF() + 0.114 * background.blueF();
            themeIsDark = luminance < 0.5;
            themeModeKnown = true;
        }
    }
    if (themeModeKnown && themeIsDark != m_darkMode) {
        m_darkMode = themeIsDark;
        emit darkModeChanged();
    }

    emit themeColorsChanged();
}

void Backend::watchOmarchyTheme()
{
    const QStringList watched = m_themeWatcher.files() + m_themeWatcher.directories();
    if (!watched.isEmpty())
        m_themeWatcher.removePaths(watched);

    const QString currentDir = QDir::homePath() + QStringLiteral("/.local/state/omarchy/current");
    const QString themeDir = currentDir + QStringLiteral("/theme");
    const QString colorsPath = themeDir + QStringLiteral("/colors.toml");
    if (QDir(currentDir).exists())
        m_themeWatcher.addPath(currentDir);
    if (QDir(themeDir).exists())
        m_themeWatcher.addPath(themeDir);
    if (QFile::exists(colorsPath))
        m_themeWatcher.addPath(colorsPath);
}

void Backend::loadSettings()
{
    QSettings settings;
    m_snapshotUrl = settings.value(QStringLiteral("snapshotUrl"), defaultSnapshotUrl).toString();
    m_useUtc = settings.value(QStringLiteral("useUtc"), false).toBool();
    m_use24h = settings.value(QStringLiteral("use24Hour"), true).toBool();
    m_showOther = settings.value(QStringLiteral("showOtherLaunches"), false).toBool();
    const QStringList followed = settings.value(QStringLiteral("followed")).toStringList();
    m_followed = QSet<QString>(followed.begin(), followed.end());
    applyDisplaySettings();
    applyFollowed();
}

void Backend::applyDisplaySettings()
{
    m_missions->setDisplay(m_useUtc, m_use24h);
    m_starship->setDisplay(m_useUtc, m_use24h);
    m_search->setDisplay(m_useUtc, m_use24h);
}

void Backend::loadCache()
{
    QFile file(cachePath());
    if (!file.open(QIODevice::ReadOnly))
        return;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject())
        return;
    ingestSnapshot(doc.object(), QSettings().value(QStringLiteral("lastUpdated")).toDateTime());
}

void Backend::saveCache(const QByteArray &bytes) const
{
    QDir().mkpath(QFileInfo(cachePath()).absolutePath());
    QFile file(cachePath());
    if (file.open(QIODevice::WriteOnly))
        file.write(bytes);
}

QString Backend::cachePath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
        + QStringLiteral("/snapshot.json");
}

void Backend::ingestSnapshot(const QJsonObject &root, const QDateTime &updatedAt)
{
    QVector<MissionRecord> records;
    const QJsonArray missions = root.value(QStringLiteral("missions")).toArray();
    records.reserve(missions.size());
    for (const QJsonValue &value : missions)
        records.append(parseMission(value.toObject()));
    m_records = records;

    m_details.clear();
    const QJsonObject details = root.value(QStringLiteral("details")).toObject();
    for (auto it = details.begin(); it != details.end(); ++it)
        m_details.insert(it.key(), it.value().toObject());

    const QJsonObject stats = root.value(QStringLiteral("stats")).toObject();
    m_stats.clear();
    m_stats.insert(QStringLiteral("totalLaunches"), stats.value(QStringLiteral("totalLaunches")).toInt());
    m_stats.insert(QStringLiteral("totalLandings"), stats.value(QStringLiteral("totalLandings")).toInt());
    m_stats.insert(QStringLiteral("totalReflights"), stats.value(QStringLiteral("totalReflights")).toInt());

    m_closures.clear();
    const QJsonArray closures = root.value(QStringLiteral("roadClosures")).toArray();
    for (const QJsonValue &value : closures) {
        const QJsonObject obj = value.toObject();
        QVariantMap row;
        row.insert(QStringLiteral("title"), obj.value(QStringLiteral("title")).toString());
        row.insert(QStringLiteral("dateText"), obj.value(QStringLiteral("dateText")).toString());
        m_closures.append(row);
    }

    m_other.clear();
    const QJsonArray other = root.value(QStringLiteral("externalLaunches")).toArray();
    for (const QJsonValue &value : other) {
        const QJsonObject obj = value.toObject();
        QVariantMap row;
        row.insert(QStringLiteral("id"), obj.value(QStringLiteral("id")).toString());
        row.insert(QStringLiteral("name"), obj.value(QStringLiteral("name")).toString());
        row.insert(QStringLiteral("provider"), obj.value(QStringLiteral("provider")).toString());
        row.insert(QStringLiteral("vehicle"), obj.value(QStringLiteral("vehicle")).toString());
        row.insert(QStringLiteral("pad"), obj.value(QStringLiteral("pad")).toString());
        row.insert(QStringLiteral("location"), obj.value(QStringLiteral("location")).toString());
        row.insert(QStringLiteral("status"), obj.value(QStringLiteral("status")).toString());
        row.insert(QStringLiteral("webcast"), obj.value(QStringLiteral("webcast")).toString());
        row.insert(QStringLiteral("image"), obj.value(QStringLiteral("image")).toString());
        bool hasNet = false;
        const QDateTime net = parseIso8601(obj.value(QStringLiteral("net")).toString());
        if (net.isValid()) {
            hasNet = true;
            row.insert(QStringLiteral("netMs"), net.toMSecsSinceEpoch());
            row.insert(QStringLiteral("netDisplay"), formatLaunchTime(net, true, m_useUtc, m_use24h));
        } else {
            row.insert(QStringLiteral("netDisplay"), QStringLiteral("NET TBD"));
        }
        row.insert(QStringLiteral("hasNet"), hasNet);
        m_other.append(row);
    }

    m_missions->setSource(m_records);
    m_starship->setSource(m_records);
    m_search->setSource(m_records);
    applyFollowed();

    if (updatedAt.isValid()) {
        m_lastUpdated = QLocale::system().toString(updatedAt.toLocalTime(), QStringLiteral("ddd d MMM HH:mm"));
        QSettings().setValue(QStringLiteral("lastUpdated"), updatedAt);
        emit lastUpdatedChanged();
    }

    if (!m_selected.value(QStringLiteral("slug")).toString().isEmpty())
        openMission(m_selected.value(QStringLiteral("slug")).toString());

    emit snapshotChanged();
}

void Backend::applyFollowed()
{
    m_missions->setFollowed(m_followed);
    m_starship->setFollowed(m_followed);
    m_search->setFollowed(m_followed);
}

void Backend::fetchTelemetry()
{
    QNetworkRequest request{QUrl(starshipTrackerUrl)};
    request.setTransferTimeout(15000);
    auto *reply = m_nam.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError)
            return;
        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        if (!doc.isObject())
            return;
        QVariantList rows;
        const QJsonObject root = doc.object();
        for (auto it = root.begin(); it != root.end(); ++it) {
            if (it.key() == QLatin1String("metadata") || !it.value().isObject())
                continue;
            const QJsonObject current = it.value().toObject().value(QStringLiteral("current")).toObject();
            if (current.isEmpty())
                continue;
            QVariantMap row;
            row.insert(QStringLiteral("id"), it.key());
            row.insert(QStringLiteral("altitude"), current.value(QStringLiteral("altitude")).toDouble());
            row.insert(QStringLiteral("speed"), current.value(QStringLiteral("speed")).toDouble());
            row.insert(QStringLiteral("missionTime"), current.value(QStringLiteral("mission_time")).toDouble());
            row.insert(QStringLiteral("latitude"), current.value(QStringLiteral("latitude")).toDouble());
            row.insert(QStringLiteral("longitude"), current.value(QStringLiteral("longitude")).toDouble());
            rows.append(row);
        }
        m_telemetry = rows;
        emit telemetryChanged();
    });
}
