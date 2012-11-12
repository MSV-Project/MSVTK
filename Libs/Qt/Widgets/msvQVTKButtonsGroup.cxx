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
#include <msvVTKSliderFixedRepresentation2D.h>
#include <vtkSliderWidget.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

#define VTK_CREATE(type, name) vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//------------------------------------------------------------------------------
// Callback respondign to vtkCommand::StateChangedEvent
class vtkButtonCallbackGroup : public vtkCommand
{
public:
  static vtkButtonCallbackGroup *New()
  {
    return new vtkButtonCallbackGroup;
  }

  virtual void Execute(vtkObject *caller, unsigned long, void*)
  {
    Q_UNUSED(caller);
    state = !state;
    // Show / Hide slider
    toolButton->showSlider(state);
    toolButton->setCameraPoistionOnPath(0);
  }

  vtkButtonCallbackGroup() : toolButton(NULL) , state (false) {}
  msvQVTKButtonsGroup *toolButton;
  bool state;
};

//------------------------------------------------------------------------------
// Callback respondign to vtkCommand::HighlightEvent
class MSV_QT_WIDGETS_EXPORT vtkButtonHighLightCallbackGroup : public vtkCommand
{
public:
  static vtkButtonHighLightCallbackGroup *New()
  {
    return new vtkButtonHighLightCallbackGroup;
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

  vtkButtonHighLightCallbackGroup():toolButton(NULL), previousHighlightState(0) {}
  msvQVTKButtonsGroup *toolButton;
  int previousHighlightState;

};

//------------------------------------------------------------------------------
// Callback respondign to vtkCommand::StateChangedEvent
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

//------------------------------------------------------------------------------
class msvQVTKButtonsGroupPrivate
{
  Q_DECLARE_PUBLIC(msvQVTKButtonsGroup);

protected:

  msvQVTKButtonsGroup* const q_ptr;

  QVector<msvQVTKButtonsInterface*> m_Elements; //< Vector of buttons
  vtkSliderWidget* m_Slider; //< Slider widget

public:

  msvQVTKButtonsGroupPrivate(msvQVTKButtonsGroup& object);
  virtual ~msvQVTKButtonsGroupPrivate();

  vtkSliderWidget* slider();
  void addElement(msvQVTKButtonsInterface* buttons);
  void removeElement(msvQVTKButtonsInterface* buttons);
  msvQVTKButtonsInterface* getElement(int index);
  int numberOfElements(){return m_Elements.size();};
  void setElementProperty(QString name, QVariant value);
  void setCurrentRenderer(vtkRenderer* renderer);
};

msvQVTKButtonsGroupPrivate::msvQVTKButtonsGroupPrivate(msvQVTKButtonsGroup& object) : m_Slider(NULL), q_ptr(&object)
{

}

msvQVTKButtonsGroupPrivate::~msvQVTKButtonsGroupPrivate()
{

}

void msvQVTKButtonsGroupPrivate::addElement(msvQVTKButtonsInterface* buttons)
{
  Q_Q(msvQVTKButtonsGroup);
  int i = 0;
  double b[6];
  buttons->bounds(b);
  double dimension = (b[1]-b[0])*(b[3]-b[2])*(b[5]-b[4]);
  q->connect(buttons, SIGNAL(show(bool)), q, SLOT(show(bool)));
  for(QVector<msvQVTKButtonsInterface*>::iterator buttonsIt = m_Elements.begin(); buttonsIt != m_Elements.end(); buttonsIt++)
  {
    if(*buttonsIt == buttons)
    {
      return;
    }
    (*buttonsIt)->bounds(b);
    double cur_dimension = (b[1]-b[0])*(b[3]-b[2])*(b[5]-b[4]);
    if(dimension > cur_dimension)
    {
      m_Elements.insert(i,buttons);
      return;
    }
    i++;
  }
  m_Elements.push_back(buttons);
}

void msvQVTKButtonsGroupPrivate::removeElement(msvQVTKButtonsInterface* buttons)
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
}

msvQVTKButtonsInterface* msvQVTKButtonsGroupPrivate::getElement(int index)
{
  if(index > m_Elements.size() - 1)
  {
    return NULL;
  }
  return m_Elements.at(index);
}

vtkSliderWidget* msvQVTKButtonsGroupPrivate::slider()
{
  Q_Q(msvQVTKButtonsGroup);
  if(!m_Slider)
  {
    msvVTKSliderFixedRepresentation2D* sliderRep = msvVTKSliderFixedRepresentation2D::New();
    sliderRep->SetMinimumValue(0.0);
    sliderRep->SetMaximumValue(100.0);
    sliderRep->SetLabelFormat("%0.f%%");
    sliderRep->SetValue(0.0);
    sliderRep->SetTitleText("");
    sliderRep->GetPoint1Coordinate()->SetCoordinateSystemToDisplay();
    sliderRep->GetPoint1Coordinate()->SetValue(24,48);
    sliderRep->GetPoint2Coordinate()->SetCoordinateSystemToDisplay();
    sliderRep->GetPoint2Coordinate()->SetValue(24,96);
    sliderRep->SetSliderLength(0.07);
    sliderRep->SetSliderWidth(0.04);
    sliderRep->SetEndCapLength(0.07);
    sliderRep->SetEndCapWidth(0.04);
    sliderRep->SetTubeWidth(0.005);
    sliderRep->GetSliderProperty()->SetColor(.35,.45,.6);
    sliderRep->GetSliderProperty()->SetOpacity(.9);
    sliderRep->GetCapProperty()->SetOpacity(.9);
    sliderRep->GetCapProperty()->SetColor(.3,.4,.5);
    sliderRep->GetSelectedProperty()->SetOpacity(.9);
    sliderRep->GetSelectedProperty()->SetColor(.4,.5,.7);
    sliderRep->GetTubeProperty()->SetOpacity(.9);
    sliderRep->GetTubeProperty()->SetColor(.3,.4,.5);
    sliderRep->SetVisibility(false);
    m_Slider = vtkSliderWidget::New();
    m_Slider->SetRepresentation(sliderRep);
    m_Slider->SetAnimationModeToAnimate();
    sliderRep->Delete();
    m_Slider->AddObserver(vtkCommand::InteractionEvent,q->getSliderCallback());
  }
  return m_Slider;
}

void msvQVTKButtonsGroupPrivate::setElementProperty(QString name, QVariant value)
{
  for(QVector<msvQVTKButtonsInterface*>::iterator buttonsIt = m_Elements.begin(); buttonsIt != m_Elements.end(); buttonsIt++)
  {
    (*buttonsIt)->setProperty(name.toStdString().c_str(),value);
  }
}

void msvQVTKButtonsGroupPrivate::setCurrentRenderer(vtkRenderer* renderer)
{
  for(QVector<msvQVTKButtonsInterface*>::iterator buttonsIt = m_Elements.begin(); buttonsIt != m_Elements.end(); buttonsIt++)
  {
    (*buttonsIt)->setCurrentRenderer(renderer);
  }
}

//------------------------------------------------------------------------------
msvQVTKButtonsGroup::msvQVTKButtonsGroup(QObject *parent) : msvQVTKButtonsInterface(), m_SliderCallback(NULL), d_ptr(new msvQVTKButtonsGroupPrivate(*this))
{
  Q_D(msvQVTKButtonsGroup);
  m_ButtonCallback = vtkButtonCallbackGroup::New();
  reinterpret_cast<vtkButtonCallbackGroup*>(m_ButtonCallback)->toolButton = this;
  m_HighlightCallback = vtkButtonHighLightCallbackGroup::New();
  reinterpret_cast<vtkButtonHighLightCallbackGroup*>(m_HighlightCallback)->toolButton = this;

  m_SliderCallback = vtkSliderCallback::New();
  reinterpret_cast<vtkSliderCallback*>(m_SliderCallback)->toolButton = this;

  button()->AddObserver(vtkCommand::StateChangedEvent,m_ButtonCallback);
  button()->GetRepresentation()->AddObserver(vtkCommand::HighlightEvent,m_HighlightCallback);
}

msvQVTKButtonsGroup::~msvQVTKButtonsGroup()
{

}

void msvQVTKButtonsGroup::setElementProperty(QString name, QVariant value) {
  Q_D(msvQVTKButtonsGroup);
  d->setElementProperty(name,value);
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
    reinterpret_cast<vtkSliderCallback*>(m_SliderCallback)->renderer = NULL;
    slider()->EnabledOff();
  }
  Q_D(msvQVTKButtonsGroup);
  d->setCurrentRenderer(renderer);
}

void msvQVTKButtonsGroup::addElement(msvQVTKButtonsInterface* buttons)
{
  Q_D(msvQVTKButtonsGroup);
  d->addElement(buttons);
}

void msvQVTKButtonsGroup::removeElement(msvQVTKButtonsInterface* buttons)
{
  Q_D(msvQVTKButtonsGroup);
  d->removeElement(buttons);
}

msvQVTKButtonsInterface* msvQVTKButtonsGroup::getElement(int index)
{
  Q_D(msvQVTKButtonsGroup);
  return d->getElement(index);
}

msvQVTKButtonsGroup* msvQVTKButtonsGroup::createGroup()
{
  Q_D(msvQVTKButtonsGroup);
  msvQVTKButtonsGroup* element = new msvQVTKButtonsGroup();
  d->addElement(element);
  return element;
}

msvQVTKButtons* msvQVTKButtonsGroup::createButtons()
{
  Q_D(msvQVTKButtonsGroup);
  msvQVTKButtons* element = new msvQVTKButtons();
  d->addElement(element);
  return element;
}

void msvQVTKButtonsGroup::update()
{
  calculatePosition();
  msvQVTKButtonsInterface::update();
}

void msvQVTKButtonsGroup::calculatePosition()
{
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

vtkSliderWidget* msvQVTKButtonsGroup::slider()
{
  Q_D(msvQVTKButtonsGroup);
  return d->slider();
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
    setShowButton(val);
    this->update();
  }
}

void msvQVTKButtonsGroup::setCameraPoistionOnPath(double ratio) {
  // get the number of buttons
  Q_D(msvQVTKButtonsGroup);
  int numOfButtons = d->numberOfElements();
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

  d->getElement(targetButton)->bounds(b1);
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
    d->getElement(targetButton+1)->bounds(b2);
    resetBounds[0] = b1[0] * (1 - subPathRatio) + b2[0] * subPathRatio;
    resetBounds[1] = b1[1] * (1 - subPathRatio) + b2[1] * subPathRatio;
    resetBounds[2] = b1[2] * (1 - subPathRatio) + b2[2] * subPathRatio;
    resetBounds[3] = b1[3] * (1 - subPathRatio) + b2[3] * subPathRatio;
    resetBounds[4] = b1[4] * (1 - subPathRatio) + b2[4] * subPathRatio;
    resetBounds[5] = b1[5] * (1 - subPathRatio) + b2[5] * subPathRatio;
  }

  slider()->GetCurrentRenderer()->ResetCamera(resetBounds);
}

vtkCommand* msvQVTKButtonsGroup::getSliderCallback()
{
  return m_SliderCallback;
}