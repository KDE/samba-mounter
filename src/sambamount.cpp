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

#include <KDebug>
#include <kpluginfactory.h>

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
    m_newMountItem->setText("New Mount");
    m_newMountItem->setData(Qt::UserRole, QVariant("New Mount"));
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
//     m_ui->remoteBtn->setEnabled(current != m_newAccountItem);`
}

void SambaMount::mountCreated(KConfigGroup group)
{
    kDebug() << "New Mount Created";
    QListWidgetItem *item = new QListWidgetItem();
    item->setIcon(QIcon::fromTheme("network-server"));
    item->setText(group.readEntry("ip", ""));
    item->setData(Qt::UserRole, group.readEntry("ip", ""));
    item->setData(Qt::UserRole + 1, QVariant::fromValue<QWidget *>(qobject_cast<QWidget *>(sender())));

    m_ui->mountList->addItem(item);

    MountInfo *widget = new MountInfo(mounts(), this);
    connect(widget, SIGNAL(mountCreated(KConfigGroup)), SLOT(mountCreated(KConfigGroup)));

    m_layout->addWidget(widget);

    m_newMountItem->setData(Qt::UserRole + 1, QVariant::fromValue<QWidget *>(widget));

    m_ui->mountList->setCurrentItem(item);
}

void SambaMount::addBtnClicked()
{
    m_ui->mountList->setCurrentItem(m_newMountItem, QItemSelectionModel::SelectCurrent);
}

void SambaMount::rmBtnClicked()
{

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
    item->setText(group.readEntry("ip", ""));
    item->setData(Qt::UserRole, group.readEntry("ip", ""));
    item->setData(Qt::UserRole + 1, QVariant::fromValue<QWidget *>(info));

    m_ui->mountList->addItem(item);
}
#include "sambamount.moc"

