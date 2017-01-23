/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 ************************************************************************************/

#include "onstart.h"

#include <QApplication>

#include <KAuthAction>
#include <KAuthExecuteJob>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include <QDebug>
#include <unistd.h>

using namespace KAuth;

OnStart::OnStart(QObject* parent)
    : QObject(parent)
    , m_someFailed(false)
{
    QMetaObject::invokeMethod(this, "mountConfiguredShares", Qt::QueuedConnection);

    connect(&m_networkConfigurationManager, &QNetworkConfigurationManager::configurationChanged, this, &OnStart::mountConfiguredShares) ;
}

OnStart::~OnStart()
{

}

void OnStart::mountConfiguredShares()
{
    if(!m_networkConfigurationManager.isOnline()) {
        return;
    }

    KConfigGroup group = KSharedConfig::openConfig("samba-mounter")->group("mounts");

    m_someFailed = false;
    QStringList nameList = group.groupList();
    Q_FOREACH(const QString &name, nameList) {
        mountSamba(group.group(name));
    }

    //if it failed, don't quit, it might be because there's no network connection
    if (!m_someFailed) {
        qDebug() << "Exiting";
        qApp->quit();
    }
}

void OnStart::mountSamba(KConfigGroup group)
{
    Action readAction("org.kde.sambamounter.mount");
    readAction.setHelperId("org.kde.sambamounter");

    readAction.addArgument("uid", QString::number(getuid()));
    readAction.addArgument("ip", group.readEntry("ip", ""));
    readAction.addArgument("locale", getenv("LANG"));
    readAction.addArgument("path", getenv("PATH"));
    readAction.addArgument("sambaDir", group.readEntry("sambaDir", "").toLocal8Bit().toBase64());
    readAction.addArgument("mountPoint", group.readEntry("mountPoint", "").toLocal8Bit().toBase64());
    readAction.addArgument("username", group.readEntry("username", "none").toLocal8Bit().toBase64());
    readAction.addArgument("password", group.readEntry("password", "none").toLocal8Bit().toBase64());

    ExecuteJob* reply = readAction.execute();
    bool ret = reply->exec();

    qDebug() << reply->data()["output"];
    qDebug() << reply->data()["error"];
    if (!ret) {
        m_someFailed = true;
    }
}
