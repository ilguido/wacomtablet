/*
 * This file is part of the KDE wacomtablet project. For copyright
 * information and license terms see the AUTHORS and COPYING files
 * in the top-level directory of this distribution.
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
#include "x11inputdevice.h"

#include <QtCore/QStringList>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput.h>
//#include <X11/extensions/XInput2.h>
#include <X11/Xutil.h>

using namespace Wacom;

/**
 * Helper struct which allows us to forward declare XDevice.
 */
struct X11InputDevice::XDevice : public ::XDevice {};

/**
 * Class for private members.
 */
namespace Wacom {
    class X11InputDevicePrivate
    {
        public:
            X11InputDevice::XDevice* device;
            Display*                 display;
            QString                  name;
    };
}


X11InputDevice::X11InputDevice() : d_ptr(new X11InputDevicePrivate)
{
    Q_D(X11InputDevice);
    d->device  = NULL;
    d->display = NULL;
}


X11InputDevice::X11InputDevice(Display* dpy, const X11InputDevice::XDeviceInfo& deviceInfo) : d_ptr(new X11InputDevicePrivate)
{
    Q_D(X11InputDevice);
    d->device  = NULL;
    d->display = NULL;

    open(dpy, deviceInfo);
}


X11InputDevice::X11InputDevice(const X11InputDevice& device) : d_ptr(new X11InputDevicePrivate)
{
    Q_D(X11InputDevice);
    d->device  = NULL;
    d->display = NULL;

    operator=(device);
}


X11InputDevice::~X11InputDevice()
{
    close();
    delete d_ptr;
}



X11InputDevice& X11InputDevice::operator= (const X11InputDevice& that)
{
    Q_D(X11InputDevice);

    // close current device
    close();

    // connect new device
    if (that.d_ptr->display && that.d_ptr->device) {
        if (open(that.d_ptr->display, that.d_ptr->device->device_id)) {
            d->name = that.d_ptr->name;
        }
    }

    return *this;
}



bool X11InputDevice::close()
{
    Q_D(X11InputDevice);

    if (d->device == NULL) {
        assert(d->display != NULL);
        assert(d->name.isEmpty());
        return false;
    }

    XCloseDevice(d->display, d->device);

    d->display = NULL;
    d->device  = NULL;
    d->name.clear();

    return true;
}



Display* X11InputDevice::getDisplay()
{
    Q_D(X11InputDevice);
    return d->display;
}



bool X11InputDevice::getFloatProperty(const QString& property, long int nelements, QList< float >& values)
{
    Q_D(X11InputDevice);

    Atom expectedType = XInternAtom (d->display, "FLOAT", False);

    if (expectedType == None) {
        kError() << QLatin1String("Float values are unsupported by this XInput implementation!");
        return false;
    }

    return getProperty<float>(property, expectedType, nelements, values);
}



bool X11InputDevice::getLongProperty(const QString& property, long int nelements, QList< long >& values)
{
    return getProperty<long>(property, XA_INTEGER, nelements, values);
}



const QString& X11InputDevice::getName() const
{
    Q_D(const X11InputDevice);
    return d->name;
}



bool X11InputDevice::hasProperty(const QString& property)
{
    Q_D(X11InputDevice);

    if (!isOpen()) {
        return false;
    }

    Atom atom;
    if (!lookupProperty(property, &atom)) {
        return false;
    }

    bool  found  = false;
    int   natoms = 0;
    Atom* atoms = XListDeviceProperties (d->display, d->device, &natoms);

    for (int i = 0 ; i < natoms ; ++i) {
        if (atoms[i] == atom) {
            found = true;
            break;
        }
    }

    XFree(atoms);

    return found;
}



bool X11InputDevice::isOpen() const
{
    Q_D(const X11InputDevice);
    return (d->device != NULL && d->display != NULL);
}



bool X11InputDevice::isTabletDevice()
{
    return hasProperty(QLatin1String("Wacom Tool Type"));
}




bool X11InputDevice::open(Display* display, const X11InputDevice::XDeviceInfo& deviceInfo)
{
    Q_D(X11InputDevice);

    if (open(display, deviceInfo.id)) {
        d->name = QLatin1String(deviceInfo.name);
        return true;
    }

    return false;
}



bool X11InputDevice::setFloatProperty(const QString& property, const QString& values)
{
    QStringList valueList = values.split (QLatin1String(" "));

    bool         ok;
    QString      svalue;
    float        fvalue;
    QList<float> fvalues;

    for (int i = 0  ; i < valueList.size() ; ++i) {
        svalue = valueList.at(i);

        if (svalue.isEmpty()) {
            continue;
        }

        fvalue = svalue.toFloat(&ok);

        if (!ok) {
            kError() << QString::fromLatin1("Could not convert value '%1' to float!").arg(svalue);
            return false;
        }

        fvalues.append(fvalue);
    }

    return setFloatProperty(property, fvalues);
}



bool X11InputDevice::setFloatProperty(const QString& property, const QList< float >& values)
{
    Q_D(X11InputDevice);

    Atom expectedType = XInternAtom (d->display, "FLOAT", False);

    if (expectedType == None) {
        kError() << QLatin1String("Float values are unsupported by this XInput implementation!");
        return false;
    }

    return setProperty<float>(property, expectedType, values);
}



bool X11InputDevice::setLongProperty(const QString& property, const QString& values)
{
    QStringList valueList = values.split (QLatin1String(" "));

    bool        ok;
    QString     svalue;
    long        lvalue = 0;
    QList<long> lvalues;

    for (int i = 0  ; i < valueList.size() ; ++i) {

        svalue = valueList.at(i);

        if (svalue.isEmpty()) {
            continue;
        }

        lvalue = svalue.toLong(&ok, 10);

        if (!ok) {
            kError() << QString::fromLatin1("Could not convert value '%1' to long!").arg(svalue);
            return false;
        }

        lvalues.append(lvalue);
    }

    return setLongProperty(property, lvalues);
}



bool X11InputDevice::setLongProperty(const QString& property, const QList<long>& values)
{
    return setProperty<long>(property, XA_INTEGER, values);
}




template<typename T>
bool X11InputDevice::getProperty(const QString& property, Atom expectedType, long nelements, QList<T>& values)
{
    Q_D(X11InputDevice);

    int expectedFormat = 32;

    // check parameters
    if (!isOpen() || nelements < 1) {
        kError() << QLatin1String("Invalid parameters to X11InputDevice::getProperty()!");
        return false;
    }

    // we expect a long value to be 4 Byte = 32 Bit which is the default size used by XInput1
    if ((sizeof(long) * 8) != expectedFormat) {
        kError() << QString::fromLatin1("Invalid XInput property value size detected!");
        return false;
    }

    long          *data         = NULL;
    unsigned long  nitems       = 0;
    unsigned long  bytes_after  = 0;
    Atom           actualType   = None;
    Atom           propertyAtom = None;
    int            actualFormat = 0;

    if (!lookupProperty(property, &propertyAtom)) {
        kError() << QString::fromLatin1("Could not get unsupported property '%1'!").arg(property);
        return false;
    }

    if (XGetDeviceProperty (d->display, d->device, propertyAtom, 0, nelements, False, AnyPropertyType, &actualType, &actualFormat, &nitems, &bytes_after, (unsigned char**)&data) != Success) {
        kError() << QString::fromLatin1("Could not get property '%1'!").arg(property);
        return false;
    }

    if (actualFormat != expectedFormat || actualType != expectedType) {
        kError() << QString::fromLatin1("Invalid property type detected!");
        XFree(data);
        return false;
    }

    for (unsigned long i = 0 ; i < nitems ; i++) {
        values.append(*((T*)(data + i)));
    }

    XFree(data);
    return true;
}



bool X11InputDevice::lookupProperty(const QString& property, X11InputDevice::Atom* atom)
{
    Q_D(X11InputDevice);

    if (!isOpen() || property.isEmpty() || atom == NULL) {
        return false;
    }

    *atom = XInternAtom (d->display, property.toLatin1().constData(), True);

    if (*atom == None) {
        kError() << QString::fromLatin1("Failed to lookup XInput property '%1'!").arg(property);
        return false;
    }

    return true;
}



bool X11InputDevice::open(Display* display, X11InputDevice::XID id)
{
    Q_D(X11InputDevice);

    if (isOpen()) {
        close();
    }

    if (display == NULL || id == 0) {
        return false;
    }

    XDevice* device = (XDevice*) XOpenDevice(display, id);

    if (device == NULL) {
        return false;
    }

    d->device  = device;
    d->display = display;

    return true;
}



template<typename T>
bool X11InputDevice::setProperty(const QString& property, Atom expectedType, const QList<T>& values)
{
    Q_D(X11InputDevice);

    int expectedFormat = 32;

    // check parameters
    if (!isOpen() || values.size() == 0) {
        kError() << QLatin1String("Invalid parameters to X11InputDevice::setProperty()!");
        return false;
    }

    // we expect a long value to be 4 Byte = 32 Bit which is the default size used by XInput1
    if ((sizeof(long) * 8) != expectedFormat) {
        kError() << QString::fromLatin1("Invalid XInput property value size detected!");
        return false;
    }

    // lookup Atom
    Atom propertyAtom = None;

    if (!lookupProperty(property, &propertyAtom)) {
        kError() << QString::fromLatin1("Could not get unsupported property '%1'!").arg(property);
        return false;
    }

    // get property so we can validate format and type
    Atom           actualType;
    int            actualFormat;
    unsigned long  nitems, bytes_after;
    unsigned char *actualData  = NULL;

    XGetDeviceProperty (d->display, d->device, propertyAtom, 0, values.size(), False, AnyPropertyType, &actualType, &actualFormat, &nitems, &bytes_after, (unsigned char **)&actualData);
    XFree(actualData);

    if (actualFormat != expectedFormat || actualType != expectedType) {
        kError() << QString::fromLatin1("Can not set incompatible Xinput property '%1'!").arg(property);
        return false;
    }

    // create new data
    long *data = new long[values.size()];

    for (int i = 0 ; i < values.size() ; ++i) {
        *((T*)(data + i)) = values.at(i);
    }

    // set property
    XChangeDeviceProperty (d->display, d->device, propertyAtom, expectedType, 32, PropModeReplace, (unsigned char*)data, values.size());
    XFlush( d->display );

    delete[] data;

    return true;
}

