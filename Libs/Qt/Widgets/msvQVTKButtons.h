/*
 *  msvQVTKButtons.h
 *  
 *
 *  Created by Daniele Giunchi on 13/01/12.
 *  Copyright 2011 B3C. All rights reserved.
 *
 *  See License at: http://tiny.cc/QXJ4D
 *
 */

#ifndef msvQVTKButtons_H
#define msvQVTKButtons_H

#include "msvQtWidgetsExport.h"
#include "msvQVTKButtonsInterface.h"

#include <QImage>

//forward declarations
class vtkRenderer;
class vtkButtonWidget;
class vtkButtonCallback;
class vtkButtonHighLightCallback;
class vtkDataSet;

/**
 Class name: msvQVTKButtons
 This is the tool representing a VTK buttons.
 */
class MSV_QT_WIDGETS_EXPORT msvQVTKButtons : public msvQVTKButtonsInterface {
    Q_OBJECT

public Q_SLOTS:
    /// Allow to execute and update the pipeline when something change.
    /*virtual*/ void update();

public:
     /// Object constructor.
    msvQVTKButtons(QObject *parent = 0);
    
    /// set the icon path
    void setIconFileName(QString iconFileName);

    // Allow to activate FlyTo animation
    void setFlyTo(bool active);

    /// add vtk button to Renderer
    void setCurrentRenderer(vtkRenderer *renderer);
    
    /// set bounds
    void setBounds(double b[6]);
    
    /// set the show/hide signal
    void setShowTooltip(bool value);
    
    /// Object destructor.
    virtual ~msvQVTKButtons();
    
    /// retrieve button pointer.
    vtkButtonWidget *button();

    QImage getPreview(int width, int height);

    /// set the data for preview
    void setData(vtkDataSet *data);

signals:
    /// signal launched with shown tooltip
    void showTooltip(QString text);
    
    /// signal launched with shown tooltip
    void hideTooltip();

private:
    /// calculate Position (center or corner)
    void calculatePosition();
    
    vtkButtonWidget *m_ButtonWidget; ///< VTK button widget.
    vtkButtonCallback *buttonCallback; ///< Callback called by picking on vtkButton
    vtkButtonHighLightCallback *highlightCallback; ///< Callback called by hovering over the button.
    
    QImage m_Image; ///< button image
    double m_Bounds[6]; ///< bounds of the data related to the button
    vtkDataSet* m_Data;
};

/////////////////////////////////////////////////////////////
// Inline methods
/////////////////////////////////////////////////////////////

inline void msvQVTKButtons::setShowTooltip(bool value) {
    if(value) {
        Q_EMIT showTooltip(m_Tooltip);
    } else {
        Q_EMIT hideTooltip();
    }
}

#endif // msvQVTKButtons_H
