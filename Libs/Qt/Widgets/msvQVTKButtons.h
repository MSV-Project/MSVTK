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

#ifndef __msvQVTKButtons_h
#define __msvQVTKButtons_h

// MSVTK includes
#include "msvQVTKButtonsInterface.h"

//forward declarations
class vtkRenderer;
class vtkButtonWidget;
class vtkButtonCallback;
class vtkButtonHighLightCallback;
class vtkRenderWindow;
class vtkDataSet;

// Pimpl
class msvQVTKButtonsPrivate;

/**
 Class name: msvQVTKButtons
 This is the tool representing a VTK buttons.
 */
class MSV_QT_WIDGETS_EXPORT msvQVTKButtons : public msvQVTKButtonsInterface
{
  Q_OBJECT
  Q_PROPERTY(bool flyTo READ flyTo WRITE setFlyTo);
  Q_PROPERTY(bool onCenter READ onCenter WRITE setOnCenter);

public Q_SLOTS:
  /// Allow to execute and update the pipeline when something change.
  /*virtual*/ void update();

  /// Allow to set button position on center or on corner
  void setOnCenter(bool onCenter);

public:
  typedef msvQVTKButtonsInterface Superclass;
  /// Object constructor.
  msvQVTKButtons(QObject *parent = 0);

  /// Object destructor.
  virtual ~msvQVTKButtons();

  /// set the icon path
  //void setIconFileName(QString iconFileName);

  // Allow to activate FlyTo animation
  void setFlyTo(bool active);

  /// set bounds
  void setBounds(double b[6]);

  /// Get the button preview image
  QImage getPreview(int width, int height);

  /// set the data for preview
  void setData(vtkDataSet *data);

  /// Return FlyTo flag
  bool flyTo();

  /// Return OnCenter flag
  bool onCenter();

  // Set the current renderer
  void setCurrentRenderer(vtkRenderer *renderer);

protected:
  QScopedPointer<msvQVTKButtonsPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(msvQVTKButtons);
  Q_DISABLE_COPY(msvQVTKButtons);
};

#endif // __msvQVTKButtons_h
