/*
 * Copyright 2009, 2010 Jörg Ehrichs <joerg.ehichs@gmx.de>
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

#include "debug.h"

#include "tablethandler.h"
#include "tabletinfo.h"
#include "deviceinterface.h"
#include "devicetype.h"
#include "xsetwacominterface.h"

// common includes
#include "dbustabletinterface.h" // required to copy TabletInformation from/to QDBusArgument
#include "deviceprofile.h"
#include "tabletdatabase.h"
#include "mainconfig.h"
#include "profilemanager.h"
#include "tabletprofile.h"

#include <KDE/KLocalizedString>

#include <QtDBus/QDBusArgument>
#include <X11/Xlib.h>

namespace Wacom
{
    class TabletHandlerPrivate
    {
        public:
            MainConfig            mainConfig;
            ProfileManager        profileManager;   /**< Profile manager which reads and writes profiles from the configuration file */
            TabletDatabase        tabletDatabase;

            TabletInformation     tabletInformation;
            DeviceInterface      *currentDevice;     //!< Handler for the current device to get/set its configuration.
            bool                  isDeviceAvailable; //!< Is a tabled device connected or not?

            QMap<QString,QString> buttonMapping;     /**< Map the hardwarebuttons 1-X to its kernel numbering scheme
                                                          @see http://sourceforge.net/mailarchive/message.php?msg_id=27512095 */
            QString               currentProfile;   /**< currently active profile */
            int                   currentDeviceId;  /**< currently conencted tablet device. id comes from x11 */
    }; // CLASS
} // NAMESPACE

using namespace Wacom;

TabletHandler::TabletHandler()
    : TabletHandlerInterface(NULL), d_ptr(new TabletHandlerPrivate)
{
    Q_D( TabletHandler );

    d->isDeviceAvailable = false;
    d->currentDevice     = NULL;

    d->profileManager.open( QLatin1String( "tabletprofilesrc" ) );
}


TabletHandler::~TabletHandler()
{
    delete d_ptr;
}



QString TabletHandler::getProperty(const QString& device, const Property& property) const
{
    Q_D( const TabletHandler );

    if( !d->currentDevice ) {
        kError() << QString::fromLatin1("Unable to get property '%1' from device '%2' as no device is currently available!").arg(property.key()).arg(device);
        return QString();
    }

    return d->currentDevice->getProperty( device, property );
}


void TabletHandler::onTabletAdded( const TabletInformation& info )
{
    Q_D( TabletHandler );

    int deviceId = info.getXDeviceId();

    // if we already have a device ... skip this step
    if( d->isDeviceAvailable || deviceId == 0) {
        return;
    }

    // No tablet available, so reload tablet information
    detectTablet(info);

    // if we found something notify about it and set the default profile to it
    if( d->isDeviceAvailable ) {

        d->currentDeviceId = deviceId;

        emit notify( QLatin1String("tabletAdded"),
                     i18n("Tablet added"),
                     i18n("New %1 tablet added", d->tabletInformation.get(TabletInfo::TabletName) ));

        emit tabletAdded(d->tabletInformation);
        setProfile(d->mainConfig.getLastProfile());
    }
}



void TabletHandler::onTabletRemoved( const TabletInformation& info )
{
    Q_D( TabletHandler );

    int deviceId = info.getXDeviceId();
    
    if( d->isDeviceAvailable && d->currentDeviceId == deviceId ) {
        emit notify( QLatin1String("tabletRemoved"),
                     i18n("Tablet removed"),
                     i18n("Tablet %1 removed", d->tabletInformation.get(TabletInfo::TabletName) ));

        d->isDeviceAvailable = false;
        clearTabletInformation();
        emit tabletRemoved();
    }
}



void TabletHandler::onScreenRotated( const TabletRotation& screenRotation )
{
    Q_D( TabletHandler );

    TabletProfile tabletProfile = d->profileManager.loadProfile(d->currentProfile);
    DeviceProfile stylusProfile = tabletProfile.getDevice(DeviceType::Stylus.key());

    kDebug() << "xRandR screen rotation detected.";

    if ( stylusProfile.getRotateWithScreen() == QLatin1String( "true" ) ) {

        kDebug() << "Rotate tablet :: " << screenRotation.key();

        QString stylusName = d->tabletInformation.getDeviceName(DeviceType::Stylus);
        QString eraserName = d->tabletInformation.getDeviceName(DeviceType::Eraser);
        QString touchName  = d->tabletInformation.getDeviceName(DeviceType::Touch);

        setProperty( stylusName, Property::Rotate, QString::fromLatin1( "%1" ).arg( screenRotation.key() ) );
        setProperty( eraserName, Property::Rotate, QString::fromLatin1( "%1" ).arg( screenRotation.key() ) );

        if( !touchName.isEmpty() ) {
            setProperty( touchName, Property::Rotate, QString::fromLatin1( "%1" ).arg( screenRotation.key() ) );
        }

        setProfile(d->currentProfile);
    }
}


void TabletHandler::onTogglePenMode()
{
    Q_D( TabletHandler );

    if( !d->currentDevice ) {
        return;
    }

    if(!d->tabletInformation.hasDevice(DeviceType::Stylus)) {
        d->currentDevice->toggleMode(d->tabletInformation.getDeviceName(DeviceType::Stylus));
    }

    if(!d->tabletInformation.hasDevice(DeviceType::Eraser)) {
        d->currentDevice->toggleMode(d->tabletInformation.getDeviceName(DeviceType::Eraser) );
    }

}



void TabletHandler::onToggleTouch()
{
    Q_D( TabletHandler );

    if( !d->currentDevice || d->tabletInformation.hasDevice(DeviceType::Touch) ) {
        return;
    }

    d->currentDevice->toggleTouch(d->tabletInformation.getDeviceName(DeviceType::Touch));
}



QStringList TabletHandler::listProfiles() const
{
    Q_D( const TabletHandler );
    // we can not reload the profile manager from a const method so we have
    // to create a new instance here and let it read the configuration file.
    ProfileManager profileManager(QLatin1String( "tabletprofilesrc" ));
    profileManager.readProfiles(d->tabletInformation.get(TabletInfo::TabletName));

    return profileManager.listProfiles();
}



void TabletHandler::setProfile( const QString &profile )
{
    Q_D( TabletHandler );

    if (!d->currentDevice) {
        return;
    }
    
    d->profileManager.readProfiles(d->tabletInformation.get(TabletInfo::TabletName));

    TabletProfile tabletProfile = d->profileManager.loadProfile(profile);

    if (tabletProfile.listDevices().isEmpty()) {
        emit notify( QLatin1String( "tabletError" ),
                     i18n( "Graphic Tablet error" ),
                     i18n( "Profile <b>%1</b> does not exist", profile ) );

    } else {
        d->currentProfile = profile;

        foreach(const DeviceType& type, DeviceType::list()) {
            if (d->tabletInformation.hasDevice(type)) {
                d->currentDevice->applyProfile (d->tabletInformation.getDeviceName(type), type, tabletProfile);
            }
        }

        d->mainConfig.setLastProfile(profile);
        emit profileChanged( profile );
    }
}



void TabletHandler::setProperty(const QString& device, const Property& property, const QString& value)
{
    Q_D( TabletHandler );

    if (!d->currentDevice) {
        kError() << QString::fromLatin1("Unable to set property '%1' on device '%2' to '%3' as no device is currently available!").arg(property.key()).arg(device).arg(value);
        return;
    }

    d->currentDevice->setProperty( device, property, value );
}



void TabletHandler::clearTabletInformation()
{
    Q_D( TabletHandler );

    TabletInformation empty;

    d->isDeviceAvailable = false;
    d->tabletInformation = empty;

    delete d->currentDevice;
    d->currentDevice = NULL;

    d->buttonMapping.clear();
}



bool TabletHandler::detectTablet(const TabletInformation& tabletInformation)
{
    Q_D( TabletHandler );

    // make a copy we can actually write to
    TabletInformation tabletInfo = tabletInformation;
    
    kDebug() << "XInput found a device! ::" << tabletInfo.tabletId;

    if (!d->tabletDatabase.lookupDevice(tabletInfo, tabletInfo.tabletId)) {
        kDebug() << "Could not find device in database: " << tabletInfo.tabletId;
        return false;
    }

    d->tabletInformation  = tabletInfo;

    // lookup button mapping
    d->tabletDatabase.lookupButtonMapping(d->buttonMapping, tabletInfo.companyId, tabletInfo.tabletId);

    // set device backend
    selectDeviceBackend( d->tabletDatabase.lookupBackend(tabletInfo.companyId) );

    // \0/
    d->isDeviceAvailable = true;

    return true;
}


void TabletHandler::selectDeviceBackend(const QString& backendName)
{
    Q_D( TabletHandler );

    if( backendName == QLatin1String( "wacom-tools" ) ) {
        d->currentDevice = new XsetwacomInterface();
        d->currentDevice->setButtonMapping(d->buttonMapping);
    }

    if( !d->currentDevice ) {
        kError() << "unknown device backend!" << backendName;
    }
}
