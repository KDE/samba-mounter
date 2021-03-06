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

#include <locale.h>
#include <unistd.h>
#include <QProcess>

ActionReply SambaHelper::mount(QVariantMap args)
{
    QString ip = args["ip"].toString();
    QString uid = args["uid"].toString();
    QString sambaDir = args["sambaDir"].toByteArray();
    QString mountPoint = args["mountPoint"].toByteArray();
    QString locale = args["locale"].toString();
    QString path = args["path"].toString();
    QString username = args["username"].toString();
    QString password = args["password"].toString();

    setenv("LANG", locale.toLocal8Bit(), 1);
    setenv("PATH", path.toLocal8Bit(), 1);
    setlocale(LC_CTYPE, locale.toLocal8Bit());

    QProcess proc;
    proc.start("samba-realmounter", {ip, sambaDir, mountPoint, uid, username, password });
    proc.waitForFinished();

    ActionReply reply;
    reply.addData("output", proc.readAllStandardOutput());
    reply.addData("error", proc.readAllStandardError());
    reply.addData("exitCode", proc.exitCode());

    return reply;
}

ActionReply SambaHelper::umount(QVariantMap args)
{
    QString locale = args["locale"].toString();
    QString path = args["path"].toString();
    setenv("LANG", locale.toLocal8Bit(), 1);
    setenv("PATH", path.toLocal8Bit(), 1);
    setlocale(LC_CTYPE, locale.toLocal8Bit());

    QProcess proc;
    proc.start("samba-realumounter", {args["mountPoint"].toString()});
    proc.waitForFinished();

    return ActionReply::SuccessReply();
}

KAUTH_HELPER_MAIN("org.kde.sambamounter", SambaHelper)
// int main(int argc, char **argv) { setlocale(LC_CTYPE, "en_US.UTF-8"); return KAuth::HelperSupport::helperMain(argc, argv, "org.kde.sambamounter", new SambaHelper ()); }
