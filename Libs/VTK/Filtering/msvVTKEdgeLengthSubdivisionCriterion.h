/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkEdgeLengthSubdivisionCriterion.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright 2003 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
  license for use of this work by or on behalf of the
  U.S. Government. Redistribution and use in source and binary forms, with
  or without modification, are permitted provided that this Notice and any
  statement of authorship are reproduced on all copies.

=========================================================================*/
#ifndef __vtkEdgeLengthSubdivisionCriterion_h
#define __vtkEdgeLengthSubdivisionCriterion_h
// .NAME vtkEdgeLengthSubdivisionCriterion - a subclass of vtkEdgeSubdivisionCriterion for vtkDataSet objects.
//
// .SECTION Description
// This is a subclass of vtkEdgeSubdivisionCriterion that is used for
// tessellating cells of a vtkDataSet, particularly nonlinear
// cells.
//
// It provides functions for setting the current cell being tessellated and a
// convenience routine, \a EvaluateFields() to evaluate field values at a
// point. You should call \a EvaluateFields() from inside \a EvaluateEdge()
// whenever the result of \a EvaluateEdge() will be true. Otherwise, do
// not call \a EvaluateFields() as the midpoint is about to be discarded.
// (<i>Implementor's note</i>: This isn't true if UGLY_ASPECT_RATIO_HACK
// has been defined. But in that case, we don't want the exact field values;
// we need the linearly interpolated ones at the midpoint for continuity.)
//
// .SECTION See Also
// vtkEdgeSubdivisionCriterion

#include "vtkDataSetEdgeSubdivisionCriterion.h"

class vtkCell;
class vtkDataSet;

class VTK_COMMON_EXPORT vtkEdgeLengthSubdivisionCriterion : public vtkDataSetEdgeSubdivisionCriterion
{
  public:
    vtkTypeMacro(vtkEdgeLengthSubdivisionCriterion,vtkDataSetEdgeSubdivisionCriterion);
    static vtkEdgeLengthSubdivisionCriterion* New();
    virtual void PrintSelf( ostream& os, vtkIndent indent );

    virtual bool EvaluateEdge( const double* p0, double* midpt, const double* p1, int field_start );

    // Maximum edge length.
    vtkSetMacro(MaxLength,double);
    vtkGetMacro(MaxLength,double);

  protected:
    vtkEdgeLengthSubdivisionCriterion();
    virtual ~vtkEdgeLengthSubdivisionCriterion();

    double MaxLength;

  private:
    vtkEdgeLengthSubdivisionCriterion( const vtkEdgeLengthSubdivisionCriterion& ); // Not implemented.
    void operator = ( const vtkEdgeLengthSubdivisionCriterion& ); // Not implemented.
};

#endif // __vtkEdgeLengthSubdivisionCriterion_h
