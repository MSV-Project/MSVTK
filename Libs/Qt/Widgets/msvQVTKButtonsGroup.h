/*
 *  msvQVTKButtonsGroup.h
 *  
 *
 *  Created by Alberto Losi on 08/08/12.
 *  Copyright 2009 B3C.s All rights reserved.
 *
 *  See License at: http://tiny.cc/QXJ4D
 *
 */

#ifndef msvQVTKButtonsGroup_H
#define msvQVTKButtonsGroup_H

// Includes list

#include "msvQtWidgetsExport.h"
#include "msvQVTKButtonsInterface.h"
#include <Qstring>
#include <QVariant>
#include <QVector>

//forward declarations
class vtkRenderer;
class msvQVTKButtons;
class vtkSliderWidget;
class vtkSliderCallback;

/**
Class name: msvQVTKButtonsGroup
Manager class to manage groups of msvQVTKButtons
*/

class MSV_QT_WIDGETS_EXPORT msvQVTKButtonsGroup : public msvQVTKButtonsInterface 
{
    Q_OBJECT

public:
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
    void setIconFileName(QString iconFileName);

    /// set the text label
    // void setLabel(QString text);

    /// Allow to activate FlyTo animation
    // void setFlyTo(bool active);

    /// Allow to set button position on center or on corner
    // void setOnCenter(bool onCenter);

    /// set the tooltip string
    // void setToolTip(QString text);

    /// add vtk button to Renderer
    void setCurrentRenderer(vtkRenderer *renderer);

    /// Update graphic objects
    void update();

    /// Get the slider widget
    vtkSliderWidget* slider();

    /// Show/hide the slider widget
    void showSlider(bool show);

    /// Set the position on the path at the specified ratio
    void setCameraPoistionOnPath(double ratio);

public slots:

    /// hide/show group
    void show(bool val);

private:
    /// Set the specified property
    void setElementProperty(QString name, QVariant value);

    /// Calculate element position
    void calculatePosition();

    /// Update element bounds
    void updateBounds();

    QVector<msvQVTKButtonsInterface*> m_Elements; //< Vector of buttons
    vtkSliderWidget* m_Slider; //< Slider widget
    vtkSliderCallback* m_SliderCallback; //< Slider callback function
};

/////////////////////////////////////////////////////////////
// Inline methods
/////////////////////////////////////////////////////////////

inline void msvQVTKButtonsGroup::setShowButton(bool show) {
  msvQVTKButtonsInterface::setShowButton(show);
  setElementProperty("showButton",show);
}

inline void msvQVTKButtonsGroup::setShowLabel(bool show) {
  msvQVTKButtonsInterface::setShowLabel(show);
  setElementProperty("showLabel",show);
}

inline void msvQVTKButtonsGroup::setIconFileName(QString iconFileName) {
  msvQVTKButtonsInterface::setIconFileName(iconFileName);
  setElementProperty("iconFileName",iconFileName);
}

// inline void msvQVTKButtonsGroup::setLabel(QString text) {
//   msvQVTKButtonsInterface::setLabel(text);
//   setElementProperty("label",text);
// }

// inline void msvQVTKButtonsGroup::setFlyTo(bool active) {
//   msvQVTKButtonsInterface::setFlyTo(active);
//   setElementProperty("flyTo",active);
// }

// inline void msvQVTKButtonsGroup::setOnCenter(bool onCenter) {
//   msvQVTKButtonsInterface::setOnCenter(onCenter);
//   setElementProperty("onCenter",onCenter);
// }

// inline void msvQVTKButtonsGroup::setToolTip(QString text) {
//   msvQVTKButtonsInterface::setToolTip(text);
//   setElementProperty("toolTip",text);
// }

#endif // msvQVTKButtonsGroup_H
