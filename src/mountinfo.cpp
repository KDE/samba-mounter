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
#include <QDir>

#include <QDebug>
#include <KColorScheme>
#include <solid/device.h>
#include <solid/storageaccess.h>
#include <KPixmapSequenceOverlayPainter>
#include <KLineEdit>
#include <KDesktopFile>
#include "kpasswdserver_interface.h"

MountInfo::MountInfo(OrgKdeKPasswdServerInterface* iface, KConfigGroup config, QWidget* parent)
: QWidget(parent)
, m_share(false)
, m_mount(false)
, m_editMode(false)
, m_process(new QProcess)
, m_config(config)
, m_painter1(new KPixmapSequenceOverlayPainter)
, m_painter2(new KPixmapSequenceOverlayPainter)
, m_interface(iface)
{
    setupUi(this);

    m_painter1->setWidget(working1);
    m_painter2->setWidget(working2);

    KColorScheme scheme(QPalette::Normal);

    QPalette palette(error->palette());
    palette.setColor(QPalette::Foreground, scheme.foreground(KColorScheme::NegativeText).color());

    error->setPalette(palette);

    sambaRequester->setUrl(QUrl("smb://"));
    connect(sambaRequester, SIGNAL(urlSelected(QUrl)), SLOT(checkValidSamba(QUrl)));
    connect(sambaRequester, SIGNAL(textChanged(QString)),SLOT(checkValidSamba(QString)));

    connect(m_process, SIGNAL(finished(int)), SLOT(nameResolveFinished(int)));

    connect(shareName, SIGNAL(textChanged(QString)), SLOT(checkMountPoint(QString)));

    connect(button, SIGNAL(clicked(bool)), SLOT(buttonClicked()));

    connect(m_interface, &OrgKdeKPasswdServerInterface::checkAuthInfoAsyncResult, this, &MountInfo::authInfoReceived);
}

MountInfo::~MountInfo()
{
    delete m_process;
    delete m_painter1;
    delete m_painter2;
}

QString MountInfo::id() const
{
    return m_id;
}

void MountInfo::setConfigGroup(const QString& name)
{
    m_id = name;
    sambaRequester->setUrl(m_config.group(name).readEntry("fullSambaUrl"));
    shareName->setText(m_config.group(name).readEntry("mountName"));
    username->setText(m_config.group(name).readEntry("username"));
    password->setText(m_config.group(name).readEntry("password"));

    setEditMode();
}

void MountInfo::checkValidSamba(const QString& url)
{
    checkValidSamba(QUrl(url));
}

void MountInfo::checkValidSamba(const QUrl &url)
{
    KIO::AuthInfo info;
    info.url = url;
    m_interface->checkAuthInfoAsync(info, window()->winId(), 0);

    qDebug() << "check valid url. Host: " << url.host() << url;
    m_process->close();

    m_share = false;
    setResult(working1, Empty);

    m_fullSambaUrl = url.url();
    //If path and file are the same thing for example smb://foo/public
    m_sambaDir = url.path();

    qDebug() << "fullSambaUrl" << m_fullSambaUrl << "sambaDir:" << m_sambaDir;
    if (m_sambaDir.isEmpty() || m_sambaDir == "/") {
        error->setText(i18n("You must select a folder"));
        setResult(working1, Fail);
        return;
    }

    m_host = url.host();
    if (isIp(m_host))
        checkValidIp(m_host);
    else
        checkValidHost(m_host);
}

bool MountInfo::isIp(const QString& host)
{
    QStringList split = host.split('.');
    if (split.count() != 4) {
        return false;
    }

    Q_FOREACH(const QString &oc, split) {
        if (oc.toInt() > 254) {
            return false;
        }
    }

    return true;
}

void MountInfo::checkValidIp(const QString& host)
{
    m_painter1->start();

    QStringList args;
    args.append("-T");
    args.append("-A");
    args.append(host);
    qDebug() << "valid IP" << args;
    m_process->start("nmblookup", args);
}

void MountInfo::checkValidHost(const QString& host)
{
    m_painter1->start();
    m_process->start("nmblookup", QStringList(host));
}

void MountInfo::nameResolveFinished(int status)
{
    qDebug() << "Status: " << status;

    m_painter1->stop();

    QByteArray output = m_process->readAllStandardOutput();
//     qDebug() << "name resolved:" << output;

    if (output.isEmpty()) {
        error->setText(i18n("Couldn't get the server IP"));
        setResult(working1, Fail);
        return;
    }

    QString ipLine;
    const QList<QByteArray> lines = output.split('\n');
    Q_FOREACH(const QString &line, lines) {
        if (line.contains(m_host)) {
            ipLine = line;
            break;
        }
    }
    QString ip = ipLine.left(ipLine.indexOf(' ')).trimmed();

    qDebug() << "Ip: " << ip;
    if (ip.isEmpty() || ip == "name_query") {
        error->setText(i18n("Couldn't get the server IP"));
        setResult(working1, Fail);
        return;
    }

    if (!isIp(ip)) {
        m_ip = m_host;
        m_host = ip.toLower();
    } else {
        m_ip = ip;
    }

    m_share = true;
    setResult(working1, Ok);

    autoFillMountName();
}

bool MountInfo::checkMountPoint(const QString& name)
{
    m_mountName = name;
    return checkMountPoint(QUrl::fromLocalFile(QDir::homePath() + "/Network/" + name));
}

bool MountInfo::checkMountPoint(const QUrl &url)
{
    qDebug() << "ckecking mount point..." << url;
    QDir dir(url.toLocalFile());

    const QString networkDir(QDir::homePath() + "/Network");
    dir.mkdir(networkDir);

    KDesktopFile cfg(networkDir + QString::fromLatin1("/.directory"));
    if (cfg.desktopGroup().readEntry("Icon", "").isEmpty()) {
        cfg.desktopGroup().writeEntry("Icon", "folder-remote");
        cfg.sync();

        cfg.reparseConfiguration();
    }

    m_mount = false;
    m_mountPoint = url.toLocalFile();

    if (dir.exists() && dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries).count() != 0) {
        error->setText(i18n("Please, choose another name"));
        setResult(working2, Fail);
        Q_EMIT checkDone();
        return false;
    }

    QList <Solid::Device> devices = Solid::Device::listFromType(Solid::DeviceInterface::StorageAccess);
    Q_FOREACH(const Solid::Device &device, devices) {
        if (device.as<Solid::StorageAccess>()->filePath() == url.toLocalFile()) {
            error->setText(i18n("Please, choose another name"));
            setResult(working2, Fail);
            return false;
        }
    }

    m_mount = true;
    setResult(working2, Ok);

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
            error->clear();
            Q_EMIT checkDone();
            break;
        case Fail:
            lbl->setPixmap(QIcon::fromTheme("dialog-close").pixmap(lbl->sizeHint()));
            Q_EMIT checkDone();
            break;
    }
}

void MountInfo::buttonClicked()
{
    checkMountPoint(shareName->text());
    checkValidSamba(sambaRequester->url());

    connect(this, SIGNAL(checkDone()), SLOT(mountIsValid()));
}

void MountInfo::mountIsValid()
{
    qDebug() << "Mount is valid";
    disconnect(this, SIGNAL(checkDone()), this, SLOT(mountIsValid()));

    if (!m_mount || !m_share) {
        return;
    }

    if (m_editMode) {
        m_config.deleteGroup(m_id);
    }

    m_id = m_fullSambaUrl + "-" + m_mountPoint;

    KConfigGroup group = m_config.group(m_id);

    saveConfig(group);

    qDebug() << "Edit mode:" << m_editMode;
    if (m_editMode) {
        Q_EMIT mountEditted(group);
        return;
    }

    Q_EMIT mountCreated(group);
    setEditMode();
}

void MountInfo::saveConfig()
{
    if (m_id.isEmpty() && !m_fullSambaUrl.isEmpty() && !m_mountPoint.isEmpty()) {
        m_id = m_fullSambaUrl + "-" + m_mountPoint;
    }

    if (m_id.isEmpty()) {
        return;
    }

    saveConfig(m_config.group(m_id));
}

void MountInfo::saveConfig(KConfigGroup group)
{
    qDebug() << "Saving mount";

    group.writeEntry("ip", m_ip);
    group.writeEntry("hostname", m_host);
    group.writeEntry("mountPoint", m_mountPoint);
    group.writeEntry("sambaDir", m_sambaDir);
    group.writeEntry("fullSambaUrl", m_fullSambaUrl);
    group.writeEntry("mountName", m_mountName);
    group.writeEntry("username", username->text());
    group.writeEntry("password", password->text());

    group.sync();

    QDir().mkdir(m_mountPoint);
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

    QString name = sambaRequester->url().fileName();

    if (!checkMountPoint(name)) {
        setResult(working2, Empty);
        error->clear();
        return;
    }

    shareName->setText(name);
}

void MountInfo::authInfoReceived(qlonglong requestId, qlonglong seqNr, const KIO::AuthInfo & info)
{
    if (!username->text().isEmpty() || !password->text().isEmpty())
        return;

    if (info.url == sambaRequester->url()) {
//         qDebug() << "awesomeeeeeeeeeee" << requestId << info.username << info.password;
        username->setText(info.username);
        password->setText(info.password);
    }
}
