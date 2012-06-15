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

// VTK includes
#include <vtkButtonWidget.h>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkCubeSource.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProp3DButtonRepresentation.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkWidgetEvent.h>

// STD includes
#include <map>

// MSVTK includes
#include "msvVTKButtonsManager.h"
#include "msvVTKProp3DButtonRepresentation.h"

//------------------------------------------------------------------------------
vtkStandardNewMacro(msvVTKButtonsManager);

//------------------------------------------------------------------------------
class msvVTKButtonsManager::vtkInternal
{
public:
  vtkInternal(msvVTKButtonsManager* external);
  ~vtkInternal();

  void CreateButtonWidgets(vtkPolyData* poly);
  void ClearButtons();

  struct ButtonProp : vtkObjectBase
    {
    ButtonProp();

    // vtkProp for the first state of our button
    vtkSmartPointer<vtkCubeSource>      Cube;
    vtkSmartPointer<vtkPolyDataMapper>  CubeMapper;
    vtkSmartPointer<vtkActor>           CubeActor;
    };

  struct ButtonHandleReprensentation : vtkObjectBase
    {
    ButtonHandleReprensentation();

    vtkSmartPointer<vtkButtonWidget> ButtonWidget;
    vtkSmartPointer<ButtonProp> PropButton;
    };

  typedef std::map<vtkIdType, vtkSmartPointer<ButtonHandleReprensentation> >
    HandleButtonWidgetsType;
  HandleButtonWidgetsType   HandleButtonWidgets;
  msvVTKButtonsManager*  External;

  // Keep time track of last button which has been in interaction.
  vtkIdType LastSelectedButton;

  const double(* Colors)[3];
  unsigned int  ColorCount;
  int           CurrentColor;
};

//------------------------------------------------------------------------------
// vtkInternal methods

//------------------------------------------------------------------------------
msvVTKButtonsManager::vtkInternal::vtkInternal(msvVTKButtonsManager* ext)
{
  this->External = ext;
  this->LastSelectedButton = -1;
  this->Colors = NULL;
  this->ColorCount = 0;
  this->CurrentColor = 0;
}

//------------------------------------------------------------------------------
msvVTKButtonsManager::vtkInternal::~vtkInternal()
{
  this->ClearButtons();
}

//------------------------------------------------------------------------------
msvVTKButtonsManager::vtkInternal::ButtonProp::ButtonProp()
{
  this->Cube = vtkCubeSource::New();
  this->CubeMapper = vtkPolyDataMapper::New();
  this->CubeMapper->SetInputConnection(Cube->GetOutputPort());
  this->CubeMapper->SetScalarRange(0,19);
  this->CubeActor = vtkActor::New();
  this->CubeActor->SetMapper(CubeMapper);
}

//------------------------------------------------------------------------------
msvVTKButtonsManager::vtkInternal::
ButtonHandleReprensentation::ButtonHandleReprensentation()
{
  this->ButtonWidget = 0;
  this->PropButton = new ButtonProp();
}

//------------------------------------------------------------------------------
void msvVTKButtonsManager::vtkInternal::CreateButtonWidgets(vtkPolyData* poly)
{
  if (this->External->NumberOfButtonWidgets < 1 ||
     !this->External->Renderer || !this->External->Renderer->GetRenderWindow()
     || poly->GetNumberOfPoints() < this->External->NumberOfButtonWidgets)
    {
    return;
    }

  // Associate random point from the polydata
  int numberOfPoints = poly->GetNumberOfPoints();
  int step = numberOfPoints / this->External->NumberOfButtonWidgets;

  // Define the callback
  vtkSmartPointer<vtkCallbackCommand> widgetCallback =
    vtkSmartPointer<vtkCallbackCommand>::New();
  widgetCallback->SetClientData(this->External);
  widgetCallback->SetCallback(msvVTKButtonsManager::ProcessWidgetsEvents);

  // Main loop instantiation
  for (int i=0; i<this->External->NumberOfButtonWidgets; ++i)
    {
    // Instantiate the ButtonHandleRepresentation
    vtkSmartPointer<ButtonHandleReprensentation>
      buttonHandle = new ButtonHandleReprensentation();

    // Instantiate the ButtonRepresentation
    vtkNew<msvVTKProp3DButtonRepresentation> rep;
    rep->SetNumberOfStates(1);
    rep->SetButtonProp(0, buttonHandle->PropButton->CubeActor);
    if (this->ColorCount > 0)
      {
      buttonHandle->PropButton->CubeActor->GetProperty()->SetColor(
        this->Colors[this->CurrentColor][0],
        this->Colors[this->CurrentColor][1],
        this->Colors[this->CurrentColor][2]);
      this->CurrentColor = (this->CurrentColor + 1) % this->ColorCount;
      }
    rep->SetPlaceFactor(1);
    rep->SetDragable(0);
    rep->SetFollowCamera(0);

    // The Manager has to manage the destruction of the widgets
    vtkNew<vtkButtonWidget> buttonWidget;
    buttonWidget->SetInteractor(this->External->
                                Renderer->GetRenderWindow()->GetInteractor());
    buttonWidget->SetRepresentation(rep.GetPointer());
    buttonWidget->SetEnabled(1);

    // Associate the current button to the callBackCommand
    buttonWidget->AddObserver(vtkCommand::StateChangedEvent, widgetCallback);

    // Insert the ButtonWidget with his associated HandleReprensentations in map
    buttonHandle->ButtonWidget = buttonWidget.GetPointer();
    this->HandleButtonWidgets.insert(std::pair<vtkIdType,
      vtkSmartPointer<ButtonHandleReprensentation> >
      (static_cast<vtkIdType>(i*step), buttonHandle.GetPointer()));
    }
}

//------------------------------------------------------------------------------
void msvVTKButtonsManager::vtkInternal::ClearButtons()
{
  this->LastSelectedButton = 0;

  // We Have to first delete the HandleReprensentation of each vtkButtonWidget
  for (HandleButtonWidgetsType::iterator it = this->HandleButtonWidgets.begin();
       it != this->HandleButtonWidgets.end(); ++it )
    {
    (*it->second).Delete();
    }

  this->HandleButtonWidgets.clear();
}

//------------------------------------------------------------------------------
// vtkMRMLSliceModelDisplayableManager methods

//------------------------------------------------------------------------------
msvVTKButtonsManager::msvVTKButtonsManager()
{
  this->Internal = new vtkInternal(this);

  this->NumberOfButtonWidgets = 0;
  this->MaxNumberOfButtonWidgets = 100;
  this->ButtonWidgetSize = 3,
  this->Renderer = 0;
}

//------------------------------------------------------------------------------
msvVTKButtonsManager::~msvVTKButtonsManager()
{
  delete this->Internal;
}

//------------------------------------------------------------------------------
void msvVTKButtonsManager::SetNumberOfButtonWidgets(int number)
{
  this->NumberOfButtonWidgets = std::min(number,this->MaxNumberOfButtonWidgets);
}

//------------------------------------------------------------------------------
void msvVTKButtonsManager::SetColors(const double (*colors)[3], unsigned int colorCount)
{
  this->Internal->Colors = colors;
  this->Internal->ColorCount = colorCount;
}

//------------------------------------------------------------------------------
void msvVTKButtonsManager::Clear()
{
  this->Internal->ClearButtons();
}

//------------------------------------------------------------------------------
void msvVTKButtonsManager::Init(vtkPolyData* polyData)
{
  if (!polyData)
    {
    return;
    }

  this->Internal->ClearButtons();
  this->Internal->CreateButtonWidgets(polyData);
}

//------------------------------------------------------------------------------
void msvVTKButtonsManager::UpdateButtonWidgets(vtkPolyData* polyData)
{
  if (!polyData ||
      this->Internal->HandleButtonWidgets.empty() ||
      polyData->GetNumberOfPoints() <
      this->Internal->HandleButtonWidgets.rbegin()->first
      )
    {
    return;
    }

  double center[3];
  double size = this->ButtonWidgetSize;
  msvVTKButtonsManager::vtkInternal::HandleButtonWidgetsType::iterator it;
  for (it = this->Internal->HandleButtonWidgets.begin();
       it != this->Internal->HandleButtonWidgets.end(); ++it)
    {
    it->second->ButtonWidget->GetRepresentation()->VisibilityOn();

    polyData->GetPoint(it->first,center);
    double bounds[6] = {bounds[0] = center[0] - size / 2,
                        bounds[1] = center[0] + size / 2,
                        bounds[2] = center[1] - size / 2,
                        bounds[3] = center[1] + size / 2,
                        bounds[4] = center[2] - size / 2,
                        bounds[5] = center[2] + size / 2};

    it->second->ButtonWidget->GetRepresentation()->PlaceWidget(bounds);
    }
}

//------------------------------------------------------------------------------
void msvVTKButtonsManager::ProcessWidgetsEvents(vtkObject *caller,
                                                  unsigned long vtkNotUsed(event),
                                                  void *clientData,
                                                  void *vtkNotUsed(calldata))
{
  vtkButtonWidget* widget = vtkButtonWidget::SafeDownCast(caller);
  msvVTKButtonsManager* self =
    reinterpret_cast<msvVTKButtonsManager *>(clientData);
  if (!widget || !clientData)
    {
    return;
    }

  msvVTKButtonsManager::vtkInternal::HandleButtonWidgetsType::iterator it;
  for (it = self->Internal->HandleButtonWidgets.begin();
       it != self->Internal->HandleButtonWidgets.end(); ++it)
    {
    if (it->second->ButtonWidget.GetPointer() == widget)
      {
      vtkIdType id = it->first;
      self->Internal->LastSelectedButton = id;
      self->InvokeEvent(vtkCommand::InteractionEvent, &id);
      return;
      }
    }

  self->InvokeEvent(vtkCommand::InteractionEvent, NULL);
}

//------------------------------------------------------------------------------
void msvVTKButtonsManager::SetLastSelectedButton(vtkIdType id)
{
  this->Internal->LastSelectedButton = id;
}

//------------------------------------------------------------------------------
vtkIdType msvVTKButtonsManager::GetLastSelectedButton() const
{
  return this->Internal->LastSelectedButton;
}

int msvVTKButtonsManager::GetIndexFromButtonId(vtkIdType id) const
{
  msvVTKButtonsManager::vtkInternal::HandleButtonWidgetsType::iterator it;
  it = this->Internal->HandleButtonWidgets.find(id);
  if (it == this->Internal->HandleButtonWidgets.end())
    {
    return -1;
    }

  return std::distance(this->Internal->HandleButtonWidgets.begin(), it);
}

//------------------------------------------------------------------------------
void msvVTKButtonsManager::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << this->Renderer;
  os << indent << "Number Max of ButtonWidgets: "
     << this->MaxNumberOfButtonWidgets;
  os << indent << "Number of ButtonWidgets: " << this->NumberOfButtonWidgets;
  os << indent << "ButtonWidgetSize: " << this->ButtonWidgetSize;

  os << indent << "ButtonWidgets: \n";
  int i=0;
  msvVTKButtonsManager::vtkInternal::HandleButtonWidgetsType::iterator it;
  for (it = this->Internal->HandleButtonWidgets.begin();
       it != this->Internal->HandleButtonWidgets.end(); ++it, ++i)
    {
    os << indent << "  (" << i << "): " << (*it).first << "\n";
    }
}
