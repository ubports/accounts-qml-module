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

#include "account-service-model.h"
#include "account-service.h"
#include "plugin.h"

#include <QDebug>
#include <QQmlComponent>

using namespace OnlineAccounts;

void Plugin::registerTypes(const char *uri)
{
    qDebug() << Q_FUNC_INFO << uri;
    qmlRegisterType<AccountServiceModel>(uri, 0, 1, "AccountServiceModel");
    qmlRegisterType<AccountService>(uri, 0, 1, "AccountService");
}
