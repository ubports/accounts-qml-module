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

#include <Accounts/AccountService>
#include <Accounts/Manager>
#include <QAbstractListModel>
#include <QDebug>
#include <QDir>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QSignalSpy>
#include <QTest>

using namespace Accounts;

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

PluginTest::PluginTest():
    QObject(0)
{
}

void PluginTest::clearDb()
{
    QDir dbroot(QString::fromLatin1(qgetenv("ACCOUNTS")));
    dbroot.remove("accounts.db");
}

QVariant PluginTest::get(const QAbstractListModel *model, int row,
                         QString roleName)
{
    QHash<int, QByteArray> roleNames = model->roleNames();

    int role = roleNames.key(roleName.toLatin1(), -1);
    return model->data(model->index(row), role);
}

void PluginTest::initTestCase()
{
    qputenv("QML2_IMPORT_PATH", "../src");
    qputenv("ACCOUNTS", "/tmp/");
    qputenv("AG_SERVICES", SERVICES_DIR);
    qputenv("AG_SERVICE_TYPES", SERVICE_TYPES_DIR);
    qputenv("AG_PROVIDERS", PROVIDERS_DIR);

    clearDb();
}

void PluginTest::testLoadPlugin()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import Ubuntu.OnlineAccounts 0.1\n"
                      "AccountServiceModel {}",
                      QUrl());
    QObject *object = component.create();
    QVERIFY(object != 0);
    delete object;
}

void PluginTest::testModel()
{
    clearDb();
    /* Create some accounts */
    Manager *manager = new Manager(this);
    Service coolMail = manager->service("coolmail");
    Service coolShare = manager->service("coolshare");
    Service badMail = manager->service("badmail");
    Service badShare = manager->service("badshare");
    Account *account1 = manager->createAccount("cool");
    QVERIFY(account1 != 0);

    account1->setEnabled(true);
    account1->setDisplayName("CoolAccount");
    account1->selectService(coolMail);
    account1->setEnabled(true);
    account1->selectService(coolShare);
    account1->setEnabled(false);
    account1->syncAndBlock();

    Account *account2 = manager->createAccount("bad");
    QVERIFY(account2 != 0);

    account2->setEnabled(true);
    account2->setDisplayName("BadAccount");
    account2->selectService(badMail);
    account2->setEnabled(true);
    account2->selectService(badShare);
    account2->setEnabled(true);
    account2->syncAndBlock();

    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import Ubuntu.OnlineAccounts 0.1\n"
                      "AccountServiceModel {}",
                      QUrl());
    QObject *object = component.create();
    QVERIFY(object != 0);
    QAbstractListModel *model = qobject_cast<QAbstractListModel*>(object);
    QVERIFY(model != 0);

    QCOMPARE(model->rowCount(), 3);
    QCOMPARE(model->property("count").toInt(), 3);

    QCOMPARE(get(model, 0, "displayName").toString(), QString("BadAccount"));
    QCOMPARE(get(model, 0, "providerName").toString(), QString("Bad provider"));
    QCOMPARE(get(model, 0, "accountId").toUInt(), account2->id());
    QCOMPARE(get(model, 1, "displayName").toString(), QString("BadAccount"));
    QCOMPARE(get(model, 1, "providerName").toString(), QString("Bad provider"));
    QCOMPARE(get(model, 2, "displayName").toString(), QString("CoolAccount"));
    QCOMPARE(get(model, 2, "providerName").toString(), QString("Cool provider"));
    QCOMPARE(get(model, 2, "accountId").toUInt(), account1->id());
    QVariant value;
    QVERIFY(QMetaObject::invokeMethod(model, "get",
                                      Q_RETURN_ARG(QVariant, value),
                                      Q_ARG(int, 2),
                                      Q_ARG(QString, "providerName")));
    QCOMPARE(value.toString(), QString("Cool provider"));
    QObject *accountService = get(model, 2, "accountService").value<QObject*>();
    QVERIFY(accountService != 0);
    QCOMPARE(accountService->metaObject()->className(), "Accounts::AccountService");

    model->setProperty("includeDisabled", true);
    QCOMPARE(model->property("includeDisabled").toBool(), true);
    QTest::qWait(10);
    QCOMPARE(model->rowCount(), 4);
    QCOMPARE(get(model, 0, "enabled").toBool(), true);
    QCOMPARE(get(model, 1, "enabled").toBool(), true);
    QCOMPARE(get(model, 2, "enabled").toBool(), true);
    QCOMPARE(get(model, 3, "enabled").toBool(), false);

    /* Test the accountId filter */
    model->setProperty("accountId", account1->id());
    QCOMPARE(model->property("accountId").toUInt(), account1->id());
    QTest::qWait(10);
    QCOMPARE(model->rowCount(), 2);
    QCOMPARE(get(model, 0, "accountId").toUInt(), account1->id());
    QCOMPARE(get(model, 1, "accountId").toUInt(), account1->id());
    model->setProperty("accountId", 0);

    /* Test the provider filter */
    model->setProperty("provider", QString("bad"));
    QCOMPARE(model->property("provider").toString(), QString("bad"));
    QTest::qWait(10);
    QCOMPARE(model->rowCount(), 2);
    QCOMPARE(get(model, 0, "providerName").toString(), QString("Bad provider"));
    QCOMPARE(get(model, 1, "providerName").toString(), QString("Bad provider"));

    /* Test the service filter */
    model->setProperty("service", QString("coolmail"));
    QCOMPARE(model->property("service").toString(), QString("coolmail"));
    QTest::qWait(10);
    QCOMPARE(model->rowCount(), 0);
    /* Reset the provider, to get some results */
    model->setProperty("provider", QString());
    QTest::qWait(10);
    QCOMPARE(model->rowCount(), 1);
    QCOMPARE(get(model, 0, "providerName").toString(), QString("Cool provider"));
    QCOMPARE(get(model, 0, "serviceName").toString(), QString("Cool Mail"));
    QCOMPARE(get(model, 0, "enabled").toBool(), true);

    /* Test the service-type filter */
    model->setProperty("serviceType", QString("sharing"));
    QCOMPARE(model->property("serviceType").toString(), QString("sharing"));
    /* Reset the service, to get some results */
    model->setProperty("service", QString());
    QTest::qWait(10);
    QCOMPARE(model->rowCount(), 2);
    QCOMPARE(get(model, 0, "serviceName").toString(), QString("Bad Share"));
    QCOMPARE(get(model, 1, "serviceName").toString(), QString("Cool Share"));

    delete manager;
    delete object;
}

void PluginTest::testModelSignals()
{
    clearDb();

    /* Create one account */
    Manager *manager = new Manager(this);
    Service coolMail = manager->service("coolmail");
    Service coolShare = manager->service("coolshare");
    Service badMail = manager->service("badmail");
    Service badShare = manager->service("badshare");
    Account *account1 = manager->createAccount("cool");
    QVERIFY(account1 != 0);

    account1->setEnabled(true);
    account1->setDisplayName("CoolAccount");
    account1->selectService(coolMail);
    account1->setEnabled(true);
    account1->selectService(coolShare);
    account1->setEnabled(false);
    account1->syncAndBlock();

    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import Ubuntu.OnlineAccounts 0.1\n"
                      "AccountServiceModel {}",
                      QUrl());
    QObject *object = component.create();
    QVERIFY(object != 0);
    QAbstractListModel *model = qobject_cast<QAbstractListModel*>(object);
    QVERIFY(model != 0);

    QCOMPARE(model->rowCount(), 1);
    QCOMPARE(model->property("count").toInt(), 1);

    QCOMPARE(get(model, 0, "displayName").toString(), QString("CoolAccount"));
    QCOMPARE(get(model, 0, "providerName").toString(),
             QString("Cool provider"));
    QCOMPARE(get(model, 0, "serviceName").toString(), QString("Cool Mail"));

    /* Enable the cool share service, and verify that it appears in the model */
    QSignalSpy countChanged(model, SIGNAL(countChanged()));
    QSignalSpy rowsInserted(model,
                            SIGNAL(rowsInserted(const QModelIndex&,int,int)));
    account1->selectService(coolShare);
    account1->setEnabled(true);
    account1->syncAndBlock();
    QTest::qWait(50);

    QCOMPARE(model->rowCount(), 2);
    QCOMPARE(rowsInserted.count(), 1);
    QCOMPARE(countChanged.count(), 1);
    rowsInserted.clear();

    /* Disable the cool mail service, and verify that it gets removed */
    QSignalSpy rowsRemoved(model,
                           SIGNAL(rowsRemoved(const QModelIndex&,int,int)));
    account1->selectService(coolMail);
    account1->setEnabled(false);
    account1->syncAndBlock();
    QTest::qWait(50);

    QCOMPARE(model->rowCount(), 1);
    QCOMPARE(rowsInserted.count(), 0);
    QCOMPARE(rowsRemoved.count(), 1);
    rowsRemoved.clear();

    /* Create a second account */
    Account *account2 = manager->createAccount("bad");
    QVERIFY(account2 != 0);

    account2->setEnabled(false);
    account2->setDisplayName("BadAccount");
    account2->selectService(badMail);
    account2->setEnabled(true);
    account2->selectService(badShare);
    account2->setEnabled(true);
    account2->syncAndBlock();
    QTest::qWait(50);

    /* It's disabled, so nothing should have changed */
    QCOMPARE(model->rowCount(), 1);
    QCOMPARE(rowsInserted.count(), 0);
    QCOMPARE(rowsRemoved.count(), 0);

    /* Enable it */
    account2->selectService();
    account2->setEnabled(true);
    account2->syncAndBlock();
    QTest::qWait(50);

    QCOMPARE(model->rowCount(), 3);
    QCOMPARE(rowsInserted.count(), 2);
    QCOMPARE(rowsRemoved.count(), 0);
    rowsInserted.clear();

    /* Include disabled */
    model->setProperty("includeDisabled", true);
    QTest::qWait(50);

    QCOMPARE(model->rowCount(), 4);
    /* The model is being reset: all rows are deleted and then re-added */
    QCOMPARE(rowsInserted.count(), 1);
    QCOMPARE(rowsRemoved.count(), 1);
    rowsInserted.clear();
    rowsRemoved.clear();

    QCOMPARE(get(model, 0, "enabled").toBool(), true);
    QCOMPARE(get(model, 1, "enabled").toBool(), true);
    QCOMPARE(get(model, 2, "enabled").toBool(), false);
    QCOMPARE(get(model, 3, "enabled").toBool(), true);

    /* Enable cool mail, and check for the dataChanged signal */
    QSignalSpy dataChanged(model,
                   SIGNAL(dataChanged(const QModelIndex&,const QModelIndex&)));
    account1->selectService(coolMail);
    account1->setEnabled(true);
    account1->syncAndBlock();
    QTest::qWait(50);

    QCOMPARE(dataChanged.count(), 1);
    QModelIndex index = qvariant_cast<QModelIndex>(dataChanged.at(0).at(0));
    QCOMPARE(index.row(), 2);
    QCOMPARE(rowsInserted.count(), 0);
    QCOMPARE(rowsRemoved.count(), 0);
    dataChanged.clear();
    QCOMPARE(get(model, 2, "enabled").toBool(), true);

    /* Delete the first account */
    account1->remove();
    account1->syncAndBlock();
    QTest::qWait(50);

    QCOMPARE(model->rowCount(), 2);
    QCOMPARE(rowsInserted.count(), 0);
    /* We expect one single signal carrying two rows */
    QCOMPARE(rowsRemoved.count(), 1);
    QCOMPARE(rowsRemoved.at(0).at(1).toInt(), 2);
    QCOMPARE(rowsRemoved.at(0).at(2).toInt(), 3);
    rowsRemoved.clear();

    /* Create a third account */
    Account *account3 = manager->createAccount("bad");
    QVERIFY(account3 != 0);

    account3->setEnabled(true);
    account3->setDisplayName("Second BadAccount");
    account3->selectService(badMail);
    account3->setEnabled(true);
    account3->selectService(badShare);
    account3->setEnabled(false);
    account3->syncAndBlock();
    QTest::qWait(50);

    QCOMPARE(model->rowCount(), 4);
    /* We expect one single signal carrying two rows */
    QCOMPARE(rowsInserted.count(), 1);
    QCOMPARE(rowsInserted.at(0).at(1).toInt(), 2);
    QCOMPARE(rowsInserted.at(0).at(2).toInt(), 3);
    QCOMPARE(rowsRemoved.count(), 0);
    rowsInserted.clear();

    delete manager;
    delete object;
}

void PluginTest::testAccountService()
{
    clearDb();

    /* Create one account */
    Manager *manager = new Manager(this);
    Service coolMail = manager->service("coolmail");
    Service coolShare = manager->service("coolshare");
    Service badMail = manager->service("badmail");
    Service badShare = manager->service("badshare");
    Account *account1 = manager->createAccount("cool");
    QVERIFY(account1 != 0);

    account1->setEnabled(true);
    account1->setDisplayName("CoolAccount");
    account1->selectService(coolMail);
    account1->setEnabled(true);
    account1->selectService(coolShare);
    account1->setEnabled(false);
    account1->syncAndBlock();

    AccountService *accountService1 = new AccountService(account1, coolMail);
    QVERIFY(accountService1 != 0);

    QQmlEngine engine;
    engine.rootContext()->setContextProperty("accountService1", accountService1);
    QQmlComponent component(&engine);
    component.setData("import Ubuntu.OnlineAccounts 0.1\n"
                      "AccountService { objectHandle: accountService1 }",
                      QUrl());
    QObject *qmlObject = component.create();
    QVERIFY(qmlObject != 0);

    QCOMPARE(qmlObject->property("objectHandle").value<AccountService*>(),
             accountService1);
    QCOMPARE(qmlObject->property("enabled").toBool(), true);
    QCOMPARE(qmlObject->property("serviceEnabled").toBool(), true);
    QCOMPARE(qmlObject->property("displayName").toString(),
             QString("CoolAccount"));
    QCOMPARE(qmlObject->property("accountId").toUInt(), account1->id());

    QVariantMap provider = qmlObject->property("provider").toMap();
    QVERIFY(!provider.isEmpty());
    QCOMPARE(provider["id"].toString(), QString("cool"));
    QCOMPARE(provider["displayName"].toString(), QString("Cool provider"));
    QCOMPARE(provider["iconName"].toString(), QString("general_myprovider"));

    QVariantMap service = qmlObject->property("service").toMap();
    QVERIFY(!service.isEmpty());
    QCOMPARE(service["id"].toString(), QString("coolmail"));
    QCOMPARE(service["displayName"].toString(), QString("Cool Mail"));
    QCOMPARE(service["iconName"].toString(), QString("general_myservice"));
    QCOMPARE(service["serviceTypeId"].toString(), QString("e-mail"));

    QVariantMap settings = qmlObject->property("settings").toMap();
    QVERIFY(!settings.isEmpty());
    QCOMPARE(settings["color"].toString(), QString("green"));
    QCOMPARE(settings["auto-explode-after"].toUInt(), uint(10));
    QCOMPARE(settings.count(), 2);

    QVariantMap authData = qmlObject->property("authData").toMap();
    QVERIFY(!authData.isEmpty());
    QCOMPARE(authData["method"].toString(), QString("oauth2"));
    QCOMPARE(authData["mechanism"].toString(), QString("user_agent"));
    QVariantMap parameters = authData["parameters"].toMap();
    QVERIFY(!parameters.isEmpty());
    QCOMPARE(parameters["host"].toString(), QString("coolmail.ex"));

    /* Delete the account service, and check that the QML object survives */
    delete accountService1;

    QCOMPARE(qmlObject->property("objectHandle").value<AccountService*>(),
             (AccountService*)0);
    QCOMPARE(qmlObject->property("enabled").toBool(), false);
    QCOMPARE(qmlObject->property("serviceEnabled").toBool(), false);
    QCOMPARE(qmlObject->property("displayName").toString(), QString());
    QCOMPARE(qmlObject->property("accountId").toUInt(), uint(0));

    provider = qmlObject->property("provider").toMap();
    QVERIFY(provider.isEmpty());

    service = qmlObject->property("service").toMap();
    QVERIFY(service.isEmpty());

    settings = qmlObject->property("settings").toMap();
    QVERIFY(settings.isEmpty());

    authData = qmlObject->property("authData").toMap();
    QVERIFY(authData.isEmpty());

    QVariantMap newSettings;
    newSettings.insert("color", QString("red"));
    bool ok;
    ok = QMetaObject::invokeMethod(qmlObject, "updateSettings",
                                   Q_ARG(QVariantMap, newSettings));
    QVERIFY(ok);
    ok = QMetaObject::invokeMethod(qmlObject, "updateServiceEnabled",
                                   Q_ARG(bool, true));
    QVERIFY(ok);

    delete manager;
    delete qmlObject;
}

void PluginTest::testAccountServiceUpdate()
{
    clearDb();

    /* Create one account */
    Manager *manager = new Manager(this);
    Service coolMail = manager->service("coolmail");
    Account *account = manager->createAccount("cool");
    QVERIFY(account != 0);

    account->setEnabled(true);
    account->setDisplayName("CoolAccount");
    account->selectService(coolMail);
    account->setEnabled(true);
    account->syncAndBlock();

    AccountService *accountService = new AccountService(account, coolMail);
    QVERIFY(accountService != 0);

    QQmlEngine engine;
    engine.rootContext()->setContextProperty("accountService", accountService);
    QQmlComponent component(&engine);
    component.setData("import Ubuntu.OnlineAccounts 0.1\n"
                      "AccountService { objectHandle: accountService }",
                      QUrl());
    QObject *qmlObject = component.create();
    QVERIFY(qmlObject != 0);

    QCOMPARE(qmlObject->property("objectHandle").value<AccountService*>(),
             accountService);
    QCOMPARE(qmlObject->property("autoSync").toBool(), true);
    /* Set it to the same value, just to increase coverage */
    QVERIFY(qmlObject->setProperty("autoSync", true));
    QCOMPARE(qmlObject->property("autoSync").toBool(), true);

    QVariantMap settings = qmlObject->property("settings").toMap();
    QVERIFY(!settings.isEmpty());
    QCOMPARE(settings["color"].toString(), QString("green"));
    QCOMPARE(settings["auto-explode-after"].toUInt(), uint(10));
    QCOMPARE(settings.count(), 2);

    QSignalSpy settingsChanged(qmlObject, SIGNAL(settingsChanged()));

    /* Update a couple of settings */
    QVariantMap newSettings;
    newSettings.insert("color", QString("red"));
    newSettings.insert("verified", true);
    QMetaObject::invokeMethod(qmlObject, "updateSettings",
                              Q_ARG(QVariantMap, newSettings));
    QTest::qWait(50);

    QCOMPARE(settingsChanged.count(), 1);
    settingsChanged.clear();
    settings = qmlObject->property("settings").toMap();
    QCOMPARE(settings["color"].toString(), QString("red"));
    QCOMPARE(settings["auto-explode-after"].toUInt(), uint(10));
    QCOMPARE(settings["verified"].toBool(), true);
    QCOMPARE(settings.count(), 3);

    /* Disable the service */
    QSignalSpy enabledChanged(qmlObject, SIGNAL(enabledChanged()));
    QMetaObject::invokeMethod(qmlObject, "updateServiceEnabled",
                              Q_ARG(bool, false));
    QTest::qWait(50);
    QCOMPARE(enabledChanged.count(), 1);
    enabledChanged.clear();
    QCOMPARE(qmlObject->property("enabled").toBool(), false);
    QCOMPARE(settingsChanged.count(), 1);
    settingsChanged.clear();
    QCOMPARE(qmlObject->property("serviceEnabled").toBool(), false);

    /* Disable autoSync, and change something else */
    qmlObject->setProperty("autoSync", false);
    QCOMPARE(qmlObject->property("autoSync").toBool(), false);

    newSettings.clear();
    newSettings.insert("verified", false);
    newSettings.insert("color", QVariant());
    QMetaObject::invokeMethod(qmlObject, "updateSettings",
                              Q_ARG(QVariantMap, newSettings));
    QTest::qWait(50);

    /* Nothing should have been changed yet */
    QCOMPARE(settingsChanged.count(), 0);
    settings = qmlObject->property("settings").toMap();
    QCOMPARE(settings["verified"].toBool(), true);

    /* Manually store the settings */
    account->sync();
    QTest::qWait(50);

    QCOMPARE(settingsChanged.count(), 1);
    settingsChanged.clear();
    settings = qmlObject->property("settings").toMap();
    QCOMPARE(settings["verified"].toBool(), false);
    QCOMPARE(settings["color"].toString(), QString("green"));

    delete accountService;
    delete manager;
    delete qmlObject;
}

void PluginTest::testAuthentication()
{
    clearDb();

    /* Create one account */
    Manager *manager = new Manager(this);
    Service coolMail = manager->service("coolmail");
    Service coolShare = manager->service("coolshare");
    Account *account1 = manager->createAccount("cool");
    QVERIFY(account1 != 0);

    account1->setEnabled(true);
    account1->setDisplayName("CoolAccount");
    account1->selectService(coolMail);
    account1->setEnabled(true);
    account1->selectService(coolShare);
    account1->setEnabled(false);
    account1->syncAndBlock();

    AccountService *accountService1 = new AccountService(account1, coolMail);
    QVERIFY(accountService1 != 0);

    QQmlEngine engine;
    engine.rootContext()->setContextProperty("accountService1", accountService1);
    QQmlComponent component(&engine);
    component.setData("import Ubuntu.OnlineAccounts 0.1\n"
                      "AccountService { objectHandle: accountService1 }",
                      QUrl());
    QObject *qmlObject = component.create();
    QVERIFY(qmlObject != 0);

    QSignalSpy authenticated(qmlObject,
                             SIGNAL(authenticated(const QVariantMap &)));
    QSignalSpy authenticationError(qmlObject,
        SIGNAL(authenticationError(const QVariantMap &)));

    QVariantMap sessionData;
    sessionData.insert("test", QString("OK"));
    QMetaObject::invokeMethod(qmlObject, "authenticate",
                              Q_ARG(QVariantMap, sessionData));
    QTest::qWait(50);

    QCOMPARE(authenticationError.count(), 0);
    QCOMPARE(authenticated.count(), 1);
    QVariantMap reply = authenticated.at(0).at(0).toMap();
    QVERIFY(!reply.isEmpty());
    QCOMPARE(reply["test"].toString(), QString("OK"));
    QCOMPARE(reply["host"].toString(), QString("coolmail.ex"));
    authenticated.clear();

    /* Test an authentication failure */
    sessionData.insert("errorCode", 123);
    sessionData.insert("errorMessage", QString("Failed!"));
    QMetaObject::invokeMethod(qmlObject, "authenticate",
                              Q_ARG(QVariantMap, sessionData));
    QTest::qWait(50);

    QCOMPARE(authenticationError.count(), 1);
    QCOMPARE(authenticated.count(), 0);
    QVariantMap error = authenticationError.at(0).at(0).toMap();
    QVERIFY(!error.isEmpty());
    QCOMPARE(error["code"].toInt(), 123);
    QCOMPARE(error["message"].toString(), QString("Failed!"));
    authenticationError.clear();

    /* Delete the account service, and check that the QML object survives */
    delete accountService1;

    QCOMPARE(qmlObject->property("objectHandle").value<AccountService*>(),
             (AccountService*)0);

    /* Authenticate now: we should get an error */
    sessionData.clear();
    sessionData.insert("test", QString("OK"));
    QMetaObject::invokeMethod(qmlObject, "authenticate",
                              Q_ARG(QVariantMap, sessionData));
    QTest::qWait(50);

    QCOMPARE(authenticationError.count(), 1);
    QCOMPARE(authenticated.count(), 0);
    error = authenticationError.at(0).at(0).toMap();
    QVERIFY(!error.isEmpty());
    QCOMPARE(error["message"].toString(), QString("Invalid AccountService"));
    authenticationError.clear();

    delete manager;
    delete qmlObject;
}

void PluginTest::testManagerCreate()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import Ubuntu.OnlineAccounts 0.1\n"
                      "Account { objectHandle: Manager.createAccount(\"cool\") }",
                      QUrl());
    QObject *qmlObject = component.create();
    QVERIFY(qmlObject != 0);

    QVariantMap provider = qmlObject->property("provider").toMap();
    QVERIFY(!provider.isEmpty());
    QCOMPARE(provider["id"].toString(), QString("cool"));
    QCOMPARE(provider["displayName"].toString(), QString("Cool provider"));
    QCOMPARE(provider["iconName"].toString(), QString("general_myprovider"));

    delete qmlObject;
}

void PluginTest::testManagerLoad()
{
    clearDb();

    /* Create one account */
    Manager *manager = new Manager(this);
    Account *account1 = manager->createAccount("cool");
    QVERIFY(account1 != 0);

    account1->syncAndBlock();
    QVERIFY(account1->id() != 0);

    QQmlEngine engine;
    engine.rootContext()->setContextProperty("account1id", uint(account1->id()));
    QQmlComponent component(&engine);
    component.setData("import Ubuntu.OnlineAccounts 0.1\n"
                      "Account { objectHandle: Manager.loadAccount(account1id) }",
                      QUrl());
    QObject *qmlObject = component.create();
    QVERIFY(qmlObject != 0);

    QCOMPARE(qmlObject->property("accountId").toUInt(), account1->id());

    QVariantMap provider = qmlObject->property("provider").toMap();
    QVERIFY(!provider.isEmpty());
    QCOMPARE(provider["id"].toString(), QString("cool"));
    QCOMPARE(provider["displayName"].toString(), QString("Cool provider"));
    QCOMPARE(provider["iconName"].toString(), QString("general_myprovider"));

    delete manager;
    delete qmlObject;
}

void PluginTest::testAccountInvalid()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import Ubuntu.OnlineAccounts 0.1\n"
                      "Account {}",
                      QUrl());
    QObject *qmlObject = component.create();
    QVERIFY(qmlObject != 0);

    QVERIFY(qmlObject->property("objectHandle").value<QObject*>() == 0);
    QCOMPARE(qmlObject->property("enabled").toBool(), false);
    QCOMPARE(qmlObject->property("displayName").toString(), QString());
    QCOMPARE(qmlObject->property("accountId").toUInt(), uint(0));
    QVariantMap provider = qmlObject->property("provider").toMap();
    QVERIFY(provider.isEmpty());

    qmlObject->setProperty("objectHandle", QVariant::fromValue<QObject*>(0));
    QVERIFY(qmlObject->property("objectHandle").value<QObject*>() == 0);

    bool ok;
    ok = QMetaObject::invokeMethod(qmlObject, "updateDisplayName",
                                   Q_ARG(QString, "dummy"));
    QVERIFY(ok);
    ok = QMetaObject::invokeMethod(qmlObject, "updateEnabled",
                                   Q_ARG(bool, "true"));
    QVERIFY(ok);
    ok = QMetaObject::invokeMethod(qmlObject, "sync");
    QVERIFY(ok);
    ok = QMetaObject::invokeMethod(qmlObject, "remove");
    QVERIFY(ok);

    delete qmlObject;
}

void PluginTest::testAccount()
{
    clearDb();

    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import Ubuntu.OnlineAccounts 0.1\n"
                      "Account { objectHandle: Manager.createAccount(\"cool\") }",
                      QUrl());
    QObject *qmlObject = component.create();
    QVERIFY(qmlObject != 0);

    QObject *objectHandle =
        qmlObject->property("objectHandle").value<QObject*>();
    Account *account = qobject_cast<Account*>(objectHandle);
    QVERIFY(account != 0);
    QVERIFY(account->id() == 0);

    QVariantMap provider = qmlObject->property("provider").toMap();
    QVERIFY(!provider.isEmpty());
    QCOMPARE(provider["id"].toString(), QString("cool"));

    bool ok;
    ok = QMetaObject::invokeMethod(qmlObject, "updateDisplayName",
                                   Q_ARG(QString, "new name"));
    QVERIFY(ok);
    ok = QMetaObject::invokeMethod(qmlObject, "updateEnabled",
                                   Q_ARG(bool, "true"));
    QVERIFY(ok);
    ok = QMetaObject::invokeMethod(qmlObject, "sync");
    QVERIFY(ok);

    QTest::qWait(50);

    /* Check that the changes have been recorded */
    QVERIFY(account->id() != 0);
    AccountId accountId = account->id();
    QCOMPARE(qmlObject->property("accountId").toUInt(), uint(account->id()));
    QCOMPARE(qmlObject->property("displayName").toString(), QString("new name"));
    QCOMPARE(qmlObject->property("enabled").toBool(), true);

    objectHandle =
        qmlObject->property("accountServiceHandle").value<QObject*>();
    AccountService *accountService =
        qobject_cast<AccountService*>(objectHandle);
    QVERIFY(accountService != 0);

    /* Set the same account instance on the account; just to improve coverage
     * of branches. */
    ok = qmlObject->setProperty("objectHandle",
                                QVariant::fromValue<QObject*>(account));
    QVERIFY(ok);

    /* Delete the account */
    ok = QMetaObject::invokeMethod(qmlObject, "remove");
    QVERIFY(ok);

    QTest::qWait(50);

    /* Check that the account has effectively been removed */
    Manager *manager = new Manager(this);
    Account *account1 = manager->account(accountId);
    QVERIFY(account1 == 0);

    delete qmlObject;
#if 0
    /* Create one account */
    Manager *manager = new Manager(this);
    Service coolMail = manager->service("coolmail");
    Service coolShare = manager->service("coolshare");
    Service badMail = manager->service("badmail");
    Service badShare = manager->service("badshare");
    Account *account1 = manager->createAccount("cool");
    QVERIFY(account1 != 0);

    account1->setEnabled(true);
    account1->setDisplayName("CoolAccount");
    account1->selectService(coolMail);
    account1->setEnabled(true);
    account1->selectService(coolShare);
    account1->setEnabled(false);
    account1->syncAndBlock();

    AccountService *accountService1 = new AccountService(account1, coolMail);
    QVERIFY(accountService1 != 0);

    QQmlEngine engine;
    engine.rootContext()->setContextProperty("accountService1", accountService1);
    QQmlComponent component(&engine);
    component.setData("import Ubuntu.OnlineAccounts 0.1\n"
                      "AccountService { objectHandle: accountService1 }",
                      QUrl());
    QObject *qmlObject = component.create();
    QVERIFY(qmlObject != 0);

    QCOMPARE(qmlObject->property("objectHandle").value<AccountService*>(),
             accountService1);
    QCOMPARE(qmlObject->property("enabled").toBool(), true);
    QCOMPARE(qmlObject->property("displayName").toString(),
             QString("CoolAccount"));
    QCOMPARE(qmlObject->property("accountId").toUInt(), account1->id());

    QVariantMap provider = qmlObject->property("provider").toMap();
    QVERIFY(!provider.isEmpty());
    QCOMPARE(provider["id"].toString(), QString("cool"));
    QCOMPARE(provider["displayName"].toString(), QString("Cool provider"));
    QCOMPARE(provider["iconName"].toString(), QString("general_myprovider"));

    QVariantMap service = qmlObject->property("service").toMap();
    QVERIFY(!service.isEmpty());
    QCOMPARE(service["id"].toString(), QString("coolmail"));
    QCOMPARE(service["displayName"].toString(), QString("Cool Mail"));
    QCOMPARE(service["iconName"].toString(), QString("general_myservice"));
    QCOMPARE(service["serviceTypeId"].toString(), QString("e-mail"));

    QVariantMap settings = qmlObject->property("settings").toMap();
    QVERIFY(!settings.isEmpty());
    QCOMPARE(settings["color"].toString(), QString("green"));
    QCOMPARE(settings["auto-explode-after"].toUInt(), uint(10));
    QCOMPARE(settings.count(), 2);

    QVariantMap authData = qmlObject->property("authData").toMap();
    QVERIFY(!authData.isEmpty());
    QCOMPARE(authData["method"].toString(), QString("oauth2"));
    QCOMPARE(authData["mechanism"].toString(), QString("user_agent"));
    QVariantMap parameters = authData["parameters"].toMap();
    QVERIFY(!parameters.isEmpty());
    QCOMPARE(parameters["host"].toString(), QString("coolmail.ex"));

    /* Delete the account service, and check that the QML object survives */
    delete accountService1;

    QCOMPARE(qmlObject->property("objectHandle").value<AccountService*>(),
             (AccountService*)0);
    QCOMPARE(qmlObject->property("enabled").toBool(), false);
    QCOMPARE(qmlObject->property("displayName").toString(), QString());
    QCOMPARE(qmlObject->property("accountId").toUInt(), uint(0));

    provider = qmlObject->property("provider").toMap();
    QVERIFY(provider.isEmpty());

    service = qmlObject->property("service").toMap();
    QVERIFY(service.isEmpty());

    settings = qmlObject->property("settings").toMap();
    QVERIFY(settings.isEmpty());

    authData = qmlObject->property("authData").toMap();
    QVERIFY(authData.isEmpty());

    delete manager;
    delete qmlObject;
#endif
}

QTEST_MAIN(PluginTest);
#include "tst_plugin.moc"
