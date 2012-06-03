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
        explicit MountInfo(QWidget* parent = 0);
        virtual ~MountInfo();

    public Q_SLOTS:
        void checkValidSamba(const KUrl &url);
        void checkValidSamba(const QString &url);
        void nameResolveFinished(int status);

        void setResult(QLabel *lbl, Status status);

    private:
        QProcess *m_process;
        KPixmapSequenceOverlayPainter *m_painter1;
        KPixmapSequenceOverlayPainter *m_painter2;
};

#endif //MOUNT_INFO_H