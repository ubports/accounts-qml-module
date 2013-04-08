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


#include "manager.h"
#include "debug.h"

#include <Accounts/Manager>

using namespace OnlineAccounts;

/*!
 * \qmltype Manager
 * \inqmlmodule Ubuntu.OnlineAccounts 0.1
 * \ingroup Ubuntu
 *
 * \brief The account manager
 *
 * The Manager element is a singleton class which can be used to create new
 * online accounts or load existing ones.
 */
Manager::Manager(QObject *parent):
    QObject(parent),
    manager(new Accounts::Manager)
{
}

Manager::~Manager()
{
    delete manager;
}

/*!
 * \qmlmethod object Manager::loadAccount(uint accountId)
 *
 * Loads the account identified by \a accountId. The returned object can be
 * used to instantiate an \l Account.
 *
 * \sa createAccount()
 */
QObject *Manager::loadAccount(uint accountId)
{
    DEBUG() << accountId;
    return manager->account(accountId);
}

/*!
 * \qmlmethod object Manager::createAccount(const QString &providerName)
 *
 * Create a new account interfacing to the provider identified by \a
 * providerName.
 *
 * \sa loadAccount()
 */
QObject *Manager::createAccount(const QString &providerName)
{
    return manager->createAccount(providerName);
}
