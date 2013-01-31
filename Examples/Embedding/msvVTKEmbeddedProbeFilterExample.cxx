/*==============================================================================

  Library: MSVTK

  Copyright (c) The University of Auckland.

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

// MSVTK includes
#include "msvVTKDataFileSeriesReader.h"
#include "msvVTKEmbeddedProbeFilter.h"

// VTK includes
#include "vtkActor.h"
#include "vtkAlgorithmOutput.h"
#include "vtkDataSetSurfaceFilter.h"
#include "vtkExtractEdges.h"
#include "vtkFieldDataToAttributeDataFilter.h"
#include "vtkNew.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyDataReader.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkTestUtilities.h"
#include "vtkUnstructuredGridReader.h"

// STD includes
#include <cstdlib>
#include <string>

// Embedding is a technique used to simplify behaviour and reduce size
// of multi-scale models.
// 
// Typically, an 'embedded' dataset is fine-scale and only fields needing
// to be resolved at that scale are defined on it. It has additional fields
// specifying parametric coordinates within cells of a 'host' dataset it
// is considered embedded in. The host is typically a coarser-scale model
// with fields defining material properties and other information not needing
// to be resolved at finer scale. Commonly the host dataset (a mesh) is
// deforming, and the geometry of the host is inherited by all datasets
// embedded in it; in such cases there can be a considerable saving in memory
// usage since time-varying field data is only stored at the coarsest scale.
// 
// Examples of embedding in multi-scale modelling:
// - Electrocardiology: fine scale computational cell points (fine spacing
//   is needed to capture detail of electrical potential) embedded in coarse
//   geometric mesh.
// - Coronary Flow: coronary artery tree embedded in surface of coarse deforming
//   heart ventricles mesh.
// - Lung Modelling: bronchial tree with enormous scale changes embedded in
//   coarse mesh representing lung continuum deformation model.
// 
// This example demonstrates the use of msvVTKEmbeddedProbeFilter which allows
// a dataset to inherit fields and geometry (i.e. point coordinates) from a
// host (source) dataset by sampling in host cells given by cell ID and
// parametric coordinate fields defined on the original dataset. It also uses
// msvVTKDataFileSeriesReader to handle the time-varying host dataset.
// [vtkMergeDataObjectFilter can also be used to incorporate time-varying field
// data at any scale without reloading geometry, topology and non-time-varying
// fields on the dataset; this is not demonstrated here.]

namespace
{

// Convenience function for requesting update at given time
void RequestTimeUpdate(vtkAlgorithm *algorithm, double time)
{
  // To request time through the pipeline you just have to follow the normal
  // procedure using an executive:
  vtkStreamingDemandDrivenPipeline* sdd =
    vtkStreamingDemandDrivenPipeline::SafeDownCast(algorithm->GetExecutive());

  // Request update at given time
  sdd->SetUpdateTimeStep(0, time);
}

}

// -----------------------------------------------------------------------------
int main(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{

  // Create the RenderWindow, Renderer and Interactor style

  vtkNew<vtkRenderer> ren1;
  ren1->SetBackground(0.1, 0.2, 0.4);
  vtkNew<vtkRenderWindow> renWin;
  renWin->AddRenderer(ren1.GetPointer());
  renWin->SetSize(480, 360);

  vtkNew<vtkRenderWindowInteractor> iren;
  vtkNew<vtkInteractorStyleTrackballCamera> irenStyle;
  iren->SetRenderWindow(renWin.GetPointer());
  iren->SetInteractorStyle(irenStyle.GetPointer());

  // Create the Pipeline.

  // Read time-varying geometry for a 2-cell block host mesh
  // The block is 2 regular cubes in its initial state, and increasingly
  // distorted by 'twisting' the ends in subsequent time steps.

  vtkNew<msvVTKDataFileSeriesReader> blockFileSeriesReader;
  vtkNew<vtkUnstructuredGridReader> unstructuredGridReader;
  blockFileSeriesReader->SetReader(unstructuredGridReader.GetPointer());
  std::string dataPath = DATA_TESTING_PATH;
  const int maxTime = 5;
  for (int i = 0; i <= maxTime; ++i)
    {
    char tmp[10];
    sprintf(tmp, "%d", i);
    std::string blockFile = dataPath + "block" + tmp + ".vtu";
    blockFileSeriesReader->AddFileName(blockFile.c_str());
    }
  blockFileSeriesReader->SetOutputTimeRange(0.0, static_cast<double>(maxTime));

  // Read a PolyData containing the vector text 'MSV' with reference point
  // coordinates to be overwritten by msvVTKEmbeddedProbeFilter using the
  // point data fields 'cellId' and 'pcoord' defined on it.

  vtkNew<vtkPolyDataReader> msvReader;
  std::string msvFile = dataPath + "msv.vtp";
  msvReader->SetFileName(msvFile.c_str());

  // Define the msvVTKEmbeddedProbeFilter to update the point coordinates
  // of the MSV PolyData by sampling coordinates of the block in cells
  // given by field 'cellId' at parametric coordinates given by field
  // 'pcoord'. Other fields defined on the block are also sampled, namely
  // field 'testval' which also varies with time.
  // This produces a time-varying, distorting polydata embedded in the host
  // block at the supplied parametric coordinates.

  vtkNew<msvVTKEmbeddedProbeFilter> msvEmbedded;
  msvEmbedded->SetInputConnection(msvReader->GetOutputPort());
  msvEmbedded->SetSourceConnection(blockFileSeriesReader->GetOutputPort());
  msvEmbedded->SetCellIdArrayName("cellId");
  msvEmbedded->SetParametricCoordinateArrayName("pcoord");

  // Set point data field 'testval' to be the scalar attribute.

  vtkNew<vtkFieldDataToAttributeDataFilter> msvEmbeddedScalar;
  msvEmbeddedScalar->SetInputConnection(msvEmbedded->GetOutputPort());
  msvEmbeddedScalar->SetInputFieldToPointDataField();
  msvEmbeddedScalar->SetOutputAttributeDataToPointData();
  msvEmbeddedScalar->SetScalarComponent(0, "testval", 0);

  // Draw the distorted MSV PolyData coloured by the scalar.

  vtkNew<vtkPolyDataMapper> msvEmbeddedScalarMapper;
  msvEmbeddedScalarMapper->SetInputConnection(msvEmbeddedScalar->GetOutputPort());
  msvEmbeddedScalarMapper->ScalarVisibilityOn();
  vtkNew<vtkActor> msvEmbeddedScalarActor;
  msvEmbeddedScalarActor->SetMapper(msvEmbeddedScalarMapper.GetPointer());
  msvEmbeddedScalarActor->VisibilityOn();
  ren1->AddActor(msvEmbeddedScalarActor.GetPointer());

  // Draw the distorting host block with edges and semi-transparent surfaces

  vtkProperty *property;
  vtkNew<vtkExtractEdges> blockEdges;
  blockEdges->SetInputConnection(blockFileSeriesReader->GetOutputPort());
  vtkNew<vtkPolyDataMapper> blockEdgesMapper;
  blockEdgesMapper->SetInputConnection(blockEdges->GetOutputPort());
  vtkNew<vtkActor> blockEdgesActor;
  blockEdgesActor->SetMapper(blockEdgesMapper.GetPointer());
  property = blockEdgesActor->GetProperty();
  property->SetColor(0.0, 0.0, 0.0);
  ren1->AddActor(blockEdgesActor.GetPointer());

  vtkNew<vtkDataSetSurfaceFilter> blockSurfaces;
  blockSurfaces->SetInputConnection(blockFileSeriesReader->GetOutputPort());
  vtkNew<vtkPolyDataMapper> blockSurfacesMapper;
  blockSurfacesMapper->SetInputConnection(blockSurfaces->GetOutputPort());
  vtkNew<vtkActor> blockSurfacesActor;
  blockSurfacesActor->SetMapper(blockSurfacesMapper.GetPointer());
  property = blockSurfacesActor->GetProperty();
  property->SetColor(0.89, 0.89, 0.37);
  property->SetOpacity(0.3);
  ren1->AddActor(blockSurfacesActor.GetPointer());

  // Recenter the camera to show the whole model.

  double extent[6];
  blockSurfacesMapper->GetBounds(extent);
  ren1->ResetCamera(extent);

  // Request update for all mappers at time=4

  double time = 4.0;
  RequestTimeUpdate(msvEmbeddedScalar.GetPointer(), time);
  RequestTimeUpdate(blockEdges.GetPointer(), time);
  RequestTimeUpdate(blockSurfaces.GetPointer(), time);

  iren->Initialize();
  renWin->Render();
  iren->Start();

  return EXIT_SUCCESS;
}
