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

#ifndef __msvQVTKButtonsInterface_h
#define __msvQVTKButtonsInterface_h

// Qt includes
#include <QImage>

// MSVTK includes
#include "msvQtWidgetsExport.h"

//forward declarations
class vtkRenderer;
class vtkButtonWidget;
class vtkCommand;
class msvQVTKButtonsAction;
class msvVTKButtonsInterface;

// Pimpl
class msvQVTKButtonsInterfacePrivate;

/// \brief Qt wrapper around msvVTKButtonsInterface.
/// It exposes Qt signal/slot mechanism for a msvVTKButtonsInterface.
/// \sa msvVTKButtonsInterface
class MSV_QT_WIDGETS_EXPORT msvQVTKButtonsInterface : public QObject {
  Q_OBJECT
  /// \sa label(), setLabel(), showLabel
  Q_PROPERTY(QString label READ label WRITE setLabel);
  /// This property controls the text shown on mouse over. The text can be
  /// html code and therefore can contain images/tables/urls...
  /// \sa toolTip(), setToolTip(), showTooltip
  Q_PROPERTY(QString toolTip READ toolTip WRITE setToolTip);
  /// \sa image(), setImage()
  //Q_PROPERTY(QString image READ image WRITE setImage);
  /// This property controls whether the button is visible or not.
  /// \sa showButton(), setShowButton()
  Q_PROPERTY(bool showButton READ showButton WRITE setShowButton);
  /// This property controls whether the label of the button is visible or not.
  /// \sa showLabel(), setShowLabel(), label
  Q_PROPERTY(bool showLabel READ showLabel WRITE setShowLabel);
  /// This property controls whether the tooltip are shown on mouse over.
  /// \sa showToolTip(), setShowTooltip(), toolTip
  Q_PROPERTY(bool showTooltip WRITE setShowTooltip);

public:
  /// Object constructor
  msvQVTKButtonsInterface(QObject *parent = 0);
  /// Object destructor.
  virtual ~msvQVTKButtonsInterface();

  /// Show/hide button in the renderer.
  /// \sa showButton, showButton()
  void setShowButton(bool visible);

  /// Return true if the button is visible.
  /// \sa showButton, setShowButton()
  bool showButton();

  /// Show/hide the label(text) of the button.
  /// \sa showLabel, showLabel()
  void setShowLabel(bool show);

  /// Set the icon of the button.
  /// \sa image, image()
  void setImage(QImage image);

  /// Return the icon of the button.
  /// \sa image, setImage()
  //QString image();

  /// Return true if the label is visible, false otherwise.
  /// \sa showLabel, setShowLabel()
  bool showLabel();

  /// Set the text label.
  /// \sa label, label()
  void setLabel(QString text);

  /// Return the text label.
  /// \sa label, setLabel()
  QString label();

  /// Set the tooltip that is visible on mouse hover.
  /// \sa tooltip, tooltip()
  void setToolTip(QString text);

  /// Return the tooltip of the button.
  /// \sa toolTip, setToolTip()
  QString toolTip();

  /// Set the tooltip visibility.
  /// \sa showTooltip
  void setShowTooltip(bool value);

  /// Return the button widget.
  vtkButtonWidget *button();

  /// Add vtk button to Renderer.
  virtual void setCurrentRenderer(vtkRenderer *renderer)=0;

  /// update graphic objects
  virtual void update() = 0;

  /// get element bounds
  void bounds(double b[6]);

  /// set the element bounds
  void setBounds(double b[6]);

  /// get vtk buttons interface widget
  msvVTKButtonsInterface* vtkButtonsInterface();

  /// set the associated vtk buttons
  void setVTKButtonsInterface(msvVTKButtonsInterface* buttons);

signals:

  /// signal launched with shown tooltip
  void showTooltip(QString text);

  /// signal launched with shown tooltip
  void hideTooltip();

  /// show/hide
  void show(bool show);

protected:
  QScopedPointer<msvQVTKButtonsInterfacePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(msvQVTKButtonsInterface);
  Q_DISABLE_COPY(msvQVTKButtonsInterface);
};


#endif // __msvQVTKButtonsInterface_h
