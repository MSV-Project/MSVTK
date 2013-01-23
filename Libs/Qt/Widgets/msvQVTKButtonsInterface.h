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

#ifndef msvQVTKButtonsInterface_H
#define msvQVTKButtonsInterface_H

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

/**
 Class name: msvQVTKButtonsInterface
 Interface class for buttons generalization
 */
class MSV_QT_WIDGETS_EXPORT msvQVTKButtonsInterface : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString label READ label WRITE setLabel);
  Q_PROPERTY(QString toolTip READ toolTip WRITE setToolTip);
  Q_PROPERTY(QString iconFileName READ iconFileName WRITE setIconFileName);
  Q_PROPERTY(bool showButton READ showButton WRITE setShowButton);
  Q_PROPERTY(bool showLabel READ showLabel WRITE setShowLabel);

public:
  /// Object constructor
  msvQVTKButtonsInterface(QObject *parent = 0);

  /// Allow to show/hide button
  void setShowButton(bool visible);

  /// Return showLabel flag
  bool showButton();

  /// Allow to show/hide label
  void setShowLabel(bool show);

  /// set the icon path
  void setIconFileName(QString iconFileName);

  /// Get the icon path
  QString iconFileName();

  /// Return showLabel flag
  bool showLabel();

  /// set the text label
  void setLabel(QString text);

  /// Get The string
  QString label();

  /// set the tooltip string
  void setToolTip(QString text);

  /// Get the tooltip string
  QString toolTip();

  /// Object destructor.
  virtual ~msvQVTKButtonsInterface();

  /// retrieve button pointer.
  vtkButtonWidget *button();

  /// add vtk button to Renderer
  void setCurrentRenderer(vtkRenderer *renderer);

  /// set the show/hide signal
  void setShowTooltip(bool value);

  /// update graphic objects
  void update();

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


#endif // msvQVTKButtonsInterface_H
