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

#include "sambamount.h"
#include "ui_kcm.h"

#include <kpluginfactory.h>

K_PLUGIN_FACTORY(SambaMountFactory, registerPlugin<SambaMount>();)
K_EXPORT_PLUGIN(SambaMountFactory("sambamount", "sambamount"))

SambaMount::SambaMount(QWidget *parent, const QVariantList&)
: KCModule(SambaMountFactory::componentData(), parent)
{

    m_ui = new Ui::KCMSambaMount();
    m_ui->setupUi(this);

    QMetaObject::invokeMethod(this, "initSambaMount", Qt::QueuedConnection);
}

SambaMount::~SambaMount()
{
    delete m_ui;
}

void SambaMount::initSambaMounts()
{

}

#include "sambamount.moc"

