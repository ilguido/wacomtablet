/*
 * Copyright 2012 Alexander Maret-Huskinson <alex@maret.de>
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

#ifndef TESTXSETWACOMADAPTOR_H
#define TESTXSETWACOMADAPTOR_H

#include <QtTest>
#include <QtCore>

#include "tabletdependenttest.h"

namespace Wacom
{
class TestXsetwacomAdaptor : public TabletDependentTest
{
    Q_OBJECT

public:

    TestXsetwacomAdaptor(QObject* parent = 0);


protected:

    void initTestCaseDependent();


private slots:

    void testSetProperty();

};
}

QTEST_MAIN(Wacom::TestXsetwacomAdaptor);

#endif
