#pragma once

#include "snapshot.h"

#include <QAbstractListModel>
#include <QSet>
#include <QVector>

class MissionListModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum Role {
        SlugRole = Qt::UserRole + 1,
        TitleRole,
        SubtitleRole,
        VehicleRole,
        LaunchSiteRole,
        StatusRole,
        TypeLabelRole,
        ImageUrlRole,
        NetMsRole,
        NetDisplayRole,
        HasNetRole,
        IsLiveRole,
        IsStarshipRole,
        FollowedRole,
        SectionRole
    };

    enum class Kind { All, Starship, Search };

    explicit MissionListModel(Kind kind, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setSource(const QVector<MissionRecord> &missions);
    void setFollowed(const QSet<QString> &followed);
    void setDisplay(bool useUtc, bool use24h);
    void setQuery(const QString &query);
    void setTypeFilter(const QString &type);
    void refreshStatus();
    const MissionRecord *bySlug(const QString &slug) const;
    Q_INVOKABLE QString slugAt(int row) const;

signals:
    void countChanged();

private:
    struct Row {
        int sourceIndex = 0;
        QString section;
    };

    void rebuild();
    QString sectionFor(const MissionRecord &mission) const;

    Kind m_kind;
    QVector<MissionRecord> m_source;
    QVector<Row> m_rows;
    QSet<QString> m_followed;
    QString m_query;
    QString m_typeFilter;
    bool m_useUtc = false;
    bool m_use24h = true;
};
