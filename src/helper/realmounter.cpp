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

#include <QProcess>
#include <QDebug>
#include <iostream>

int main(int argc, char** argv)
{
    if (argc < 7) {
        std::cerr << "Not enough arguments" << std::endl;
        return 1;
    }

    QString sambaDir = QString::fromUtf8(QByteArray::fromBase64(argv[2]));
    QString mountPoint = QString::fromUtf8(QByteArray::fromBase64(argv[3]));
    QString share = QString::fromUtf8("//");
    share.append(argv[1]);
    share.append(sambaDir);
    QString uid (argv[4]);
    QString username(argv[5]);
    QString password(argv[6]);

    QStringList arguments;
    arguments.append("-t");
    arguments.append("cifs");
    arguments.append(share);
    arguments.append(mountPoint);
    arguments.append("-o");

    QString options;
    if (username == "none") {
        options.append("guest");
    } else {
        options.append("user=" + username);
    }
    if (password != "none") {
        options.append(",password=" + password);
    }

    options.append(",sec=ntlmv2,uid=" + uid);

    arguments.append(options);
    QProcess proc;
    proc.start("mount", arguments);
    proc.waitForFinished();

    std::cerr << proc.readAllStandardError().toStdString() << std::endl;
    std::cerr << proc.readAllStandardOutput().toStdString() << std::endl;

    return proc.exitCode();
}
