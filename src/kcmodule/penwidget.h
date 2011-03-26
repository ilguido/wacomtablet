/*
 * Copyright 2009 Jörg Ehrichs <joerg.ehichs@gmx.de>
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

#ifndef PENWIDGET_H
#define PENWIDGET_H

//Qt includes
#include <QtGui/QWidget>
#include <QtCore/QMap>

namespace Ui
{
class PenWidget;
}

class KComboBox;

namespace Wacom
{
class ProfileManagement;

/**
  * The PenWidget class holds all settings for the stylus/eraser pen.
  * Through the xsetwacom interface the widget can manipulate the two buttons,
  * tip press curve, absolute/relative mode, double tab speed and across monitor support
  * via the Twinview options.
  */
class PenWidget : public QWidget
{
    Q_OBJECT

public:
    /**
      * default constructor
      * Initialize the widget.
      *
      * @param profileManager Handles the connection to the config files
      * @param parent parent Widget
      */
    explicit PenWidget(ProfileManagement *profileManager, QWidget *parent = 0);

    /**
      * default destructor
      */
    ~PenWidget();

    /**
      * Defines if the widget is used for the stylus or the eraser side of the pen.
      * default is stylus set false for eraser
      *
      * @param setStylus @c true for stylus (default) @c false for eraser.
      */
    void isStylus(bool setStylus);

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
      * Opens a new dialogue for the buttons to select the button function.
      * Fires the changed() signal afterwards to inform the main widget that unsaved changes are available
      *
      * @param selection button index @see WacomInterface::PenButton Enumaration
      */
    void selectKeyFunction(int selection);

    /**
      * Called whenever a value other than the pen buttons is changed.
      * Fires the changed() signal afterwards to inform the main widget that unsaved changes are available
      */
    void profileChanged();

    /**
      * Opens a dialogue that allows the visual selection of the presscurve
      * If Qt detects the tablet (through the xorg.conf file) the changes can be tested
      * directly in the dialogue as well
      */
    void changePressCurve();

signals:
    /**
      * Used to inform the main widget that unsaved changes in the current profile are available
      */
    void changed();

private:
    /**
      * Fills the button selection combobox with all available values
      * Used in this way to get no redundant strings in the ui file for
      * any given translator. Thus reduce the strings necessary to translate.
      *
      * @param comboBox the combobox where the values are added to
      */
    void fillComboBox(KComboBox *comboBox);

    Ui::PenWidget     *m_ui;                /**< Handler to the penwidget.ui file */
    ProfileManagement *m_profileManagement; /**< Handler for the profile config connection */
    bool               m_stylus;            /**< Widget used for the stylus or the eraser side of the pen? */
    QMap<QString,QString> m_buttonConfig;   /**< Saves for each KCombobox the values selected and used in a way xsetwacom needs it */
};

}

#endif /*PENWIDGET_H*/
