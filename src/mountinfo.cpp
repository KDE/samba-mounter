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

    sambaRequester->setUrl(QUrl("smb:/"));

    connect(sambaRequester, SIGNAL(urlSelected(QUrl)), SLOT(checkValidSamba(QUrl)));
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
    qDebug() << url;
    checkValidSamba(QUrl(url));
}

void MountInfo::checkValidSamba(const QUrl &url)
{
    qDebug() << url;
    qDebug() << "Host: " << url.host();
    m_process->close();

    m_share = false;
    setResult(working1, Empty);

    m_fullSambaUrl = url.url();
    //If path and file are the same thing for example smb://foo/public
    if (url.path().indexOf(url.fileName()) == 1) {
        m_sambaDir = url.path();
    } else {
        m_sambaDir = url.path() + "/" + url.fileName();
    }

    qDebug() << "fullSambaUrl" << m_fullSambaUrl;
    qDebug() << "sambaDir:" << m_sambaDir;
    if (m_sambaDir.isEmpty() || m_sambaDir == "/") {
        error->setText(i18n("You must select a folder"));
        setResult(working1, Fail);
        Q_EMIT checkDone();
        return;
    }

    m_host = url.host();
    if (isIp(m_host)) {
        checkValidIp(m_host);
        return;
    }

    checkValidHost(m_host);
}

bool MountInfo::isIp(const QString& host)
{
    QStringList split = host.split(".");
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
    qDebug() << args;
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

    QString output = m_process->readAllStandardOutput();
    qDebug() << output;

    if (output.isEmpty()) {
        error->setText(i18n("Couldn't get the server IP"));
        setResult(working1, Fail);
        Q_EMIT checkDone();
        return;
    }

    QString ipLine;
    const QStringList lines = output.split("\n");
    Q_FOREACH(const QString line, lines) {
        if (line.contains(m_host)) {
            ipLine = line;
        }
    }
    QString ip = ipLine.left(ipLine.indexOf(" ")).trimmed();

    qDebug() << "Ip: " << ip;
    if (ip.isEmpty() || ip == "name_query") {
        error->setText(i18n("Couldn't get the server IP"));
        setResult(working1, Fail);
        Q_EMIT checkDone();
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
    Q_EMIT checkDone();
    error->setText("");

    autoFillMountName();
}

bool MountInfo::checkMountPoint(const QString& name)
{
    m_mountName = name;
    QUrl url(QDir::homePath());
    url = url.adjusted(QUrl::StripTrailingSlash);
    url.setPath(url.path() + '/' + "Network");
    url = url.adjusted(QUrl::StripTrailingSlash);
    url.setPath(url.path() + '/' + name);

    return checkMountPoint(QUrl(url));
}

bool MountInfo::checkMountPoint(const QUrl &url)
{
    qDebug() << url;
    QString urlPath = url.path();
    QDir dir(urlPath);

    QUrl networkDir(QDir::homePath());
    networkDir = networkDir.adjusted(QUrl::StripTrailingSlash);
    networkDir.setPath(networkDir.path() + '/' + "Network");
    dir.mkdir(networkDir.path());

    KDesktopFile cfg(networkDir.toLocalFile() + QString::fromLatin1(".directory"));
    if (cfg.desktopGroup().readEntry("Icon", "").isEmpty()) {
        cfg.desktopGroup().writeEntry("Icon", "folder-remote");
        cfg.sync();

        cfg.reparseConfiguration();
    }

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

    QString name = QUrl(sambaRequester->lineEdit()->text()).fileName();

    if (!checkMountPoint(name)) {
        setResult(working2, Empty);
        error->setText("");
        return;
    }

    shareName->setText(name);
}