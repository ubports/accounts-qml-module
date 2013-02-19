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

#ifndef ONLINE_ACCOUNTS_ACCOUNT_SERVICE_H
#define ONLINE_ACCOUNTS_ACCOUNT_SERVICE_H

#include <QObject>
#include <QPointer>
#include <QQmlParserStatus>
#include <QVariantMap>

namespace Accounts {
    class AccountService;
};

namespace SignOn {
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

    Q_INVOKABLE void authenticate(const QVariantMap &sessionData);

    // reimplemented virtual methods
    void classBegin();
    void componentComplete();

Q_SIGNALS:
    void objectHandleChanged();
    void enabledChanged();
    void displayNameChanged();
    void settingsChanged();

    void authenticated(const QVariantMap &reply);
    void authenticationError(const QVariantMap &error);

private Q_SLOTS:
    void onAuthSessionResponse(const SignOn::SessionData &sessionData);
    void onAuthSessionError(const SignOn::Error &error);

private:
    QPointer<Accounts::AccountService> accountService;
    SignOn::Identity *identity;
    bool constructed;
};

}; // namespace

#endif // ONLINE_ACCOUNTS_ACCOUNT_SERVICE_H
