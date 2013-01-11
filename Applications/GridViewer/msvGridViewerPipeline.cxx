/*==============================================================================

  Library: MSVTK

  Copyright (c) The University of Auckland

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

// std includes
#include <fstream>

// MSV includes
#include "msvGridViewerPipeline.h"
#include "msvVTKDataFileSeriesReader.h"
#include "msvVTKEmbeddedProbeFilter.h"

// VTK includes
#include "vtkActor.h"
#include "vtkActorCollection.h"
#include "vtkAlgorithmOutput.h"
#include "vtkDataObjectReader.h"
#include "vtkDataSetSurfaceFilter.h"
#include "vtkExtractEdges.h"
#include "vtkFieldDataToAttributeDataFilter.h"
//#include "vtkMergeFilter.h"
#include "vtkMergeDataObjectFilter.h"
#include "vtkNew.h"
#include "vtkOrientationMarkerWidget.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyDataReader.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkSmartPointer.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStructuredGridReader.h"
#include "vtkTemporalDataSetCache.h"
#include "vtkUnstructuredGridReader.h"

//------------------------------------------------------------------------------
// msvGridViewerPipeline methods

msvGridViewerPipeline::msvGridViewerPipeline()
{
  // Renderer
  this->threeDRenderer = vtkSmartPointer<vtkRenderer>::New();
  this->threeDRenderer->SetBackground(0.1, 0.2, 0.4);
  this->threeDRenderer->SetBackground2(0.2, 0.4, 0.8);
  this->threeDRenderer->SetGradientBackground(true);

  this->axes = vtkSmartPointer<vtkAxesActor>::New();
  this->orientationMarker = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
  this->orientationMarker->SetOutlineColor(0.9300, 0.5700, 0.1300);
  this->orientationMarker->SetOrientationMarker(axes);
}

//------------------------------------------------------------------------------
msvGridViewerPipeline::~msvGridViewerPipeline()
{
  this->clear();
}

//------------------------------------------------------------------------------
void msvGridViewerPipeline::clear()
{
  this->threeDRenderer->RemoveAllViewProps();
  this->endMapper = 0;
}

int msvGridViewerPipeline::readCommand(std::istream &gridFile,
  std::string &command, std::vector<std::string> &options)
{
  command = "";
  options.clear();
  char c;
  while (true)
    {
    c = gridFile.peek();
    if (isspace(c))
      {
      gridFile.get();
      }
    else if (c == '#')
      {
      char comment[256];
      do
        {
        gridFile.getline(comment, sizeof(comment));
        } while (gridFile.fail());
      }
    else
      {
      break;
      }
    }
  if (gridFile.eof())
    {
    return 1;
    }
  while (true)
    {
    c = gridFile.peek();
    if (!isalpha(c))
      {
      break;
      }
    gridFile.get();
    command += c;
    }
  if (command.size() <= 0)
    {
    cerr << "Missing command.\n";
    return 0;
    }
  while (isspace(gridFile.peek()))
    {
    gridFile.get();
    }
  c = gridFile.get();
  if (c != '(')
    {
    cerr << "Command '" << command << "' is followed by illegal character '" << c << "'. Expected '(<name> <options...>)\n";
    return 0;
    }

  // read whitespace separated name and options until final ')'
  while (true)
    {
    if (gridFile.eof())
      {
      cerr << "Unexpected end of file before closing brace of command '" << command << "'.\n";
      return 0;
      }
    c = gridFile.peek();
    if (isspace(c))
      {
      gridFile.get();
      }
    else if (c == ')')
      {
      gridFile.get();
      break;
      }
    else
      {
      bool quoted = (c == '"');
      if (quoted)
        {
        gridFile.get();
        }
      std::string token;
      while (!gridFile.eof())
        {
        c = gridFile.peek();
        if (quoted)
          {
          if (c == '"')
            {
            gridFile.get();
            quoted = false;
            break;
            }
          }
        else if (isspace(c) || (c == ')'))
          {
          break;
          }
        c = gridFile.get();
        token += c;
        }
      if (quoted && gridFile.eof())
        {
        cerr << "Unexpected end of file before closing quote of command '" << command << "'.\n";
        return 0;
        }
      options.push_back(token);
      }
    }
  //cout << command << "\n";
  //for (int i = 0; i < options.size(); i++)
  //  {
  //  cout << "  " << options[i] << "\n";
  //  }
  return 1;
}

//------------------------------------------------------------------------------
// Caller is responsible for calling Delete on any returned pointer
vtkDataReader *msvGridViewerPipeline::createDataReader(std::string &readerName)
{
  if (readerName == "vtkDataObjectReader")
    return vtkDataObjectReader::New();
  else if (readerName == "vtkPolyDataReader")
    return vtkPolyDataReader::New();
  else if (readerName == "vtkStructuredGridReader")
    return vtkStructuredGridReader::New();
  else if (readerName == "vtkUnstructuredGridReader")
    return vtkUnstructuredGridReader::New();
  return 0;
}

//------------------------------------------------------------------------------
int msvGridViewerPipeline::checkOption(const char *token,
  std::string &name, std::vector<std::string> &options, int &i, int minArgs)
{
  if (!token)
    return 0;
  if ((i >= options.size()) || (options[i] != token))
    return 0;
  ++i;
  if (i + minArgs > options.size())
    {
    cerr << "'" << name << "' is missing " << token << " argument.\n";
    i = static_cast<int>(options.size());
    return 0;
    }
  return 1;
}

//------------------------------------------------------------------------------
vtkAlgorithm *msvGridViewerPipeline::checkAlgorithmOption(const char *token,
  std::string &name, std::vector<std::string> &options, int &i,
  vtkNameMap &objects)
{
  if (!this->checkOption(token, name, options, i, /*minArgs*/1))
    return 0;
  vtkSmartPointer<vtkObject> inputObject = objects[options[i]];
  if (!inputObject)
    {
    cerr << "'" << name << token << " argument '" << options[i] << "' not found.\n";
    i = static_cast<int>(options.size());
    return 0;
    }
  vtkAlgorithm *inputAlgorithm = vtkAlgorithm::SafeDownCast(inputObject);
  if (!inputAlgorithm)
    {
    cerr << "'" << name << token << " argument '" << options[i] << "' must be a vtkAlgorithm.\n";
    i = static_cast<int>(options.size());
    return 0;
    }
  ++i;
  return inputAlgorithm;
}

//------------------------------------------------------------------------------
int msvGridViewerPipeline::readGridFile(const char *gridFileName)
{
  this->clear();
  if (0 == gridFileName)
    {
    return 0;
    }
  vtkNameMap objects;

  std::ifstream gridFile(gridFileName);
  if (gridFile.fail())
    {
    return 0;
    }
  std::string command;
  std::vector<std::string> options;

  while ((this->readCommand(gridFile, command, options) && (command.size() > 0)))
    {
    if (options.size() < 1)
      {
      cerr << "Missing object name for command '" << command << "'\n";
      return 0;
      }
    std::string name = options[0];
    vtkSmartPointer<vtkObject> existingObject = objects[name];
    if (0 != existingObject.GetPointer())
      {
      cerr << "Command " << command << " redefines name '" << name << "'.\n";
      return 0;
      }
    vtkSmartPointer<vtkObject> object;
    int optionIndex = 1;
    vtkDataReader *tmpDataReader;

    if (0 != (tmpDataReader = this->createDataReader(command)))
      {
      object = tmpDataReader;
      vtkSmartPointer<vtkDataReader> dataReader;
      dataReader.TakeReference(tmpDataReader);
      if (this->checkOption("FILE", name, options, optionIndex, /*minArgs*/1))
        {
        dataReader->SetFileName(options[optionIndex].c_str());
        ++optionIndex;
        }
      if ((0 == dataReader->GetFileName()) || (optionIndex < options.size()))
        {
          cerr << "'" << name << "' requires only FILE <filename> option.\n";
          return 0;
        }
      }

    else if (command == "msvVTKDataFileSeriesReader")
      {
      vtkNew<msvVTKDataFileSeriesReader> fileSeriesReader;
      object = fileSeriesReader.GetPointer();
      while (optionIndex < options.size())
        {
        if (this->checkOption("READER", name, options, optionIndex, /*numArgs*/1))
          {
          vtkSmartPointer<vtkDataReader> dataReader;
          dataReader.TakeReference(this->createDataReader(options[optionIndex]));
          if (0 == dataReader.GetPointer())
            {
            cerr << "Unrecognised reader '" << options[optionIndex] << "'\n";
            return 0;
            }
          ++optionIndex;
          fileSeriesReader->SetReader(dataReader.GetPointer());
          }
        else if (this->checkOption("TIMERANGE", name, options, optionIndex, /*minArgs*/2))
          {
          double timeRange[2];
          timeRange[0] = atof(options[optionIndex].c_str());
          ++optionIndex;
          timeRange[1] = atof(options[optionIndex].c_str());
          ++optionIndex;
          fileSeriesReader->SetOutputTimeRange(timeRange);
          }
        else if (this->checkOption("FILES", name, options, optionIndex, /*minArgs*/1))
          {
          while (optionIndex < options.size())
            {
            fileSeriesReader->AddFileName(options[optionIndex].c_str());
            ++optionIndex;
            }
          }
        else
          {
          if (optionIndex < options.size())
            {
            cerr << "'" << name << "' has unrecognised token '" << options[optionIndex] << "'\n";
            return 0;
            }
          }
        }
      }

    else if (command == "msvVTKEmbeddedProbeFilter")
      {
      vtkNew<msvVTKEmbeddedProbeFilter> embeddedProbe;
      object = embeddedProbe.GetPointer();
      while (optionIndex < options.size())
        {
        vtkAlgorithm *inputAlgorithm = 0;
        if (0 != (inputAlgorithm = this->checkAlgorithmOption("INPUT", name, options, optionIndex, objects)))
          {
          embeddedProbe->SetInputConnection(inputAlgorithm->GetOutputPort());
          }
        else if (0 != (inputAlgorithm = this->checkAlgorithmOption("SOURCE", name, options, optionIndex, objects)))
          {
          embeddedProbe->SetSourceConnection(inputAlgorithm->GetOutputPort());
          }
        else if (this->checkOption("PARAMETRICCOORDINATEARRAYNAME", name, options, optionIndex, /*minArgs*/1))
          {
          embeddedProbe->SetParametricCoordinateArrayName(options[optionIndex].c_str());
          ++optionIndex;
          }
        else if (this->checkOption("CELLIDARRAYNAME", name, options, optionIndex, /*minArgs*/1))
          {
          embeddedProbe->SetCellIdArrayName(options[optionIndex].c_str());
          ++optionIndex;
          }
        else
          {
          if (optionIndex < options.size())
            {
            cerr << "'" << name << "' has unrecognised token '" << options[optionIndex] << "'\n";
            return 0;
            }
          }
        }
      }

    else if (command == "vtkActor")
      {
      vtkNew<vtkActor> actor;
      object = actor.GetPointer();
      while (optionIndex < options.size())
        {
        if (this->checkOption("MAPPER", name, options, optionIndex, /*minArgs*/1))
          {
          vtkSmartPointer<vtkObject> inputObject = objects[options[optionIndex]];
          if (!inputObject)
            {
            cerr << "'" << name << " MAPPER argument '" << options[optionIndex] << "' not found.\n";
            optionIndex = static_cast<int>(options.size());
            return 0;
            }
          vtkMapper *mapper = vtkMapper::SafeDownCast(inputObject);
          if (!mapper)
            {
            cerr << "'" << name << " MAPPER argument '" << options[optionIndex] << "' must be a vtkMapper.\n";
            optionIndex = static_cast<int>(options.size());
            return 0;
            }
          ++optionIndex;
          actor->SetMapper(mapper);
          this->endMapper = mapper;
          }
        else if (this->checkOption("COLOR", name, options, optionIndex, /*minArgs*/3))
          {
          double color[3];
          for (int comp = 0; comp < 3; ++comp)
            {
            color[comp] = atof(options[optionIndex].c_str());
            ++optionIndex;
            }
          actor->GetProperty()->SetColor(color);
          }
        else if (this->checkOption("OPACITY", name, options, optionIndex, /*minArgs*/1))
          {
          double opacity = atof(options[optionIndex].c_str());
          ++optionIndex;
          actor->GetProperty()->SetOpacity(opacity);
          }
        else if (this->checkOption("VISIBILITY", name, options, optionIndex, /*minArgs*/1))
          {
          int vis = atoi(options[optionIndex].c_str());
          ++optionIndex;
          actor->SetVisibility(vis);
          }
        else
          {
          if (optionIndex < options.size())
            {
            cerr << "'" << name << "' has unrecognised token '" << options[optionIndex] << "'\n";
            return 0;
            }
          }
        }
        if (0 == actor->GetMapper())
          {
          cerr << "'" << name << "' requires the MAPPER <name> option\n";
          }
        this->threeDRenderer->AddActor(actor.GetPointer());
      }

    else if (command == "vtkDataSetSurfaceFilter")
      {
      vtkNew<vtkDataSetSurfaceFilter> dataSetSurface;
      object = dataSetSurface.GetPointer();
      vtkAlgorithm *inputAlgorithm = 0;
      if (0 != (inputAlgorithm = this->checkAlgorithmOption("INPUT", name, options, optionIndex, objects)))
        {
        dataSetSurface->SetInputConnection(inputAlgorithm->GetOutputPort());
        }
      if (!inputAlgorithm || (optionIndex < options.size()))
        {
          cerr << "'" << name << "' requires only INPUT <algorithm> option.\n";
        }
      }

    else if (command == "vtkExtractEdges")
      {
      vtkNew<vtkExtractEdges> edges;
      object = edges.GetPointer();
      vtkAlgorithm *inputAlgorithm = 0;
      if (0 != (inputAlgorithm = this->checkAlgorithmOption("INPUT", name, options, optionIndex, objects)))
        {
        edges->SetInputConnection(inputAlgorithm->GetOutputPort());
        }
      if (!inputAlgorithm || (optionIndex < options.size()))
        {
          cerr << "'" << name << "' requires only INPUT <algorithm> option.\n";
        }
      }

    else if (command == "vtkFieldDataToAttributeDataFilter")
      {
      vtkNew<vtkFieldDataToAttributeDataFilter> fieldToAttribute;
      object = fieldToAttribute.GetPointer();
      while (optionIndex < options.size())
        {
        vtkAlgorithm *inputAlgorithm = 0;
        if (0 != (inputAlgorithm = this->checkAlgorithmOption("INPUT", name, options, optionIndex, objects)))
          {
          fieldToAttribute->SetInputConnection(inputAlgorithm->GetOutputPort());
          }
        else if (this->checkOption("SCALAR", name, options, optionIndex, /*minArgs*/1))
          {
          fieldToAttribute->SetScalarComponent(0, options[optionIndex].c_str(), 0);
          ++optionIndex;
          }
        else if (this->checkOption("INPUTCELLDATA", name, options, optionIndex, /*minArgs*/0))
          {
          fieldToAttribute->SetInputFieldToCellDataField();
          }
        else if (this->checkOption("INPUTFIELDDATA", name, options, optionIndex, /*minArgs*/0))
          {
          fieldToAttribute->SetInputFieldToDataObjectField();
          }
        else if (this->checkOption("INPUTPOINTDATA", name, options, optionIndex, /*minArgs*/0))
          {
          fieldToAttribute->SetInputFieldToPointDataField();
          }
        else if (this->checkOption("OUTPUTCELLDATA", name, options, optionIndex, /*minArgs*/0))
          {
          fieldToAttribute->SetOutputAttributeDataToCellData();
          }
        else if (this->checkOption("OUTPUTPOINTDATA", name, options, optionIndex, /*minArgs*/0))
          {
          fieldToAttribute->SetOutputAttributeDataToPointData();
          }
        else
          {
          if (optionIndex < options.size())
            {
            cerr << "'" << name << "' has unrecognised token '" << options[optionIndex] << "'\n";
            return 0;
            }
          }
        }
      }

    else if (command == "vtkMergeDataObjectFilter")
      {
      vtkNew<vtkMergeDataObjectFilter> mergedDataSet;
      object = mergedDataSet.GetPointer();
      while (optionIndex < options.size())
        {
        vtkAlgorithm *inputAlgorithm = 0;
        if (0 != (inputAlgorithm = this->checkAlgorithmOption("INPUT", name, options, optionIndex, objects)))
          {
          mergedDataSet->SetInputConnection(inputAlgorithm->GetOutputPort());
          }
        else if (0 != (inputAlgorithm = this->checkAlgorithmOption("DATAINPUT", name, options, optionIndex, objects)))
          {
          // GRC following doesn't pipeline time:
          mergedDataSet->SetInputConnection(1, inputAlgorithm->GetOutputPort());
          }
        else if (this->checkOption("OUTPUTCELLDATA", name, options, optionIndex, /*minArgs*/0))
          {
          mergedDataSet->SetOutputFieldToCellDataField();
          }
        else if (this->checkOption("OUTPUTFIELDDATA", name, options, optionIndex, /*minArgs*/0))
          {
          mergedDataSet->SetOutputFieldToDataObjectField();
          }
        else if (this->checkOption("OUTPUTPOINTDATA", name, options, optionIndex, /*minArgs*/0))
          {
          mergedDataSet->SetOutputFieldToPointDataField();
          }
        else
          {
          if (optionIndex < options.size())
            {
            cerr << "'" << name << "' has unrecognised token '" << options[optionIndex] << "'\n";
            return 0;
            }
          }
        }
      }

    else if (command == "vtkPolyDataMapper")
      {
      vtkNew<vtkPolyDataMapper> polyMapper;
      object = polyMapper.GetPointer();
      while (optionIndex < options.size())
        {
        vtkAlgorithm *inputAlgorithm = 0;
        if (0 != (inputAlgorithm = this->checkAlgorithmOption("INPUT", name, options, optionIndex, objects)))
          {
          polyMapper->SetInputConnection(inputAlgorithm->GetOutputPort());
          }
        else if (this->checkOption("SCALARVISIBILITYON", name, options, optionIndex, /*minArgs*/0))
          {
          polyMapper->ScalarVisibilityOn();
          }
        else
          {
          if (optionIndex < options.size())
            {
            cerr << "'" << name << "' has unrecognised token '" << options[optionIndex] << "'\n";
            return 0;
            }
          }
        }
      }

    else if (command == "vtkTemporalDataSetCache")
      {
      vtkNew<vtkTemporalDataSetCache> temporalCache;
      object = temporalCache.GetPointer();
      while (optionIndex < options.size())
        {
        vtkAlgorithm *inputAlgorithm = 0;
        if (0 != (inputAlgorithm = this->checkAlgorithmOption("INPUT", name, options, optionIndex, objects)))
          {
          temporalCache->SetInputConnection(inputAlgorithm->GetOutputPort());
          }
        else if (this->checkOption("CACHESIZE", name, options, optionIndex, /*minArgs*/1))
          {
          int size = atoi(options[optionIndex].c_str());
          ++optionIndex;
          temporalCache->SetCacheSize(size);
          }
        else
          {
          if (optionIndex < options.size())
            {
            cerr << "'" << name << "' has unrecognised token '" << options[optionIndex] << "'\n";
            return 0;
            }
          }
        }
      }

    else
      {
      cerr << "Unrecognised command or vtk class '" << command << "'\n";
      return 0;
      }

    if (object.GetPointer())
      {
      objects[options[0]] = object;
      }
    }
  gridFile.close();

  double extent[6];
  this->endMapper->GetBounds(extent);
  this->threeDRenderer->ResetCamera(extent);

  return 1;
}

void msvGridViewerPipeline::addToRenderWindow(vtkRenderWindow *renderWindow)
{
  renderWindow->AddRenderer(this->threeDRenderer);

  // Marker annotation
  this->orientationMarker->SetInteractor
    (this->threeDRenderer->GetRenderWindow()->GetInteractor());
  this->orientationMarker->SetEnabled(1);
  this->orientationMarker->InteractiveOn();
}

void msvGridViewerPipeline::updateTime(double time)
{
  // request update at time for all visible actors' mappers
  vtkActorCollection *actors = this->threeDRenderer->GetActors();
  if (actors)
    {
    actors->InitTraversal();
    vtkActor *actor = 0;
    while (0 != (actor = actors->GetNextActor()))
      {
      if (actor->GetVisibility())
        {
        vtkMapper *mapper = actor->GetMapper();
        if (mapper)
          {
          for (int portIndex = 0;
              portIndex < mapper->GetNumberOfInputPorts();
              ++portIndex)
            {
            for (int connectionIndex = 0;
              connectionIndex < mapper->GetNumberOfInputConnections(portIndex);
              ++connectionIndex)
              {
                vtkStreamingDemandDrivenPipeline* sdd = vtkStreamingDemandDrivenPipeline::
                  SafeDownCast(mapper->GetInputConnection(portIndex, connectionIndex)
                    ->GetProducer()->GetExecutive());

                sdd->SetUpdateTimeStep(portIndex, time);  // Request a time update
              }
            }
          }
        }
      }
    }
}
