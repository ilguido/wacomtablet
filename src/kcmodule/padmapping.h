/*
 * Copyright 2009,2010 Jörg Ehrichs <joerg.ehichs@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PADMAPPING_H
#define PADMAPPING_H

//Qt includes
#include <QtGui/QWidget>

namespace Ui
{
class PadMapping;
}

namespace Wacom
{
class ProfileManagement;

/**
  * This class implements the widget to change the pad mapping
  * The pad mapping includes the rotation and the actual working area
  */
class PadMapping : public QWidget
{
    Q_OBJECT

public:
    /**
      * default constructor
      *
      * @param profileManager Handles the connection to the config files
      * @param parent the parent widget
      */
    explicit PadMapping(ProfileManagement *profileManager, QWidget *parent = 0);

    /**
      * default destructor
      */
    ~PadMapping();

    /**
      * Saves all values to the current profile
      */
    void saveToProfile();

    /**
      * Reloads the widget when the status of the tablet device changes (connects/disconnects)
      *
      */
    void reloadWidget();

public slots:
    /**
      * Called whenever the profile is switched or the widget needs to be reinitialized.
      *
      * Updates all values on the widget to the values from the profile.
      */
    void loadFromProfile();

    /**
      * Called whenever a value is changed.
      * Fires the changed() signal afterwards to inform the main widget that unsaved changes are available.
      */
    void profileChanged();

signals:
    /**
      * Used to inform the main widget that unsaved changes in the current profile are available.
      */
    void changed();

private:
    Ui::PadMapping      *m_ui;                /**< Handler to the padmapping.ui file */
    ProfileManagement   *m_profileManagement; /**< Handler for the profile config connection */
};

}

#endif // PADMAPPING_H
