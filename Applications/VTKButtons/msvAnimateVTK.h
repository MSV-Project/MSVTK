/*
 *  msvAnimateVTK.h
 *  mafPluginVTK
 *
 *  Created by Roberto Mucci on 21/02/12.
 *  Copyright 2009 B3C.s All rights reserved.
 *
 *  See License at: http://tiny.cc/QXJ4D
 *
 */

#ifndef MSVANIMATEVTK_H
#define MSVANIMATEVTK_H

// Includes list

#include "msvVTKButtonsExport.h"
#include <QVTKWidget.h>


/**
Class name: msvAnimateVTK
This is an utility class to animate VTKCamera.
*/

class MSV_VTKButtons_EXPORT msvAnimateVTK : public QObject
{
    Q_OBJECT

public:
    /// Object constructor.
    msvAnimateVTK(const QString code_location = "");

    /// Animate the camera to zoom on the passed bounding box.
    void flyTo(QVTKWidget *widget, double bounds[6], int numberOfSteps = 120);


    /// Object destructor.
    /* virtual */ ~msvAnimateVTK();

};

#endif // MSVANIMATEVTK_H
