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

#include "signon.h"

#include <QDebug>

using namespace SignOn;

Identity::Identity(const quint32 id, QObject *parent):
    QObject(parent),
    m_id(id)
{
}

Identity::~Identity()
{
}

Identity *Identity::existingIdentity(const quint32 id, QObject *parent)
{
    return new Identity(id, parent);
}

AuthSessionP Identity::createSession(const QString &methodName)
{
    return new AuthSession(m_id, methodName, this);
}

AuthSession::AuthSession(quint32 id, const QString &methodName,
                         QObject *parent):
    QObject(parent),
    m_id(id),
    m_method(methodName)
{
    responseTimer.setSingleShot(true);
    responseTimer.setInterval(10);
    QObject::connect(&responseTimer, SIGNAL(timeout()),
                     this, SLOT(respond()));
}

AuthSession::~AuthSession()
{
}

void AuthSession::process(const SessionData &sessionData,
                          const QString &mechanism)
{
    m_mechanism = mechanism;
    m_sessionData = sessionData.toMap();

    responseTimer.start();
}

void AuthSession::respond()
{
    if (m_sessionData.contains("errorCode")) {
        Error err(m_sessionData["errorCode"].toInt(),
                  m_sessionData["errorMessage"].toString());
        Q_EMIT error(err);
    } else {
        Q_EMIT response(m_sessionData);
    }
}
