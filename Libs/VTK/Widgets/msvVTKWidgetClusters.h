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
// .NAME msvVTKWidgetClusters - button de-clutering manager
// .SECTION Description
// This class implements a button clustering strategy to decluter buttons from
// screen.

// .SECTION See Also
// vtkButtonWidget msvVTKProp3DButtonRepresentation

#ifndef __msvVTKWidgetClusters_h
#define __msvVTKWidgetClusters_h

// VTK includes
#include "vtkObject.h"

class vtkButtonWidget;
class vtkInformationDoubleVectorKey;
class vtkInformationIdTypeKey;
class vtkInformationIntegerVectorKey;
class vtkLookupTable;
class vtkPoints;
class vtkRenderer;

class msvVTKWidgetClusters : public vtkObject
{
public:
  static msvVTKWidgetClusters* New();
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set / Get the Renderer to process widget
  virtual void SetRenderer(vtkRenderer *renderer);
  vtkGetMacro(Renderer,vtkRenderer*);

  // Description:
  // Set / Get the clusters radius parameter
  vtkSetMacro(PixelRadius,double);
  vtkGetMacro(PixelRadius,double);

  // Description:
  // Set / Get the improved clustering parameter
  vtkSetMacro(UseImprovedClustering,bool);
  vtkGetMacro(UseImprovedClustering,bool);
  vtkBooleanMacro(UseImprovedClustering,bool);

  // Description:
  // Set / Get the clustering status
  vtkSetMacro(ClusteringEnabled,bool);
  vtkGetMacro(ClusteringEnabled,bool);
  vtkBooleanMacro(ClusteringEnabled,bool);

  // Description:
  // Set / Get the size of the button reprensentation
  vtkSetMacro(ButtonWidgetSize,double);
  vtkGetMacro(ButtonWidgetSize,double);

  // Description:
  // Set / Get the size of the button reprensentation
  virtual void SetColorLookUpTable(vtkLookupTable *arg);
  vtkGetMacro(ColorLookUpTable,vtkLookupTable*);
  
  // Description:
  // Add widget positions
  virtual void SetDataSet(size_t level, size_t idx,
                          vtkPoints* dataSet);

  // Description:
  // Set the number of data set at a given level.
  void SetNumberOfDataSets(size_t level, size_t numdatasets);

  // Description:
  // Returns the number of data sets available at any level.
  size_t GetNumberOfDataSets(size_t level);

  // Description:
  // Set the number of refinement levels. This call might cause
  // allocation if the new number of levels is larger than the
  // current one.
  void SetNumberOfLevels(size_t numLevels);

  // Description:
  // Returns the number of levels.
  size_t GetNumberOfLevels();
  
  static vtkInformationIdTypeKey* CLUSTER_IDX();
  static vtkInformationIdTypeKey* DATASET_BUTTONS_OFFSET();
  static vtkInformationIntegerVectorKey* CLUSTER_BUTTONS_OFFSET();

  virtual void UpdateWidgets();
  virtual void Clear();
  virtual void SetCustersButtonsVisibility(bool show);
  virtual void ShowButtons();
  virtual void HideButtons();
  virtual void ShowClusterButtons();
  virtual void HideClusterButtons();
  virtual void ShowButtons(size_t level);
  virtual void HideButtons(size_t level);
  virtual void ShowClusterButtons(size_t level);
  virtual void HideClusterButtons(size_t level);

protected:
  msvVTKWidgetClusters();
  virtual ~msvVTKWidgetClusters();

  // Description:
  // Callback used to process the update
  static void UpdateWidgets(vtkObject *   caller,
                            unsigned long event,
                            void *        clientData,
                            void *        callData);

  double       ButtonWidgetSize;
  double       PixelRadius;
  bool         UseImprovedClustering;
  bool         ClusteringEnabled;
  vtkRenderer* Renderer;

private:
  msvVTKWidgetClusters(const msvVTKWidgetClusters&);  // Not implemented.
  void operator=(const msvVTKWidgetClusters&);           // Not implemented.

  class vtkInternal;
  vtkInternal* Internal;

  vtkLookupTable *ColorLookUpTable;
  
};

#endif // __msvVTKWidgetClusters_h
