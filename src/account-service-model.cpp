/*!
 * \qmltype AccountServiceModel
 * \inqmlmodule Ubuntu.OnlineAccounts 0.1
 * \ingroup Ubuntu
 *
 * \brief A model of the user's Online Accounts
 *
 * The AccountServiceModel is a model representing the user's Online Accounts
 * services.
 * Please note that an Online Account can offer several different services
 * (chat, e-mail, micro-blogging, etc.); these are the items represented by
 * this model, and not the user accounts as a whole.
 * Since most applications are interested on a small subset of the user's
 * accounts, AccountServiceModel offers some filtering functionalities: it is
 * possible to restrict it to only one account provider, to a specific service
 * type (for instance, an e-mail application will probably be interested in
 * only those accounts which offer an e-mail service), or to a specific service
 * (e.g., picasa; this is often equivalent to filtering by provider and by
 * service-type, because it's rare for a provider to offer two different
 * services of the same type).
 * By default, only enabled accounts are returned. Use the \l includeDisabled
 * property to list also disabled accounts; keep in mind, though, that an
 * application should never use an account which has been disabled by the user.
 *
 * The model defines the following roles:
 * \list
 * \li \c displayName is the name of the account (usually the user's login)
 * \li \c providerName is the name of the account provider (e.g., "Google")
 * \li \c serviceName is the name of the service (e.g., "Picasa")
 * \li \c enabled
 * \li \c settings is a dictionary of the settings of the account service
 * \li \c accountService is a handle to the underlying Qt object which can be
 *     used to instantiate an AccountService from QML
 * \li \c accountId is numeric ID of the account
 * \endlist
 *
 * Examples of use:
 *
 * 1. Model of all enabled microblogging accounts:
 * \qml
 * Item {
 *     AccountServiceModel {
 *         id: accounts
 *         serviceType: "microblogging"
 *     }
 *
 *     ListView {
 *         model: accounts
 *         delegate: Text { text: model.displayName + " by " + model.providerName }
 *     }
 * }
 * \endqml
 *
 * 2. List all Facebook account services:
 * \qml
 * Item {
 *     AccountServiceModel {
 *         id: accounts
 *         provider: "facebook"
 *         includeDisabled: true
 *     }
 *
 *     ListView {
 *         model: accounts
 *         delegate: Text { text: model.serviceName + " on " + model.displayName }
 *     }
 * \endqml
 *
 * 3. List all Flickr accounts enabled for uploading:
 * \qml
 * Item {
 *     AccountServiceModel {
 *         id: accounts
 *         service: "flickr-sharing"
 *     }
 *
 *     ListView {
 *         model: accounts
 *         delegate: Rectangle {
 *             id: rect
 *
 *             Text { text: rect.model.displayName }
 *
 *             AccountService {
 *                 id: accountService
 *                 objectHandle: rect.model.accountService
 *
 *                 onAuthenticated: { console.log("Access token is " + reply.AccessToken }
 *                 onAuthenticationError: { console.log("Authentication failed, code " + error.code }
 *             }
 *
 *             MouseArea {
 *                 anchors.fill: parent
 *                 onClicked: accountService.authenticate()
 *             }
 *         }
 *     }
 * \endqml
 */

/*!
 * \qmlproperty string AccountServiceModel::provider
 * If set, the model will list only those accounts services provided by this provider.
 */

/*!
 * \qmlproperty string AccountServiceModel::serviceType
 * If set, the model will list only those accounts services supporting this
 * service type. Each provider-specific service is an instance of a generic
 * service type (such as "e-mail", "IM", etc.) which identifies the main
 * functionality provided by the service.
 */

/*!
 * \qmlproperty string AccountServiceModel::service
 * If set, the model will list only those accounts services for this
 * specific service.
 */

/*!
 * \qmlproperty bool AccountServiceModel::includeDisabled
 * If true, even disabled account services will be listed. Note that an
 * application should never use a disabled account.
 *
 * By default, this property is false.
 */
