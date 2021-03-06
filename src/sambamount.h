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

#ifndef sambamount_H
#define sambamount_H

#include <kcmodule.h>
#include <KConfigGroup>
#include <KIO/AuthInfo>

class QStackedLayout;
class QListWidgetItem;
class OrgKdeKPasswdServerInterface;
namespace Ui {
    class KCMSambaMount;
}

class SambaMount : public KCModule
{
Q_OBJECT
public:
    SambaMount(QWidget *parent, const QVariantList&);
    virtual ~SambaMount();

    void save() override;

private Q_SLOTS:
    void initSambaMounts();
    void currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void mountCreated(KConfigGroup group);
    void mountEditted(KConfigGroup group);

    void addBtnClicked();
    void rmBtnClicked();

private:
    void addMount(KConfigGroup group);
    KConfigGroup mounts();

    bool mountSamba(KConfigGroup group);
    bool umountSamba(const QString &name);
    bool executeJob(KAuth::ExecuteJob* reply);

    bool m_selectNew = false;
    QListWidgetItem *m_newMountItem;
    Ui::KCMSambaMount *m_ui;
    QStackedLayout *m_layout;
    OrgKdeKPasswdServerInterface* m_interface;
};

#endif // sambamount_h
