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

#include "account-service-model.h"
#include "debug.h"

#include <Accounts/AccountService>
#include <Accounts/Manager>

using namespace OnlineAccounts;

static bool sortByProviderAndDisplayName(const Accounts::AccountService *as1,
                                         const Accounts::AccountService *as2)
{
    const Accounts::Account *a1 = as1->account();
    const Accounts::Account *a2 = as2->account();

    int diff = QString::compare(a1->providerName(), a2->providerName());
    if (diff < 0) return true;
    if (diff > 0) return false;

    diff = QString::compare(a1->displayName(), a2->displayName());
    if (diff < 0) return true;
    if (diff > 0) return false;

    // last, sort by service
    return as1->service().name() < as2->service().name();
}

namespace OnlineAccounts {

typedef QList<Accounts::AccountService *> AccountServices;

class AccountServiceModelPrivate: public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(AccountServiceModel)

public:
    AccountServiceModelPrivate(AccountServiceModel *model);
    ~AccountServiceModelPrivate();

    void queueUpdate();
    AccountServices listAccountServices(Accounts::AccountId accountId) const;

    void addServicesFromAccount(Accounts::AccountId id);
    void watchItems(const AccountServices &items);
    void addItems(const AccountServices &added);
    void removeItems(const AccountServices &removed);
    void sortItems();

public Q_SLOTS:
    void update();
    void onAccountCreated(Accounts::AccountId id);
    void onAccountRemoved(Accounts::AccountId id);
    void onAccountServiceEnabled(bool enabled);

private:
    mutable AccountServiceModel *q_ptr;
    QHash<int, QByteArray> roleNames;
    bool componentCompleted;
    bool updateQueued;
    bool accountIdChanged;
    bool providerChanged;
    bool serviceTypeChanged;
    bool serviceChanged;
    bool includeDisabled;
    Accounts::AccountId accountId;
    QString providerId;
    QString serviceTypeId;
    QString serviceId;
    Accounts::Manager *manager;
    AccountServices allItems;
    AccountServices modelItems;
    bool (*sortFunction)(const Accounts::AccountService *as1,
                         const Accounts::AccountService *as2);
};
}; // namespace

AccountServiceModelPrivate::AccountServiceModelPrivate(AccountServiceModel *model):
    QObject(model),
    q_ptr(model),
    componentCompleted(false),
    updateQueued(true),
    accountIdChanged(false),
    providerChanged(false),
    serviceTypeChanged(false),
    serviceChanged(false),
    includeDisabled(false),
    accountId(0),
    manager(0),
    sortFunction(sortByProviderAndDisplayName)
{
}

AccountServiceModelPrivate::~AccountServiceModelPrivate()
{
    qDeleteAll(allItems);
    delete manager;
    manager = 0;
}

void AccountServiceModelPrivate::queueUpdate()
{
    if (updateQueued) return;

    updateQueued = true;
    QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);
}

AccountServices
AccountServiceModelPrivate::listAccountServices(Accounts::AccountId accountId) const
{
    AccountServices ret;

    if (this->accountId != 0 && this->accountId != accountId) return ret;

    Accounts::Account *account = manager->account(accountId);
    if (Q_UNLIKELY(account == 0)) return ret;

    if (!providerId.isEmpty() && account->providerName() != providerId)
        return ret;

    foreach (Accounts::Service service, account->services()) {
        if (!serviceId.isEmpty() && service.name() != serviceId) continue;
        ret.append(new Accounts::AccountService(account, service));
    }

    return ret;
}

void
AccountServiceModelPrivate::addServicesFromAccount(Accounts::AccountId id)
{
    AccountServices accountServices = listAccountServices(id);
    watchItems(accountServices);

    AccountServices newModelItems;

    foreach (Accounts::AccountService *accountService, accountServices) {
        if (includeDisabled || accountService->enabled())
            newModelItems.append(accountService);
    }

    qSort(newModelItems.begin(), newModelItems.end(), sortFunction);
    addItems(newModelItems);
}

void AccountServiceModelPrivate::watchItems(const AccountServices &items)
{
    foreach (Accounts::AccountService *accountService, items) {
        QObject::connect(accountService, SIGNAL(enabled(bool)),
                         this, SLOT(onAccountServiceEnabled(bool)));
    }
    allItems.append(items);
}

/*
 * NOTE: @added must be already sorted!
 */
void AccountServiceModelPrivate::addItems(const AccountServices &added)
{
    Q_Q(AccountServiceModel);

    AccountServices newModelItems = modelItems;

    QModelIndex root;

    QMap<int,int> addedIndexes;
    foreach (Accounts::AccountService *accountService, added) {
        // Find where the item should be inserted
        AccountServices::iterator i =
            qLowerBound(modelItems.begin(), modelItems.end(),
                        accountService, sortFunction);
        int index = i - modelItems.begin();
        addedIndexes[index]++;
    }

    // update the list
    int inserted = 0;
    for (QMap<int,int>::const_iterator i = addedIndexes.constBegin();
         i != addedIndexes.constEnd();
         i++) {
        int start = i.key();
        int count = i.value();
        q->beginInsertRows(root,
                           start + inserted,
                           start + inserted + count - 1);
        for (int j = 0; j < count; j++) {
            Accounts::AccountService *accountService = added.at(inserted + j);
            modelItems.insert(start + inserted + j, accountService);
        }
        q->endInsertRows();
        inserted += count;
    }
}

void
AccountServiceModelPrivate::removeItems(const AccountServices &removed)
{
    Q_Q(AccountServiceModel);

    QModelIndex root;

    QList<int> removedIndexes;
    foreach (Accounts::AccountService *accountService, removed) {
        int index = modelItems.indexOf(accountService);
        if (Q_UNLIKELY(index < 0)) {
            qWarning() << "Item already deleted!" << accountService;
            continue;
        }
        removedIndexes.append(index);
    }
    // sort the indexes from highest to lower, and start updating the list
    qSort(removedIndexes.begin(), removedIndexes.end(), qGreater<int>());
    int first = -1;
    int last = -1;
    foreach (int index, removedIndexes) {
        // Check whether the indexes are contiguous
        if (index != first - 1) {
            // if we have a valid range, update the list for that range
            if (first != -1) {
                q->beginRemoveRows(root, first, last);
                for (int i = last; i >= first; i--)
                    modelItems.removeAt(i);
                q->endRemoveRows();
            }
            // a new range starts
            last = index;
        }
        first = index;
    }
    if (first != -1) {
        q->beginRemoveRows(root, first, last);
        for (int i = last; i >= first; i--)
            modelItems.removeAt(i);
        q->endRemoveRows();
    }
}

void AccountServiceModelPrivate::sortItems()
{
    qSort(modelItems.begin(), modelItems.end(), sortFunction);
}

void AccountServiceModelPrivate::update()
{
    Q_Q(AccountServiceModel);

    updateQueued = false;
    DEBUG();

    q->beginRemoveRows(QModelIndex(), 0, modelItems.count() - 1);
    modelItems.clear();
    qDeleteAll(allItems);
    allItems.clear();
    q->endRemoveRows();

    if (serviceTypeChanged || manager == 0) {
        delete manager;
        if (serviceTypeId.isEmpty()) {
            manager = new Accounts::Manager(this);
        } else {
            manager = new Accounts::Manager(serviceTypeId, this);
        }
        QObject::connect(manager, SIGNAL(accountCreated(Accounts::AccountId)),
                         this, SLOT(onAccountCreated(Accounts::AccountId)));
        QObject::connect(manager, SIGNAL(accountRemoved(Accounts::AccountId)),
                         this, SLOT(onAccountRemoved(Accounts::AccountId)));
    }

    foreach (Accounts::AccountId accountId, manager->accountList()) {
        AccountServices accountServices = listAccountServices(accountId);
        watchItems(accountServices);
    }

    AccountServices newModelItems;
    if (includeDisabled) {
        newModelItems = allItems;
    } else {
        foreach (Accounts::AccountService *accountService, allItems) {
            if (accountService->enabled())
                newModelItems.append(accountService);
        }
    }

    q->beginInsertRows(QModelIndex(), 0, newModelItems.count() - 1);
    modelItems = newModelItems;
    sortItems();
    q->endInsertRows();

    accountIdChanged = false;
    providerChanged = false;
    serviceTypeChanged = false;
    serviceChanged = false;
}

void AccountServiceModelPrivate::onAccountCreated(Accounts::AccountId id)
{
    DEBUG() << id;
    addServicesFromAccount(id);
}

void AccountServiceModelPrivate::onAccountRemoved(Accounts::AccountId id)
{
    DEBUG() << id;
    AccountServices removed;
    foreach (Accounts::AccountService *accountService, allItems) {
        if (accountService->account()->id() == id) {
            removed.append(accountService);
        }
    }

    /* Remove the items from the model */
    removeItems(removed);

    /* Last, delete the items */
    foreach (Accounts::AccountService *accountService, removed) {
        allItems.removeOne(accountService);
        delete accountService;
    }
}

void AccountServiceModelPrivate::onAccountServiceEnabled(bool enabled)
{
    Q_Q(AccountServiceModel);

    Accounts::AccountService *accountService =
        qobject_cast<Accounts::AccountService *>(sender());

    DEBUG() << enabled;
    int row = modelItems.indexOf(accountService);
    if (row > 0) {
        QModelIndex index = q->index(row);
        q->dataChanged(index, index);
    }

    if (!includeDisabled) {
        /* The item might need to be added or removed from the model */
        AccountServices accountServices;
        accountServices.append(accountService);
        if (row < 0 && enabled) {
            addItems(accountServices);
        } else if (row >= 0 && !enabled) {
            removeItems(accountServices);
        }
    }
}

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

AccountServiceModel::AccountServiceModel(QObject *parent):
    QAbstractListModel(parent),
    d_ptr(new AccountServiceModelPrivate(this))
{
    Q_D(AccountServiceModel);
    d->roleNames[DisplayNameRole] = "displayName";
    d->roleNames[ProviderNameRole] = "providerName";
    d->roleNames[ServiceNameRole] = "serviceName";
    d->roleNames[EnabledRole] = "enabled";
    d->roleNames[AccountServiceRole] = "accountService";
    d->roleNames[AccountIdRole] = "accountId";

    QObject::connect(this, SIGNAL(rowsInserted(const QModelIndex &,int,int)),
                     this, SIGNAL(countChanged()));
    QObject::connect(this, SIGNAL(rowsRemoved(const QModelIndex &,int,int)),
                     this, SIGNAL(countChanged()));
}

AccountServiceModel::~AccountServiceModel()
{
}

void AccountServiceModel::classBegin()
{
}

void AccountServiceModel::componentComplete()
{
    Q_D(AccountServiceModel);
    d->componentCompleted = true;
    d->update();
}

/*!
 * \qmlproperty quint32 AccountServiceModel::accountId
 * If set, the model will list only those accounts services available in the
 * given account.
 */
void AccountServiceModel::setAccountId(quint32 accountId)
{
    Q_D(AccountServiceModel);

    if (accountId == d->accountId) return;
    d->accountId = accountId;
    d->accountIdChanged = true;
    d->queueUpdate();
    Q_EMIT accountIdChanged();
}

quint32 AccountServiceModel::accountId() const
{
    Q_D(const AccountServiceModel);
    return d->accountId;
}

/*!
 * \qmlproperty string AccountServiceModel::provider
 * If set, the model will list only those accounts services provided by this provider.
 */
void AccountServiceModel::setProvider(const QString &providerId)
{
    Q_D(AccountServiceModel);

    if (providerId == d->providerId) return;
    d->providerId = providerId;
    d->providerChanged = true;
    d->queueUpdate();
    Q_EMIT providerChanged();
}

QString AccountServiceModel::provider() const
{
    Q_D(const AccountServiceModel);
    return d->providerId;
}

/*!
 * \qmlproperty string AccountServiceModel::serviceType
 * If set, the model will list only those accounts services supporting this
 * service type. Each provider-specific service is an instance of a generic
 * service type (such as "e-mail", "IM", etc.) which identifies the main
 * functionality provided by the service.
 */
void AccountServiceModel::setServiceType(const QString &serviceTypeId)
{
    Q_D(AccountServiceModel);

    if (serviceTypeId == d->serviceTypeId) return;
    d->serviceTypeId = serviceTypeId;
    d->serviceTypeChanged = true;
    d->queueUpdate();
    Q_EMIT serviceTypeChanged();
}

QString AccountServiceModel::serviceType() const
{
    Q_D(const AccountServiceModel);
    return d->serviceTypeId;
}

/*!
 * \qmlproperty string AccountServiceModel::service
 * If set, the model will list only those accounts services for this
 * specific service.
 */
void AccountServiceModel::setService(const QString &serviceId)
{
    Q_D(AccountServiceModel);

    if (serviceId == d->serviceId) return;
    d->serviceId = serviceId;
    d->serviceChanged = true;
    d->queueUpdate();
    Q_EMIT serviceChanged();
}

QString AccountServiceModel::service() const
{
    Q_D(const AccountServiceModel);
    return d->serviceId;
}

/*!
 * \qmlproperty bool AccountServiceModel::includeDisabled
 * If true, even disabled account services will be listed. Note that an
 * application should never use a disabled account.
 *
 * By default, this property is false.
 */
void AccountServiceModel::setIncludeDisabled(bool includeDisabled)
{
    Q_D(AccountServiceModel);

    if (includeDisabled == d->includeDisabled) return;
    d->includeDisabled = includeDisabled;
    d->queueUpdate();
    Q_EMIT includeDisabledChanged();
}

bool AccountServiceModel::includeDisabled() const
{
    Q_D(const AccountServiceModel);
    return d->includeDisabled;
}

/*
 * \qmlmethod variant AccountServiceModel::get(int row, string roleName)
 *
 * Returns the data at \a row for the role \a roleName.
 */
QVariant AccountServiceModel::get(int row, const QString &roleName) const
{
    int role = roleNames().key(roleName.toLatin1(), -1);
    return data(index(row), role);
}

int AccountServiceModel::rowCount(const QModelIndex &parent) const
{
    Q_D(const AccountServiceModel);
    Q_UNUSED(parent);
    return d->modelItems.count();
}

QVariant AccountServiceModel::data(const QModelIndex &index, int role) const
{
    Q_D(const AccountServiceModel);

    if (index.row() >= d->modelItems.count()) return QVariant();

    const Accounts::AccountService *accountService = d->modelItems.at(index.row());
    QVariant ret;

    switch (role) {
    case Qt::DisplayRole:
        ret = QString("%1 - %2").
            arg(accountService->account()->displayName()).
            arg(accountService->service().displayName());
        break;
    case DisplayNameRole:
        ret = accountService->account()->displayName();
        break;
    case ProviderNameRole:
        // TODO: use the new account->provider() method
        {
            QString providerName = accountService->account()->providerName();
            Accounts::Provider provider = d->manager->provider(providerName);
            ret = provider.displayName();
        }
        break;
    case ServiceNameRole:
        ret = accountService->service().displayName();
        break;
    case EnabledRole:
        ret = accountService->enabled();
        break;
    case AccountServiceRole:
        ret = QVariant(QMetaType::QObjectStar, &accountService);
        break;
    case AccountIdRole:
        ret = accountService->account()->id();
        break;
    }

    return ret;
}

QHash<int, QByteArray> AccountServiceModel::roleNames() const
{
    Q_D(const AccountServiceModel);
    return d->roleNames;
}

#include "account-service-model.moc"
