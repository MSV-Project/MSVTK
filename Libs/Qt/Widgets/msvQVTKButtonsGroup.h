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

#ifndef __msvQVTKButtonsGroup_h
#define __msvQVTKButtonsGroup_h

// Qt includes
#include <QVariant>

// MSVTK Includes
#include "msvQVTKButtonsInterface.h"
#include "msvQtWidgetsExport.h"

// forward declarations
class vtkRenderer;
class msvQVTKButtons;
class vtkSliderWidget;
class vtkCommand;

// Pimpl
class msvQVTKButtonsGroupPrivate;

/// \brief Qt wrapper around msvVTKButtonsGroup.
/// It exposes Qt signal/slot mechanism for a msvVTKButtonsGroup.
/// \sa msvVTKButtonsGroup
class MSV_QT_WIDGETS_EXPORT msvQVTKButtonsGroup : public msvQVTKButtonsInterface
{
  Q_OBJECT

public:
  typedef msvQVTKButtonsInterface Superclass;

  /// Object constructor.
  msvQVTKButtonsGroup(QObject *parent = 0);

  /// Object destructor.
  virtual ~msvQVTKButtonsGroup();

  /// Add a buttons to the buttons' vector
  void addElement(msvQVTKButtonsInterface* buttons);

  /// Remove a buttons to the buttons' vector
  void removeElement(msvQVTKButtonsInterface* buttons);

  /// Get the specified element
  msvQVTKButtonsInterface* getElement(int index);

  /// create a new element
  msvQVTKButtonsGroup* createGroup();

  /// create a new element
  msvQVTKButtons* createButtons();

  /// Allow to show/hide button
  void setShowButton(bool show);

  /// Allow to show/hide label
  void setShowLabel(bool show);

  /// set the icon path
  void setImage(QImage image);

  /// add vtk button to Renderer
  void setCurrentRenderer(vtkRenderer *renderer);

  /// Update graphic objects
  void update();

  /// Get the slider widget
  vtkSliderWidget* slider();

public slots:

  /// hide/show group
  void show(bool val);

protected:
  QScopedPointer<msvQVTKButtonsGroupPrivate> d_ptr;

  /// Set the specified property
  void setElementProperty(QString name, QVariant value);

  /// Calculate element position
  void calculatePosition();

private:
  Q_DECLARE_PRIVATE(msvQVTKButtonsGroup);
  Q_DISABLE_COPY(msvQVTKButtonsGroup);
};

#endif // __msvQVTKButtonsGroup_h
