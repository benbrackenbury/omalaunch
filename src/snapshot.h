#pragma once

#include <QDateTime>
#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

struct MissionRecord {
    QString slug;
    QString title;
    QString subtitle;
    QString vehicle;
    QString launchSite;
    QString returnSite;
    QString missionType;
    QString rawStatus;
    QString effectiveStatus;
    QString youtubeId;
    QString streamingType;
    QString imageUrl;
    QString heroUrl;
    QString typeLabel;
    QDateTime net;
    bool hasNet = false;
    bool isLive = false;
    bool isOngoing = false;
    bool isStarship = false;
    QJsonObject json;
};

QDateTime parseIso8601(const QString &value);
QDateTime protobufTimestamp(const QJsonValue &value);
QDateTime easternWallClock(const QString &date, const QString &time);
QDateTime missionNet(const QJsonObject &mission, bool *ok);
QString pickImageUrl(const QJsonValue &image, bool hero);
QString missionImageUrl(const QJsonObject &mission, bool inProgress, bool hero);
QString displayTitle(const QString &title);
QString missionTypeLabel(const QString &type);
QString stripHtml(const QString &html);
QString webcastUrl(const QString &videoId, const QString &streamingType);
qint64 timelineOffsetSeconds(const QString &time);
QString effectiveStatus(const MissionRecord &mission, const QDateTime &now);
MissionRecord parseMission(const QJsonObject &json);
QVariantMap missionToMap(const MissionRecord &mission, bool followed, bool useUtc, bool use24h);
QString formatLaunchTime(const QDateTime &net, bool hasNet, bool useUtc, bool use24h, bool dateOnly = false);
QString countdownText(const QDateTime &net, bool hasNet, const QDateTime &now);
QVariantList parseTimeline(const QJsonObject &details);
QVariantList parseParagraphs(const QJsonObject &details);
QVariantList parseWebcasts(const QJsonObject &details);
