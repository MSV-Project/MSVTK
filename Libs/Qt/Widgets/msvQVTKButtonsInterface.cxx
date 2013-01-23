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
#include <vtkQImageToImageSource.h>
#include <vtkRenderer.h>

// MSVTK includes
#include "msvQVTKButtonsInterface.h"
#include "msvVTKButtonsInterface.h"

#define VTK_CREATE(type, name) vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//------------------------------------------------------------------------------
class msvQVTKButtonsInterfacePrivate
{
  Q_DECLARE_PUBLIC(msvQVTKButtonsInterface);

protected:
  msvQVTKButtonsInterface* const q_ptr; ///< PIMPL pointer
  msvVTKButtonsInterface* m_VTKButtons;
  QString m_IconFileName;
  msvQVTKButtonsAction* m_Action;

public:
  inline void setLabel(QString label){this->getVTKButtonsInterface()->SetLabel(label.toAscii());};
  inline QString label(){return QString(this->getVTKButtonsInterface()->GetLabel());};
  inline void setTooltip(QString tooltip){this->getVTKButtonsInterface()->SetTooltip(tooltip.toStdString().c_str());};
  inline QString tooltip(){return QString(this->getVTKButtonsInterface()->GetTooltip());};
  inline QString iconFileName(){return m_IconFileName;};
  inline void setShowButton(bool show){this->getVTKButtonsInterface()->SetShowButton(show);};
  inline bool showButton(){return this->getVTKButtonsInterface()->GetShowButton();};
  inline void setShowLabel(bool show){this->getVTKButtonsInterface()->SetShowLabel(show);};
  inline bool showLabel(){return this->getVTKButtonsInterface()->GetShowLabel();};
  inline void setAction(msvQVTKButtonsAction* action){m_Action = action;};
  inline msvQVTKButtonsAction* action(){return m_Action;};
  inline vtkButtonWidget* button(){return this->getVTKButtonsInterface()->GetButton();};
  inline void setBounds(double bds[6]){this->getVTKButtonsInterface()->SetBounds(bds);};
  inline void bounds(double bds[6]){this->getVTKButtonsInterface()->GetBounds(bds);};
  inline void update(){this->getVTKButtonsInterface()->Update();};
  inline void setCurrentRenderer(vtkRenderer* renderer){this->getVTKButtonsInterface()->SetCurrentRenderer(renderer);};
  inline void setVTKButtonsInterface(msvVTKButtonsInterface *buttons){this->m_VTKButtons = buttons;};
  void setIconFileName(QString iconfilename);
  msvQVTKButtonsInterfacePrivate(msvQVTKButtonsInterface& object);
  virtual msvVTKButtonsInterface* getVTKButtonsInterface();
  virtual ~msvQVTKButtonsInterfacePrivate();
};

//------------------------------------------------------------------------------
msvQVTKButtonsInterfacePrivate::msvQVTKButtonsInterfacePrivate(msvQVTKButtonsInterface& object)
  : m_VTKButtons(NULL), q_ptr(&object)
{
  //m_VTKButtonsInterface = msvVTKButtonsInterface::New();
}

//------------------------------------------------------------------------------
/*virtual*/ msvVTKButtonsInterface* msvQVTKButtonsInterfacePrivate::getVTKButtonsInterface()
{
    return m_VTKButtons;
}

//------------------------------------------------------------------------------
msvQVTKButtonsInterfacePrivate::~msvQVTKButtonsInterfacePrivate()
{
  //this->getVTKButtons()->Delete();
}

//------------------------------------------------------------------------------
void msvQVTKButtonsInterfacePrivate::setIconFileName(QString iconfilename)
{
  m_IconFileName = iconfilename;  QImage image;
  image.load(m_IconFileName);
  vtkQImageToImageSource *imageToVTK = vtkQImageToImageSource::New();
  imageToVTK->SetQImage(&image);
  imageToVTK->Update();
  this->getVTKButtonsInterface()->SetImage(imageToVTK->GetOutput());
  imageToVTK->Delete();
}

//------------------------------------------------------------------------------
msvQVTKButtonsInterface::msvQVTKButtonsInterface(QObject *parent)
  : QObject(parent), d_ptr(new msvQVTKButtonsInterfacePrivate(*this))
{

}

//------------------------------------------------------------------------------
msvQVTKButtonsInterface::~msvQVTKButtonsInterface()
{

}

//------------------------------------------------------------------------------
vtkButtonWidget *msvQVTKButtonsInterface::button()
{
  Q_D(msvQVTKButtonsInterface);
  return d->button();
}

//------------------------------------------------------------------------------
void msvQVTKButtonsInterface::setIconFileName(QString iconFileName)
{
  Q_D(msvQVTKButtonsInterface);
  d->setIconFileName(iconFileName);
}

//------------------------------------------------------------------------------
void msvQVTKButtonsInterface::update()
{
  Q_D(msvQVTKButtonsInterface);
  d->update();
}

//------------------------------------------------------------------------------
void msvQVTKButtonsInterface::setCurrentRenderer(vtkRenderer *renderer)
{
  Q_D(msvQVTKButtonsInterface);
  d->setCurrentRenderer(renderer);
}

//------------------------------------------------------------------------------
void msvQVTKButtonsInterface::bounds(double b[6])
{
  Q_D(msvQVTKButtonsInterface);
  d->bounds(b);
}

//------------------------------------------------------------------------------
void msvQVTKButtonsInterface::setShowButton(bool visible)
{
  Q_D(msvQVTKButtonsInterface);
  d->setShowButton(visible);
  //Q_EMIT(show(visible));
}

//------------------------------------------------------------------------------
void msvQVTKButtonsInterface::setShowTooltip(bool value)
{
  Q_D(msvQVTKButtonsInterface);
  if(value) {
    Q_EMIT showTooltip(d->tooltip());
  } else {
    Q_EMIT hideTooltip();
  }
}

//------------------------------------------------------------------------------
bool msvQVTKButtonsInterface::showButton()
{
  Q_D(msvQVTKButtonsInterface);
  return d->showButton();
}

//------------------------------------------------------------------------------
void msvQVTKButtonsInterface::setShowLabel(bool show)
{
  Q_D(msvQVTKButtonsInterface);
  d->setShowLabel(show);
}

//------------------------------------------------------------------------------
bool msvQVTKButtonsInterface::showLabel()
{
  Q_D(msvQVTKButtonsInterface);
  return d->showLabel();
}

//------------------------------------------------------------------------------
void msvQVTKButtonsInterface::setLabel(QString text)
{
  Q_D(msvQVTKButtonsInterface);
  d->setLabel(text);
}

//------------------------------------------------------------------------------
QString msvQVTKButtonsInterface::label()
{
  Q_D(msvQVTKButtonsInterface);
  return d->label();
}

//------------------------------------------------------------------------------
QString msvQVTKButtonsInterface::toolTip()
{
  Q_D(msvQVTKButtonsInterface);
  return d->tooltip();
}

//------------------------------------------------------------------------------
QString msvQVTKButtonsInterface::iconFileName()
{
  Q_D(msvQVTKButtonsInterface);
  return d->iconFileName();
}

//------------------------------------------------------------------------------
void msvQVTKButtonsInterface::setToolTip(QString text)
{
  Q_D(msvQVTKButtonsInterface);
  d->setTooltip(text);
}

//------------------------------------------------------------------------------
void msvQVTKButtonsInterface::setBounds(double b[6])
{
  Q_D(msvQVTKButtonsInterface);
  d->setBounds(b);
}

//------------------------------------------------------------------------------
msvVTKButtonsInterface* msvQVTKButtonsInterface::getVTKButtonsInterface()
{
  Q_D(msvQVTKButtonsInterface);
  return d->getVTKButtonsInterface();
}

//------------------------------------------------------------------------------
void msvQVTKButtonsInterface::setVTKButtonsInterface(msvVTKButtonsInterface* buttons)
{
  Q_D(msvQVTKButtonsInterface);
  return d->setVTKButtonsInterface(buttons);
}
