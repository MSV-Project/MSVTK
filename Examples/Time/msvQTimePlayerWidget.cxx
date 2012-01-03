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

// This example shows the usage of the msvQTimePlayerWidget
// It uses the msvVTKPolyDataFileSeriesReader module and two ".vtk" files which
// each contains sparse points (PolyData).
// The pipeline is defined as follow:
// [msvVTKPolyDataFileSeriesReader]-[vtkPolyDataMapper]
//              |- vtkPolyDataReader        |- msvQTimePlayerWidget

// -----------------------------------------------------------------------------
int main(int argc, char * argv[])
{
  QApplication app(argc, argv);

  // Get the data test files
  // The two files are two point clouds in ".vtk" format.
  std::string dataPath = DATA_TESTING_PATH;
  std::string file0  = dataPath + "Polydata00.vtk";
  std::string file1  = dataPath + "Polydata01.vtk";

  // Create the pipeline:
  // A polydata reader is instantiated and associated to
  // the msvVTKPolyDataFileSeriesReader.
  vtkNew<vtkPolyDataReader> polyDataReader;
  vtkNew<msvVTKPolyDataFileSeriesReader> fileSeriesReader;
  fileSeriesReader->SetReader(polyDataReader.GetPointer());

  // A vtkPolyDataMapper is then connected to the output of the file series
  // reader to represent the current point cloud in the VTK 3D render view.
  // In order to get a final visualization we would create a vtkActor then.
  vtkNew<vtkPolyDataMapper> polyMapper;
  polyMapper->SetInputConnection(fileSeriesReader->GetOutputPort());

  // We instantiate the widget msvQTimePlayerWidget & connect him on the output
  // filter as we want to manage the output. In our case we thus might
  // connect him the the polydata mapper.
  msvQTimePlayerWidget* timePlayerWidget = new msvQTimePlayerWidget();
  timePlayerWidget->setFilter(polyMapper.GetPointer());

  // Populate the pipeline:
  // Once we have instantiated the fileSeries reader, we can add the files.
  fileSeriesReader->AddFileName(file0.c_str());
  fileSeriesReader->AddFileName(file1.c_str());

  // Create Instance of vtkDataObject for all outputs ports
  // Call REQUEST_DATA_OBJECT && REQUEST_INFORMATION
  fileSeriesReader->UpdateInformation();

  // Update the Widget given the info provided by the pipeline.
  timePlayerWidget->updateFromFilter();

  // You can now animate the visualization using
  // a full range of functionalities and signals.

  // Examples:

  // Player basics
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

  // Time setting
  timePlayerWidget->setCurrentTime(1.);
  timePlayerWidget->setCurrentTime(-1.);

  // Using a QEventLoop to manage the time in non-GUI context.
  QEventLoop* loop = new QEventLoop();
  QObject::connect(timePlayerWidget, SIGNAL(onTimeout()), loop, SLOT(quit()));

  // The player play the animation in a repeated mode
  // timePlayerWidget->setRepeat(true);
  // timePlayerWidget->onPlayReverse(true);

  // while(true)
  //  {
    // loop->exec();

    // Actions and/or checks
    // ...
  // }

  timePlayerWidget->show();
  return app.exec();
}
