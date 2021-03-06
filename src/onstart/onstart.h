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

#ifndef ONSTART_H
#define ONSTART_H

#include <QtCore/QObject>
#include <QNetworkConfigurationManager>

#include <KConfigGroup>
#include <QEventLoopLocker>

class KJob;

class OnStart : public QObject
{
    Q_OBJECT
    public:
        explicit OnStart(QObject* parent = 0);
        virtual ~OnStart();

    private Q_SLOTS:
        void mountConfiguredShares();

    private:
        bool m_someFailed;
        void mountSamba(KConfigGroup group);

        QNetworkConfigurationManager m_networkConfigurationManager;

        //lock the event loop as this class is instantiated
        QEventLoopLocker m_lock;
};

#endif //ONSTART_H
