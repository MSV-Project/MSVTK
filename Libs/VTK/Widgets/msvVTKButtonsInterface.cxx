/*==============================================================================

  Library: MSVTK

  Copyright (c) SCS s.r.l. (B3C)

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
#include "vtkBalloonRepresentation.h"
#include "vtkButtonWidget.h"
#include "vtkCommand.h"
#include "vtkImageData.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkProperty2D.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkTextProperty.h"
#include "vtkTexturedButtonRepresentation2D.h"

// MSVTK includes
#include "msvVTKButtonsInterface.h"

vtkStandardNewMacro(msvVTKButtonsInterface);

//-----------------------------------------------------------------------------
msvVTKButtonsInterface::msvVTKButtonsInterface()
{
  this->ButtonCallback = NULL;
  this->HighlightCallback = NULL;
  this->LabelText = NULL;
  this->Tooltip = "";
  this->ShowButton = true;
  this->ShowLabel = true;
  this->ButtonWidget = NULL;
  this->Image = NULL;

  // Bounds of the data related to the buttonWin
  vtkMath::UninitializeBounds(this->Bounds);

  vtkTexturedButtonRepresentation2D* rep = vtkTexturedButtonRepresentation2D::New();
  rep->SetNumberOfStates(1);
  this->GetButton()->SetRepresentation(rep);
  rep->Delete();
//  if(ButtonCallback)
//    this->GetButton()->AddObserver(vtkCommand::StateChangedEvent,ButtonCallback);
//  if(HighlightCallback)
//  {
//    this->GetButton()->GetRepresentation()->AddObserver(
//      vtkCommand::HighlightEvent,HighlightCallback);
//  }
}

//-----------------------------------------------------------------------------
msvVTKButtonsInterface::~msvVTKButtonsInterface()
{
  if (this->LabelText != NULL)
    {
    delete [] this->LabelText;
    this->LabelText = NULL;
    }
}

//-----------------------------------------------------------------------------
vtkButtonWidget *msvVTKButtonsInterface::GetButton()
{
  if (this->ButtonWidget == NULL)
    {
    this->ButtonWidget = vtkButtonWidget::New();
    }
  return this->ButtonWidget;
}

//-----------------------------------------------------------------------------
void msvVTKButtonsInterface::SetCurrentRenderer(vtkRenderer* renderer)
{
  this->GetButton()->SetInteractor(
    renderer ? renderer->GetRenderWindow()->GetInteractor() : NULL);
  this->GetButton()->SetCurrentRenderer(renderer); //to check
  this->GetButton()->SetEnabled(renderer ? true : false);
}

//-----------------------------------------------------------------------------
void msvVTKButtonsInterface::SetLabel(const char* label)
{
  if(this->LabelText != NULL)
    {
    delete [] this->LabelText;
    this->LabelText = NULL;
    }
  this->LabelText = new char[strlen(label)+1];
  strcpy(this->LabelText,label);
}

//-----------------------------------------------------------------------------
void msvVTKButtonsInterface::Update()
{
  vtkTexturedButtonRepresentation2D *rep =
    vtkTexturedButtonRepresentation2D::SafeDownCast(
      this->GetButton()->GetRepresentation());

  if (this->GetShowLabel())
    {
    //Add a label to the button and change its text property
    rep->GetBalloon()->SetBalloonText(this->GetLabel());
    vtkTextProperty *textProp = rep->GetBalloon()->GetTextProperty();
    rep->GetBalloon()->SetPadding(2);
    textProp->SetFontSize(13);
    textProp->BoldOff();
    //textProp->SetColor(0.9,0.9,0.9);

    //Set label position
    rep->GetBalloon()->SetBalloonLayoutToImageLeft();

    //This method allows to set the label's background opacity
    rep->GetBalloon()->GetFrameProperty()->SetOpacity(0.65);
    }
  else
    {
    rep->GetBalloon()->SetBalloonText("");
    }

  this->GetButton()->GetRepresentation()->SetVisibility(this->GetShowButton());
  this->GetButton()->SetEnabled(this->GetShowButton() ? true : false);
}

//-----------------------------------------------------------------------------
void msvVTKButtonsInterface::SetImage(vtkImageData* image)
{
  this->Image = image;
  vtkTexturedButtonRepresentation2D *rep =
    static_cast<vtkTexturedButtonRepresentation2D *>(
    this->GetButton()->GetRepresentation());
  rep->SetButtonTexture(0,Image);
  int size[2]; size[0] = 16; size[1] = 16;
  rep->GetBalloon()->SetImageSize(size);
  this->Update();
}
