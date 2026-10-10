#include "snapshot.h"

#include <QJsonArray>
#include <QLocale>
#include <QRegularExpression>
#include <QTimeZone>

QDateTime parseIso8601(const QString &value)
{
    QDateTime dt = QDateTime::fromString(value, Qt::ISODateWithMs);
    if (!dt.isValid())
        dt = QDateTime::fromString(value, Qt::ISODate);
    if (dt.isValid())
        return dt.toUTC();
    return {};
}

QDateTime protobufTimestamp(const QJsonValue &value)
{
    if (!value.isObject())
        return {};
    const QJsonValue secondsValue = value.toObject().value(QStringLiteral("Seconds"));
    const qint64 seconds = secondsValue.isDouble() ? qint64(secondsValue.toDouble())
                                                   : secondsValue.toInteger();
    if (seconds <= 0)
        return {};
    return QDateTime::fromSecsSinceEpoch(seconds, QTimeZone::UTC);
}

QDateTime easternWallClock(const QString &date, const QString &time)
{
    const QString stamp = date + QLatin1Char(' ') + time;
    QDateTime dt = QDateTime::fromString(stamp, QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    if (!dt.isValid())
        dt = QDateTime::fromString(stamp, QStringLiteral("yyyy-MM-dd HH:mm"));
    if (!dt.isValid())
        return {};
    dt.setTimeZone(QTimeZone(QByteArrayLiteral("America/New_York")));
    return dt.toUTC();
}

QDateTime missionNet(const QJsonObject &mission, bool *ok)
{
    if (ok)
        *ok = false;

    const QJsonValue net = mission.value(QStringLiteral("net"));
    if (net.isString()) {
        const QDateTime parsed = parseIso8601(net.toString());
        if (parsed.isValid()) {
            if (ok)
                *ok = true;
            return parsed;
        }
    } else if (net.isDouble()) {
        const double value = net.toDouble();
        const double seconds = value > 1'000'000'000'000.0 ? value / 1000.0 : value;
        if (ok)
            *ok = true;
        return QDateTime::fromMSecsSinceEpoch(qint64(seconds * 1000.0), QTimeZone::UTC);
    }

    const QJsonObject future = mission.value(QStringLiteral("futureMissionData")).toObject();
    if (!future.isEmpty()) {
        const bool paused = future.value(QStringLiteral("TZeroPaused")).toBool(false);
        const bool stale = future.value(QStringLiteral("TZeroStale")).toBool(false);
        const QDateTime tzero = protobufTimestamp(future.value(QStringLiteral("TZeroLaunchDate")));
        if (tzero.isValid() && !paused && !stale) {
            if (ok)
                *ok = true;
            return tzero;
        }
        const QDateTime primary = protobufTimestamp(future.value(QStringLiteral("PrimaryLaunchDate")));
        if (primary.isValid()) {
            if (ok)
                *ok = true;
            return primary;
        }
        if (tzero.isValid()) {
            if (ok)
                *ok = true;
            return tzero;
        }
    }

    const QString launchDate = mission.value(QStringLiteral("launchDate")).toString();
    if (launchDate.isEmpty())
        return {};
    const QString launchTime = mission.value(QStringLiteral("launchTime")).toString(QStringLiteral("00:00:00"));
    const QDateTime eastern = easternWallClock(launchDate, launchTime);
    if (eastern.isValid()) {
        if (ok)
            *ok = true;
        return eastern;
    }
    return {};
}

QString pickImageUrl(const QJsonValue &image, bool hero)
{
    if (!image.isObject())
        return {};
    const QJsonObject formats = image.toObject().value(QStringLiteral("formats")).toObject();
    const QStringList keys = hero
        ? QStringList{QStringLiteral("large"), QStringLiteral("medium"), QStringLiteral("small"), QStringLiteral("thumbnail")}
        : QStringList{QStringLiteral("small"), QStringLiteral("medium"), QStringLiteral("thumbnail"), QStringLiteral("large")};
    for (const QString &key : keys) {
        const QString url = formats.value(key).toObject().value(QStringLiteral("url")).toString();
        if (!url.isEmpty())
            return url;
    }
    return image.toObject().value(QStringLiteral("url")).toString();
}

QString missionImageUrl(const QJsonObject &mission, bool inProgress, bool hero)
{
    if (inProgress) {
        const QString ongoing = pickImageUrl(mission.value(QStringLiteral("ongoingMissionImageMobile")), hero);
        if (!ongoing.isEmpty())
            return ongoing;
        const QString ongoingDesktop = pickImageUrl(mission.value(QStringLiteral("ongoingMissionImageDesktop")), hero);
        if (!ongoingDesktop.isEmpty())
            return ongoingDesktop;
    }
    const QString mobile = pickImageUrl(mission.value(QStringLiteral("imageMobile")), hero);
    if (!mobile.isEmpty())
        return mobile;
    return pickImageUrl(mission.value(QStringLiteral("imageDesktop")), hero);
}

QString displayTitle(const QString &title)
{
    if (title.contains(QStringLiteral("Starlink"), Qt::CaseInsensitive))
        return title;
    const int idx = title.indexOf(QStringLiteral(" Mission"));
    if (idx > 0)
        return title.left(idx);
    return title;
}

QString missionTypeLabel(const QString &type)
{
    if (type == QLatin1String("commercialSatellite"))
        return QStringLiteral("Commercial Satellite");
    if (type == QLatin1String("hsf"))
        return QStringLiteral("Human Spaceflight");
    if (type == QLatin1String("nssl"))
        return QStringLiteral("NSSL");
    if (type == QLatin1String("resupply"))
        return QStringLiteral("Resupply");
    if (type == QLatin1String("rideshare"))
        return QStringLiteral("Rideshare");
    if (type == QLatin1String("science"))
        return QStringLiteral("Science");
    if (type == QLatin1String("starlink"))
        return QStringLiteral("Starlink");
    if (type == QLatin1String("starship"))
        return QStringLiteral("Starship");
    return type;
}

QString stripHtml(const QString &html)
{
    QString text = html;
    static const QRegularExpression tags(QStringLiteral("<[^>]+>"));
    text.replace(tags, QString());
    return text.trimmed();
}

QString webcastUrl(const QString &videoId, const QString &streamingType)
{
    if (videoId.isEmpty())
        return {};
    if (streamingType == QLatin1String("x.com"))
        return QStringLiteral("https://x.com/spacex/status/") + videoId;
    return QStringLiteral("https://www.youtube.com/watch?v=") + videoId;
}

qint64 timelineOffsetSeconds(const QString &time)
{
    const QString trimmed = time.trimmed();
    if (!trimmed.startsWith(QLatin1Char('T')))
        return 0;
    qint64 sign = 0;
    QString rest;
    if (trimmed.startsWith(QLatin1String("T-"))) {
        sign = -1;
        rest = trimmed.mid(2).trimmed();
    } else if (trimmed.startsWith(QLatin1String("T+"))) {
        sign = 1;
        rest = trimmed.mid(2).trimmed();
    } else {
        return 0;
    }
    const QStringList parts = rest.split(QLatin1Char(':'));
    if (parts.size() != 3)
        return 0;
    bool okH = false, okM = false, okS = false;
    const qint64 hours = parts[0].toLongLong(&okH);
    const qint64 minutes = parts[1].toLongLong(&okM);
    const qint64 seconds = parts[2].toLongLong(&okS);
    if (!okH || !okM || !okS)
        return 0;
    return sign * (hours * 3600 + minutes * 60 + seconds);
}

QString effectiveStatus(const MissionRecord &mission, const QDateTime &now)
{
    if (mission.rawStatus == QLatin1String("final"))
        return QStringLiteral("final");
    if (mission.rawStatus == QLatin1String("in-progress") || mission.isLive || mission.isOngoing)
        return QStringLiteral("in-progress");
    if (mission.rawStatus == QLatin1String("upcoming") && mission.hasNet && mission.net < now) {
        if (mission.net.addSecs(6 * 3600) < now)
            return QStringLiteral("final");
        return QStringLiteral("in-progress");
    }
    return mission.rawStatus.isEmpty() ? QStringLiteral("upcoming") : mission.rawStatus;
}

MissionRecord parseMission(const QJsonObject &json)
{
    MissionRecord mission;
    mission.json = json;
    mission.slug = json.value(QStringLiteral("link")).toString();
    mission.title = displayTitle(json.value(QStringLiteral("title")).toString());
    mission.subtitle = json.value(QStringLiteral("subtitle")).toString().trimmed();
    mission.vehicle = json.value(QStringLiteral("vehicle")).toString();
    mission.launchSite = json.value(QStringLiteral("launchSite")).toString();
    mission.returnSite = json.value(QStringLiteral("returnSite")).toString();
    mission.missionType = json.value(QStringLiteral("missionType")).toString();
    mission.rawStatus = json.value(QStringLiteral("missionStatus")).toString();
    mission.youtubeId = json.value(QStringLiteral("youtubeVideoId")).toString();
    mission.streamingType = json.value(QStringLiteral("streamingVideoType")).toString();
    mission.isLive = json.value(QStringLiteral("isLive")).toBool();
    mission.isOngoing = json.value(QStringLiteral("isOngoing")).toBool();
    mission.isStarship = mission.missionType == QLatin1String("starship")
        || mission.vehicle.contains(QStringLiteral("Starship"), Qt::CaseInsensitive);
    mission.typeLabel = missionTypeLabel(mission.missionType);
    mission.net = missionNet(json, &mission.hasNet);
    mission.effectiveStatus = effectiveStatus(mission, QDateTime::currentDateTimeUtc());
    const bool inProgress = mission.effectiveStatus == QLatin1String("in-progress");
    mission.imageUrl = missionImageUrl(json, inProgress, false);
    mission.heroUrl = missionImageUrl(json, inProgress, true);
    return mission;
}

QString formatLaunchTime(const QDateTime &net, bool hasNet, bool useUtc, bool use24h, bool dateOnly)
{
    if (!hasNet || !net.isValid())
        return QStringLiteral("NET TBD");
    const QDateTime shown = useUtc ? net.toUTC() : net.toLocalTime();
    QString format;
    if (dateOnly)
        format = QStringLiteral("d MMM yyyy");
    else if (use24h)
        format = QStringLiteral("ddd d MMM HH:mm");
    else
        format = QStringLiteral("ddd d MMM h:mm AP");
    if (useUtc && !dateOnly)
        format += QStringLiteral(" 'UTC'");
    return QLocale::system().toString(shown, format);
}

QString countdownText(const QDateTime &net, bool hasNet, const QDateTime &now)
{
    if (!hasNet || !net.isValid())
        return QStringLiteral("NET TBD");
    qint64 seconds = now.secsTo(net);
    QChar sign = QLatin1Char('-');
    if (seconds < 0) {
        sign = QLatin1Char('+');
        seconds = -seconds;
    }
    const qint64 days = seconds / 86400;
    seconds %= 86400;
    const qint64 hours = seconds / 3600;
    seconds %= 3600;
    const qint64 minutes = seconds / 60;
    seconds %= 60;
    const QString clock = QStringLiteral("%1:%2:%3")
        .arg(hours, 2, 10, QLatin1Char('0'))
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(seconds, 2, 10, QLatin1Char('0'));
    if (days > 0)
        return QStringLiteral("T%1 %2d %3").arg(sign).arg(days).arg(clock);
    return QStringLiteral("T%1 %2").arg(sign).arg(clock);
}

QVariantMap missionToMap(const MissionRecord &mission, bool followed, bool useUtc, bool use24h)
{
    QVariantMap map;
    map.insert(QStringLiteral("slug"), mission.slug);
    map.insert(QStringLiteral("title"), mission.title);
    map.insert(QStringLiteral("subtitle"), mission.subtitle);
    map.insert(QStringLiteral("vehicle"), mission.vehicle);
    map.insert(QStringLiteral("launchSite"), mission.launchSite);
    map.insert(QStringLiteral("returnSite"), mission.returnSite);
    map.insert(QStringLiteral("missionType"), mission.missionType);
    map.insert(QStringLiteral("typeLabel"), mission.typeLabel);
    map.insert(QStringLiteral("status"), mission.effectiveStatus);
    map.insert(QStringLiteral("youtubeId"), mission.youtubeId);
    map.insert(QStringLiteral("streamingType"), mission.streamingType);
    map.insert(QStringLiteral("webcastUrl"), webcastUrl(mission.youtubeId, mission.streamingType));
    map.insert(QStringLiteral("imageUrl"), mission.imageUrl);
    map.insert(QStringLiteral("heroUrl"), mission.heroUrl);
    map.insert(QStringLiteral("isLive"), mission.isLive);
    map.insert(QStringLiteral("isStarship"), mission.isStarship);
    map.insert(QStringLiteral("followed"), followed);
    map.insert(QStringLiteral("hasNet"), mission.hasNet);
    map.insert(QStringLiteral("netMs"), mission.hasNet ? mission.net.toMSecsSinceEpoch() : 0);
    map.insert(QStringLiteral("netDisplay"), formatLaunchTime(mission.net, mission.hasNet, useUtc, use24h));
    map.insert(QStringLiteral("spacexUrl"), QStringLiteral("https://www.spacex.com/launches/") + mission.slug);
    map.insert(QStringLiteral("quickDetail"), mission.json.value(QStringLiteral("quickDetail")).toString());
    return map;
}

static QVariantList timelineEntries(const QJsonValue &timeline)
{
    QVariantList rows;
    const QJsonArray entries = timeline.toObject().value(QStringLiteral("timelineEntries")).toArray();
    for (const QJsonValue &value : entries) {
        const QJsonObject obj = value.toObject();
        QVariantMap row;
        row.insert(QStringLiteral("time"), obj.value(QStringLiteral("time")).toString());
        row.insert(QStringLiteral("description"), obj.value(QStringLiteral("description")).toString());
        rows.append(row);
    }
    return rows;
}

QVariantList parseTimeline(const QJsonObject &details)
{
    QVariantList rows = timelineEntries(details.value(QStringLiteral("preLaunchTimeline")));
    rows.append(timelineEntries(details.value(QStringLiteral("postLaunchTimeline"))));
    return rows;
}

QVariantList parseParagraphs(const QJsonObject &details)
{
    QVariantList rows;
    const QJsonArray paragraphs = details.value(QStringLiteral("paragraphs")).toArray();
    for (const QJsonValue &value : paragraphs) {
        const QString content = stripHtml(value.toObject().value(QStringLiteral("content")).toString());
        if (!content.isEmpty())
            rows.append(content);
    }
    return rows;
}

QVariantList parseWebcasts(const QJsonObject &details)
{
    QVariantList rows;
    const QJsonArray webcasts = details.value(QStringLiteral("webcasts")).toArray();
    for (const QJsonValue &value : webcasts) {
        const QJsonObject obj = value.toObject();
        QVariantMap row;
        const QString videoId = obj.value(QStringLiteral("videoId")).toString();
        const QString streaming = obj.value(QStringLiteral("streamingVideoType")).toString();
        row.insert(QStringLiteral("title"), obj.value(QStringLiteral("title")).toString(QStringLiteral("Webcast")));
        row.insert(QStringLiteral("url"), webcastUrl(videoId, streaming));
        rows.append(row);
    }
    return rows;
}
