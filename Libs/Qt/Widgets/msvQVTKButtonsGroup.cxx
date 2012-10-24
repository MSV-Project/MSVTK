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

#include "msvQVTKButtonsGroup.h"
#include "msvQVTKButtons.h"
#include <vtkButtonRepresentation.h>
#include <vtkTexturedButtonRepresentation2D.h>
#include <vtkObject.h>
#include <vtkCommand.h>
#include <vtkButtonWidget.h>
#include <vtkSliderRepresentation2D.h>
#include <vtkSliderWidget.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

#define VTK_CREATE(type, name) vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

// Callback respondign to vtkCommand::StateChangedEvent
class vtkButtonCallback2 : public vtkCommand
{
public:
  static vtkButtonCallback2 *New()
  { 
    return new vtkButtonCallback2; 
  }

  virtual void Execute(vtkObject *caller, unsigned long, void*)
  {
    Q_UNUSED(caller);
    state = !state;
    // Show / Hide slider
    toolButton->showSlider(state);
    toolButton->setCameraPoistionOnPath(0);
  }

  vtkButtonCallback2() : toolButton(NULL) , state (false) {}
  msvQVTKButtonsGroup *toolButton;
  bool state;
};

// Callback respondign to vtkCommand::HighlightEvent
class MSV_QT_WIDGETS_EXPORT vtkButtonHighLightCallback2 : public vtkCommand
{
public:
  static vtkButtonHighLightCallback2 *New()
  { 
    return new vtkButtonHighLightCallback2; 
  }

  virtual void Execute(vtkObject *caller, unsigned long, void*)
  {
    vtkTexturedButtonRepresentation2D *rep = reinterpret_cast<vtkTexturedButtonRepresentation2D*>(caller);
    int highlightState = rep->GetHighlightState();

    if ( highlightState == vtkButtonRepresentation::HighlightHovering && previousHighlightState == vtkButtonRepresentation::HighlightNormal )
    {
      //show tooltip (not if previous state was selecting
      toolButton->setShowTooltip(true);        
    } 
    else if ( highlightState == vtkButtonRepresentation::HighlightNormal)
    {
      //hide tooltip
      toolButton->setShowTooltip(false);
    } 
    previousHighlightState = highlightState;
  }

  vtkButtonHighLightCallback2():toolButton(NULL), previousHighlightState(0) {}
  msvQVTKButtonsGroup *toolButton;
  int previousHighlightState;

};

class vtkSliderCallback : public vtkCommand
{
public:
  static vtkSliderCallback *New() 
  {
    return new vtkSliderCallback;
  }
  virtual void Execute(vtkObject *caller, unsigned long, void*)
  {
    vtkSliderWidget *sliderWidget = reinterpret_cast<vtkSliderWidget*>(caller);
    double ratio = reinterpret_cast<vtkSliderRepresentation*>(sliderWidget->GetRepresentation())->GetValue();
    toolButton->setCameraPoistionOnPath(ratio);
  }

  vtkSliderCallback(): renderer(0) {}
  vtkRenderer *renderer;
  msvQVTKButtonsGroup *toolButton;
};

msvQVTKButtonsGroup::msvQVTKButtonsGroup(QObject *parent) : msvQVTKButtonsInterface(), m_Slider(NULL)
{
  m_ButtonCallback = vtkButtonCallback2::New();
  reinterpret_cast<vtkButtonCallback2*>(m_ButtonCallback)->toolButton = this;
  m_HighlightCallback = vtkButtonHighLightCallback2::New();
  reinterpret_cast<vtkButtonHighLightCallback2*>(m_HighlightCallback)->toolButton = this;

  m_SliderCallback = vtkSliderCallback::New();
  reinterpret_cast<vtkSliderCallback*>(m_SliderCallback)->toolButton = this;

  button()->AddObserver(vtkCommand::StateChangedEvent,m_ButtonCallback);
  button()->GetRepresentation()->AddObserver(vtkCommand::HighlightEvent,m_HighlightCallback);
}

void msvQVTKButtonsGroup::setElementProperty(QString name, QVariant value) {
  for(QVector<msvQVTKButtonsInterface*>::iterator buttonsIt = m_Elements.begin(); buttonsIt != m_Elements.end(); buttonsIt++)
  {
    (*buttonsIt)->setProperty(name.toStdString().c_str(),value);
  }
}

void msvQVTKButtonsGroup::setCurrentRenderer(vtkRenderer *renderer) {
  msvQVTKButtonsInterface::setCurrentRenderer(renderer);
  if(renderer)
  {
    slider()->SetInteractor(renderer->GetRenderWindow()->GetInteractor());
    slider()->SetCurrentRenderer(renderer); //to check
    slider()->GetRepresentation()->SetRenderer(renderer);
    reinterpret_cast<vtkSliderCallback*>(m_SliderCallback)->renderer = renderer;
    slider()->EnabledOn();
  }
  else
  {
    slider()->SetInteractor(NULL);
    slider()->SetCurrentRenderer(NULL);
    slider()->GetRepresentation()->SetRenderer(NULL);
    reinterpret_cast<vtkSliderCallback*>(m_ButtonCallback)->renderer = NULL;
    slider()->EnabledOff();
  }

  for(QVector<msvQVTKButtonsInterface*>::iterator buttonsIt = m_Elements.begin(); buttonsIt != m_Elements.end(); buttonsIt++)
  {
      (*buttonsIt)->setCurrentRenderer(renderer);
  }
}

void msvQVTKButtonsGroup::addElement(msvQVTKButtonsInterface* buttons)
{
  int i = 0;
  double b[6];
  buttons->getBounds(b);
  double dimension = (b[1]-b[0])*(b[3]-b[2])*(b[5]-b[4]);
  for(QVector<msvQVTKButtonsInterface*>::iterator buttonsIt = m_Elements.begin(); buttonsIt != m_Elements.end(); buttonsIt++)
  {
    if(*buttonsIt == buttons)
    {
      return;
    }
    (*buttonsIt)->getBounds(b);
    double cur_dimension = (b[1]-b[0])*(b[3]-b[2])*(b[5]-b[4]);
    if(dimension > cur_dimension)
    {
      m_Elements.insert(i,buttons);
      return;
    }
    i++;
  }
  connect(buttons, SIGNAL(show(bool)), this, SLOT(show(bool)));
  m_Elements.push_back(buttons);
  updateBounds();
}

void msvQVTKButtonsGroup::removeElement(msvQVTKButtonsInterface* buttons)
{
  int index = 0;
  for(QVector<msvQVTKButtonsInterface*>::iterator buttonsIt = m_Elements.begin(); buttonsIt != m_Elements.end(); buttonsIt++)
  {
    if(*buttonsIt == buttons)
    {
      m_Elements.remove(index);
      return;
    }
    index++;
  }
  updateBounds();
}

msvQVTKButtonsGroup::~msvQVTKButtonsGroup()
{

}

msvQVTKButtonsInterface* msvQVTKButtonsGroup::getElement(int index)
{
  if(index > m_Elements.size() - 1)
  {
    return NULL;
  }
  return m_Elements.at(index);
}

msvQVTKButtonsGroup* msvQVTKButtonsGroup::createGroup(
{
  msvQVTKButtonsGroup* element = new msvQVTKButtonsGroup();
  addElement(element);
  return element;
}

msvQVTKButtons* msvQVTKButtonsGroup::createButtons()
{
  msvQVTKButtons* element = new msvQVTKButtons();
  addElement(element);
  return element;
}

void msvQVTKButtonsGroup::update()
{
  calculatePosition();
  msvQVTKButtonsInterface::update();
}

void msvQVTKButtonsGroup::calculatePosition()
{
  updateBounds();

  //modify position of the vtkButton 
  double bds[6];

  bds[0] = 0;
  bds[1] = 16;
  bds[2] = 0;
  bds[3] = 16;
  bds[4] = 0;
  bds[5] = 2;

  vtkTexturedButtonRepresentation2D *rep = static_cast<vtkTexturedButtonRepresentation2D *>(button()->GetRepresentation());
  rep->PlaceWidget(bds);
  rep->Modified();
  button()->SetRepresentation(rep);
}


void msvQVTKButtonsGroup::updateBounds()
{
  /// always invalid bounds?
}

vtkSliderWidget* msvQVTKButtonsGroup::slider()
{
  if(!m_Slider)
  {
    vtkSliderRepresentation2D* sliderRep = vtkSliderRepresentation2D::New();
    sliderRep->SetMinimumValue(0.0);
    sliderRep->SetMaximumValue(100.0);
    sliderRep->SetLabelFormat("%0.f");
    sliderRep->SetValue(0.0);
    sliderRep->SetTitleText("");
    sliderRep->GetPoint1Coordinate()->SetCoordinateSystemToDisplay();
    sliderRep->GetPoint1Coordinate()->SetValue(24,24);
    sliderRep->GetPoint2Coordinate()->SetCoordinateSystemToDisplay();
    sliderRep->GetPoint2Coordinate()->SetValue(24,96);
    sliderRep->SetSliderLength(0.015);
    sliderRep->SetSliderWidth(0.04);
    sliderRep->SetEndCapLength(0.015);
    sliderRep->SetEndCapWidth(0.04);
    sliderRep->SetTubeWidth(0.005);
    sliderRep->GetSliderProperty()->SetColor(.7,.7,.7);
    sliderRep->GetSliderProperty()->SetOpacity(.8);
    sliderRep->GetCapProperty()->SetOpacity(.8);
    sliderRep->GetSelectedProperty()->SetColor(.9,.9,.9);
    sliderRep->GetSelectedProperty()->SetOpacity(.8);
    sliderRep->GetTubeProperty()->SetOpacity(.8);
    sliderRep->SetVisibility(false);
    m_Slider = vtkSliderWidget::New();
    m_Slider->SetRepresentation(sliderRep);
    m_Slider->SetAnimationModeToAnimate();
    m_Slider->AddObserver(vtkCommand::InteractionEvent,m_SliderCallback);
    
  }
  return m_Slider;
}

void msvQVTKButtonsGroup::showSlider(bool show)
{
  if(slider())
  {
    slider()->GetRepresentation()->SetVisibility(show);
    slider()->Render();
  }
}

void msvQVTKButtonsGroup::show(bool val) {
  if(val)
  {
    m_ShowButton = true;
    this->update();
  }
}

void msvQVTKButtonsGroup::setCameraPoistionOnPath(double ratio) {
  // get the number of buttons
  int numOfButtons = m_Elements.size();
  if(numOfButtons < 1)
  {
    return;
  }
  double ratioPerButton = 100. / double(numOfButtons-1);
  int targetButton = int(ratio / ratioPerButton);

  double subPathRatio = (ratio - (double(targetButton) * ratioPerButton)) / ratioPerButton; // (0-1 value)

  // calculate intermediate bounds
  double resetBounds[6];
  double b1[6];
  double b2[6];

  m_Elements.at(targetButton)->getBounds(b1);
  if(ratio == 0 || targetButton == numOfButtons-1)
  {
    resetBounds[0] = b1[0];
    resetBounds[1] = b1[1];
    resetBounds[2] = b1[2];
    resetBounds[3] = b1[3];
    resetBounds[4] = b1[4];
    resetBounds[5] = b1[5];
  }
  else
  {
    
    m_Elements.at(targetButton+1)->getBounds(b2);
    resetBounds[0] = b1[0] * (1 - subPathRatio) + b2[0] * subPathRatio;
    resetBounds[1] = b1[1] * (1 - subPathRatio) + b2[1] * subPathRatio;
    resetBounds[2] = b1[2] * (1 - subPathRatio) + b2[2] * subPathRatio;
    resetBounds[3] = b1[3] * (1 - subPathRatio) + b2[3] * subPathRatio;
    resetBounds[4] = b1[4] * (1 - subPathRatio) + b2[4] * subPathRatio;
    resetBounds[5] = b1[5] * (1 - subPathRatio) + b2[5] * subPathRatio;
  }

  slider()->GetCurrentRenderer()->ResetCamera(resetBounds);
}