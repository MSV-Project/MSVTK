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
#include <QtTest/QTest>

// MSVTK
#include "msvQTimePlayerWidget.h"
#include "msvVTKPolyDataFileSeriesReader.h"

// VTK includes
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyDataReader.h"
#include "vtkStringArray.h"
//#include "vtkTestUtilities.h"

// STD includes
#include <cstdlib>
#include <iostream>

#include "ctkTest.h"

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
  std::cout << "GetIn Treatment" << std::endl;

  QFETCH(QString, Property);
  QFETCH(QVariant, InputPropertyValue);
  QFETCH(QVariant, ExpectedOuputPropertyValue);

  QRegExp rx("Icon");
  if (rx.indexIn(Property) >= 0)
    {
    std::cout << "Icon Treatment" << std::endl;
    }
  else
    {
    std::cout << "Non IconTreadtment" << std::endl;
    //playerWidget.setProperty(Property, InputPropertyValue);
    //QCOMPARE(playerWidget.property(), ExpectedOuputPropertyValue);
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
  /*QTest::newRow("playReverseVisibility") << "playReverseVisibility" << false << false;
  QTest::newRow("boundFramesVisibility") << "boundFramesVisibility" << false << false;
  QTest::newRow("seekFrameVisibility") << "seekFrameVisibility" << false << false;
  QTest::newRow("timeSpinBoxVisibility") << "timeSpinBoxVisibility" << false << false;
  QTest::newRow("playReverseVisibility") << "playReverseVisibility" << true << true;
  QTest::newRow("boundFramesVisibility") << "boundFramesVisibility" << true << true;
  QTest::newRow("seekFrameVisibility") << "seekFrameVisibility" << true << true;
  QTest::newRow("timeSpinBoxVisibility") << "timeSpinBoxVisibility" << true << true;

  // Slider properties
  QTest::newRow("sliderDecimals") << "sliderDecimals" << 2 << 2;
  QTest::newRow("sliderPageStep") << "sliderPageStep" << 5. << 5.;
  QTest::newRow("sliderSingleStep") << "sliderSingleStep" << 5. << 5.;
  QTest::newRow("sliderSingleStep -> Autommatic mode correspounding to a frame")
    << "sliderSingleStep" << -1. << 1.;

  // Direction
  QTest::newRow("direction") << "direction"
                             << static_cast<int>(QAbstractAnimation::Backward)
                             << static_cast<int>(QAbstractAnimation::Backward);
  QTest::newRow("direction") << "direction"
                             << static_cast<int>(QAbstractAnimation::Forward)
                             << static_cast<int>(QAbstractAnimation::Forward);

  // Playback
  QTest::newRow("repeat") << "repeat" << true << true;
  QTest::newRow("repeat") << "repeat" << false << false;
  QTest::newRow("sliderDecimals") << "sliderDecimals" << -1 << 0;
  QTest::newRow("sliderDecimals") << "sliderDecimals" << 2 << 2;
  QTest::newRow("maxFramerate") << "maxFramerate" << 30 << 30;
  QTest::newRow("maxFramerate -> restoreDefault") << "maxFramerate" << 0 << 60;
  QTest::newRow("maxFramerate -> restoreDefault") << "maxFramerate" << -1 << 60;
  QTest::newRow("playSpeed") << "playSpeed" << 4 << 4;
  QTest::newRow("playSpeed") << "playSpeed" << 0 << 1;
  QTest::newRow("playSpeed") << "playSpeed" << -1 << 1;

  QTest::newRow("currentTime") << "currentTime" << 1 << 1;
  QTest::newRow("currentTime") << "currentTime" << -1 << 0;
  QTest::newRow("currentTime") << "currentTime" << 5 << 2;*/
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN(msvQTimePlayerWidgetTest)
#include "moc_msvQTimePlayerWidgetTest.cxx"

// -----------------------------------------------------------------------------
/*int msvQTimePlayerWidgetTest(int argc, char * argv[])
{
  QApplication app(argc, argv);

  // Get the data test files
  const char* file0 =
    vtkTestUtilities::ExpandDataFileName(argc,argv,"Polydata00.vtk");
  const char* file1 =
    vtkTestUtilities::ExpandDataFileName(argc,argv,"Polydata01.vtk");

  // Create the pipeline
  vtkNew<vtkPolyDataReader> polyDataReader;
  vtkNew<msvVTKPolyDataFileSeriesReader> fileSeriesReader;
  fileSeriesReader->SetReader(polyDataReader.GetPointer());

  vtkNew<vtkPolyDataMapper> polyMapper;
  polyMapper->SetInputConnection(fileSeriesReader->GetOutputPort());

  // Instantiate the widget & connect him to the mapper
  msvQTimePlayerWidget* timePlayerWidget = new msvQTimePlayerWidget();
  timePlayerWidget->setFilter(polyMapper.GetPointer());

  if (timePlayerWidget->filter()!=polyMapper.GetPointer()) {
    std::cerr << "TimePlayerWidget unablabe to set the fileSeriesReader"
              << std::endl;
    return EXIT_FAILURE;
    }

  timePlayerWidget->play(true);

  fileSeriesReader->AddFileName(file0);
  fileSeriesReader->AddFileName(file1);

  // Create Instance of vtkDataObject for all outputs ports
  // Call REQUEST_DATA_OBJECT && REQUEST_INFORMATION
  fileSeriesReader->UpdateInformation();

  // Update the Widget given the info provided
  timePlayerWidget->updateFromFilter();

  // Use player
  timePlayerWidget->goToNextFrame();
  timePlayerWidget->goToPreviousFrame();
  timePlayerWidget->goToLastFrame();
  timePlayerWidget->setCurrentTime(1.);
  timePlayerWidget->goToFirstFrame();
  timePlayerWidget->play(false);
  timePlayerWidget->play(true);

  // Wait until the end of the player
  //QTimer::singleShot(50, &app, SLOT(quit()));
  app.exec();

  //return EXIT_SUCCESS;
}*/
