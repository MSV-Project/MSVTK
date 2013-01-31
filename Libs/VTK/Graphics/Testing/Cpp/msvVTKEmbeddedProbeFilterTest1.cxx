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

// MSVTK
#include "msvVTKEmbeddedProbeFilter.h"

// VTK includes
#include "vtkDataArray.h"
#include "vtkDataSet.h"
#include "vtkNew.h"
#include "vtkPointData.h"
#include "vtkPolyDataReader.h"
#include "vtkTestUtilities.h"
#include "vtkUnstructuredGridReader.h"

// STD includes
#include <cmath>

namespace
{

// return true if actualValues is within valueTol of expectedValues,
// false if not. Error message is output in false/fail case.
bool checkValues(int numberOfComponents, const double *actualValues,
  const double *expectedValues, const double valueTol, const char *valueName)
{
  for (int i = 0; i < numberOfComponents; ++i)
    {
    double diff = fabs(actualValues[i] - expectedValues[i]);
    if (diff > valueTol)
      {
      std::cerr << "Error: " << valueName << " value (";
      for (int j = 0; j < numberOfComponents; ++j)
        {
        if (j)
          {
          std::cerr << ",";
          }
        std::cerr << actualValues[j];
        }
      std::cerr << ") does not match expected value (";
      for (int j = 0; j < numberOfComponents; ++j)
        {
        if (j)
          {
          std::cerr << ",";
          }
        std::cerr << expectedValues[j];
        }
      std::cerr << ")"<< std::endl;
      return false;
      }
    }
  return true;
}

}

// -----------------------------------------------------------------------------
int msvVTKEmbeddedProbeFilterTest1(int argc, char* argv[])
{
  // Get the data test files
  const char* file0 =
    vtkTestUtilities::ExpandDataFileName(argc,argv,"block5.vtu");
  const char* file1 =
    vtkTestUtilities::ExpandDataFileName(argc,argv,"msv.vtp");

  // Input
  vtkNew<vtkUnstructuredGridReader> blockReader;
  blockReader->SetFileName(file0);

  // Source
  vtkNew<vtkPolyDataReader> msvReader;
  msvReader->SetFileName(file1);

  vtkNew<msvVTKEmbeddedProbeFilter> embeddedProbe;
  embeddedProbe->SetInputConnection(msvReader->GetOutputPort());
  embeddedProbe->SetSourceConnection(blockReader->GetOutputPort());
  embeddedProbe->SetCellIdArrayName("cellId");
  embeddedProbe->SetParametricCoordinateArrayName("pcoord");

  // check values of embedded probe output
  embeddedProbe->Update();
  vtkDataObject *outputDataObject = embeddedProbe->GetOutputDataObject(0);
  vtkDataSet *output = vtkDataSet::SafeDownCast(outputDataObject);
  if (!output)
    {
    std::cerr << "Error: No output produced" << std::endl;
    return EXIT_FAILURE;
    }

  const double valueTol = 1E-5;
  double coord[3];

  // check point coordinates are shifted to correct locations

  double expectedCoord0[3] = { 0.1, 0.0805887, 0.160589 };
  output->GetPoint(0, coord);
  if (!checkValues(3, coord, expectedCoord0, valueTol, "Output point 0"))
    {
    return EXIT_FAILURE;
    }
  double expectedCoord11[3] = { 1.45, 0.909706, 0.330294 };
  output->GetPoint(11, coord);
  if (!checkValues(3, coord, expectedCoord11, valueTol, "Output point 11"))
    {
    return EXIT_FAILURE;
    }

  // check correct number of arrays in output point data

  vtkPointData *pd = output->GetPointData();
  int numberOfArrays = pd->GetNumberOfArrays();
  if (numberOfArrays != 4)
    {
    std::cerr << "Error: Output point data has " << numberOfArrays <<
      " arrays; expected 4" << std::endl;
    return EXIT_FAILURE;
    }

  // check Source point and cell fields are correctly sampled

  vtkDataArray *dataArray;
  double TestVal;

  dataArray = pd->GetArray("PointTestVal");
  if (!dataArray)
    {
    std::cerr << "Error: Missing output array 'PointTestVal'" << std::endl;
    return EXIT_FAILURE;
    }
  double expectedPointTestVal0 = 0.644;
  TestVal = dataArray->GetTuple1(0);
  if (!checkValues(1, &TestVal, &expectedPointTestVal0, valueTol,
      "Output array PointTestVal 0"))
    {
    return EXIT_FAILURE;
    }
  double expectedPointTestVal11 = 0.704;
  TestVal = dataArray->GetTuple1(11);
  if (!checkValues(1, &TestVal, &expectedPointTestVal11, valueTol,
    "Output array PointTestVal 11"))
    {
    return EXIT_FAILURE;
    }

  dataArray = pd->GetArray("CellTestVal");
  if (!dataArray)
    {
    std::cerr << "Error: Missing output array 'CellTestVal'" << std::endl;
    return EXIT_FAILURE;
    }
  double expectedCellTestVal0 = 5.0;
  TestVal = dataArray->GetTuple1(0);
  if (!checkValues(1, &TestVal, &expectedCellTestVal0, valueTol, "Output array CellTestVal 0"))
    {
    return EXIT_FAILURE;
    }
  double expectedCellTestVal11 = 17;
  TestVal = dataArray->GetTuple1(11);
  if (!checkValues(1, &TestVal, &expectedCellTestVal11, valueTol, "Output array CellTestVal 11"))
    {
    return EXIT_FAILURE;
    }

  // check pcoord is passed through

  dataArray = pd->GetArray("pcoord");
  if (!dataArray)
    {
    std::cerr << "Error: Missing output array 'pcoord'" << std::endl;
    return EXIT_FAILURE;
    }

  double PCoord[3];
  double expectedPCoord0[3] = {  0.2, 0.5, 0.1 };
  dataArray->GetTuple(0, PCoord);
  if (!checkValues(3, PCoord, expectedPCoord0, valueTol, "Output pcoord 0"))
    {
    return EXIT_FAILURE;
    }

  embeddedProbe->Print(std::cout);

  return EXIT_SUCCESS;
}
