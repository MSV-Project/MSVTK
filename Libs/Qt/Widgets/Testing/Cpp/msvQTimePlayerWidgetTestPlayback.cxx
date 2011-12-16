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
#include <QEventLoop>
#include <QTimer>

// MSVTK
#include "msvQTimePlayerWidget.h"
#include "msvVTKPolyDataFileSeriesReader.h"

// VTK includes
#include "vtkNew.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyDataReader.h"
#include "vtkStringArray.h"
#include "vtkTestUtilities.h"

// STD includes
#include <cstdlib>
#include <iostream>

// -----------------------------------------------------------------------------
int msvQTimePlayerWidgetTestPlayback(int argc, char * argv[])
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

  // Time setting
  timePlayerWidget->setCurrentTime(1.);
  if (timePlayerWidget->currentTime()!=1.) {
    std::cerr << "Current time (1.) not as expected (1.)"
              << std::endl;
    return EXIT_FAILURE;
    }

  timePlayerWidget->setCurrentTime(-1.);
  if (timePlayerWidget->currentTime()!=0.) {
    std::cerr << "Current time (-1.) not as expected (0.)"
              << std::endl;
    return EXIT_FAILURE;
    }

  timePlayerWidget->setCurrentTime(5);
  if (timePlayerWidget->currentTime()!=1.) {
    std::cerr << "Current time (5.) not as expected (1.)"
              << std::endl;
    return EXIT_FAILURE;
    }

  // Use player basics
  timePlayerWidget->goToNextFrame();
  timePlayerWidget->goToPreviousFrame();
  timePlayerWidget->goToLastFrame();
  timePlayerWidget->setCurrentTime(1.);
  timePlayerWidget->goToFirstFrame();
  timePlayerWidget->play(false);
  timePlayerWidget->onPlay(true);
  timePlayerWidget->onPlayReverse(true);
  timePlayerWidget->pause();
  timePlayerWidget->stop();

  // We reset the Slider to the initial value if we play from the end
  timePlayerWidget->goToLastFrame();
  timePlayerWidget->onPlay(true);

  QEventLoop* loop = new QEventLoop();
  QObject::connect(timePlayerWidget, SIGNAL(onTimeout()), loop, SLOT(quit()));

  // When the time arrives at the end of the range, it stops itself
  // when it plays in the forward direction.
  timePlayerWidget->goToFirstFrame();
  timePlayerWidget->setRepeat(false);
  timePlayerWidget->onPlay(true);
  loop->exec();
  loop->exec();
  if (timePlayerWidget->currentTime() != 1) {
    std::cerr << "The timer has not stopped at the end as expected."
              << std::endl;
    }

  // When the time arrives at the beginning of the range, it stops itself
  // when it plays in the backward direction.
  timePlayerWidget->goToLastFrame();
  timePlayerWidget->onPlayReverse(true);
  loop->exec();
  loop->exec();
  if (timePlayerWidget->currentTime() != 0) {
    std::cerr << "The timer has not stopped at the beginning as expected."
              << std::endl;
    }

  // When the time arrives at the end of the range, it loops
  // when it plays in the forward direction.
  timePlayerWidget->goToFirstFrame();
  timePlayerWidget->setRepeat(true);
  timePlayerWidget->onPlay(true);
  loop->exec();
  loop->exec();
  if (timePlayerWidget->currentTime() != 0) {
    std::cerr << "The timer has stopped, instead of looping."
              << std::endl;
  }

  // When the time arrives at the beginning of the range, it loops
  // when it plays in the backward direction.
  timePlayerWidget->goToLastFrame();
  timePlayerWidget->setRepeat(true);
  timePlayerWidget->onPlayReverse(true);
  loop->exec();
  loop->exec();
  if (timePlayerWidget->currentTime() != 1) {
    std::cerr << "The timer has stopped, instead of looping."
              << std::endl;
  }

  // Wait until the end of the player
  QTimer::singleShot(50, &app, SLOT(quit()));
  return app.exec();
}
