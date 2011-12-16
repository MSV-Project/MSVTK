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
#include "vtkNew.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPolyDataReader.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStringArray.h"
#include "vtkTestUtilities.h"

// STD includes
#include <cstdlib>
#include <iostream>
#include <string>

// Define an implementation of the msvVTKFileSeriesReader
class msvVTKTestPolyDataFileSeriesReader : public msvVTKFileSeriesReader
{
public:
  vtkTypeMacro(msvVTKTestPolyDataFileSeriesReader, msvVTKFileSeriesReader);
  static msvVTKTestPolyDataFileSeriesReader *New();
  virtual void PrintSelf(ostream &os, vtkIndent indent)
    {this->Superclass::PrintSelf(os, indent);}

  virtual void SetReader(vtkAlgorithm* reader)
    {this->Superclass::SetReader(vtkPolyDataReader::SafeDownCast(reader));}

  virtual int CanReadFile(const char* filename)
  {
    //this->Superclass::CanReadFile(filename);
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
          return msvVTKTestPolyDataFileSeriesReader::
            CanReadFile(this->Reader,dataFiles->GetValue(0).c_str());
          }
        }
      return 0;
      }
    else
      {
      return msvVTKTestPolyDataFileSeriesReader::CanReadFile(this->Reader,
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
  msvVTKTestPolyDataFileSeriesReader(){}
  virtual ~msvVTKTestPolyDataFileSeriesReader(){}
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
  msvVTKTestPolyDataFileSeriesReader(const msvVTKTestPolyDataFileSeriesReader&);
  void operator=(const msvVTKTestPolyDataFileSeriesReader&);
};
vtkStandardNewMacro(msvVTKTestPolyDataFileSeriesReader);

// -----------------------------------------------------------------------------
int msvVTKFileSeriesReaderTest1(int argc, char* argv[])
{
  // Get the data test files
  const char* file0 =
    vtkTestUtilities::ExpandDataFileName(argc,argv,"Polydata00.vtk");
  const char* file1 =
    vtkTestUtilities::ExpandDataFileName(argc,argv,"Polydata00.vtk");

  // Create the fileSeriesReader
  vtkNew<vtkPolyDataReader> polyDataReader;
  vtkNew<msvVTKTestPolyDataFileSeriesReader> fileSeriesReader;

  if (fileSeriesReader->CanReadFile(file0))
    {
    std::cerr << "Error: method CanReadFile must return 0 when no reader set."
              << std::endl;
    return EXIT_FAILURE;
    }

  fileSeriesReader->SetReader(polyDataReader.GetPointer());
  if (fileSeriesReader->GetReader() != polyDataReader.GetPointer())
    {
    std::cerr << "Error: internal reader is not the one expected." << std::endl;
    return EXIT_FAILURE;
    }
  if (msvVTKFileSeriesReader::CanReadFile(polyDataReader.GetPointer(), file0))
    {
    std::cerr << "Error: static method CanReadFile must return 0." << std::endl;
    return EXIT_FAILURE;
    }
  if (fileSeriesReader->GetFileNameMethod())
    {
    std::cerr << "Error: FileNameMethod must not be set." << std::endl;
    return EXIT_FAILURE;
    }

  fileSeriesReader->GetMTime();
  fileSeriesReader->RemoveAllFileNames();

  fileSeriesReader->AddFileName(file0);
  fileSeriesReader->AddFileName(file1);

  if (fileSeriesReader->GetNumberOfFileNames() != 2)
    {
    std::cerr << "Error: NumberOfFileNames != to the number of file added"
              << std::endl;
    return EXIT_FAILURE;
    }

  if (fileSeriesReader->GetMetaFileName())
    {
    std::cerr << "Error: return a metafileName when none set." << std::endl;
    return EXIT_FAILURE;
    }

  fileSeriesReader->SetUseMetaFile(true);
  if (!fileSeriesReader->GetUseMetaFile())
    {
    std::cerr << "Error: UseMetaFile true not set." << std::endl;
    return EXIT_FAILURE;
    }

  if (fileSeriesReader->CanReadFile(file0))
    {
    std::cerr << "Error: filename doesn not really points to a metafile, "
              << "must return 0" << std::endl;
    return EXIT_FAILURE;
    }

  fileSeriesReader->UseMetaFileOff();
  if (fileSeriesReader->GetUseMetaFile())
    {
    std::cerr << "Error: UseMetaFile boolean not set." << std::endl;
    return EXIT_FAILURE;
    }

  if (!fileSeriesReader->CanReadFile(file0))
    {
    std::cerr << "Error: reader must be able to read the file." << std::endl;
    return EXIT_FAILURE;
    }

  fileSeriesReader->SetIgnoreReaderTime(false);
  if (fileSeriesReader->GetIgnoreReaderTime())
    {
    std::cerr << "Error: IgnoreReaderTime false not set." << std::endl;
    return EXIT_FAILURE;
    }

  fileSeriesReader->IgnoreReaderTimeOn();
  if (!fileSeriesReader->GetIgnoreReaderTime())
    {
    std::cerr << "Error: IgnoreReaderTime boolean not set." << std::endl;
    return EXIT_FAILURE;
    }

  if (fileSeriesReader->GetFileName(5))
    {
    std::cerr << "Error: retrieve a FileName out of range." << std::endl;
    return EXIT_FAILURE;
    }

  if (strcmp(fileSeriesReader->GetFileName(1),file1) != 0)
    {
    std::cerr << "Error: GetFileName different than expected: " << std::endl
              << "Expected fileName: " << file1 << std::endl
              << "Filename retrived: " << fileSeriesReader->GetFileName(1)
              << std::endl;
    return EXIT_FAILURE;
    }

  // Set the default reader
  fileSeriesReader->SetReader(0);
  if (fileSeriesReader->CanReadFile(file1))
    {
    std::cerr << "CanReadFile return true without having a reader" << std::endl;
    return EXIT_FAILURE;
    }

  fileSeriesReader->SetReader(polyDataReader.GetPointer());
  if (!fileSeriesReader->CanReadFile(file1))
    {
    std::cerr << "CanReadFile return false on proper vtk file" << std::endl;
    return EXIT_FAILURE;
    }

  // Requests without files
  fileSeriesReader->RemoveAllFileNames();
  vtkNew<vtkInformation> infoRequest;
  infoRequest->Set(vtkStreamingDemandDrivenPipeline::REQUEST_INFORMATION());
  fileSeriesReader->ProcessRequest(
    infoRequest.GetPointer(), 0,
    fileSeriesReader->GetExecutive()->GetOutputInformation());

  // Repopulate the fileSerieReader
  fileSeriesReader->AddFileName(file0);
  fileSeriesReader->AddFileName(file1);

  // Create instances of vtkDataObject for all outputsPorts
  vtkNew<vtkInformation> dataObjRequest;
  dataObjRequest->Set(vtkStreamingDemandDrivenPipeline::REQUEST_DATA_OBJECT());
  fileSeriesReader->ProcessRequest(
    dataObjRequest.GetPointer(), 0,
    fileSeriesReader->GetExecutive()->GetOutputInformation());

  // Provide/Generate information on the output data
  vtkNew<vtkInformation> infoRequest1;
  infoRequest1->Set(vtkStreamingDemandDrivenPipeline::REQUEST_INFORMATION());
  fileSeriesReader->ProcessRequest(
    infoRequest1.GetPointer(), 0,
    fileSeriesReader->GetExecutive()->GetOutputInformation());

  int numberOfTimeSteps = fileSeriesReader->GetExecutive()->
    GetOutputInformation()->GetInformationObject(0)->
    Length(vtkStreamingDemandDrivenPipeline::TIME_STEPS());

  if (strcmp(fileSeriesReader->GetCurrentFileName(), file1))
    {
    std::cerr << "Error: GetCurrentFileName different than expected: "
              << std::endl
              << "Expected fileName: " << file1 << std::endl
              << "Filename retrived: " << fileSeriesReader->GetCurrentFileName()
              << std::endl;
    return EXIT_FAILURE;
    }

  if (numberOfTimeSteps < 2)
    {
    std::cerr << "Error: Wrong Time Steps initialization." << std::endl;
    return EXIT_FAILURE;
    }

  // Request for the Next frame
  double time = 1;
  vtkNew<vtkInformation> nextFrameReq;
  nextFrameReq->Set(vtkStreamingDemandDrivenPipeline::REQUEST_UPDATE_EXTENT());
  vtkNew<vtkInformation> nextFrameOutput;
  nextFrameOutput->Set(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEPS(),
    &time, 1);
  vtkNew<vtkInformationVector> nextFrameOutputs;
  nextFrameOutputs->Append(nextFrameOutput.GetPointer());

  fileSeriesReader->ProcessRequest(
    nextFrameReq.GetPointer(), 0, nextFrameOutputs.GetPointer());

  double timeOutputTimeRange[2] = {0,250};
  fileSeriesReader->SetOutputTimeRange(timeOutputTimeRange[0],
                                       timeOutputTimeRange[1]);
  double getTimeOutputTimeRange[2];
  fileSeriesReader->GetOutputTimeRange(getTimeOutputTimeRange);
  fileSeriesReader->Update();
  if (timeOutputTimeRange[0] != getTimeOutputTimeRange[0] ||
      timeOutputTimeRange[1] != getTimeOutputTimeRange[1])
    {
    std::cerr << "Error: when setting the timeRange." << std::endl;
    return EXIT_FAILURE;
    }

  fileSeriesReader->Print(std::cout);

  return EXIT_SUCCESS;
}
