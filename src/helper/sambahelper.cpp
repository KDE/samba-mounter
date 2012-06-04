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

#include "sambahelper.h"

#include <QtDebug>
#include <QFile>
#include <QTextStream>
#include <unistd.h>
#include <QEventLoop>
#include <KProcess>


ActionReply SambaHelper::mount(QVariantMap args)
{
    QString ip = args["ip"].toString();
    QString mountPoint = args["mountPoint"].toString();

    QStringList arguments;
    arguments.append("-t");
    arguments.append("cifs");
    arguments.append(QString("//") + QString("192.168.0.152/Public/s"));
    arguments.append(mountPoint);
    arguments.append("-o");
    arguments.append("guest,uid=1000");

    QProcess proc;
    proc.start("mount", arguments);
    proc.waitForFinished();

    ActionReply reply;
    reply.addData("output", proc.readAllStandardError());

    return reply;
}

ActionReply SambaHelper::umount(QVariantMap args)
{
    QString mountPoint = args["mountPoint"];

    QStringList arguments;
    arguments.append(args["mountPoint"].toString());

    QProcess proc;
    proc.start("umount", arguments);
    proc.waitForFinished();

    return ActionReply::SuccessReply;
}

KDE4_AUTH_HELPER_MAIN("org.kde.sambamounter", SambaHelper)
