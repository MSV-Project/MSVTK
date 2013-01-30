// Filename: msvVTKIBSourceGen.C
// Created on 28 Apr 2011 by Boyce Griffith
//
// Copyright (c) 2002-2010, Boyce Griffith
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of New York University nor the names of its
//      contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include "msvVTKIBSourceGen.h"

/////////////////////////////// INCLUDES /////////////////////////////////////

#ifndef included_IBAMR_config
// #include <IBAMR_config.h>
#define included_IBAMR_config
#endif

#ifndef included_SAMRAI_config
#include <SAMRAI_config.h>
#define included_SAMRAI_config
#endif

// IBAMR INCLUDES
#include <ibamr/IBSourceSpec.h>
#include <ibamr/namespaces.h>

// SAMRAI INCLUDES
#include <tbox/RestartManager.h>

// MSV INCLUDES
#include <msvVTKBoundaryEdgeSources.h>

// VTK INCLUDES
#include <vtkPoints.h>
#include <vtkNew.h>
#include <vtkDoubleArray.h>
#include <vtkPolyData.h>

/////////////////////////////// NAMESPACE ////////////////////////////////////

namespace IBAMR
{
/////////////////////////////// STATIC ///////////////////////////////////////

std::vector<int> msvVTKIBSourceGen::s_num_sources;
std::vector<std::vector<double> > msvVTKIBSourceGen::s_source_radii;
std::vector<vtkSmartPointer<vtkDataSet> > msvVTKIBSourceGen::polyData;

/////////////////////////////// PUBLIC ///////////////////////////////////////

msvVTKIBSourceGen::msvVTKIBSourceGen()
    : d_n_src(),
      d_r_src(),
      d_num_perimeter_nodes(),
      d_Q_src(),
      d_P_src()
{
    RestartManager::getManager()->registerRestartItem("msvVTKIBSourceGen", this);
    const bool from_restart = RestartManager::getManager()->isFromRestart();
    if (from_restart) getFromRestart();
    return;
}// msvVTKIBSourceGen

msvVTKIBSourceGen::~msvVTKIBSourceGen()
{
    // intentionally blank
    return;
}// ~msvVTKIBSourceGen

void
msvVTKIBSourceGen::setNumSources(
    const int ln,
    const unsigned int num_sources)
{
    s_num_sources.resize(std::max(static_cast<int>(s_num_sources.size()),ln+1),0);
    s_num_sources[ln] = num_sources;
    return;
}// getNumSources

unsigned int
msvVTKIBSourceGen::getNumSources(
    const int ln)
{
    return s_num_sources[ln];
}// getNumSources

void
msvVTKIBSourceGen::setSourceRadii(
    const int ln,
    const std::vector<double>& radii)
{
    s_source_radii.resize(std::max(static_cast<int>(s_source_radii.size()),ln+1));
    s_source_radii[ln] = radii;
    return;
}// getSourceRadii

const std::vector<double>&
msvVTKIBSourceGen::getSourceRadii(
    const int ln)
{
    return s_source_radii[ln];
}// getSourceRadii

std::vector<double>&
msvVTKIBSourceGen::getSourceStrengths(
    const int ln)
{
    return d_Q_src[ln];
}// getSourceStrengths

const std::vector<double>&
msvVTKIBSourceGen::getSourceStrengths(
    const int ln) const
{
    return d_Q_src[ln];
}// getSourceStrengths

const std::vector<double>&
msvVTKIBSourceGen::getSourcePressures(
    const int ln) const
{
    return d_P_src[ln];
}// getSourcePressures

void msvVTKIBSourceGen::setDataSet(int ln, vtkDataSet *dataSet)
{
  polyData.resize(std::max(static_cast<int>(polyData.size()),ln+1));
  polyData[ln] = vtkPolyData::New();
  polyData[ln]->CopyStructure(dataSet);
} // setDataSet

void
msvVTKIBSourceGen::initializeLevelData(
    const Pointer<PatchHierarchy<NDIM> > /*hierarchy*/,
    const int level_number,
    const double /*init_data_time*/,
    const bool /*initial_time*/,
    IBTK::LDataManager* const /*l_data_manager*/)
{
    d_n_src              .resize(std::max(level_number+1,static_cast<int>(d_n_src.size())),0);
    d_n_src[level_number] = getNumSources(level_number);

    return;
}// initializeLevelData

unsigned int
msvVTKIBSourceGen::getNumSources(
    const Pointer<PatchHierarchy<NDIM> > /*hierarchy*/,
    const int level_number,
    const double /*data_time*/,
    LDataManager* const /*l_data_manager*/)
{
#ifdef DEBUG_CHECK_ASSERTIONS
    TBOX_ASSERT(d_n_src[level_number] >= 0);
#endif
    return d_n_src[level_number];
}// getNumSources

void
msvVTKIBSourceGen::getSourceLocations(
    std::vector<blitz::TinyVector<double,NDIM> >& X_src,
    std::vector<double>& r_src,
    Pointer<LData> X_data,
    const Pointer<PatchHierarchy<NDIM> > /*hierarchy*/,
    const int level_number,
    const double /*data_time*/,
    LDataManager* const l_data_manager)
{
    if (this->polyData[level_number].GetPointer() == NULL) return;

    vtkPolyData *data = vtkPolyData::SafeDownCast(polyData[level_number]);
#ifdef DEBUG_CHECK_ASSERTIONS
    TBOX_ASSERT(X_src.size() == static_cast<unsigned int>(d_n_src[level_number]));
    TBOX_ASSERT(r_src.size() == static_cast<unsigned int>(d_n_src[level_number]));
#endif

    // Determine the positions of the sources.
    std::fill(X_src.begin(), X_src.end(), blitz::TinyVector<double,NDIM>(0.0));
    Vec X_petsc_vec = X_data->getVec();
    Vec X_temp;
    VecDuplicate(X_petsc_vec, &X_temp);
    l_data_manager->scatterPETScToLagrangian(X_petsc_vec,X_temp,level_number);
    double *X;
    int size;
    VecGetArray(X_temp,&X);
    VecGetSize(X_temp,&size);

    vtkNew<vtkDoubleArray> pointArray;
    vtkNew<vtkPoints> points;
    pointArray->SetArray(X,size,1);
    pointArray->SetNumberOfComponents(3);
    points->SetData(pointArray.GetPointer());

    data->SetPoints(points.GetPointer());

    vtkNew<msvVTKBoundaryEdgeSources> sourceDataset;

    sourceDataset->SetInput(data);
    sourceDataset->Update();

    vtkPoints *sourcePoints = sourceDataset->GetOutput()->GetPoints();

    // Set the radii of the sources.
    r_src = sourceDataset->GetRadii();
    this->setNumSources(level_number,sourcePoints->GetNumberOfPoints());

    for(vtkIdType i = 0; i < sourcePoints->GetNumberOfPoints(); ++i)
    {
      double p[3] = {0};
      sourcePoints->GetPoint(i,p);
      X_src[i][0] = p[0];
      X_src[i][1] = p[1];
      X_src[i][2] = p[2];
    }
    VecDestroy(&X_temp);
    return;
}// getSourceLocations

void
msvVTKIBSourceGen::setSourcePressures(
    const std::vector<double>& P_src,
    const Pointer<PatchHierarchy<NDIM> > /*hierarchy*/,
    const int level_number,
    const double /*data_time*/,
    LDataManager* const /*l_data_manager*/)
{
    d_P_src[level_number] = P_src;
    return;
}// setSourcePressures

void
msvVTKIBSourceGen::computeSourceStrengths(
    std::vector<double>& Q_src,
    const Pointer<PatchHierarchy<NDIM> > /*hierarchy*/,
    const int level_number,
    const double /*data_time*/,
    LDataManager* const /*l_data_manager*/)
{
    Q_src = d_Q_src[level_number];
    return;
}// computeSourceStrengths

void
msvVTKIBSourceGen::putToDatabase(
    Pointer<Database> db)
{
#ifdef DEBUG_CHECK_ASSERTIONS
    TBOX_ASSERT(!db.isNull());
#endif

    return;
}// putToDatabase

/////////////////////////////// PROTECTED ////////////////////////////////////

/////////////////////////////// PRIVATE //////////////////////////////////////

void
msvVTKIBSourceGen::getFromRestart()
{

    return;
}// getFromRestart

/////////////////////////////// NAMESPACE ////////////////////////////////////

} // namespace IBAMR

//////////////////////////////////////////////////////////////////////////////
