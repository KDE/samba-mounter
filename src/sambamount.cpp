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
 *************************************************************************************/

#include "sambamount.h"
#include "mountinfo.h"
#include "ui_kcm.h"

#include <QStackedLayout>

#include <KAction>
#include <KAuth/Action>
#include <KAuth/ActionWatcher>
#include <QDebug>
#include <kpluginfactory.h>
#include <unistd.h>

using namespace KAuth;

K_PLUGIN_FACTORY(SambaMountFactory, registerPlugin<SambaMount>();)
K_EXPORT_PLUGIN(SambaMountFactory("sambamount", "sambamount"))

SambaMount::SambaMount(QWidget *parent, const QVariantList&)
: KCModule(SambaMountFactory::componentData(), parent)
, m_layout(new QStackedLayout)
{
    setButtons(KCModule::Help);
    m_ui = new Ui::KCMSambaMount();
    m_ui->setupUi(this);

    m_ui->mountInfo->setLayout(m_layout);
    m_ui->mountList->setIconSize(QSize(48, 48));

    connect(m_ui->remoteBtn, SIGNAL(clicked(bool)), SLOT(rmBtnClicked()));
    connect(m_ui->addBtn, SIGNAL(clicked(bool)), SLOT(addBtnClicked()));
    connect(m_ui->mountList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(currentItemChanged(QListWidgetItem*,QListWidgetItem*)));

    QMetaObject::invokeMethod(this, "initSambaMounts", Qt::QueuedConnection);
}

SambaMount::~SambaMount()
{
    QListWidgetItem* item = m_ui->mountList->currentItem();
    QWidget *widget = item->data(Qt::UserRole + 1).value<QWidget *>();
    MountInfo *info = qobject_cast<MountInfo*>(widget);
    if (info) {
        info->saveConfig();
        if (!info->id().isEmpty()) {
            mountSamba(mounts().group(info->id()));
        }
    }

    delete m_ui;
}

void SambaMount::initSambaMounts()
{
    KConfigGroup configMounts = mounts();
    if (!configMounts.groupList().isEmpty()) {
        QStringList ids = configMounts.groupList();
        Q_FOREACH(const QString &id, ids) {
            addMount(configMounts.group(id));
        }
    }

    MountInfo *widget = new MountInfo(mounts(), this);
    connect(widget, SIGNAL(mountCreated(KConfigGroup)), SLOT(mountCreated(KConfigGroup)));

    m_layout->addWidget(widget);

    m_newMountItem = new QListWidgetItem();
    m_newMountItem->setIcon(QIcon::fromTheme("applications-education-miscellaneous"));
    m_newMountItem->setText(i18n("New Share"));
    m_newMountItem->setData(Qt::UserRole, QVariant(i18n("New Share")));
    m_newMountItem->setData(Qt::UserRole + 1, QVariant::fromValue<QWidget *>(widget));

    m_ui->mountList->addItem(m_newMountItem);

    if (m_ui->mountList->count() == 0) {
        m_ui->mountList->setCurrentItem(m_newMountItem);
        return;
    }

    m_ui->mountList->setCurrentItem(m_ui->mountList->item(0));
}

void SambaMount::currentItemChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
    if (!current) {
        return;
    }

    m_ui->mountInfo->setTitle(current->text());
    m_layout->setCurrentWidget(current->data(Qt::UserRole + 1).value<QWidget *>());
    m_ui->remoteBtn->setEnabled(current != m_newMountItem);
}

void SambaMount::mountCreated(KConfigGroup group)
{
    qDebug() << "New Mount Created";
    QListWidgetItem *item = new QListWidgetItem();
    item->setIcon(QIcon::fromTheme("network-server"));
    item->setText(KUrl(group.readEntry("fullSambaUrl", "")).fileName() + " on " + group.readEntry("hostname", ""));
    item->setData(Qt::UserRole, group.readEntry("ip", ""));
    item->setData(Qt::UserRole + 1, QVariant::fromValue<QWidget *>(qobject_cast<QWidget *>(sender())));
    item->setData(Qt::UserRole + 2, group.name());

    m_ui->mountList->addItem(item);

    MountInfo *widget = new MountInfo(mounts(), this);
    connect(widget, SIGNAL(mountCreated(KConfigGroup)), SLOT(mountCreated(KConfigGroup)));

    m_layout->addWidget(widget);

    m_newMountItem->setData(Qt::UserRole + 1, QVariant::fromValue<QWidget *>(widget));

    int row = m_ui->mountList->row(m_newMountItem);

    m_ui->mountList->takeItem(row);
    m_ui->mountList->insertItem(row, item);
    m_ui->mountList->addItem(m_newMountItem);
    m_ui->mountList->setCurrentItem(item);

    mountSamba(group);
}

void SambaMount::addBtnClicked()
{
    m_ui->mountList->setCurrentItem(m_newMountItem, QItemSelectionModel::SelectCurrent);
}

void SambaMount::rmBtnClicked()
{
    QListWidgetItem *item = m_ui->mountList->currentItem();
    if (item == m_newMountItem) {
        return;
    }

    QString groupName = item->data(Qt::UserRole + 2).toString();
    umountSamba(groupName);
    QDir().rmdir(mounts().group(groupName).readEntry("mountPoint", ""));
    mounts().deleteGroup(groupName);

    QWidget *widget = item->data(Qt::UserRole + 1).value<QWidget *>();
    item->setData(Qt::UserRole + 1, 0);
    m_layout->removeWidget(widget);
    m_ui->mountList->removeItemWidget(item);

    delete widget;
    delete item;
}

KConfigGroup SambaMount::mounts()
{
    return KSharedConfig::openConfig("samba-mounter")->group("mounts");
}

void SambaMount::addMount(KConfigGroup group)
{
    MountInfo *info = new MountInfo(mounts(), this);
    info->setConfigGroup(group.name());
    m_layout->addWidget(info);

    QListWidgetItem *item = new QListWidgetItem();
    item->setIcon(QIcon::fromTheme("network-server"));
    item->setText(KUrl(group.readEntry("fullSambaUrl", "")).fileName() + " on " + group.readEntry("hostname", ""));
    item->setData(Qt::UserRole, group.readEntry("ip", ""));
    item->setData(Qt::UserRole + 1, QVariant::fromValue<QWidget *>(info));
    item->setData(Qt::UserRole + 2, group.name());

    m_ui->mountList->addItem(item);
}

void SambaMount::mountSamba(KConfigGroup group)
{
     Action readAction("org.kde.sambamounter.mount");
    readAction.setHelperID("org.kde.sambamounter");

    readAction.addArgument("uid", QString::number(getuid()));
    readAction.addArgument("ip", group.readEntry("ip", ""));
    readAction.addArgument("locale", getenv("LANG"));
    readAction.addArgument("sambaDir", group.readEntry("sambaDir", "").toLocal8Bit().toBase64());
    readAction.addArgument("mountPoint", group.readEntry("mountPoint", "").toLocal8Bit().toBase64());
    ActionReply reply = readAction.execute();

    qDebug() << reply.data()["output"];
}

void SambaMount::umountSamba(const QString& name)
{
    KConfigGroup group = mounts().group(name);
    Action readAction("org.kde.sambamounter.umount");
    readAction.setHelperID("org.kde.sambamounter");

    readAction.addArgument("locale", getenv("LANG"));
    readAction.addArgument("mountPoint", group.readEntry("mountPoint", "").toLocal8Bit().toBase64());
    ActionReply reply = readAction.execute();

}

#include "sambamount.moc"

