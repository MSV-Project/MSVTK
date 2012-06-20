/*
 *  msvToolVTKButtons.h
 *  VTKButtons
 *
 *  Created by Roberto Mucci on 13/01/12.
 *  Copyright 2011 B3C. All rights reserved.
 *
 *  See License at: http://tiny.cc/QXJ4D
 *
 */

#ifndef MSVTOOLVTKBUTTONS_H
#define MSVTOOLVTKBUTTONS_H

#include "msvVTKButtonsExport.h"

#include <QObject.h>
class vtkButtonWidget;
class vtkButtonCallback;
class vtkButtonHighLightCallback;
class vtkRenderer;

/**
 Class name: msvToolVTKButtons
 This is the tool representing a VTK buttons.
 */
class MSV_VTKButtons_EXPORT msvToolVTKButtons : public QObject {
    Q_OBJECT
    
public Q_SLOTS:
    /// Allow to execute and update the pipeline when something change.
    /*virtual*/ void update();

public:
     /// Object constructor.
    msvToolVTKButtons();

    /// Allow to take the tool to the initial conditions.
    /*virtual*/ void resetTool();

    /// Allow to show/hide button
    void setShowButton(bool show);

    /// Return showLabel flag

    bool showButton() const;

    /// Allow to show/hide label
    void setShowLabel(bool show);

    /// Return showLabel flag
    bool showLabel() const;
    
    void setLabel(QString &text);

    /// Allow to activate FlyTo animation
    void setFlyTo(bool active);

    /// Return FlyTo flag
    bool FlyTo() const;

    /// Allow to set button position on center or on corner
    void setOnCenter(bool onCenter);

    /// Return OnCenter flag
    bool OnCenter() const;

    /// Show tooltip
    void showTooltip();

    /// Hide tooltip
    void hideTooltip();

    /// add vtk button to Renderer
    void setCurrentRenderer(vtkRenderer *renderer);
    
    /// set buonds
    void setBounds(double b[6]);
    
protected:
    /// Object destructor.
    /* virtual */ ~msvToolVTKButtons();

private:
    /// calculate Position (center or corner)
    void calculatePosition();
    
    vtkButtonWidget *m_ButtonWidget; ///< VTK button widget.
    vtkButtonCallback *buttonCallback; ///< Callback called by picking on vtkButton
    vtkButtonHighLightCallback *highlightCallback; ///< Callback called by hovering over the button.
    
    QString m_Label; ///< label of the button
    QString m_IconFileName; ///< File name of the image to be applied to the button.
    bool m_ShowButton; ///< Flag to show/hide button
    bool m_ShowLabel; ///< Flag to show/hide label
    bool m_FlyTo; ///< Flag to activate FlyTo animation
    bool m_OnCenter; ///< Flag to set button position on center or on corner (can be refactored with a enum??)
    double bounds[6];
};

/////////////////////////////////////////////////////////////
// Inline methods
/////////////////////////////////////////////////////////////

inline void msvToolVTKButtons::setShowButton(bool show) {
    m_ShowButton = show;
}

inline bool msvToolVTKButtons::showButton() const {
    return m_ShowButton;
}

inline void msvToolVTKButtons::setShowLabel(bool show) {
    m_ShowLabel = show;
}

inline bool msvToolVTKButtons::showLabel() const {
    return m_ShowLabel;
}

inline void msvToolVTKButtons::setFlyTo(bool active) {
    m_FlyTo = active;
}

inline bool msvToolVTKButtons::FlyTo() const {
    return m_FlyTo;
}

inline void msvToolVTKButtons::setOnCenter(bool onCenter) {
    m_OnCenter = onCenter;
}

inline bool msvToolVTKButtons::OnCenter() const {
    return m_OnCenter;
}

inline void msvToolVTKButtons::setLabel(QString &text) {
    m_Label = text;
}


#endif // MSVTOOLVTKBUTTONS_H
