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
#include <vtkButtonRepresentation.h>
#include <vtkButtonWidget.h>
#include <vtkCommand.h>
#include <vtkObject.h>
#include <vtkProperty2D.h>
#include <vtkQImageToImageSource.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSliderWidget.h>
#include <vtkTexturedButtonRepresentation2D.h>

// MSVTK includes
#include "msvQVTKButtons.h"
#include "msvQVTKButtonsGroup.h"
#include "msvVTKButtonsGroup.h"
#include "msvVTKSliderFixedRepresentation2D.h"

#define VTK_CREATE(type, name) vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//------------------------------------------------------------------------------
class msvQVTKButtonsGroupPrivate
{
  Q_DECLARE_PUBLIC(msvQVTKButtonsGroup);

protected:

  msvQVTKButtonsGroup* const q_ptr;
  QVector<msvQVTKButtonsInterface*> Elements;
  msvVTKButtonsGroup* VtkButtonsGroup;

public:
  msvQVTKButtonsGroupPrivate(msvQVTKButtonsGroup& object);
  virtual ~msvQVTKButtonsGroupPrivate();

  inline vtkSliderWidget* slider(){
    msvVTKButtonsGroup::SafeDownCast(this->vtkButtons())->GetSlider();};

  inline void setCurrentRenderer(vtkRenderer* renderer){
    return msvVTKButtonsGroup::SafeDownCast(
          this->vtkButtons())->SetCurrentRenderer(renderer);};

  inline void update(){
    msvVTKButtonsGroup::SafeDownCast(this->vtkButtons())->Update();};

  inline int numberOfElements(){return Elements.size();};

  msvQVTKButtonsInterface* getElement(int index);
  void addElement(msvQVTKButtonsInterface* buttons);
  void removeElement(msvQVTKButtonsInterface* buttons);
  virtual msvVTKButtonsInterface* vtkButtons();
  void setImage(QImage image);
};

//------------------------------------------------------------------------------
msvQVTKButtonsGroupPrivate::msvQVTKButtonsGroupPrivate(msvQVTKButtonsGroup& object)
  : VtkButtonsGroup(NULL), q_ptr(&object)
{
  Q_Q(msvQVTKButtonsGroup);
  q->setVTKButtonsInterface(this->vtkButtons());
}

//------------------------------------------------------------------------------
msvVTKButtonsInterface* msvQVTKButtonsGroupPrivate::vtkButtons()
{
  if (!this->VtkButtonsGroup)
  {
    this->VtkButtonsGroup = msvVTKButtonsGroup::New();
  }
  return this->VtkButtonsGroup;
}

//------------------------------------------------------------------------------
msvQVTKButtonsGroupPrivate::~msvQVTKButtonsGroupPrivate()
{

}

//------------------------------------------------------------------------------
void msvQVTKButtonsGroupPrivate::addElement(msvQVTKButtonsInterface* buttons)
{
  // add elements on both vectors
  Q_Q(msvQVTKButtonsGroup);
  msvVTKButtonsGroup::SafeDownCast(this->vtkButtons())->AddElement(
        buttons->vtkButtonsInterface());
  int i = 0;
  double b[6];
  buttons->bounds(b);
  double dimension = (b[1]-b[0])*(b[3]-b[2])*(b[5]-b[4]);
  q->connect(buttons, SIGNAL(show(bool)), q, SLOT(show(bool)));
  for (QVector<msvQVTKButtonsInterface*>::iterator buttonsIt = Elements.begin();
    buttonsIt != Elements.end(); ++buttonsIt)
  {
    if (*buttonsIt == buttons)
    {
      return;
    }
    (*buttonsIt)->bounds(b);
    double cur_dimension = (b[1]-b[0])*(b[3]-b[2])*(b[5]-b[4]);
    if (dimension > cur_dimension)
    {
      Elements.insert(i,buttons);
      return;
    }
    ++i;
  }
  Elements.push_back(buttons);
}

//------------------------------------------------------------------------------
void msvQVTKButtonsGroupPrivate::removeElement(msvQVTKButtonsInterface* buttons)
{
  // remove elements on both vectors
  int index = 0;
  for (QVector<msvQVTKButtonsInterface*>::iterator buttonsIt = Elements.begin();
    buttonsIt != Elements.end(); ++buttonsIt)
  {
    if (*buttonsIt == buttons)
    {
      Elements.remove(index);
      return;
    }
    ++index;
  }
  return msvVTKButtonsGroup::SafeDownCast(
        this->vtkButtons())->RemoveElement(buttons->vtkButtonsInterface());
}

//------------------------------------------------------------------------------
msvQVTKButtonsInterface* msvQVTKButtonsGroupPrivate::getElement(int index)
{
  if (index > Elements.size() - 1)
  {
    return NULL;
  }
  return Elements.at(index);
}

//------------------------------------------------------------------------------
void msvQVTKButtonsGroupPrivate::setImage(QImage image)
{
  vtkQImageToImageSource *imageToVTK = vtkQImageToImageSource::New();
  imageToVTK->SetQImage(&image);
  imageToVTK->Update();
  return msvVTKButtonsGroup::SafeDownCast(
        this->vtkButtons())->SetImage(imageToVTK->GetOutput());
  imageToVTK->Delete();
}

//------------------------------------------------------------------------------
msvQVTKButtonsGroup::msvQVTKButtonsGroup(QObject *parent)
  : msvQVTKButtonsInterface(), d_ptr(new msvQVTKButtonsGroupPrivate(*this))
{
  Q_UNUSED(parent);
}

//------------------------------------------------------------------------------
msvQVTKButtonsGroup::~msvQVTKButtonsGroup()
{

}

//------------------------------------------------------------------------------
void msvQVTKButtonsGroup::setCurrentRenderer(vtkRenderer *renderer) {
  //msvQVTKButtonsInterface::setCurrentRenderer(renderer);
  Q_D(msvQVTKButtonsGroup);
  d->setCurrentRenderer(renderer);
}

//------------------------------------------------------------------------------
void msvQVTKButtonsGroup::addElement(msvQVTKButtonsInterface* buttons)
{
  Q_D(msvQVTKButtonsGroup);
  d->addElement(buttons);
}

//------------------------------------------------------------------------------
void msvQVTKButtonsGroup::removeElement(msvQVTKButtonsInterface* buttons)
{
  Q_D(msvQVTKButtonsGroup);
  d->removeElement(buttons);
}

//------------------------------------------------------------------------------
msvQVTKButtonsInterface* msvQVTKButtonsGroup::getElement(int index)
{
  Q_D(msvQVTKButtonsGroup);
  return d->getElement(index);
}

//------------------------------------------------------------------------------
msvQVTKButtonsGroup* msvQVTKButtonsGroup::createGroup()
{
  Q_D(msvQVTKButtonsGroup);
  msvQVTKButtonsGroup* element = new msvQVTKButtonsGroup();
  d->addElement(element);
  return element;
}

//------------------------------------------------------------------------------
msvQVTKButtons* msvQVTKButtonsGroup::createButtons()
{
  Q_D(msvQVTKButtonsGroup);
  msvQVTKButtons* element = new msvQVTKButtons();
  d->addElement(element);
  return element;
}

//------------------------------------------------------------------------------
void msvQVTKButtonsGroup::update()
{
  Q_D(msvQVTKButtonsGroup);
  //msvQVTKButtonsInterface::update();
  d->update();
}

//------------------------------------------------------------------------------
vtkSliderWidget* msvQVTKButtonsGroup::slider()
{
  Q_D(msvQVTKButtonsGroup);
  return d->slider();
}

//------------------------------------------------------------------------------
void msvQVTKButtonsGroup::show(bool val)
{
  if (val)
  {
    msvQVTKButtonsInterface::setShowButton(val);
    this->update();
  }
}
