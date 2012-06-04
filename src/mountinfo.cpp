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
#include <KColorScheme>
#include <solid/device.h>
#include <solid/storageaccess.h>
#include <KPixmapSequenceOverlayPainter>
#include <KLineEdit>

MountInfo::MountInfo(KConfigGroup config, QWidget* parent)
: QWidget(parent)
, m_share(false)
, m_mount(false)
, m_editMode(false)
, m_process(new QProcess)
, m_config(config)
, m_painter1(new KPixmapSequenceOverlayPainter)
, m_painter2(new KPixmapSequenceOverlayPainter)
{
    setupUi(this);

    m_painter1->setWidget(working1);
    m_painter2->setWidget(working2);

    KColorScheme scheme(QPalette::Normal);
    KColorScheme::ForegroundRole role;

    QPalette palette(error->palette());
    palette.setColor(QPalette::Foreground, scheme.foreground(KColorScheme::NegativeText).color());

    error->setPalette(palette);

    sambaRequester->setUrl(KUrl("smb:/"));

    connect(sambaRequester, SIGNAL(urlSelected(KUrl)), SLOT(checkValidSamba(KUrl)));
    connect(sambaRequester, SIGNAL(textChanged(QString)),SLOT(checkValidSamba(QString)));
    connect(m_process, SIGNAL(finished(int)), SLOT(nameResolveFinished(int)));

    connect(shareName, SIGNAL(textChanged(QString)), SLOT(checkMountPoint(QString)));

    connect(button, SIGNAL(clicked(bool)), SLOT(buttonClicked()));
}

MountInfo::~MountInfo()
{
    delete m_process;
    delete m_painter1;
    delete m_painter2;
}

void MountInfo::setConfigGroup(const QString& name)
{
    sambaRequester->setUrl(m_config.group(name).readEntry("fullSambaUrl"));
    shareName->setText(m_config.group(name).readEntry("mountName"));

    setEditMode();
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
    kDebug() << "Dir: " << url.directory(KUrl::AppendTrailingSlash) + url.fileName();
    m_process->close();

    m_share = false;
    setResult(working1, Empty);

    m_host = url.host();
    m_fullSambaUrl = url.url();
    m_sambaDir = url.directory(KUrl::AppendTrailingSlash) + url.fileName();

    if (m_sambaDir.isEmpty() || m_sambaDir == "/") {
        error->setText(i18n("You must select a folder"));
        setResult(working1, Fail);
        Q_EMIT checkDone();
        return;
    }

    m_painter1->start();
    m_process->start("nmblookup", QStringList(m_host));
}

void MountInfo::nameResolveFinished(int status)
{
    kDebug() << "Status: " << status;

    m_painter1->stop();

    QString output = m_process->readAllStandardOutput();
    kDebug() << output;

    if (output.isEmpty()) {
        error->setText(i18n("Couldn't get the server IP"));
        setResult(working1, Fail);
        Q_EMIT checkDone();
        return;
    }

    QString line = output.split("\n").at(1);
    QString ip = line.left(line.indexOf(" "));

    kDebug() << "Ip: " << ip;
    if (ip.isEmpty() || ip == "name_query") {
        error->setText(i18n("Couldn't get the server IP"));
        setResult(working1, Fail);
        Q_EMIT checkDone();
        return;
    }

    m_ip = ip;
    m_share = true;
    setResult(working1, Ok);
    Q_EMIT checkDone();
    error->setText("");

    autoFillMountName();
}

bool MountInfo::checkMountPoint(const QString& name)
{
    m_mountName = name;
    KUrl url(QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
    url.addPath("Network");
    url.addPath(name);

    return checkMountPoint(KUrl(url));
}

bool MountInfo::checkMountPoint(const KUrl& url)
{
    kDebug() << url;
    QString urlPath = url.path();
    QDir dir(urlPath);

    KUrl networkDir(QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
    networkDir.addPath("Network");
    dir.mkdir(networkDir.path());

    m_mount = false;
    m_mountPoint = url.path();

    if (dir.exists() && dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries).count() != 0) {
        error->setText(i18n("Please, choose another name"));
        setResult(working2, Fail);
        Q_EMIT checkDone();
        return false;
    }

    QList <Solid::Device> devices = Solid::Device::listFromType(Solid::DeviceInterface::StorageAccess);

    Q_FOREACH(Solid::Device device, devices) {
        if (device.as<Solid::StorageAccess>()->filePath() == urlPath) {
            error->setText(i18n("Please, choose another name"));
            setResult(working2, Fail);
            Q_EMIT checkDone();
            return false;
        }
    }

    m_mount = true;
    setResult(working2, Ok);
    error->setText("");
    Q_EMIT checkDone();

    return true;
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

void MountInfo::buttonClicked()
{
    checkMountPoint(shareName->text());
    checkValidSamba(sambaRequester->lineEdit()->text());

    connect(this, SIGNAL(checkDone()), SLOT(mountIsValid()));
}

void MountInfo::mountIsValid()
{
    if (!m_mount || !m_share) {
        return;
    }

    kDebug() << "Saving mount";

    KConfigGroup group = m_config.group(m_fullSambaUrl + "-" + m_mountPoint);

    group.writeEntry("ip", m_ip);
    group.writeEntry("hostname", m_host);
    group.writeEntry("mountPoint", m_mountPoint);
    group.writeEntry("sambaDir", m_sambaDir);
    group.writeEntry("fullSambaUrl", m_fullSambaUrl);
    group.writeEntry("mountName", m_mountName);

    group.sync();

    QDir().mkdir(m_mountPoint);

    if (m_editMode) {
        Q_EMIT mountEditted(group);
        return;
    }

    Q_EMIT mountCreated(group);
    setEditMode();
}

void MountInfo::setEditMode()
{
    setResult(working1, Empty);
    setResult(working2, Empty);

    m_editMode = true;
}

void MountInfo::autoFillMountName()
{
    if (!shareName->text().isEmpty()) {
        return;
    }

    QString name = KUrl(sambaRequester->lineEdit()->text()).host();

    if (!checkMountPoint(name)) {
        setResult(working2, Empty);
        error->setText("");
        return;
    }

    shareName->setText(name);
}