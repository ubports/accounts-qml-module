/*!
 * \qmltype AccountService
 * \inqmlmodule Ubuntu.OnlineAccounts 0.1
 * \ingroup Ubuntu
 *
 * \brief Represents an instance of a service in an Online Accounts
 *
 * The AccountService element represent a service within an existing online account.
 * It can be used to obtain an authentication token to use the service it refers to.
 *
 * Currently, an AccountService is valid only if its \a objectHandle property
 * is set to a value obtained from a AccountServiceModel.
 *
 * See AccountServiceModel's documentation for usage examples.
 */

/*!
 * \qmlproperty object AccountService::objectHandle
 * An opaque handle to the underlying C++ object. Until the property is set,
 * the AccountService element is uninitialized. Similarly, if the C++ object is
 * destroyed (for instance, because the AccountServiceModel which owns it is
 * destroyed or if the account is deleted), expect the AccountService to become
 * invalid.
 */

/*!
 * \qmlproperty bool AccountService::enabled
 * This read-only property tells whether the AccountService is enabled. An
 * application shouldn't use an AccountService which is disabled.
 */

/*!
 * \qmlproperty jsobject AccountService::provider
 * An immutable object representing the provider which provides the account.
 * The returned object will have at least these members:
 * \list
 * \li \c id is the unique identified for this provider
 * \li \c displayName
 * \li \c description
 * \li \c iconName
 * \endlist
 */

/*!
 * \qmlproperty jsobject AccountService::service
 * An immutable object representing the service which this AccountService
 * instantiates.
 * The returned object will have at least these members:
 * \list
 * \li \c id is the unique identified for this service
 * \li \c displayName
 * \li \c description
 * \li \c iconName
 * \li \c serviceTypeId identifies the provided service type
 * \endlist
 */

/*!
 * \qmlproperty string AccountService::displayName
 * The account's display name (usually the user's login or ID); note that all
 * AccountService objects which work on the same online account will share the
 * same display name.
 */

/*!
 * \qmlproperty jsobject AccountService::settings
 * A dictionary of all the account service's settings. This does not
 * include the authentication settings, which are available from the
 * AccountService::authData property.
 */

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
