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

#ifndef ONLINE_ACCOUNTS_TST_PLUGIN_H
#define ONLINE_ACCOUNTS_TST_PLUGIN_H

#include <QTest>

class QAbstractListModel;

class PluginTest: public QObject
{
    Q_OBJECT

public:
    PluginTest();

private Q_SLOTS:
    void initTestCase();
    void testLoadPlugin();
    void testModel();
    void testModelSignals();
    void testAccountService();
    void testAccountServiceUpdate();
    void testAuthentication();
    void testManagerCreate();
    void testManagerLoad();
    void testAccountInvalid();
    void testAccount();

private:
    void clearDb();
    QVariant get(const QAbstractListModel *model, int row, QString roleName);
};

#endif // ONLINE_ACCOUNTS_TST_PLUGIN_H
