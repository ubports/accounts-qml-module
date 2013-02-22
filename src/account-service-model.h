/*
 * Copyright (C) 2013 Canonical Ltd.
 *
 * Contact: Alberto Mardegan <alberto.mardegan@canonical.com>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ONLINE_ACCOUNTS_ACCOUNT_SERVICE_MODEL_H
#define ONLINE_ACCOUNTS_ACCOUNT_SERVICE_MODEL_H

#include <QAbstractListModel>
#include <QQmlParserStatus>
#include <QString>

namespace OnlineAccounts {

class AccountServiceModelPrivate;

class AccountServiceModel: public QAbstractListModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(bool includeDisabled READ includeDisabled \
               WRITE setIncludeDisabled NOTIFY includeDisabledChanged)
    Q_PROPERTY(quint32 accountId READ accountId WRITE setAccountId \
               NOTIFY accountIdChanged)
    Q_PROPERTY(QString provider READ provider WRITE setProvider \
               NOTIFY providerChanged)
    Q_PROPERTY(QString serviceType READ serviceType WRITE setServiceType \
               NOTIFY serviceTypeChanged)
    Q_PROPERTY(QString service READ service WRITE setService \
               NOTIFY serviceChanged)

public:
    AccountServiceModel(QObject *parent = 0);
    ~AccountServiceModel();

    enum Roles {
        DisplayNameRole = Qt::UserRole + 1,
        ProviderNameRole,
        ServiceNameRole,
        EnabledRole,
        AccountServiceRole,
        AccountIdRole,
    };

    void setIncludeDisabled(bool includeDisabled);
    bool includeDisabled() const;

    void setAccountId(quint32 accountId);
    quint32 accountId() const;

    void setProvider(const QString &providerId);
    QString provider() const;

    void setServiceType(const QString &serviceTypeId);
    QString serviceType() const;

    void setService(const QString &serviceId);
    QString service() const;

    // reimplemented virtual methods
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QHash<int, QByteArray> roleNames() const;

    void classBegin();
    void componentComplete();

Q_SIGNALS:
    void includeDisabledChanged();
    void accountIdChanged();
    void providerChanged();
    void serviceTypeChanged();
    void serviceChanged();

private:
    AccountServiceModelPrivate *d_ptr;
    Q_DECLARE_PRIVATE(AccountServiceModel)
};

}; // namespace

#endif // ONLINE_ACCOUNTS_ACCOUNT_SERVICE_MODEL_H
