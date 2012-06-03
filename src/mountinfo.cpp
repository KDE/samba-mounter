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

#include "mountinfo.h"

#include <QtCore/QProcess>
#include <QtGui/QDesktopServices>

#include <KDebug>
#include <solid/device.h>
#include <solid/storageaccess.h>
#include <KPixmapSequenceOverlayPainter>

MountInfo::MountInfo(QWidget* parent)
: QWidget(parent)
, m_process(new QProcess)
, m_painter1(new KPixmapSequenceOverlayPainter)
, m_painter2(new KPixmapSequenceOverlayPainter)
{
    setupUi(this);

    m_painter1->setWidget(working1);
    m_painter2->setWidget(working2);

    sambaRequester->setUrl(KUrl("smb:/"));

    connect(sambaRequester, SIGNAL(urlSelected(KUrl)), SLOT(checkValidSamba(KUrl)));
    connect(sambaRequester, SIGNAL(textChanged(QString)),SLOT(checkValidSamba(QString)));
    connect(m_process, SIGNAL(finished(int)), SLOT(nameResolveFinished(int)));

    mountPointRequester->setUrl(KUrl(QDesktopServices::storageLocation(QDesktopServices::HomeLocation)));
    connect(mountPointRequester, SIGNAL(urlSelected(KUrl)), SLOT(checkMountPoint(KUrl)));
    connect(mountPointRequester, SIGNAL(textChanged(QString)), SLOT(checkMountPoint(QString)));
}

MountInfo::~MountInfo()
{
    delete m_process;
    delete m_painter1;
    delete m_painter2;
}

void MountInfo::checkValidSamba(const QString& url)
{
    kDebug() << url;
    checkValidSamba(KUrl(url));
}

void MountInfo::checkValidSamba(const KUrl& url)
{
    kDebug() << url;
    kDebug() << "Host: " << url.host();
    m_process->close();

    setResult(working1, Empty);

    m_painter1->start();

    m_process->start("nmblookup", QStringList(url.host()));
}

void MountInfo::nameResolveFinished(int status)
{
    kDebug() << "Status: " << status;

    m_painter1->stop();

    QString output = m_process->readAllStandardOutput();
    kDebug() << output;

    if (output.isEmpty()) {
        setResult(working1, Fail);
        return;
    }
    QString line = output.split("\n").at(1);
    QString ip = line.left(line.indexOf(" "));

    kDebug() << "Ip: " << ip;
    if (ip.isEmpty() || ip == "name_query") {
        setResult(working1, Fail);
        return;
    }

    setResult(working1, Ok);
}

void MountInfo::checkMountPoint(const QString& url)
{
    checkMountPoint(KUrl(url));
}

void MountInfo::checkMountPoint(const KUrl& url)
{
    QString urlPath = url.path();
    QDir dir(urlPath);

    if (dir.count() != 0) {
        setResult(working2, Fail);
        return;
    }

    QList <Solid::Device> devices = Solid::Device::listFromType(Solid::DeviceInterface::StorageAccess);

    Q_FOREACH(Solid::Device device, devices) {
        if (device.as<Solid::StorageAccess>()->filePath() == urlPath) {
            setResult(working2, Fail);
            return;
        }
    }

    setResult(working2, Ok);
    return;
}

void MountInfo::setResult(QLabel* lbl, MountInfo::Status status)
{

    switch(status)
    {
        case Empty:
            lbl->setPixmap(QPixmap());
            break;
        case Ok:
            lbl->setPixmap(QIcon::fromTheme("dialog-ok-apply").pixmap(lbl->sizeHint()));
            break;
        case Fail:
            lbl->setPixmap(QIcon::fromTheme("dialog-close").pixmap(lbl->sizeHint()));
            break;
    }
}