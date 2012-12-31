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
/*=========================================================================

  Program:   Visualization Toolkit
  Module:    msvVTKButtonsInterface.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkBalloonRepresentation.h"
#include "vtkButtonWidget.h"
#include "vtkCommand.h"
#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkProperty2D.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkTextProperty.h"
#include "vtkTexturedButtonRepresentation2D.h"

// MSV includes
#include "msvVTKButtonsInterface.h"

vtkStandardNewMacro(msvVTKButtonsInterface);

//----------------------------------------------------------------------
msvVTKButtonsInterface::msvVTKButtonsInterface()
{
//  this->ButtonCallback=NULL;
//  this->HighlightCallback=NULL;
  this->LabelText=NULL;
  this->Tooltip="";
  this->ShowButton=true;
  this->ShowLabel=true;
  this->ButtonWidget=NULL;
  this->Image=NULL;

  // Bounds of the data related to the buttonWin
  double Bounds[6];
  vtkTexturedButtonRepresentation2D* rep = vtkTexturedButtonRepresentation2D::New();
  rep->SetNumberOfStates(1);
  GetButton()->SetRepresentation(rep);
  rep->Delete();
//  if(ButtonCallback)
//    this->GetButton()->AddObserver(vtkCommand::StateChangedEvent,ButtonCallback);
//  if(HighlightCallback)
//  {
//    this->GetButton()->GetRepresentation()->AddObserver(
//      vtkCommand::HighlightEvent,HighlightCallback);
//  }
}

//----------------------------------------------------------------------
msvVTKButtonsInterface::~msvVTKButtonsInterface()
{
  if(NULL!=this->LabelText)
  {
    delete[] this->LabelText;
    this->LabelText = NULL;
  }
}

//----------------------------------------------------------------------
vtkButtonWidget *msvVTKButtonsInterface::GetButton()
{
  if(this->ButtonWidget == NULL)
  {
    this->ButtonWidget = vtkButtonWidget::New();
  }
  return this->ButtonWidget;
}

//----------------------------------------------------------------------
void msvVTKButtonsInterface::SetCurrentRenderer(vtkRenderer* renderer)
{
  if(renderer)
  {
    this->GetButton()->SetInteractor(
      renderer->GetRenderWindow()->GetInteractor());
    this->GetButton()->SetCurrentRenderer(renderer); //to check
    this->GetButton()->EnabledOn();
  }
  else
  {
    this->GetButton()->SetInteractor(NULL);
    this->GetButton()->SetCurrentRenderer(NULL); //to check
    this->GetButton()->EnabledOff();
  }
}

void msvVTKButtonsInterface::SetLabel(const char* label)
{
  if(NULL!=this->LabelText)
  {
    delete[] this->LabelText;
    this->LabelText = NULL;
  }
  this->LabelText = new char[strlen(label)+1];
  strcpy(this->LabelText,label);
}

//----------------------------------------------------------------------
void msvVTKButtonsInterface::Update()
{
  vtkTexturedButtonRepresentation2D *rep =
    reinterpret_cast<vtkTexturedButtonRepresentation2D*>(
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

  if(this->GetShowButton())
  {
    this->GetButton()->GetRepresentation()->SetVisibility(true);
    this->GetButton()->EnabledOn();
  }
  else
  {
    this->GetButton()->GetRepresentation()->SetVisibility(false);
    this->GetButton()->EnabledOff();
  }
}

void msvVTKButtonsInterface::SetImage(vtkImageData* image)
{
  Image = image;
  vtkTexturedButtonRepresentation2D *rep =
    static_cast<vtkTexturedButtonRepresentation2D *>(
    this->GetButton()->GetRepresentation());
  rep->SetButtonTexture(0,Image);
  int size[2]; size[0] = 16; size[1] = 16;
  rep->GetBalloon()->SetImageSize(size);
  this->Update();
}
