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

//------------------------------------------------------------------------------
class msvQVTKButtonsInterfacePrivate
{
  Q_DECLARE_PUBLIC(msvQVTKButtonsInterface);

protected:

  msvQVTKButtonsInterface* const q_ptr; ///< PIMPL pointer
  QString m_Label; ///< label of the button
  QString m_Tooltip; ///< tooltip associated to the button
  QString m_IconFileName; ///< File name of the image to be applied to the button.
  bool m_ShowButton;///< Flag to show/hide button
  bool m_ShowLabel; ///< Flag to show/hide label
  msvQVTKButtonsAction* m_Action; ///< Action performed when the vtk button is pressed (e.g. fly to)
  vtkButtonWidget *m_ButtonWidget; ///< VTK button widget.
  //vtkCommand *m_ButtonCallback; ///< Callback called by picking on vtkButton
  //vtkCommand *m_HighlightCallback; ///< Callback called by hovering over the button.
  QImage m_Image; ///< Button image
  double m_Bounds[6]; ///< Bounds of the data related to the button

public:

  // Setter and getter
  // @ToDo remove unused
  inline void setLabel(QString label){m_Label = label;};
  inline QString label(){return m_Label;};
  inline void setTooltip(QString tooltip){m_Tooltip = tooltip;};
  inline QString tooltip(){return m_Tooltip;};
  inline void setIconFileName(QString iconfilename){m_IconFileName = iconfilename;};
  inline QString iconFileName(){return m_IconFileName;};
  inline void setShowButton(bool show){m_ShowButton = show;};
  inline bool showButton(){return m_ShowButton;};
  inline void setShowLabel(bool show){m_ShowLabel = show;};
  inline bool showLabel(){return m_ShowLabel;};
  inline void setAction(msvQVTKButtonsAction* action){m_Action = action;};
  inline msvQVTKButtonsAction* action(){return m_Action;};
  inline void setButton(vtkButtonWidget* button){m_ButtonWidget = button;};
  vtkButtonWidget* button();
  //inline void setButtonCallback(vtkCommand* callback){m_ButtonCallback = callback;};
  //inline vtkCommand* buttonCallback(){return m_ButtonCallback;};
  //inline void setHighlightCallback(vtkCommand* callback){m_HighlightCallback = callback;};
  //inline vtkCommand* highlightCallback(){return m_HighlightCallback;};
  inline void setImage(QImage image){m_Image = image;};
  inline QImage image(){return m_Image;};
  void setBounds(double bds[6]);
  void bounds(double bds[6]);
  msvQVTKButtonsInterfacePrivate(msvQVTKButtonsInterface& object);
  virtual ~msvQVTKButtonsInterfacePrivate();
};

msvQVTKButtonsInterfacePrivate::msvQVTKButtonsInterfacePrivate(msvQVTKButtonsInterface& object) : m_ShowButton(false), m_ShowLabel(true), m_ButtonWidget(NULL), q_ptr(&object)
{
  vtkTexturedButtonRepresentation2D* rep = vtkTexturedButtonRepresentation2D::New();
  rep->SetNumberOfStates(1);
  button()->SetRepresentation(rep);
  rep->Delete();
}

msvQVTKButtonsInterfacePrivate::~msvQVTKButtonsInterfacePrivate()
{
  button()->Delete();
}

vtkButtonWidget* msvQVTKButtonsInterfacePrivate::button()
{
  if(m_ButtonWidget == NULL)
  {
    m_ButtonWidget = vtkButtonWidget::New();
  }
  return m_ButtonWidget;
}

void msvQVTKButtonsInterfacePrivate::setBounds(double bds[6])
{
  for(int i = 0; i < 6; i++)
  {
    m_Bounds[i] = bds[i];
  }
}

void msvQVTKButtonsInterfacePrivate::bounds(double bds[6])
{
  for(int i = 0; i < 6; i++)
  {
    bds[i] = m_Bounds[i];
  }
}

//------------------------------------------------------------------------------
msvQVTKButtonsInterface::msvQVTKButtonsInterface(QObject *parent) : QObject(parent), m_ButtonCallback(NULL), m_HighlightCallback(NULL)
{

}

msvQVTKButtonsInterface::~msvQVTKButtonsInterface()
{
  if(m_ButtonCallback)
    button()->AddObserver(vtkCommand::StateChangedEvent,m_ButtonCallback);
  if(m_HighlightCallback)
    button()->GetRepresentation()->AddObserver(vtkCommand::HighlightEvent,m_HighlightCallback);
}

vtkButtonWidget *msvQVTKButtonsInterface::button()
{
  Q_D(msvQVTKButtonsInterface);
  return d->button();
}

void msvQVTKButtonsInterface::setIconFileName(QString iconFileName)
{
  Q_D(msvQVTKButtonsInterface);
  d->setIconFileName(iconFileName);
  QImage image;
  image.load(d->iconFileName());
  vtkQImageToImageSource *imageToVTK = vtkQImageToImageSource::New();
  imageToVTK->SetQImage(&image);
  imageToVTK->Update();
  vtkTexturedButtonRepresentation2D *rep = static_cast<vtkTexturedButtonRepresentation2D *>(button()->GetRepresentation());
  rep->SetButtonTexture(0, imageToVTK->GetOutput());
  imageToVTK->Delete();
  int size[2]; size[0] = 16; size[1] = 16;
  rep->GetBalloon()->SetImageSize(size);

  update();
}

void msvQVTKButtonsInterface::update()
{
  Q_D(msvQVTKButtonsInterface);
  vtkTexturedButtonRepresentation2D *rep = reinterpret_cast<vtkTexturedButtonRepresentation2D*>(button()->GetRepresentation());

  if (d->showLabel())
  {
    //Add a label to the button and change its text property
    rep->GetBalloon()->SetBalloonText(d->label().toAscii());
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

  if(d->showButton())
  {
    button()->GetRepresentation()->SetVisibility(true);
    button()->EnabledOn();
  }
  else
  {
    button()->GetRepresentation()->SetVisibility(false);
    button()->EnabledOff();
  }
}

void msvQVTKButtonsInterface::setCurrentRenderer(vtkRenderer *renderer)
{
  if(renderer)
  {
    button()->SetInteractor(renderer->GetRenderWindow()->GetInteractor());
    button()->SetCurrentRenderer(renderer); //to check
    button()->EnabledOn();
  }
  else
  {
    button()->SetInteractor(NULL);
    button()->SetCurrentRenderer(NULL); //to check
    button()->EnabledOff();
  }
}

void msvQVTKButtonsInterface::bounds(double b[6])
{
  Q_D(msvQVTKButtonsInterface);
  double bds[6];
  d->bounds(bds);
  for(int i = 0; i < 6; i++)
  {
    b[i] = bds[i];
  }
}

void msvQVTKButtonsInterface::setShowButton(bool visible)
{
  Q_D(msvQVTKButtonsInterface);
  d->setShowButton(visible);
  Q_EMIT(show(visible));
}

inline void msvQVTKButtonsInterface::setShowTooltip(bool value)
{
  Q_D(msvQVTKButtonsInterface);
  if(value) {
    Q_EMIT showTooltip(d->tooltip());
  } else {
    Q_EMIT hideTooltip();
  }
}

inline bool msvQVTKButtonsInterface::showButton()
{
  Q_D(msvQVTKButtonsInterface);
  return d->showButton();
}

inline void msvQVTKButtonsInterface::setShowLabel(bool show)
{
  Q_D(msvQVTKButtonsInterface);
  d->setShowLabel(show);
}

inline bool msvQVTKButtonsInterface::showLabel()
{
  Q_D(msvQVTKButtonsInterface);
  return d->showLabel();
}

inline void msvQVTKButtonsInterface::setLabel(QString text)
{
  Q_D(msvQVTKButtonsInterface);
  d->setLabel(text);
}

inline QString msvQVTKButtonsInterface::label()
{
  Q_D(msvQVTKButtonsInterface);
  return d->label();
}

inline QString msvQVTKButtonsInterface::toolTip()
{
  Q_D(msvQVTKButtonsInterface);
  return d->tooltip();
}

inline QString msvQVTKButtonsInterface::iconFileName()
{
  Q_D(msvQVTKButtonsInterface);
  return d->iconFileName();
}

inline void msvQVTKButtonsInterface::setToolTip(QString text)
{
  Q_D(msvQVTKButtonsInterface);
  d->setTooltip(text);
}

void msvQVTKButtonsInterface::setBounds(double b[6])
{
  Q_D(msvQVTKButtonsInterface);
  setBounds(b);
}