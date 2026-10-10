#include "missionlistmodel.h"

#include <QDateTime>
#include <algorithm>

MissionListModel::MissionListModel(Kind kind, QObject *parent)
    : QAbstractListModel(parent)
    , m_kind(kind)
{
}

int MissionListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_rows.size();
}

QVariant MissionListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size())
        return {};
    const Row &row = m_rows.at(index.row());
    const MissionRecord &mission = m_source.at(row.sourceIndex);
    const bool followed = m_followed.contains(mission.slug);

    switch (role) {
    case SlugRole:
        return mission.slug;
    case TitleRole:
        return mission.title;
    case SubtitleRole:
        return mission.subtitle;
    case VehicleRole:
        return mission.vehicle;
    case LaunchSiteRole:
        return mission.launchSite;
    case StatusRole:
        return mission.effectiveStatus;
    case TypeLabelRole:
        return mission.typeLabel;
    case ImageUrlRole:
        return mission.imageUrl;
    case NetMsRole:
        return mission.hasNet ? mission.net.toMSecsSinceEpoch() : 0;
    case NetDisplayRole:
        return formatLaunchTime(mission.net, mission.hasNet, m_useUtc, m_use24h);
    case HasNetRole:
        return mission.hasNet;
    case IsLiveRole:
        return mission.isLive || mission.effectiveStatus == QLatin1String("in-progress");
    case IsStarshipRole:
        return mission.isStarship;
    case FollowedRole:
        return followed;
    case SectionRole:
        return row.section;
    default:
        return {};
    }
}

QHash<int, QByteArray> MissionListModel::roleNames() const
{
    return {
        {SlugRole, "slug"},
        {TitleRole, "title"},
        {SubtitleRole, "subtitle"},
        {VehicleRole, "vehicle"},
        {LaunchSiteRole, "launchSite"},
        {StatusRole, "status"},
        {TypeLabelRole, "typeLabel"},
        {ImageUrlRole, "imageUrl"},
        {NetMsRole, "netMs"},
        {NetDisplayRole, "netDisplay"},
        {HasNetRole, "hasNet"},
        {IsLiveRole, "isLive"},
        {IsStarshipRole, "isStarship"},
        {FollowedRole, "followed"},
        {SectionRole, "section"},
    };
}

void MissionListModel::setSource(const QVector<MissionRecord> &missions)
{
    m_source = missions;
    rebuild();
}

void MissionListModel::setFollowed(const QSet<QString> &followed)
{
    m_followed = followed;
    rebuild();
}

void MissionListModel::setDisplay(bool useUtc, bool use24h)
{
    m_useUtc = useUtc;
    m_use24h = use24h;
    if (!m_rows.isEmpty())
        emit dataChanged(index(0), index(m_rows.size() - 1), {NetDisplayRole});
}

void MissionListModel::setQuery(const QString &query)
{
    if (m_query == query)
        return;
    m_query = query;
    rebuild();
}

void MissionListModel::setTypeFilter(const QString &type)
{
    if (m_typeFilter == type)
        return;
    m_typeFilter = type;
    rebuild();
}

void MissionListModel::refreshStatus()
{
    const QDateTime now = QDateTime::currentDateTimeUtc();
    for (MissionRecord &mission : m_source)
        mission.effectiveStatus = effectiveStatus(mission, now);
    rebuild();
}

const MissionRecord *MissionListModel::bySlug(const QString &slug) const
{
    for (const MissionRecord &mission : m_source) {
        if (mission.slug == slug)
            return &mission;
    }
    return nullptr;
}

QString MissionListModel::slugAt(int row) const
{
    if (row < 0 || row >= m_rows.size())
        return {};
    return m_source.at(m_rows.at(row).sourceIndex).slug;
}

QString MissionListModel::sectionFor(const MissionRecord &mission) const
{
    if (m_kind != Kind::Search && m_followed.contains(mission.slug)
        && mission.effectiveStatus != QLatin1String("final"))
        return QStringLiteral("Following");
    if (mission.effectiveStatus == QLatin1String("in-progress"))
        return QStringLiteral("In Progress");
    if (mission.effectiveStatus == QLatin1String("upcoming"))
        return QStringLiteral("Upcoming");
    return QStringLiteral("Past");
}

void MissionListModel::rebuild()
{
    beginResetModel();
    m_rows.clear();

    const QString needle = m_query.trimmed();
    QVector<int> matching;
    matching.reserve(m_source.size());
    for (int i = 0; i < m_source.size(); ++i) {
        const MissionRecord &mission = m_source.at(i);
        if (m_kind == Kind::Starship && !mission.isStarship)
            continue;
        if (!m_typeFilter.isEmpty() && mission.missionType != m_typeFilter)
            continue;
        if (m_kind == Kind::Search) {
            if (needle.isEmpty())
                continue;
            const QString haystack = mission.title + QLatin1Char(' ') + mission.vehicle
                + QLatin1Char(' ') + mission.launchSite + QLatin1Char(' ') + mission.slug
                + QLatin1Char(' ') + mission.typeLabel;
            if (!haystack.contains(needle, Qt::CaseInsensitive))
                continue;
        }
        matching.append(i);
    }

    auto byNetAsc = [&](int a, int b) {
        const MissionRecord &left = m_source.at(a);
        const MissionRecord &right = m_source.at(b);
        if (left.hasNet != right.hasNet)
            return left.hasNet;
        if (left.hasNet && left.net != right.net)
            return left.net < right.net;
        return left.title.localeAwareCompare(right.title) < 0;
    };
    auto byNetDesc = [&](int a, int b) {
        const MissionRecord &left = m_source.at(a);
        const MissionRecord &right = m_source.at(b);
        if (left.hasNet != right.hasNet)
            return left.hasNet;
        if (left.hasNet && left.net != right.net)
            return left.net > right.net;
        return left.title.localeAwareCompare(right.title) < 0;
    };

    QVector<int> following, inProgress, upcoming, past;
    for (int index : matching) {
        const MissionRecord &mission = m_source.at(index);
        const QString section = sectionFor(mission);
        if (section == QLatin1String("Following"))
            following.append(index);
        else if (section == QLatin1String("In Progress"))
            inProgress.append(index);
        else if (section == QLatin1String("Upcoming"))
            upcoming.append(index);
        else
            past.append(index);
    }

    std::sort(following.begin(), following.end(), byNetAsc);
    std::sort(inProgress.begin(), inProgress.end(), byNetAsc);
    std::sort(upcoming.begin(), upcoming.end(), byNetAsc);
    std::sort(past.begin(), past.end(), byNetDesc);

    auto appendSection = [&](const QVector<int> &indexes, const QString &section) {
        for (int index : indexes)
            m_rows.append({index, section});
    };

    if (m_kind == Kind::Search) {
        QVector<int> combined = following + inProgress + upcoming + past;
        for (int index : combined)
            m_rows.append({index, QString()});
    } else {
        appendSection(following, QStringLiteral("Following"));
        appendSection(inProgress, QStringLiteral("In Progress"));
        appendSection(upcoming, QStringLiteral("Upcoming"));
        appendSection(past, QStringLiteral("Past"));
    }

    endResetModel();
    emit countChanged();
}
