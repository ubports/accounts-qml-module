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

#ifndef ONLINE_ACCOUNTS_ACCOUNT_SERVICE_H
#define ONLINE_ACCOUNTS_ACCOUNT_SERVICE_H

#include <QObject>
#include <QPointer>
#include <QQmlParserStatus>
#include <QVariantMap>

namespace Accounts {
    class AccountService;
    class Account;
};

namespace SignOn {
    class AuthSession;
    class Error;
    class Identity;
    class SessionData;
};

namespace OnlineAccounts {

class AccountService: public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QObject *objectHandle READ objectHandle \
               WRITE setObjectHandle NOTIFY objectHandleChanged)
    Q_PROPERTY(bool enabled READ enabled NOTIFY enabledChanged)
    Q_PROPERTY(QVariantMap provider READ provider NOTIFY objectHandleChanged)
    Q_PROPERTY(QVariantMap service READ service NOTIFY objectHandleChanged)
    Q_PROPERTY(QString displayName READ displayName NOTIFY displayNameChanged)
    Q_PROPERTY(uint accountId READ accountId NOTIFY objectHandleChanged)
    Q_PROPERTY(QVariantMap settings READ settings NOTIFY settingsChanged)
    Q_PROPERTY(QVariantMap authData READ authData NOTIFY settingsChanged)
    Q_PROPERTY(bool autoSync READ autoSync WRITE setAutoSync \
               NOTIFY autoSyncChanged)

public:
    AccountService(QObject *parent = 0);
    ~AccountService();

    void setObjectHandle(QObject *object);
    QObject *objectHandle() const;

    bool enabled() const;
    QVariantMap provider() const;
    QVariantMap service() const;
    QString displayName() const;
    uint accountId() const;
    QVariantMap settings() const;
    QVariantMap authData() const;

    void setAutoSync(bool autoSync);
    bool autoSync() const;

    Q_INVOKABLE void authenticate(const QVariantMap &sessionData);
    Q_INVOKABLE void updateServiceEnabled(bool enabled);
    Q_INVOKABLE void updateSettings(const QVariantMap &settings);

    // reimplemented virtual methods
    void classBegin();
    void componentComplete();

Q_SIGNALS:
    void objectHandleChanged();
    void enabledChanged();
    void displayNameChanged();
    void settingsChanged();
    void autoSyncChanged();

    void authenticated(const QVariantMap &reply);
    void authenticationError(const QVariantMap &error);

private Q_SLOTS:
    void onAuthSessionResponse(const SignOn::SessionData &sessionData);
    void onAuthSessionError(const SignOn::Error &error);

private:
    void ensureAccount();
    void syncIfDesired();

private:
    QPointer<Accounts::AccountService> accountService;
    Accounts::Account *account;
    SignOn::Identity *identity;
    QPointer<SignOn::AuthSession> authSession;
    bool constructed;
    bool m_autoSync;
};

}; // namespace

#endif // ONLINE_ACCOUNTS_ACCOUNT_SERVICE_H
