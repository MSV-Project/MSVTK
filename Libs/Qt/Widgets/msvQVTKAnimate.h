/*
 *  msvQVTKAnimate.h
 *  
 *
 *  Created by Daniele Giunchi on 21/02/12.
 *  Copyright 2009 B3C.s All rights reserved.
 *
 *  See License at: http://tiny.cc/QXJ4D
 *
 */

#ifndef msvQVTKAnimate_H
#define msvQVTKAnimate_H

// Includes list

#include "msvQtWidgetsExport.h"
#include <vtkRenderer.h>


/**
Class name: msvQVTKAnimate
This is an utility class to animate VTKCamera.
*/

class MSV_QT_WIDGETS_EXPORT msvQVTKAnimate 
{

public:
    /// Object constructor.
    msvQVTKAnimate();

    /// Animate the camera to zoom on the passed bounding box.
    void flyTo(vtkRenderer *renderer, double bounds[6], int numberOfSteps = 120);

    /// Object destructor.
    virtual ~msvQVTKAnimate();

};

#endif // msvQVTKAnimate_H
