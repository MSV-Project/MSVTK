/*
 *  msvQVTKButtonsInterface.h
 *  
 *
 *  Created by Alberto Losi on 10/08/12.
 *  Copyright 2011 B3C. All rights reserved.
 *
 *  See License at: http://tiny.cc/QXJ4D
 *
 */

#include "msvQVTKButtonsInterface.h"

#include <vtkTexturedButtonRepresentation.h>
#include <vtkTexturedButtonRepresentation2D.h>
#include <vtkButtonWidget.h>
#include <vtkSmartPointer.h>
#include <vtkBalloonRepresentation.h>
#include <vtkCommand.h>
#include <vtkQImageToImageSource.h>
#include <vtkTextProperty.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

#define VTK_CREATE(type, name) vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

msvQVTKButtonsInterface::msvQVTKButtonsInterface(QObject *parent) : QObject(parent), m_ShowButton(false), m_ShowLabel(true), m_ButtonWidget(NULL), m_ButtonCallback(NULL), m_HighlightCallback(NULL) {
    VTK_CREATE(vtkTexturedButtonRepresentation2D, rep);
    rep->SetNumberOfStates(1);
    button()->SetRepresentation(rep);
    if(m_ButtonCallback)
      button()->AddObserver(vtkCommand::StateChangedEvent,m_ButtonCallback);
    if(m_HighlightCallback)
      rep->AddObserver(vtkCommand::HighlightEvent,m_HighlightCallback);
}

msvQVTKButtonsInterface::~msvQVTKButtonsInterface() {
    button()->Delete();
}

vtkButtonWidget *msvQVTKButtonsInterface::button() {
  if(m_ButtonWidget == NULL) {
    m_ButtonWidget = vtkButtonWidget::New();
  }
  return m_ButtonWidget;
}

void msvQVTKButtonsInterface::setIconFileName(QString iconFileName) {
  m_IconFileName = iconFileName;
  QImage image;
  image.load(m_IconFileName);
  VTK_CREATE(vtkQImageToImageSource, imageToVTK);
  imageToVTK->SetQImage(&image);
  imageToVTK->Update();
  vtkTexturedButtonRepresentation2D *rep = static_cast<vtkTexturedButtonRepresentation2D *>(button()->GetRepresentation());
  rep->SetButtonTexture(0, imageToVTK->GetOutput());

  int size[2]; size[0] = 16; size[1] = 16;
  rep->GetBalloon()->SetImageSize(size);

  update();
}

void msvQVTKButtonsInterface::update() {
  vtkTexturedButtonRepresentation2D *rep = reinterpret_cast<vtkTexturedButtonRepresentation2D*>(button()->GetRepresentation());

  if (m_ShowLabel) {
    //Add a label to the button and change its text property
    rep->GetBalloon()->SetBalloonText(m_Label.toAscii());
    vtkTextProperty *textProp = rep->GetBalloon()->GetTextProperty();
    rep->GetBalloon()->SetPadding(2);
    textProp->SetFontSize(13);
    textProp->BoldOff();
    //textProp->SetColor(0.9,0.9,0.9);

    //Set label position
    rep->GetBalloon()->SetBalloonLayoutToImageLeft();

    //This method allows to set the label's background opacity
    rep->GetBalloon()->GetFrameProperty()->SetOpacity(0.65);
  } else {
    rep->GetBalloon()->SetBalloonText("");
  }

  if(m_ShowButton) {
    button()->GetRepresentation()->SetVisibility(true);
    button()->EnabledOn();
  } else {
    button()->GetRepresentation()->SetVisibility(false);
    button()->EnabledOff();
  }
}
void msvQVTKButtonsInterface::setCurrentRenderer(vtkRenderer *renderer) {
  if(renderer) {
    button()->SetInteractor(renderer->GetRenderWindow()->GetInteractor());
    button()->SetCurrentRenderer(renderer); //to check
    button()->EnabledOn();
  } else {
    button()->SetInteractor(NULL);
    button()->SetCurrentRenderer(NULL); //to check
    button()->EnabledOff();
  }
}

void msvQVTKButtonsInterface::getBounds(double b[6]) {
    for(int i = 0; i < 6; i++)
        b[i] = m_Bounds[i];
}

void msvQVTKButtonsInterface::setShowButton(bool visible) {
  m_ShowButton = visible;
  Q_EMIT(show(visible));
}
