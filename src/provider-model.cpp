/*
 * Copyright (C) 2013 Canonical Ltd.
 *
 * Contact: Alberto Mardegan <alberto.mardegan@canonical.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "debug.h"
#include "provider-model.h"

#include <Accounts/Provider>
#include <Accounts/Manager>

using namespace OnlineAccounts;

/*!
 * \qmltype ProviderModel
 * \inqmlmodule Ubuntu.OnlineAccounts 0.1
 * \ingroup Ubuntu
 *
 * \brief A model of the account providers
 *
 * The ProviderModel is a model representing the account providers installed on
 * the system.
 *
 * The model defines the following roles:
 * \list
 * \li \c displayName
 * \li \c providerId is the unique identifier of the account provider
 * \li \c iconName
 * \endlist
 */

ProviderModel::ProviderModel(QObject *parent):
    QAbstractListModel(parent),
    manager(SharedManager::instance())
{
    /* Given that the list is currently immutable, retrieve it once for all */
    providers = manager->providerList();
}

ProviderModel::~ProviderModel()
{
}

/*
 * \qmlproperty int ProviderModel::count
 * The number of items in the model.
 */
int ProviderModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return providers.count();
}

/*
 * \qmlmethod variant ProviderModel::get(int row, string roleName)
 *
 * Returns the data at \a row for the role \a roleName.
 */
QVariant ProviderModel::get(int row, const QString &roleName) const
{
    int role = roleNames().key(roleName.toLatin1(), -1);
    return data(index(row), role);
}

QVariant ProviderModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= providers.count())
        return QVariant();

    const Accounts::Provider &provider = providers.at(index.row());
    QVariant ret;

    switch (role) {
    case Qt::DisplayRole:
        ret = provider.displayName();
        break;
    case ProviderIdRole:
        ret = provider.name();
        break;
    case IconNameRole:
        ret = provider.iconName();
        break;
    }

    return ret;
}

QHash<int, QByteArray> ProviderModel::roleNames() const
{
    static QHash<int, QByteArray> roles;
    if (roles.isEmpty()) {
        roles[Qt::DisplayRole] = "displayName";
        roles[ProviderIdRole] = "providerId";
        roles[IconNameRole] = "iconName";
    }
    return roles;
}
