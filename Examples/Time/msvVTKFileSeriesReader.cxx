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

// MSVTK
#include "msvVTKFileSeriesReader.h"

// VTK includes
#include "vtkActor.h"
#include "vtkAlgorithmOutput.h"
#include "vtkNew.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyDataReader.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkSmartPointer.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStringArray.h"
#include "vtkTestUtilities.h"

// STD includes
#include <cstdlib>
#include <iostream>
#include <string>

// msvVTKFileSeriesReader is a meta-reader that can work with various
// readers to load file series. To the pipeline, it looks like a reader
// that supports time. It updates the file name to the internal reader
// whenever a different time step is requested.
//
// If the reader already supports time, then this meta-filter will multiplex the
// time.  It will union together all the times and forward time requests to the
// file with the correct time.  Overlaps are handled by requesting data from the
// file with the upper range the farthest in the future.
//
// There are two ways to specify a series of files.  The first way is by adding
// the filenames one at a time with the AddFileName method.  The second way is
// by providing a single "meta" file.  This meta file is a simple text file that
// lists a file per line.  The files can be relative to the meta file.  This
// method is useful when the actual reader points to a set of files itself.  The
// UseMetaFile toggles between these two methods of specifying files.

// This example shows the usage of the msvVTKFileSeriesReader
// It defines an implementation of the msvVTKFileSeriesReader through
// msvVTKPolyDataFileSeriesReader
// and uses two ".vtk" files which each contains sparse points (PolyData).
// The pipeline is defined as follow:
//          [msvVTKFileSeriesReader]
//                    ^
//                    |
// [msvVTKExamplePolyDataFileSeriesReader]-[vtkPolyDataMapper]->[vtkActor]
//                    |- vtkPolyDataReader

// We first define an implementation of the msvVTKFileSeriesReader as it is an
// abstract class. We here implement the file series reader for polydata files.
class msvVTKExamplePolyDataFileSeriesReader : public msvVTKFileSeriesReader
{
public:
  vtkTypeMacro(msvVTKExamplePolyDataFileSeriesReader, msvVTKFileSeriesReader);
  static msvVTKExamplePolyDataFileSeriesReader *New();
  virtual void PrintSelf(ostream &os, vtkIndent indent)
    {this->Superclass::PrintSelf(os, indent);}

  // We force the FileSeriesReader to only use a vtkPolyDataReader here.
  virtual void SetReader(vtkAlgorithm* reader)
    {this->Superclass::SetReader(vtkPolyDataReader::SafeDownCast(reader));}

  virtual int CanReadFile(const char* filename)
  {
    if (!this->Reader)
      {
      return 0;
      }

    if (this->UseMetaFile)
      {
      // filename really points to a metafile.
      vtkNew<vtkStringArray> dataFiles;
      if (this->ReadMetaDataFile(filename, dataFiles.GetPointer(), 1))
        {
        if (dataFiles->GetNumberOfValues() > 0)
          {
          return msvVTKExamplePolyDataFileSeriesReader::
            CanReadFile(this->Reader,dataFiles->GetValue(0).c_str());
          }
        }
      return 0;
      }
    else
      {
      return msvVTKExamplePolyDataFileSeriesReader::CanReadFile(this->Reader,
                                                             filename);
      }
  }
  static int CanReadFile(vtkAlgorithm* algo, const char* filename)
    {
    vtkPolyDataReader* reader = vtkPolyDataReader::SafeDownCast(algo);
    if(!reader || !filename)
      {
      return 0;
      }

    reader->SetFileName(filename);
    return reader->IsFileValid("polydata");
    }

protected:
  msvVTKExamplePolyDataFileSeriesReader(){}
  virtual ~msvVTKExamplePolyDataFileSeriesReader(){}
  virtual void SetReaderFileName(const char* fname)
    {
    vtkPolyDataReader* reader = vtkPolyDataReader::SafeDownCast(this->Reader);
    if (reader)
      {
      // We want to suppress the modification time change in the Reader.  See
      // msvVTKFileSeriesReader::GetMTime() for details on how this works.
      this->SavedReaderModification = this->GetMTime();
      reader->SetFileName(fname);
      this->HiddenReaderModification = this->Reader->GetMTime();
      }
    this->SetCurrentFileName(fname);
    }

private:
  msvVTKExamplePolyDataFileSeriesReader
    (const msvVTKExamplePolyDataFileSeriesReader&);
  void operator=(const msvVTKExamplePolyDataFileSeriesReader&);
};
vtkStandardNewMacro(msvVTKExamplePolyDataFileSeriesReader);

// -----------------------------------------------------------------------------
int main(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  // Create the RenderWindow, Renderer and Interactor style
  //
  vtkNew<vtkRenderer> ren1;
  ren1->SetBackground(0.1, 0.2, 0.4);
  vtkNew<vtkRenderWindow> renWin;
  renWin->AddRenderer(ren1.GetPointer());

  vtkNew<vtkRenderWindowInteractor> iren;
  vtkNew<vtkInteractorStyleTrackballCamera> irenStyle;
  iren->SetRenderWindow(renWin.GetPointer());
  iren->SetInteractorStyle(irenStyle.GetPointer());

  // Create the Pipeline.
  vtkNew<msvVTKExamplePolyDataFileSeriesReader> polyDataSeriesReader;
  // The reader associated to our fileSeriesReader.
  vtkNew<vtkPolyDataReader> polyDataReader;
  // The mapper mapping data to graphics primitives.
  // Will be connected to the outputPort of the polyDataSeriesReader.
  vtkNew<vtkPolyDataMapper> polyDataMapper;
  // Then a vtkActor is used to represent an entity in a rendering scene.
  vtkNew<vtkActor> polyDataActor;

  // Build the Pipeline as previously defined
  polyDataSeriesReader->SetReader(polyDataReader.GetPointer());
  polyDataMapper->ScalarVisibilityOff();
  polyDataMapper->SetInputConnection(polyDataSeriesReader->GetOutputPort());
  polyDataActor->SetMapper(polyDataMapper.GetPointer());

  // Get the data test files and add them to the series reader.
  // Note: the two files are two point clouds in ".vtk" format.
  std::string dataPath = DATA_TESTING_PATH;
  std::string file0  = dataPath + "Polydata00.vtk";
  std::string file1  = dataPath + "Polydata01.vtk";
  polyDataSeriesReader->AddFileName(file0.c_str());
  polyDataSeriesReader->AddFileName(file1.c_str());

  // Once the file have been added, we can call Update information method.
  // The method will do two request at this stage:
  // REQUEST_DATA_OBJECT :
  // Create an instance of vtkDataObject for all outputs ports
  // REQUEST_INFORMATION :
  // Provide/Generate information about the output data
  polyDataSeriesReader->UpdateInformation();

  // The pipeline is now ready.
  // You can use all the API of the fileSeriesReader

  // Examples:

  // Set the output time range; it lets you specifying you own time range if
  // the reader does not manage this information. It linearly associates
  // each step to an interval of time.
  // By default the range do a 1:1 mapping between the
  // Time_Range and the Time_Steps.

  // Thus, by default we have here a timerange of [0,1].
  double timeRange[2] = {0, 250};
  // After setting this timeRange of [0,250] we will have the first time step
  // (i.e. the first .vtk file) corresponding to the interval [0,125[
  // and the second to the intervale [125,250[;
  polyDataSeriesReader->SetOutputTimeRange(timeRange);

  // If you would like to restore the default timerange you can just proceed
  // using the number of file:
  double upperTimeRange = polyDataSeriesReader->GetNumberOfFileNames()-1;
  polyDataSeriesReader->SetOutputTimeRange(0.,upperTimeRange);

  // To request time through the pipeline you just have to follow the normal
  // procedure using an executive:
  vtkStreamingDemandDrivenPipeline* sdd = vtkStreamingDemandDrivenPipeline::
    SafeDownCast(polyDataMapper->GetInputConnection(0,0)->
                 GetProducer()->GetExecutive());

  sdd->SetUpdateTimeStep(0, 1.);  // Request a time update at t = 1.

  // Render
  polyDataActor->VisibilityOn();
  ren1->AddActor(polyDataActor.GetPointer());

  // Recenter the camera given the bound of our current point cloud.
  double extent[6];
  polyDataMapper->GetBounds(extent);
  ren1->ResetCamera(extent);

  iren->Initialize();
  renWin->Render();
  iren->Start();

  return EXIT_SUCCESS;
}
