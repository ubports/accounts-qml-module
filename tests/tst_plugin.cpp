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

#include "tst_plugin.h"

#include <QDebug>
#include <QQmlComponent>
#include <QQmlEngine>

PluginTest::PluginTest():
    QObject(0)
{
}

void PluginTest::initTestCase()
{
    qputenv("QML2_IMPORT_PATH", "../src");
}

void PluginTest::testLoadPlugin()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import Ubuntu.OnlineAccounts 0.1\n"
                      "AccountServiceModel {}",
                      QUrl());
    QTest::qWait(10);
    QObject *object = component.create();
    QVERIFY(object != 0);
}

QTEST_MAIN(PluginTest);
