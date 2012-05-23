/*==============================================================================

  Program: MSVTK

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

#include "msvVTKLODWidget.h"
#include "msvVTKProp3DButtonRepresentation.h"
#include "vtkCallbackCommand.h"
#include "vtkCompositeDataSet.h"
#include "vtkDataObject.h"
#include "vtkHardwareSelector.h"
#include "vtkInformation.h"
#include "vtkMapper.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkWidgetEventTranslator.h"
#include "vtkWidgetCallbackMapper.h"
#include "vtkWidgetRepresentation.h"
#include "vtkEvent.h"
#include "vtkWidgetEvent.h"

vtkStandardNewMacro(msvVTKLODWidget);

//-------------------------------------------------------------------------
msvVTKLODWidget::msvVTKLODWidget()
{
  this->SelectX = 0;
  this->SelectY = 0;
  this->WidgetState = msvVTKLODWidget::Start;

  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkWidgetEvent::Select,
                                          this, msvVTKLODWidget::SelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                          vtkWidgetEvent::EndSelect,
                                          this, msvVTKLODWidget::EndSelectAction);
  //this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonPressEvent,
  //                                        vtkWidgetEvent::Select,
  //                                        this, msvVTKLODWidget::SelectAction);
  //this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonReleaseEvent,
  //                                        vtkWidgetEvent::EndSelect,
  //                                        this, msvVTKLODWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MouseMoveEvent,
                                          vtkWidgetEvent::Move,
                                          this, msvVTKLODWidget::MouseHoverAction);
}

//-------------------------------------------------------------------------
msvVTKLODWidget::~msvVTKLODWidget()
{
}

//-------------------------------------------------------------------------
void msvVTKLODWidget::SetCursor(int cState)
{
/*
  if(!this->Resizable && cState != vtkBorderRepresentation::Inside)
    {
    this->RequestCursorShape(VTK_CURSOR_DEFAULT);
    return;
    }

  switch (cState)
    {
    case vtkBorderRepresentation::AdjustingP0:
      this->RequestCursorShape(VTK_CURSOR_SIZESW);
      break;
    case vtkBorderRepresentation::AdjustingP1:
      this->RequestCursorShape(VTK_CURSOR_SIZESE);
      break;
    case vtkBorderRepresentation::AdjustingP2:
      this->RequestCursorShape(VTK_CURSOR_SIZENE);
      break;
    case vtkBorderRepresentation::AdjustingP3:
      this->RequestCursorShape(VTK_CURSOR_SIZENW);
      break;
    case vtkBorderRepresentation::AdjustingE0:
    case vtkBorderRepresentation::AdjustingE2:
      this->RequestCursorShape(VTK_CURSOR_SIZENS);
      break;
    case vtkBorderRepresentation::AdjustingE1:
    case vtkBorderRepresentation::AdjustingE3:
      this->RequestCursorShape(VTK_CURSOR_SIZEWE);
      break;
    case vtkBorderRepresentation::Inside:
      if ( reinterpret_cast<vtkBorderRepresentation*>(this->WidgetRep)->GetMoving() )
        {
        this->RequestCursorShape(VTK_CURSOR_SIZEALL);
        }
      else
        {
        this->RequestCursorShape(VTK_CURSOR_HAND);
        }
      break;
    default:
      this->RequestCursorShape(VTK_CURSOR_DEFAULT);
    }
*/
}

//-------------------------------------------------------------------------
void msvVTKLODWidget::SelectAction(vtkAbstractWidget *w)
{
  msvVTKLODWidget *self = reinterpret_cast<msvVTKLODWidget*>(w);

  if ( self->SubclassSelectAction() ||
       self->WidgetState != msvVTKLODWidget::Selectable)
    {
    return;
    }

  // We are definitely selected
  self->GrabFocus(self->EventCallbackCommand);
  self->WidgetState = msvVTKLODWidget::Selected;

  // Picked something inside the widget
  self->SelectX = self->Interactor->GetEventPosition()[0];
  self->SelectY = self->Interactor->GetEventPosition()[1];

  // This is redundant but necessary on some systems (windows) because the
  // cursor is switched during OS event processing and reverts to the default
  // cursor (i.e., the MoveAction may have set the cursor previously, but this
  // method is necessary to maintain the proper cursor shape)..
  self->SetCursor(self->WidgetRep->GetInteractionState());

  //self->SelectRegion(eventPos);

  self->EventCallbackCommand->SetAbortFlag(1);
  //self->StartInteraction();
  //self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
}

//-------------------------------------------------------------------------
void msvVTKLODWidget::EndSelectAction(vtkAbstractWidget *w)
{
  msvVTKLODWidget *self = reinterpret_cast<msvVTKLODWidget*>(w);

  if ( self->SubclassEndSelectAction() ||
       self->WidgetState != msvVTKLODWidget::Selected)
    {
    return;
    }

  // Return state to not selected
  self->ReleaseFocus();
  // stop adjusting
  self->EventCallbackCommand->SetAbortFlag(1);
  //self->EndInteraction();
  //self->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);

  // compute some info we need for all cases
  int endSelectX = self->Interactor->GetEventPosition()[0];
  int endSelectY = self->Interactor->GetEventPosition()[1];
  vtkAssemblyPath* pickedPath =
    self->GetRepresentation()->GetRenderer()->PickProp(endSelectX, endSelectY);
  bool cursorOverLODObject = (pickedPath != 0);
  self->WidgetState = cursorOverLODObject ? msvVTKLODWidget::Selectable : msvVTKLODWidget::Start;

  vtkNew<vtkHardwareSelector> hardwareSelector;
  hardwareSelector->SetFieldAssociation(vtkDataObject::FIELD_ASSOCIATION_POINTS);
  hardwareSelector->SetRenderer(self->GetRepresentation()->GetRenderer());

  hardwareSelector->SetArea(static_cast<unsigned int>(self->SelectX),
    static_cast<unsigned int>(self->SelectY),
    static_cast<unsigned int>(endSelectX),
    static_cast<unsigned int>(endSelectY));

  vtkSelection *res = hardwareSelector->Select();
  // /todo handle more than 1 selection node
  if (res->GetNumberOfNodes())
    {
    vtkSelectionNode* node = res->GetNode(0);
    vtkObjectBase* object = node->GetProperties()->Get(vtkSelectionNode::PROP());
    vtkActor* actor = vtkActor::SafeDownCast(object);
    vtkCompositeDataSet* composite = vtkCompositeDataSet::SafeDownCast(actor->GetMapper());
    if (composite)
      {
      vtkIdType composite = node->GetProperties()->Get(vtkSelectionNode::COMPOSITE_INDEX());
      std::cout << "Composite index: " << composite << std::endl;
      }
    }
  res->Print(std::cout);
}

//-------------------------------------------------------------------------
void msvVTKLODWidget::MouseHoverAction(vtkAbstractWidget *w)
{
  msvVTKLODWidget *self = reinterpret_cast<msvVTKLODWidget*>(w);

  if ( self->SubclassMouseHoverAction() ||
       self->WidgetState == msvVTKLODWidget::Selected)
    {
    return;
    }

  // compute some info we need for all cases
  int mouseHoverX = self->Interactor->GetEventPosition()[0];
  int mouseHoverY = self->Interactor->GetEventPosition()[1];
  vtkAssemblyPath* pickedPath =
    self->GetRepresentation()->GetRenderer()->PickPropFrom(mouseHoverX, mouseHoverY, 0);
  bool cursorOverLODObject = (pickedPath != 0);
  if (cursorOverLODObject)
    {
    // Return state to not selected
    self->GrabFocus(self->EventCallbackCommand);
    self->EventCallbackCommand->SetAbortFlag(1);
    self->WidgetState = msvVTKLODWidget::Selectable;
    }
  else
    {
    // Return state to not selected
    self->ReleaseFocus();
    self->WidgetState = msvVTKLODWidget::Start;
    }
}

//----------------------------------------------------------------------
void msvVTKLODWidget::CreateDefaultRepresentation()
{
  if ( ! this->WidgetRep )
    {
    this->WidgetRep = msvVTKProp3DButtonRepresentation::New();
    }
}

//-------------------------------------------------------------------------
void msvVTKLODWidget::SelectRegion(double* vtkNotUsed(eventPos[2]))
{
  this->InvokeEvent(vtkCommand::WidgetActivateEvent,NULL);
}

//-------------------------------------------------------------------------
void msvVTKLODWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
