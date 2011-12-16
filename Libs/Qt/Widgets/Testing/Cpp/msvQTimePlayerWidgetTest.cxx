/*==============================================================================

  Library: MSVTK

  Copyright (c) Kitware Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// QT includes
#include <QApplication>
#include <QIcon>
#include <QRegExp>
#include <QStyle>
#include <QtTest/QTest>
#include <QVariant>

// MSVTK
#include "msvQTimePlayerWidget.h"
#include "msvVTKPolyDataFileSeriesReader.h"

// STD includes
#include <cstdlib>
#include <iostream>

#include "ctkTest.h"

Q_DECLARE_METATYPE(QVariant);

// ----------------------------------------------------------------------------
class msvQTimePlayerWidgetTester : public QObject
{
  Q_OBJECT
private slots:
  void testButtonProperties();
  void testButtonProperties_data();
};

// ----------------------------------------------------------------------------
void msvQTimePlayerWidgetTester::testButtonProperties()
{
  msvQTimePlayerWidget playerWidget;

  QFETCH(QString, Property);
  QFETCH(QVariant, InputPropertyValue);
  QFETCH(QVariant, ExpectedOuputPropertyValue);

  QRegExp rx("Icon");
  // Specific test for icons
  if (rx.indexIn(Property) >= 0)
    {
    QVariant testIcon = QVariant::fromValue(
      playerWidget.style()->QStyle::standardIcon(QStyle::SP_MediaPlay));

    playerWidget.setProperty(Property.toUtf8(), testIcon);
    QCOMPARE(
      qvariant_cast<QIcon>(playerWidget.property(Property.toUtf8())).cacheKey(),
      qvariant_cast<QIcon>(testIcon).cacheKey());
    }
  else
    {
    playerWidget.setProperty(Property.toUtf8(), InputPropertyValue);
    QCOMPARE(playerWidget.property(Property.toUtf8()), ExpectedOuputPropertyValue);
    }
}

// ----------------------------------------------------------------------------
void msvQTimePlayerWidgetTester::testButtonProperties_data()
{
  // We organize our data as the following:
  // First column is the property name, then the other columns as follow
  // [Property][Value][Expected Return]
  // An exception is computed for QIcon as we need to set an Image Icon.
  QTest::addColumn<QString>("Property");
  QTest::addColumn<QVariant>("InputPropertyValue");
  QTest::addColumn<QVariant>("ExpectedOuputPropertyValue");

  // Icons
  QTest::newRow("firstFrameIcon") << "firstFrameIcon"
                                  << QVariant::fromValue(0)
                                  << QVariant::fromValue(0);
  QTest::newRow("previousFrameIcon") << "previousFrameIcon"
                                     << QVariant::fromValue(0)
                                     << QVariant::fromValue(0);
  QTest::newRow("playReverseIcon") << "playReverseIcon"
                                   << QVariant::fromValue(0)
                                   << QVariant::fromValue(0);
  QTest::newRow("nextFrameIcon") << "nextFrameIcon"
                                 << QVariant::fromValue(0)
                                 << QVariant::fromValue(0);
  QTest::newRow("lastFrameIcon") << "lastFrameIcon"
                                 << QVariant::fromValue(0)
                                 << QVariant::fromValue(0);
  QTest::newRow("playIcon") << "playIcon"
                            << QVariant::fromValue(0)
                            << QVariant::fromValue(0);
  QTest::newRow("repeatIcon") << "repeatIcon"
                              << QVariant::fromValue(0)
                              << QVariant::fromValue(0);

  // Visibility
  QTest::newRow("playReverseVisibility") << "playReverseVisibility"
                                         << QVariant::fromValue(false)
                                         << QVariant::fromValue(false);
  QTest::newRow("boundFramesVisibility") << "boundFramesVisibility"
                                         << QVariant::fromValue(false)
                                         << QVariant::fromValue(false);
  QTest::newRow("goToVisibility") << "goToVisibility"
                                       << QVariant::fromValue(false)
                                       << QVariant::fromValue(false);
  QTest::newRow("timeSpinBoxVisibility") << "timeSpinBoxVisibility"
                                         << QVariant::fromValue(false)
                                         << QVariant::fromValue(false);
  QTest::newRow("playReverseVisibility") << "playReverseVisibility"
                                         << QVariant::fromValue(true)
                                         << QVariant::fromValue(true);
  QTest::newRow("boundFramesVisibility") << "boundFramesVisibility"
                                         << QVariant::fromValue(true)
                                         << QVariant::fromValue(true);
  QTest::newRow("seekFrameVisibility") << "seekFrameVisibility"
                                       << QVariant::fromValue(true)
                                       << QVariant::fromValue(true);
  QTest::newRow("timeSpinBoxVisibility") << "timeSpinBoxVisibility"
                                         << QVariant::fromValue(true)
                                         << QVariant::fromValue(true);

  // Slider properties
  QTest::newRow("sliderDecimals") << "sliderDecimals"
                                  << QVariant::fromValue(2)
                                  << QVariant::fromValue(2);
  QTest::newRow("sliderPageStep") << "sliderPageStep"
                                  << QVariant::fromValue(5.)
                                  << QVariant::fromValue(5.);
  QTest::newRow("sliderSingleStep") << "sliderSingleStep"
                                    << QVariant::fromValue(5.)
                                    << QVariant::fromValue(5.);
  QTest::newRow("sliderSingleStep -> Autommatic mode correspounding to a frame")
    << "sliderSingleStep"
    << QVariant::fromValue(-1.)
    << QVariant::fromValue(1.);

  // Direction
  QTest::newRow("direction") << "direction"
                             << QVariant::fromValue(static_cast<int>(
                                  QAbstractAnimation::Backward))
                             << QVariant::fromValue(
                                  static_cast<int>(QAbstractAnimation::Backward));
  QTest::newRow("direction") << "direction"
                             << QVariant::fromValue(
                                  static_cast<int>(QAbstractAnimation::Forward))
                             << QVariant::fromValue(
                                  static_cast<int>(QAbstractAnimation::Forward));

  // Playback
  QTest::newRow("repeat") << "repeat"
                          << QVariant::fromValue(false)
                          << QVariant::fromValue(false);
  QTest::newRow("repeat") << "repeat"
                          << QVariant::fromValue(true)
                          << QVariant::fromValue(true);
  QTest::newRow("maxFramerate") << "maxFramerate"
                                << QVariant::fromValue(1.5)
                                << QVariant::fromValue(1.5);
  QTest::newRow("maxFramerate -> restoreDefault") << "maxFramerate"
                                                  << QVariant::fromValue(0.)
                                                  << QVariant::fromValue(60.);
  QTest::newRow("maxFramerate -> restoreDefault") << "maxFramerate"
                                                  << QVariant::fromValue(-1.)
                                                  << QVariant::fromValue(60.);
  QTest::newRow("playSpeed") << "playSpeed"
                             << QVariant::fromValue(4.)
                             << QVariant::fromValue(4.);
  QTest::newRow("playSpeed") << "playSpeed"
                             << QVariant::fromValue(0.)
                             << QVariant::fromValue(1.);
  QTest::newRow("playSpeed") << "playSpeed"
                             << QVariant::fromValue(-1.)
                             << QVariant::fromValue(1.);
}

// -----------------------------------------------------------------------------
CTK_TEST_MAIN(msvQTimePlayerWidgetTest)
#include "moc_msvQTimePlayerWidgetTest.cxx"
