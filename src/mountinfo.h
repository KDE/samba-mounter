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

#ifndef MOUNT_INFO_H
#define MOUNT_INFO_H

#include "ui_mount.h"

#include <QtGui/QWidget>

class QProcess;
class KPixmapSequenceOverlayPainter;
class MountInfo : public QWidget, Ui::MountInfo
{
    Q_OBJECT
    public:
         enum Status {
            Empty   = 1,
            Ok      = 2,
            Fail    = 4
        };
        explicit MountInfo(KConfigGroup config, QWidget* parent = 0);
        virtual ~MountInfo();

        QString id() const;
        void setConfigGroup(const QString &name);

    public Q_SLOTS:
        void checkValidSamba(const KUrl &url);
        void checkValidSamba(const QString &url);
        void nameResolveFinished(int status);

        bool checkMountPoint(const KUrl &url);
        bool checkMountPoint(const QString& name);

        void setResult(QLabel *lbl, Status status);

        void buttonClicked();
        void mountIsValid();
        void saveConfig();
        void saveConfig(KConfigGroup group);

    Q_SIGNALS:
        void checkDone();
        void mountCreated(KConfigGroup);
        void mountEditted(KConfigGroup);

    private:
        void setEditMode();
        void autoFillMountName();
        bool isIp(const QString &host);
        void checkValidIp(const QString &url);
        void checkValidHost(const QString &url);

    private:
        bool m_share, m_mount, m_editMode;
        QProcess *m_process;
        KConfigGroup m_config;
        KPixmapSequenceOverlayPainter *m_painter1;
        KPixmapSequenceOverlayPainter *m_painter2;

        QString m_id;
        QString m_host;
        QString m_ip;
        QString m_sambaDir;
        QString m_fullSambaUrl;
        QString m_mountPoint;
        QString m_mountName;
};

#endif //MOUNT_INFO_H