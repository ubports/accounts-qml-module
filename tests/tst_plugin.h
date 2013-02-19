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
    void testAuthentication();

private:
    void clearDb();
    QVariant get(const QAbstractListModel *model, int row, QString roleName);
};

#endif // ONLINE_ACCOUNTS_TST_PLUGIN_H
