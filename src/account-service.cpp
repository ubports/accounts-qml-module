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


#include "account-service.h"
#include "debug.h"

#include <Accounts/AccountService>
#include <Accounts/Manager>
#include <Accounts/Provider>
#include <SignOn/AuthSession>
#include <SignOn/Identity>

using namespace OnlineAccounts;

static QVariantMap mergeMaps(const QVariantMap &map1,
                             const QVariantMap &map2)
{
    if (map1.isEmpty()) return map2;
    if (map2.isEmpty()) return map1;

    QVariantMap map = map1;
    //map2 values will overwrite map1 values for the same keys.
    QMapIterator<QString, QVariant> it(map2);
    while (it.hasNext()) {
        it.next();
        map.insert(it.key(), it.value());
    }
    return map;
}

void AccountService::ensureAccount()
{
    if (account == 0) {
        account = accountService->account();
        /* By default, the parent is the manager; but we don't want that. */
        account->setParent(this);
    }
}

void AccountService::syncIfDesired()
{
    if (m_autoSync) {
        ensureAccount();
        /* If needed, we could optimize this to call account->sync() when
         * re-entering the main loop, in order to reduce the number or writes.
         * But this would be better done in the Account class itself (and even
         * better, in libaccounts-glib). */
        account->sync();
    }
}

/*!
 * \qmltype AccountService
 * \inqmlmodule Ubuntu.OnlineAccounts 0.1
 * \ingroup Ubuntu
 *
 * \brief Represents an instance of a service in an Online Accounts
 *
 * The AccountService element represents a service within an existing online account.
 * It can be used to obtain an authentication token to use the service it refers to.
 *
 * Currently, an AccountService is valid only if its \a objectHandle property
 * is set to a value obtained from an AccountServiceModel or an Account.
 *
 * See AccountServiceModel's documentation for usage examples.
 */
AccountService::AccountService(QObject *parent):
    QObject(parent),
    accountService(0),
    account(0),
    identity(0),
    constructed(false),
    m_autoSync(true)
{
}

AccountService::~AccountService()
{
    delete account;
}

/*!
 * \qmlproperty object AccountService::objectHandle
 * An opaque handle to the underlying C++ object. Until the property is set,
 * the AccountService element is uninitialized. Similarly, if the C++ object is
 * destroyed (for instance, because the AccountServiceModel which owns it is
 * destroyed or if the account is deleted), expect the AccountService to become
 * invalid.
 */
void AccountService::setObjectHandle(QObject *object)
{
    DEBUG() << object;
    Accounts::AccountService *as =
        qobject_cast<Accounts::AccountService*>(object);
    if (Q_UNLIKELY(as == 0)) return;

    if (as == accountService) return;
    accountService = as;
    QObject::connect(accountService, SIGNAL(changed()),
                     this, SIGNAL(settingsChanged()));
    QObject::connect(accountService, SIGNAL(enabled(bool)),
                     this, SIGNAL(enabledChanged()));
    delete identity;
    identity = 0;
    delete account;
    account = 0;
    Q_EMIT objectHandleChanged();

    /* Emit the changed signals for all other properties, to make sure
     * that all bindings are updated. */
    Q_EMIT enabledChanged();
    Q_EMIT displayNameChanged();
    Q_EMIT settingsChanged();
}

QObject *AccountService::objectHandle() const
{
    return accountService;
}

/*!
 * \qmlproperty bool AccountService::enabled
 * This read-only property tells whether the AccountService is enabled. An
 * application shouldn't use an AccountService which is disabled.
 */
bool AccountService::enabled() const
{
    if (Q_UNLIKELY(accountService == 0)) return false;
    return accountService->enabled();
}

/*!
 * \qmlproperty jsobject AccountService::provider
 * An immutable object representing the provider which provides the account.
 * The returned object will have at least these members:
 * \list
 * \li \c id is the unique identified for this provider
 * \li \c displayName
 * \li \c iconName
 * \endlist
 */
QVariantMap AccountService::provider() const
{
    QVariantMap map;
    if (Q_UNLIKELY(accountService == 0)) return map;

    // TODO: use the new account->provider() method
    Accounts::Account *account = accountService->account();
    QString providerName = account->providerName();
    Accounts::Provider provider = account->manager()->provider(providerName);
    map.insert("id", provider.name());
    map.insert("displayName", provider.displayName());
    map.insert("iconName", provider.iconName());
    return map;
}

/*!
 * \qmlproperty jsobject AccountService::service
 * An immutable object representing the service which this AccountService
 * instantiates.
 * The returned object will have at least these members:
 * \list
 * \li \c id is the unique identified for this service
 * \li \c displayName
 * \li \c iconName
 * \li \c serviceTypeId identifies the provided service type
 * \endlist
 */
QVariantMap AccountService::service() const
{
    QVariantMap map;
    if (Q_UNLIKELY(accountService == 0)) return map;

    Accounts::Service service = accountService->service();
    map.insert("id", service.name());
    map.insert("displayName", service.displayName());
    map.insert("iconName", service.iconName());
    map.insert("serviceTypeId", service.serviceType());
    return map;
}


/*!
 * \qmlproperty string AccountService::displayName
 * The account's display name (usually the user's login or ID); note that all
 * AccountService objects which work on the same online account will share the
 * same display name.
 */
QString AccountService::displayName() const
{
    if (Q_UNLIKELY(accountService == 0)) return QString();
    return accountService->account()->displayName();
}

/*!
 * \qmlproperty string AccountService::accountId
 * The account's numeric ID; note that all AccountService objects which work on
 * the same online account will have the same ID.
 */
uint AccountService::accountId() const
{
    if (Q_UNLIKELY(accountService == 0)) return 0;
    return accountService->account()->id();
}

/*!
 * \qmlproperty jsobject AccountService::settings
 * A dictionary of all the account service's settings. This does not
 * include the authentication settings, which are available from the
 * AccountService::authData property.
 */
QVariantMap AccountService::settings() const
{
    QVariantMap map;
    if (Q_UNLIKELY(accountService == 0)) return map;

    foreach (const QString &key, accountService->allKeys()) {
        if (key.startsWith("auth") || key == "enabled") continue;
        map.insert(key, accountService->value(key));
    }
    return map;
}

/*!
 * \qmlproperty jsobject AccountService::authData
 * An object providing information about the authentication.
 * The returned object will have at least these members:
 * \list
 * \li \c method is the authentication method
 * \li \c mechanism is the authentication mechanism (a sub-specification of the
 *     method)
 * \li \c parameters is a dictionary of authentication parameters
 * \endlist
 */
QVariantMap AccountService::authData() const
{
    QVariantMap map;
    if (Q_UNLIKELY(accountService == 0)) return map;

    Accounts::AuthData data = accountService->authData();
    map.insert("method", data.method());
    map.insert("mechanism", data.mechanism());
    map.insert("credentialsId", data.credentialsId());
    map.insert("parameters", data.parameters());
    return map;
}

/*!
 * \qmlproperty bool AccountService::autoSync
 * This property tells whether the AccountService should invoke the
 * Account::sync() method whenever updateSettings() or updateServiceEnabled()
 * are called.
 * By default, this property is true.
 */
void AccountService::setAutoSync(bool autoSync)
{
    if (autoSync == m_autoSync) return;
    m_autoSync = autoSync;
    Q_EMIT autoSyncChanged();
}

bool AccountService::autoSync() const
{
    return m_autoSync;
}

/*!
 * \qmlmethod void AccountService::updateServiceEnabled(bool enabled)
 *
 * Enables or disables the service within the account configuration.
 * Since the \l enabled property is the combination of the global account's
 * enabledness status and the specific service's status, its value might not
 * change after this method is called.
 *
 * \sa enabled, serviceEnabled, autoSync
 */
void AccountService::updateServiceEnabled(bool enabled)
{
    if (Q_UNLIKELY(accountService == 0)) return;
    ensureAccount();
    account->selectService(accountService->service());
    account->setEnabled(enabled);
    syncIfDesired();
}

/*!
 * \qmlmethod void AccountService::updateSettings(jsobject settings)
 *
 * Change some settings. Only the settings which are present in the \a settings
 * dictionary will be changed; all others settings will not be affected.
 * To remove a settings, set its value to null.
 *
 * \sa autoSync
 */
void AccountService::updateSettings(const QVariantMap &settings)
{
    if (Q_UNLIKELY(accountService == 0)) return;

    QMapIterator<QString, QVariant> it(settings);
    while (it.hasNext()) {
        it.next();
        if (it.value().isNull()) {
            accountService->remove(it.key());
        } else {
            accountService->setValue(it.key(), it.value());
        }
    }
    syncIfDesired();
}


/*!
 * \qmlmethod void AccountService::authenticate(jsobject sessionData)
 *
 * Perform the authentication on this account. The \a sessionData dictionary is
 * optional and if not given the value of \a authData.parameters will be used.
 *
 * Each call to this method will cause either of authenticated() or
 * authenticationError() signals to be emitted at some time later. Note that
 * the authentication might involve interactions with the network or with the
 * end-user, so don't expect these signals to be emitted immediately.
 *
 * \sa authenticated(), authenticationError()
 */
void AccountService::authenticate(const QVariantMap &sessionData)
{
    DEBUG();
    if (Q_UNLIKELY(accountService == 0)) {
        QVariantMap error;
        error.insert("code", SignOn::Error::WrongState);
        error.insert("message", QLatin1String("Invalid AccountService"));
        Q_EMIT authenticationError(error);
        return;
    }

    Accounts::AuthData authData = accountService->authData();
    if (identity == 0) {
        identity =
            SignOn::Identity::existingIdentity(authData.credentialsId(), this);
    }
    if (authSession == 0) {
        authSession = identity->createSession(authData.method());
        QObject::connect(authSession, SIGNAL(response(const SignOn::SessionData&)),
                         this,
                         SLOT(onAuthSessionResponse(const SignOn::SessionData&)));
        QObject::connect(authSession, SIGNAL(error(const SignOn::Error&)),
                         this, SLOT(onAuthSessionError(const SignOn::Error&)));
    }

    QVariantMap allSessionData = mergeMaps(authData.parameters(),
                                           sessionData);
    authSession->process(allSessionData, authData.mechanism());
}

/*!
 * \qmlsignal AccountService::authenticated(jsobject reply)
 *
 * Emitted when the authentication has been successfully completed. The \a
 * reply object will contain the authentication data, which depends on the
 * authentication method used.
 */

/*!
 * \qmlsignal AccountService::authenticationError(jsobject error)
 *
 * Emitted when the authentication fails. The \a error object will contain the
 * following fields:
 * \list
 * \li \c code is a numeric error code (see Signon::Error for the meaning)
 * \li \c message is a textual description of the error, not meant for the end-user
 * \endlist
 */


void AccountService::classBegin()
{
}

void AccountService::componentComplete()
{
    constructed = true;
}

void AccountService::onAuthSessionResponse(const SignOn::SessionData &sessionData)
{
    Q_EMIT authenticated(sessionData.toMap());
}

void AccountService::onAuthSessionError(const SignOn::Error &error)
{
    QVariantMap e;
    e.insert("code", error.type());
    e.insert("message", error.message());
    Q_EMIT authenticationError(e);
}
