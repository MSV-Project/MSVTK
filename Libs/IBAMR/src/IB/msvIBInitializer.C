// Filename: msvIBInitializer.C
// Created on 22 Nov 2006 by Boyce Griffith
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

#include "msvIBInitializer.h"

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
#include <ibamr/IBAnchorPointSpec.h>
#include <ibamr/IBBeamForceSpec.h>
#include <ibamr/IBInstrumentationSpec.h>
#include <ibamr/IBRodForceSpec.h>
#include <ibamr/IBSourceSpec.h>
#include <ibamr/IBSpringForceSpec.h>
#include <ibamr/IBStandardSourceGen.h>
#include <ibamr/IBTargetPointForceSpec.h>
#include <ibamr/namespaces.h>

// IBTK INCLUDES
#include <ibtk/IndexUtilities.h>
#include <ibtk/LNodeSet.h>
#include <ibtk/LNodeSetData.h>

// SAMRAI INCLUDES
#include <Box.h>
#include <CartesianGridGeometry.h>
#include <CartesianPatchGeometry.h>
#include <CellData.h>
#include <CellIterator.h>
#include <Index.h>
#include <tbox/MathUtilities.h>
#include <tbox/RestartManager.h>
#include <tbox/SAMRAI_MPI.h>
#include <tbox/Utilities.h>

// C++ STDLIB INCLUDES
#include <fstream>
#include <iostream>
#include <limits>

// VTK INCLUDES
#include <vtkSmartPointer.h>
#include <vtkNew.h>
#include <vtkExtractEdges.h>
#include <vtkPolyData.h>
#include <vtkCell.h>

// MSV INCLUDES
#include "msvVTKIBSourceGen.h"
#include <msvVTKBoundaryEdgeSources.h>

/////////////////////////////// NAMESPACE ////////////////////////////////////

namespace IBAMR
{
/////////////////////////////// STATIC ///////////////////////////////////////

namespace
{
inline std::string
discard_comments(
    const std::string& input_string)
{
    // Create a copy of the input string, but without any text following a '!',
    // '#', or '%' character.
    std::string output_string = input_string;
    std::istringstream string_stream;

    // Discard any text following a '!' character.
    string_stream.str(output_string);
    std::getline(string_stream, output_string, '!');
    string_stream.clear();

    // Discard any text following a '#' character.
    string_stream.str(output_string);
    std::getline(string_stream, output_string, '#');
    string_stream.clear();

    // Discard any text following a '%' character.
    string_stream.str(output_string);
    std::getline(string_stream, output_string, '%');
    string_stream.clear();
    return output_string;
}// discard_comments
}

/////////////////////////////// PUBLIC ///////////////////////////////////////

msvIBInitializer::msvIBInitializer(
    const std::string& object_name,
    Pointer<Database> input_db,
    vtkSmartPointer<vtkDataSet> data)
    : d_object_name(object_name),
      d_use_file_batons(true),
      d_max_levels(-1),
      d_level_is_initialized(),
      d_silo_writer(NULL),
      d_base_filename(),
      d_length_scale_factor(1.0),
      d_posn_shift(0.0),
      d_num_vertex(),
      d_vertex_offset(),
      d_vertex_posn(),
      d_enable_springs(),
      d_spring_edge_map(),
      d_spring_spec_data(),
      d_using_uniform_spring_stiffness(),
      d_uniform_spring_stiffness(),
      d_using_uniform_spring_rest_length(),
      d_uniform_spring_rest_length(),
      d_using_uniform_spring_force_fcn_idx(),
      d_uniform_spring_force_fcn_idx(),
#if ENABLE_SUBDOMAIN_INDICES
      d_using_uniform_spring_subdomain_idx(),
      d_uniform_spring_subdomain_idx(),
#endif
      d_enable_beams(),
      d_beam_spec_data(),
      d_using_uniform_beam_bend_rigidity(),
      d_uniform_beam_bend_rigidity(),
      d_using_uniform_beam_curvature(),
      d_uniform_beam_curvature(),
#if ENABLE_SUBDOMAIN_INDICES
      d_using_uniform_beam_subdomain_idx(),
      d_uniform_beam_subdomain_idx(),
#endif
      d_enable_rods(),
      d_rod_edge_map(),
      d_rod_spec_data(),
      d_using_uniform_rod_properties(),
      d_uniform_rod_properties(),
#if ENABLE_SUBDOMAIN_INDICES
      d_using_uniform_rod_subdomain_idx(),
      d_uniform_rod_subdomain_idx(),
#endif
      d_enable_target_points(),
      d_target_spec_data(),
      d_using_uniform_target_stiffness(),
      d_uniform_target_stiffness(),
      d_using_uniform_target_damping(),
      d_uniform_target_damping(),
#if ENABLE_SUBDOMAIN_INDICES
      d_using_uniform_target_subdomain_idx(),
      d_uniform_target_subdomain_idx(),
#endif
      d_enable_anchor_points(),
      d_anchor_spec_data(),
#if ENABLE_SUBDOMAIN_INDICES
      d_using_uniform_anchor_subdomain_idx(),
      d_uniform_anchor_subdomain_idx(),
#endif
      d_enable_bdry_mass(),
      d_bdry_mass_spec_data(),
      d_using_uniform_bdry_mass(),
      d_uniform_bdry_mass(),
      d_using_uniform_bdry_mass_stiffness(),
      d_uniform_bdry_mass_stiffness(),
      d_directors(),
      d_enable_instrumentation(),
      d_instrument_idx(),
      d_enable_sources(),
      d_source_idx(),
      d_global_index_offset()
{
#ifdef DEBUG_CHECK_ASSERTIONS
    TBOX_ASSERT(!object_name.empty());
    TBOX_ASSERT(!input_db.isNull());
#endif
    
    // Register the specification objects with the StreamableManager class.
    IBAnchorPointSpec     ::registerWithStreamableManager();
    IBBeamForceSpec       ::registerWithStreamableManager();
    IBInstrumentationSpec ::registerWithStreamableManager();
    IBRodForceSpec        ::registerWithStreamableManager();
    IBSourceSpec          ::registerWithStreamableManager();
    IBSpringForceSpec     ::registerWithStreamableManager();
    IBTargetPointForceSpec::registerWithStreamableManager();

    // Initialize object with data read from the input database.
    getFromInput(input_db,data);

    // Check to see if we are starting from a restart file.
    RestartManager* restart_manager = RestartManager::getManager();
    const bool is_from_restart = restart_manager->isFromRestart();

    // Process the input files only if we are not starting from a restart file.
    if (!is_from_restart)
    {
        // Process the vertex information.
        readVertexDataset();

        // Process the (optional) spring information.
        readSpringDataset();

        // Process the (optional) beam information.
//         readBeamFiles();
// 
//         // Process the (optional) rod information.
//         readRodFiles();
// 
        // Process the (optional) target point information.
//         readTargetPointDataset();
// 
//         // Process the (optional) anchor point information.
//         readAnchorPointFiles();
// 
//         // Process the (optional) mass information.
//         readBoundaryMassFiles();
// 
//         // Process the (optional) directors information.
//         readDirectorFiles();
// 
//         // Process the (optional) instrumentation information.
//         readInstrumentationFiles();
// 
        // Process the (optional) source information.
        readSourceDatasets();
    }
    return;
}// msvIBInitializer

msvIBInitializer::~msvIBInitializer()
{
    pout << d_object_name << ":  Deallocating initialization data.\n";
    return;
}// ~msvIBInitializer

void
msvIBInitializer::registerLSiloDataWriter(
    Pointer<LSiloDataWriter> silo_writer)
{
#ifdef DEBUG_CHECK_ASSERTIONS
    TBOX_ASSERT(!silo_writer.isNull());
#endif

    // Cache a pointer to the data writer.
    d_silo_writer = silo_writer;

    // Check to see if we are starting from a restart file.
    RestartManager* restart_manager = RestartManager::getManager();
    const bool is_from_restart = restart_manager->isFromRestart();

    // Initialize the Silo data writer only if we are not starting from a
    // restart file.
    if (!is_from_restart)
    {
        for (int ln = 0; ln < d_max_levels; ++ln)
        {
            if (d_level_is_initialized[ln])
            {
                initializeLSiloDataWriter(ln);
            }
        }
    }
    return;
}// registerLSiloDataWriter

bool
msvIBInitializer::getLevelHasLagrangianData(
    const int level_number,
    const bool /*can_be_refined*/) const
{
    return !d_num_vertex[level_number].empty();
}// getLevelHasLagrangianData

unsigned int
msvIBInitializer::computeLocalNodeCountOnPatchLevel(
    const Pointer<PatchHierarchy<NDIM> > hierarchy,
    const int level_number,
    const double /*init_data_time*/,
    const bool can_be_refined,
    const bool /*initial_time*/)
{
    // Loop over all patches in the specified level of the patch level and count
    // the number of local vertices.
    int local_node_count = 0;
    Pointer<PatchLevel<NDIM> > level = hierarchy->getPatchLevel(level_number);
    for (PatchLevel<NDIM>::Iterator p(level); p; p++)
    {
        Pointer<Patch<NDIM> > patch = level->getPatch(p());

        // Count the number of vertices whose initial locations will be within
        // the given patch.
        std::vector<std::pair<int,int> > patch_vertices;
        getPatchVertices(patch_vertices, patch, level_number, can_be_refined);
        local_node_count += patch_vertices.size();
    }
    return local_node_count;
}// computeLocalNodeCountOnPatchLevel

void
msvIBInitializer::initializeStructureIndexingOnPatchLevel(
    std::map<int,std::string>& strct_id_to_strct_name_map,
    std::map<int,std::pair<int,int> >& strct_id_to_lag_idx_range_map,
    const int level_number,
    const double /*init_data_time*/,
    const bool /*can_be_refined*/,
    const bool /*initial_time*/,
    LDataManager* const /*l_data_manager*/)
{
    int offset = 0;
    for (int j = 0; j < static_cast<int>(d_base_filename[level_number].size()); ++j)
    {
        strct_id_to_strct_name_map   [j] = d_base_filename[level_number][j];
        strct_id_to_lag_idx_range_map[j] = std::make_pair(offset,offset+d_num_vertex[level_number][j]);
        offset += d_num_vertex[level_number][j];
    }
    return;
}// initializeStructureIndexingOnPatchLevel

unsigned int
msvIBInitializer::initializeDataOnPatchLevel(
    const int lag_node_index_idx,
    const unsigned int global_index_offset,
    const unsigned int local_index_offset,
    Pointer<LData> X_data,
    Pointer<LData> U_data,
    const Pointer<PatchHierarchy<NDIM> > hierarchy,
    const int level_number,
    const double /*init_data_time*/,
    const bool can_be_refined,
    const bool /*initial_time*/,
    LDataManager* const /*l_data_manager*/)
{
    // Determine the extents of the physical domain.
    Pointer<CartesianGridGeometry<NDIM> > grid_geom = hierarchy->getGridGeometry();
    const double* const XLower = grid_geom->getXLower();
    const double* const XUpper = grid_geom->getXUpper();

    // Set the global index offset.  This is equal to the number of Lagrangian
    // indices that have already been initialized on the specified level.
    d_global_index_offset[level_number] = global_index_offset;

    // Loop over all patches in the specified level of the patch level and
    // initialize the local vertices.
    blitz::Array<double,2>& X_array = *X_data->getLocalFormVecArray();
    blitz::Array<double,2>& U_array = *U_data->getLocalFormVecArray();
    int local_idx = -1;
    int local_node_count = 0;
    Pointer<PatchLevel<NDIM> > level = hierarchy->getPatchLevel(level_number);
    for (PatchLevel<NDIM>::Iterator p(level); p; p++)
    {
        Pointer<Patch<NDIM> > patch = level->getPatch(p());
        const Pointer<CartesianPatchGeometry<NDIM> > patch_geom = patch->getPatchGeometry();
        const Box<NDIM>& patch_box = patch->getBox();
        const CellIndex<NDIM>& patch_lower = patch_box.lower();
        const CellIndex<NDIM>& patch_upper = patch_box.upper();
        const double* const xLower = patch_geom->getXLower();
        const double* const xUpper = patch_geom->getXUpper();
        const double* const dx = patch_geom->getDx();

        Pointer<LNodeSetData> index_data = patch->getPatchData(lag_node_index_idx);

        // Initialize the vertices whose initial locations will be within the
        // given patch.
        std::vector<std::pair<int,int> > patch_vertices;
        getPatchVertices(patch_vertices, patch, level_number, can_be_refined);
        local_node_count += patch_vertices.size();
        for (std::vector<std::pair<int,int> >::const_iterator it = patch_vertices.begin();
             it != patch_vertices.end(); ++it)
        {
            const std::pair<int,int>& point_idx = (*it);
            const int lagrangian_idx = getCanonicalLagrangianIndex(point_idx, level_number) + global_index_offset;
            const int local_petsc_idx = ++local_idx + local_index_offset;
            const int global_petsc_idx = local_petsc_idx+global_index_offset;

            // Get the coordinates of the present vertex.
            const blitz::TinyVector<double,NDIM> X = getVertexPosn(point_idx, level_number);

            // Initialize the location of the present vertex.
            for (int d = 0; d < NDIM; ++d)
            {
                X_array(local_petsc_idx,d) = X[d];

                if (X[d] <= XLower[d])
                {
                    TBOX_ERROR(d_object_name << "::initializeDataOnPatchLevel():\n"
                               << "  encountered node below lower physical boundary\n"
                               << "  please ensure that all nodes are within the computational domain."<< std::endl);
                }

                if (X[d] >= XUpper[d])
                {
                    TBOX_ERROR(d_object_name << "::initializeDataOnPatchLevel():\n"
                               << "  encountered node above upper physical boundary\n"
                               << "  please ensure that all nodes are within the computational domain."<< std::endl);
                }
            }

            // Get the index of the cell in which the present vertex is
            // initially located.
            const CellIndex<NDIM> idx = IndexUtilities::getCellIndex(X, xLower, xUpper, dx, patch_lower, patch_upper);

            // Initialize the specification objects associated with the present
            // vertex.
            std::vector<Pointer<Streamable> > specs = initializeSpecs(point_idx, global_index_offset, level_number);

            // Create or retrieve a pointer to the LNodeSet associated with the
            // current Cartesian grid cell.
            if (!index_data->isElement(idx))
            {
                index_data->appendItemPointer(idx, new LNodeSet());
            }
            LNodeSet* const node_set = index_data->getItem(idx);
            static const IntVector<NDIM> periodic_offset(0);
            static const blitz::TinyVector<double,NDIM> periodic_displacement(0.0);
            node_set->push_back(new LNode(lagrangian_idx, global_petsc_idx, local_petsc_idx, periodic_offset, periodic_displacement, specs));

            // Initialize the velocity of the present vertex.
            std::fill(&U_array(local_petsc_idx,0),&U_array(local_petsc_idx,0)+NDIM,0.0);
        }
    }
    X_data->restoreArrays();
    U_data->restoreArrays();

    d_level_is_initialized[level_number] = true;

    // If a Lagrangian Silo data writer is registered with the initializer,
    // setup the visualization data corresponding to the present level of the
    // locally refined grid.
    if (!d_silo_writer.isNull())
    {
        initializeLSiloDataWriter(level_number);
    }
    return local_node_count;
}// initializeDataOnPatchLevel

unsigned int
msvIBInitializer::initializeMassDataOnPatchLevel(
    const unsigned int /*global_index_offset*/,
    const unsigned int local_index_offset,
    Pointer<LData> M_data,
    Pointer<LData> K_data,
    const Pointer<PatchHierarchy<NDIM> > hierarchy,
    const int level_number,
    const double /*init_data_time*/,
    const bool can_be_refined,
    const bool /*initial_time*/,
    LDataManager* const /*l_data_manager*/)
{
    // Loop over all patches in the specified level of the patch level and
    // initialize the local vertices.
    blitz::Array<double,2>& M_array = *M_data->getLocalFormVecArray();
    blitz::Array<double,2>& K_array = *K_data->getLocalFormVecArray();
    int local_idx = -1;
    int local_node_count = 0;
    Pointer<PatchLevel<NDIM> > level = hierarchy->getPatchLevel(level_number);
    for (PatchLevel<NDIM>::Iterator p(level); p; p++)
    {
        Pointer<Patch<NDIM> > patch = level->getPatch(p());

        // Initialize the vertices whose initial locations will be within the
        // given patch.
        std::vector<std::pair<int,int> > patch_vertices;
        getPatchVertices(patch_vertices, patch, level_number, can_be_refined);
        local_node_count += patch_vertices.size();
        for (std::vector<std::pair<int,int> >::const_iterator it = patch_vertices.begin();
             it != patch_vertices.end(); ++it)
        {
            const std::pair<int,int>& point_idx = (*it);
            const int local_petsc_idx = ++local_idx + local_index_offset;

            // Initialize the mass and penalty stiffness coefficient
            // corresponding to the present vertex.
            const BdryMassSpec& spec = getVertexBdryMassSpec(point_idx, level_number);
            const double M = spec.bdry_mass;
            const double K = spec.stiffness;

            // Avoid division by zero at massless nodes.
            if (MathUtilities<double>::equalEps(M,0.0))
            {
                M_array(local_petsc_idx) = std::numeric_limits<double>::epsilon();
                K_array(local_petsc_idx) = 0.0;
            }
            else
            {
                M_array(local_petsc_idx) = M;
                K_array(local_petsc_idx) = K;
            }
        }
    }
    M_data->restoreArrays();
    K_data->restoreArrays();
    return local_node_count;
}// initializeMassOnPatchLevel

unsigned int
msvIBInitializer::initializeDirectorDataOnPatchLevel(
    const unsigned int /*global_index_offset*/,
    const unsigned int local_index_offset,
    Pointer<LData> D_data,
    const Pointer<PatchHierarchy<NDIM> > hierarchy,
    const int level_number,
    const double /*init_data_time*/,
    const bool can_be_refined,
    const bool /*initial_time*/,
    LDataManager* const /*l_data_manager*/)
{
    // Loop over all patches in the specified level of the patch level and
    // initialize the local vertices.
    blitz::Array<double,2>& D_array = *D_data->getLocalFormVecArray();
    int local_idx = -1;
    int local_node_count = 0;
    Pointer<PatchLevel<NDIM> > level = hierarchy->getPatchLevel(level_number);
    for (PatchLevel<NDIM>::Iterator p(level); p; p++)
    {
        Pointer<Patch<NDIM> > patch = level->getPatch(p());

        // Initialize the vertices whose initial locations will be within the
        // given patch.
        std::vector<std::pair<int,int> > patch_vertices;
        getPatchVertices(patch_vertices, patch, level_number, can_be_refined);
        local_node_count += patch_vertices.size();
        for (std::vector<std::pair<int,int> >::const_iterator it = patch_vertices.begin();
             it != patch_vertices.end(); ++it)
        {
            const std::pair<int,int>& point_idx = (*it);
            const int local_petsc_idx = ++local_idx + local_index_offset;

            // Initialize the director corresponding to the present vertex.
            const std::vector<double>& D = getVertexDirectors(point_idx, level_number);
            for (int d = 0; d < 3*3; ++d)
            {
                D_array(local_petsc_idx,d) = D[d];
            }
        }
    }
    D_data->restoreArrays();
    return local_node_count;
}// initializeDirectorOnPatchLevel

void
msvIBInitializer::tagCellsForInitialRefinement(
    const Pointer<PatchHierarchy<NDIM> > hierarchy,
    const int level_number,
    const double /*error_data_time*/,
    const int tag_index)
{
    // Loop over all patches in the specified level of the patch level and tag
    // cells for refinement wherever there are vertices assigned to a finer
    // level of the Cartesian grid.
    Pointer<PatchLevel<NDIM> > level = hierarchy->getPatchLevel(level_number);
    for (PatchLevel<NDIM>::Iterator p(level); p; p++)
    {
        Pointer<Patch<NDIM> > patch = level->getPatch(p());
        const Pointer<CartesianPatchGeometry<NDIM> > patch_geom = patch->getPatchGeometry();
        const Box<NDIM>& patch_box = patch->getBox();
        const CellIndex<NDIM>& patch_lower = patch_box.lower();
        const CellIndex<NDIM>& patch_upper = patch_box.upper();
        const double* const xLower = patch_geom->getXLower();
        const double* const xUpper = patch_geom->getXUpper();
        const double* const dx = patch_geom->getDx();

        Pointer<CellData<NDIM,int> > tag_data = patch->getPatchData(tag_index);

        // Tag cells for refinement whenever there are vertices whose initial
        // locations will be within the index space of the given patch, but on
        // the finer levels of the AMR patch hierarchy.
        const bool can_be_refined = level_number+2 < d_max_levels;
        for (int ln = level_number+1; ln < d_max_levels; ++ln)
        {
            std::vector<std::pair<int,int> > patch_vertices;
            getPatchVertices(patch_vertices, patch, ln, can_be_refined);
            for (std::vector<std::pair<int,int> >::const_iterator it = patch_vertices.begin();
                 it != patch_vertices.end(); ++it)
            {
                const std::pair<int,int>& point_idx = (*it);

                // Get the coordinates of the present vertex.
                const blitz::TinyVector<double,NDIM> X = getVertexPosn(point_idx, ln);

                // Get the index of the cell in which the present vertex is
                // initially located.
                const CellIndex<NDIM> i = IndexUtilities::getCellIndex(X, xLower, xUpper, dx, patch_lower, patch_upper);

                // Tag the cell for refinement.
                if (patch_box.contains(i)) (*tag_data)(i) = 1;
            }
        }
    }
    return;
}// tagCellsForInitialRefinement

/////////////////////////////// PROTECTED ////////////////////////////////////

/////////////////////////////// PRIVATE //////////////////////////////////////

void
msvIBInitializer::initializeLSiloDataWriter(
    const int level_number)
{
#ifdef DEBUG_CHECK_ASSERTIONS
    TBOX_ASSERT(level_number >= 0);
    TBOX_ASSERT(level_number < d_max_levels);
    TBOX_ASSERT(d_level_is_initialized[level_number]);
#endif

    // WARNING: This code does not work if the global node offset is nonzero on
    // any of the levels of the locally refined Cartesian grid.
    if (d_global_index_offset[level_number] != 0)
    {
        TBOX_ERROR("This is broken --- please submit a bug report if you encounter this error.\n");
    }

    // WARNING: For now, we just register the visualization data on MPI process
    // 0.  This will fail if the structure is too large to be stored in the
    // memory available to a single MPI process.
    if (SAMRAI_MPI::getRank() == 0)
    {
        for (unsigned int j = 0; j < d_num_vertex[level_number].size(); ++j)
        {
            const std::string postfix = "_vertices";
            d_silo_writer->registerMarkerCloud(
                d_base_filename[level_number][j] + postfix,
                d_num_vertex[level_number][j], d_vertex_offset[level_number][j], level_number);
        }

        bool registered_spring_edge_map = false;
        for (unsigned int j = 0; j < d_num_vertex[level_number].size(); ++j)
        {
            if (d_spring_edge_map[level_number][j].size() > 0)
            {
                registered_spring_edge_map = true;
                const std::string postfix = "_mesh";
                d_silo_writer->registerUnstructuredMesh(
                    d_base_filename[level_number][j] + postfix,
                    d_spring_edge_map[level_number][j], level_number);
            }
        }

        for (unsigned int j = 0; j < d_num_vertex[level_number].size(); ++j)
        {
            if (d_rod_edge_map[level_number][j].size() > 0)
            {
                const std::string postfix = registered_spring_edge_map ? "_rod_mesh" : "_mesh";
                d_silo_writer->registerUnstructuredMesh(
                    d_base_filename[level_number][j] + postfix,
                    d_rod_edge_map[level_number][j], level_number);
            }
        }
    }
    return;
}// initializeLSiloDataWriter

void 
msvIBInitializer::AddPolyDataSet(int ln, vtkDataSet* data_set)
{
  d_data_sets[ln].push_back(data_set);
}

void
msvIBInitializer::readVertexDataset()
{
    std::string line_string;
    const int rank = SAMRAI_MPI::getRank();
    const int nodes = SAMRAI_MPI::getNodes();
    int flag = 1;
    int sz = 1;

    for (int ln = 0; ln < d_max_levels; ++ln)
    {
        const unsigned int num_datasets = d_data_sets[ln].size();
        d_num_vertex[ln].resize(num_datasets,std::numeric_limits<int>::max());
        d_vertex_offset[ln].resize(num_datasets,std::numeric_limits<int>::max());
        d_vertex_posn[ln].resize(num_datasets);
        for (unsigned int j = 0; j < num_datasets; ++j)
        {
            // Wait for the previous MPI process to finish reading the current dataset.
            if (d_use_file_batons && rank != 0) SAMRAI_MPI::recv(&flag, sz, rank-1, false, j);

            if (j == 0)
            {
                d_vertex_offset[ln][j] = 0;
            }
            else
            {
                d_vertex_offset[ln][j] = d_vertex_offset[ln][j-1]+d_num_vertex[ln][j-1];
            }

            d_num_vertex[ln][j] = d_data_sets[ln][j]->GetNumberOfPoints();
            const std::string dataset_name = d_base_filename[ln][j];
            if (d_num_vertex[ln][j] <= 0)
            {
              TBOX_ERROR(d_object_name << ":\n  Dataset contains no data " << dataset_name << std::endl);
            }

            // Copy initial position of each vertex from 
            // the corresponding dataset.
            d_vertex_posn[ln][j].resize(d_num_vertex[ln][j]);
            for (int k = 0; k < d_num_vertex[ln][j]; ++k)
            {
              double *x = &d_vertex_posn[ln][j][k][0];
              d_data_sets[ln][j]->GetPoint(k,x);
              d_vertex_posn[ln][j][k] *= d_length_scale_factor;
              d_vertex_posn[ln][j][k] += d_posn_shift;
            }

            plog << d_object_name << ":  "
            << "read " << d_num_vertex[ln][j] << " vertices from dataset named " << dataset_name << std::endl
                 << "  on MPI process " << SAMRAI_MPI::getRank() << std::endl;

            // Free the next MPI process to start reading the current dataset.
            if (d_use_file_batons && rank != nodes-1) SAMRAI_MPI::send(&flag, sz, rank+1, false, j);
        }
    }

    // Synchronize the processes.
    if (d_use_file_batons) SAMRAI_MPI::barrier();
    return;
}// readVertexDataset


void
msvIBInitializer::readSpringDataset()
{
    std::string line_string;
    const int rank = SAMRAI_MPI::getRank();
    const int nodes = SAMRAI_MPI::getNodes();
    int flag = 1;
    int sz = 1;

    for (int ln = 0; ln < d_max_levels; ++ln)
    {
      const unsigned int num_datasets = d_data_sets[ln].size();
      d_spring_edge_map[ln].resize(num_datasets);
      d_spring_spec_data[ln].resize(num_datasets);
      for (unsigned int j = 0; j < num_datasets; ++j)
      {
            bool warned = false;

            // Wait for the previous MPI process to finish reading the current dataset.
            if (d_use_file_batons && rank != 0) SAMRAI_MPI::recv(&flag, sz, rank-1, false, j);

            vtkSmartPointer<vtkExtractEdges> edge_filter = vtkSmartPointer<vtkExtractEdges>::New();
            edge_filter->SetInput(d_data_sets[ln][j]);
            edge_filter->Update();
            
            vtkSmartPointer<vtkPolyData> edges = edge_filter->GetOutput();
            
            int num_edges = edges->GetNumberOfCells();
            
            for (int k = 0; k < num_edges; ++k)
            {
              Edge e;              
              double kappa, length;
              int force_fcn_idx;
#if ENABLE_SUBDOMAIN_INDICES
              int subdomain_idx;
#endif
              vtkCell *edge = edges->GetCell(k);
              e.first = edge->GetPointId(0);
              e.second = edge->GetPointId(1);
              
              if ((e.first < 0) || (e.first >= d_num_vertex[ln][j]))
              {
                TBOX_ERROR(d_object_name << ":\n  Invalid entry in dataset encountered " << std::endl
                << "  vertex index " << e.first << " is out of range" << std::endl);
              }
              
              if ((e.second < 0) || (e.second >= d_num_vertex[ln][j]))
              {
                TBOX_ERROR(d_object_name << ":\n  Invalid entry in dataset encountered " << std::endl
                << "  vertex index " << e.second << " is out of range" << std::endl);
              }
              
              kappa = .25;
              
              if (kappa < 0.0)
              {
                TBOX_ERROR(d_object_name << ":\n  Invalid entry in dataset encountered " <<  std::endl
                << "  spring constant is negative" << std::endl);
              }
              
              length = std::sqrt(edge->GetLength2());
              
              if (length < 0.0)
              {
                TBOX_ERROR(d_object_name << ":\n  Invalid entry in input file encountered on line " << std::endl
                << "  spring resting length is negative" << std::endl);
              }
                            
              length *= d_length_scale_factor;
              
              force_fcn_idx = 0;  // default force function specification.
              
#if ENABLE_SUBDOMAIN_INDICES
              subdomain_idx = -1;  // default subdomain index.
#endif

              // Modify kappa, length, and subdomain_idx according to
              // whether uniform values are to be employed for this
              // particular structure.
              if (d_using_uniform_spring_stiffness[ln][j])
              {
                kappa = d_uniform_spring_stiffness[ln][j];
              }
              if (d_using_uniform_spring_rest_length[ln][j])
              {
                length = d_uniform_spring_rest_length[ln][j];
              }
              if (d_using_uniform_spring_force_fcn_idx[ln][j])
              {
                force_fcn_idx = d_uniform_spring_force_fcn_idx[ln][j];
              }
              
#if ENABLE_SUBDOMAIN_INDICES
              if (d_using_uniform_spring_subdomain_idx[ln][j])
              {
                subdomain_idx = d_uniform_spring_subdomain_idx[ln][j];
              }
#endif

              // Correct the edge numbers to be in the global Lagrangian indexing
              // scheme.
              e.first  += d_vertex_offset[ln][j];
              e.second += d_vertex_offset[ln][j];

              // Always place the lower index first.
              if (e.first > e.second)
              {
                std::swap<int>(e.first, e.second);
              }
              
              // Check to see if the edge has already been inserted in the edge map.
              bool duplicate_edge = false;
#ifdef DEBUG_CHECK_ASSERTIONS
              for (std::multimap<int,Edge>::const_iterator it = d_spring_edge_map[ln][j].lower_bound(e.first);
                   it != d_spring_edge_map[ln][j].upper_bound(e.first); ++it)
                   {
                     const Edge& other_e = it->second;
                     if (e.first  == other_e.first &&
                       e.second == other_e.second)
                     {
                       // This is a duplicate edge and should not be inserted into the
                       // edge map.
                       duplicate_edge = true;
                       
                       // Ensure that the link information is consistent.
                       if (!MathUtilities<double>::equalEps(d_spring_spec_data[ln][j].find(e)->second.stiffness  , kappa ) ||
                         !MathUtilities<double>::equalEps(d_spring_spec_data[ln][j].find(e)->second.rest_length, length) ||
                         (d_spring_spec_data[ln][j].find(e)->second.force_fcn_idx != force_fcn_idx)
#if ENABLE_SUBDOMAIN_INDICES
                         || (d_spring_spec_data[ln][j].find(e)->second.subdomain_idx != subdomain_idx)
#endif
                       )
                       {
                         TBOX_ERROR(d_object_name << ":\n  Inconsistent duplicate edges in input file encountered on line " << k+2 << " of dataset " << d_data_sets[ln][j] << std::endl
                         << "  first vertex = " << e.first-d_vertex_offset[ln][j] << " second vertex = " << e.second-d_vertex_offset[ln][j] << std::endl
                         << "  original spring constant      = " << d_spring_spec_data[ln][j].find(e)->second.stiffness     << std::endl
                         << "  original resting length       = " << d_spring_spec_data[ln][j].find(e)->second.rest_length   << std::endl
                         << "  original force function index = " << d_spring_spec_data[ln][j].find(e)->second.force_fcn_idx << std::endl);
                       }
                     }
                   }
#endif
                   // Initialize the map data corresponding to the present edge.
                   //
                   // Note that in the edge map, each edge is associated with only the
                   // first vertex.
                   if (!duplicate_edge)
                   {
                     d_spring_edge_map[ln][j].insert(std::make_pair(e.first,e));
                     SpringSpec& spec_data = d_spring_spec_data[ln][j][e];
                     spec_data.stiffness     = kappa;
                     spec_data.rest_length   = length;
                     spec_data.force_fcn_idx = force_fcn_idx;
#if ENABLE_SUBDOMAIN_INDICES
                     spec_data.subdomain_idx = subdomain_idx;
#endif
                   }
                   
                   // Check to see if the spring constant is zero and, if so,
                   // emit a warning.
                   if (!warned && d_enable_springs[ln][j] &&
                     (kappa == 0.0 || MathUtilities<double>::equalEps(kappa,0.0)))
                   {
                     TBOX_WARNING(d_object_name << ":\n  Spring with zero spring constant encountered in ASCII input file named " << std::endl);
                     warned = true;
                   }

                  plog << d_object_name << ":  "
                     << "read " << num_edges << " edges from ASCII input file named " << std::endl
                     << "  on MPI process " << SAMRAI_MPI::getRank() << std::endl;
            }
            
            // Free the next MPI process to start reading the current file.
            if (d_use_file_batons && rank != nodes-1) SAMRAI_MPI::send(&flag, sz, rank+1, false, j);
        }
    }

    // Synchronize the processes.
    if (d_use_file_batons) SAMRAI_MPI::barrier();
    return;
}// readSpringFiles

void
msvIBInitializer::readSourceDatasets()
{
    std::string line_string;
    const int rank = SAMRAI_MPI::getRank();
    const int nodes = SAMRAI_MPI::getNodes();
    int flag = 1;
    int sz = 1;
    for (int ln = 0; ln < d_max_levels; ++ln)
    {
        const unsigned int num_base_filename = d_data_sets[ln].size();
        for (unsigned int j = 0; j < num_base_filename; ++j)
        {
	    msvVTKIBSourceGen::setDataSet(ln,d_data_sets[ln][j].GetPointer());
	    vtkNew<msvVTKBoundaryEdgeSources> sourceDataset;
	    sourceDataset->SetInput(d_data_sets[ln][j]);
	    sourceDataset->Update();

	    unsigned int num_sources = sourceDataset->GetOutput()->GetNumberOfPoints();

	    msvVTKIBSourceGen::setNumSources(ln,num_sources);
        }
    }
    return;
}// readSourceFiles

void
msvIBInitializer::getPatchVertices(
    std::vector<std::pair<int,int> >& patch_vertices,
    const Pointer<Patch<NDIM> > patch,
    const int level_number,
    const bool /*can_be_refined*/) const
{
    // Loop over all of the vertices to determine the indices of those vertices
    // within the present patch.
    //
    // NOTE: This is clearly not the best way to do this, but it will work for
    // now.
    const Pointer<CartesianPatchGeometry<NDIM> > patch_geom = patch->getPatchGeometry();
    const double* const xLower = patch_geom->getXLower();
    const double* const xUpper = patch_geom->getXUpper();

    for (unsigned int j = 0; j < d_num_vertex[level_number].size(); ++j)
    {
        for (int k = 0; k < d_num_vertex[level_number][j]; ++k)
        {
            const blitz::TinyVector<double,NDIM>& X = d_vertex_posn[level_number][j][k];
            const bool patch_owns_node =
                ((  xLower[0] <= X[0])&&(X[0] < xUpper[0]))
#if (NDIM > 1)
                &&((xLower[1] <= X[1])&&(X[1] < xUpper[1]))
#if (NDIM > 2)
                &&((xLower[2] <= X[2])&&(X[2] < xUpper[2]))
#endif
#endif
                ;
            if (patch_owns_node) patch_vertices.push_back(std::make_pair(j,k));
        }
    }
    return;
}// getPatchVertices

int
msvIBInitializer::getCanonicalLagrangianIndex(
    const std::pair<int,int>& point_index,
    const int level_number) const
{
    return d_vertex_offset[level_number][point_index.first]+point_index.second;
}// getCanonicalLagrangianIndex

blitz::TinyVector<double,NDIM>
msvIBInitializer::getVertexPosn(
    const std::pair<int,int>& point_index,
    const int level_number) const
{
    return d_vertex_posn[level_number][point_index.first][point_index.second];
}// getVertexPosn

const msvIBInitializer::TargetSpec&
msvIBInitializer::getVertexTargetSpec(
    const std::pair<int,int>& point_index,
    const int level_number) const
{
    return d_target_spec_data[level_number][point_index.first][point_index.second];
}// getVertexTargetSpec

const msvIBInitializer::AnchorSpec&
msvIBInitializer::getVertexAnchorSpec(
    const std::pair<int,int>& point_index,
    const int level_number) const
{
    return d_anchor_spec_data[level_number][point_index.first][point_index.second];
}// getVertexAnchorSpec

const msvIBInitializer::BdryMassSpec&
msvIBInitializer::getVertexBdryMassSpec(
    const std::pair<int,int>& point_index,
    const int level_number) const
{
    return d_bdry_mass_spec_data[level_number][point_index.first][point_index.second];
}// getVertexBdryMassSpec

const std::vector<double>&
msvIBInitializer::getVertexDirectors(
    const std::pair<int,int>& point_index,
    const int level_number) const
{
    return d_directors[level_number][point_index.first][point_index.second];
}// getVertexDirectors

std::pair<int,int>
msvIBInitializer::getVertexInstrumentationIndices(
    const std::pair<int,int>& point_index,
    const int level_number) const
{
    std::map<int,std::pair<int,int> >::const_iterator it = d_instrument_idx[level_number][point_index.first].find(point_index.second);
    if (it != d_instrument_idx[level_number][point_index.first].end())
    {
        return it->second;
    }
    else
    {
        return std::make_pair(-1,-1);
    }
}// getVertexInstrumentationIndices

int
msvIBInitializer::getVertexSourceIndices(
    const std::pair<int,int>& point_index,
    const int level_number) const
{
    std::map<int,int>::const_iterator it = d_source_idx[level_number][point_index.first].find(point_index.second);
    if (it != d_source_idx[level_number][point_index.first].end())
    {
        return it->second;
    }
    else
    {
        return -1;
    }
}// getVertexSourceIndices

std::vector<Pointer<Streamable> >
msvIBInitializer::initializeSpecs(
    const std::pair<int,int>& point_index,
    const unsigned int global_index_offset,
    const int level_number) const
{
    std::vector<Pointer<Streamable> > vertex_specs;

    const int j = point_index.first;
    const int mastr_idx = getCanonicalLagrangianIndex(point_index, level_number);

    // Initialize any spring specifications associated with the present vertex.
    if (d_enable_springs[level_number][j])
    {
        std::vector<int> slave_idxs, force_fcn_idxs;
        std::vector<double> stiffness, rest_length;
#if ENABLE_SUBDOMAIN_INDICES
        std::vector<int> subdomain_idxs;
#endif
        for (std::multimap<int,Edge>::const_iterator it = d_spring_edge_map[level_number][j].lower_bound(mastr_idx);
             it != d_spring_edge_map[level_number][j].upper_bound(mastr_idx); ++it)
        {
#ifdef DEBUG_CHECK_ASSERTIONS
            TBOX_ASSERT(mastr_idx == it->first);
#endif
            // The connectivity information.
            const Edge& e = it->second;
            if (e.first == mastr_idx)
            {
                slave_idxs.push_back(e.second+global_index_offset);
            }
            else
            {
                slave_idxs.push_back(e.first +global_index_offset);
            }

            // The material properties.
            const SpringSpec& spec_data = d_spring_spec_data[level_number][j].find(e)->second;
            stiffness     .push_back(spec_data.stiffness    );
            rest_length   .push_back(spec_data.rest_length  );
            force_fcn_idxs.push_back(spec_data.force_fcn_idx);
#if ENABLE_SUBDOMAIN_INDICES
            subdomain_idxs.push_back(spec_data.subdomain_idx);
#endif
        }
        if (slave_idxs.size() > 0)
        {
            vertex_specs.push_back(new IBSpringForceSpec(mastr_idx, slave_idxs, force_fcn_idxs, stiffness, rest_length
#if ENABLE_SUBDOMAIN_INDICES
                                                         , subdomain_idxs
#endif
                                                         ));
        }
    }

    // Initialize any beam specifications associated with the present vertex.
    if (d_enable_beams[level_number][j])
    {
        std::vector<std::pair<int,int> > beam_neighbor_idxs;
        std::vector<double> beam_bend_rigidity;
        std::vector<blitz::TinyVector<double,NDIM> > beam_mesh_dependent_curvature;
#if ENABLE_SUBDOMAIN_INDICES
        std::vector<int> beam_subdomain_idxs;
#endif
        for (std::multimap<int,BeamSpec>::const_iterator it = d_beam_spec_data[level_number][j].lower_bound(mastr_idx);
             it != d_beam_spec_data[level_number][j].upper_bound(mastr_idx); ++it)
        {
            const BeamSpec& spec_data = it->second;
            beam_neighbor_idxs.push_back(spec_data.neighbor_idxs);
            beam_bend_rigidity.push_back(spec_data.bend_rigidity);
            beam_mesh_dependent_curvature.push_back(spec_data.curvature);
#if ENABLE_SUBDOMAIN_INDICES
            beam_subdomain_idxs.push_back(spec_data.subdomain_idx);
#endif
        }
        if (!beam_neighbor_idxs.empty())
        {
            vertex_specs.push_back(new IBBeamForceSpec(mastr_idx, beam_neighbor_idxs, beam_bend_rigidity, beam_mesh_dependent_curvature
#if ENABLE_SUBDOMAIN_INDICES
                                                       , beam_subdomain_idxs
#endif
                                                       ));
        }
    }

    // Initialize any rod specifications associated with the present vertex.
    if (d_enable_rods[level_number][j])
    {
        std::vector<int> rod_next_idxs;
        std::vector<blitz::TinyVector<double,IBRodForceSpec::NUM_MATERIAL_PARAMS> > rod_material_params;
#if ENABLE_SUBDOMAIN_INDICES
        std::vector<int> rod_subdomain_idxs;
#endif
        for (std::multimap<int,Edge>::const_iterator it = d_rod_edge_map[level_number][j].lower_bound(mastr_idx);
             it != d_rod_edge_map[level_number][j].upper_bound(mastr_idx); ++it)
        {
#ifdef DEBUG_CHECK_ASSERTIONS
            TBOX_ASSERT(mastr_idx == it->first);
#endif
            // The connectivity information.
            const Edge& e = it->second;
            if (e.first == mastr_idx)
            {
                rod_next_idxs.push_back(e.second+global_index_offset);
            }
            else
            {
                rod_next_idxs.push_back(e.first +global_index_offset);
            }

            // The material properties.
            const RodSpec& spec_data = d_rod_spec_data[level_number][j].find(e)->second;
            rod_material_params.push_back(spec_data.properties);
#if ENABLE_SUBDOMAIN_INDICES
            rod_subdomain_idxs.push_back(spec_data.subdomain_idx);
#endif
        }
        if (!rod_next_idxs.empty())
        {
            vertex_specs.push_back(new IBRodForceSpec(mastr_idx, rod_next_idxs, rod_material_params
#if ENABLE_SUBDOMAIN_INDICES
                                                      , rod_subdomain_idxs
#endif
                                                      ));
        }
    }

    // Initialize any target point specifications associated with the present
    // vertex.
    if (d_enable_target_points[level_number][j])
    {
        const TargetSpec& spec_data = getVertexTargetSpec(point_index, level_number);
        const double kappa_target = spec_data.stiffness;
        const double eta_target = spec_data.damping;
#if ENABLE_SUBDOMAIN_INDICES
        const int subdomain_idx = spec_data.subdomain_idx;
#endif
        const blitz::TinyVector<double,NDIM> X_target = getVertexPosn(point_index, level_number);
        vertex_specs.push_back(new IBTargetPointForceSpec(mastr_idx, kappa_target, eta_target, X_target
#if ENABLE_SUBDOMAIN_INDICES
                                                          , subdomain_idx
#endif
                                                          ));
    }

    // Initialize any anchor point specifications associated with the present
    // vertex.
    if (d_enable_anchor_points[level_number][j])
    {
        const AnchorSpec& spec_data = getVertexAnchorSpec(point_index, level_number);
        const bool is_anchor_point = spec_data.is_anchor_point;
#if ENABLE_SUBDOMAIN_INDICES
        const int subdomain_idx = spec_data.subdomain_idx;
#endif
        if (is_anchor_point)
        {
            vertex_specs.push_back(new IBAnchorPointSpec(mastr_idx
#if ENABLE_SUBDOMAIN_INDICES
                                                         , subdomain_idx
#endif
                                                         ));
        }
    }

    // Initialize any instrumentation specifications associated with the present
    // vertex.
    if (d_enable_instrumentation[level_number][j])
    {
        const std::pair<int,int> inst_idx = getVertexInstrumentationIndices(point_index, level_number);
        if (inst_idx.first != -1 && inst_idx.second != -1)
        {
            vertex_specs.push_back(new IBInstrumentationSpec(mastr_idx, inst_idx.first, inst_idx.second));
        }
    }

    // Initialize any source specifications associated with the present
    // vertex.
    if (d_enable_sources[level_number][j])
    {
        const int source_idx = getVertexSourceIndices(point_index, level_number);
        if (source_idx != -1)
        {
            vertex_specs.push_back(new IBSourceSpec(mastr_idx, source_idx));
        }
    }
    return vertex_specs;
}// initializeSpecs

void
msvIBInitializer::getFromInput(
    Pointer<Database> db,
    vtkDataSet *data)
{
#ifdef DEBUG_CHECK_ASSERTIONS
    TBOX_ASSERT(!db.isNull());
#endif

    // Determine whether to use "batons" to prevent multiple MPI processes from
    // reading the same file at once.
    d_use_file_batons = db->getBoolWithDefault("use_file_batons",d_use_file_batons);

    // Determine the (maximum) number of levels in the locally refined grid.
    // Note that each piece of the Lagrangian structure must be assigned to a
    // particular level of the grid.
    if (db->keyExists("max_levels"))
    {
        d_max_levels = db->getInteger("max_levels");
    }
    else
    {
        TBOX_ERROR(d_object_name << ":  "
                   << "Key data `max_levels' not found in input.");
    }

    if (d_max_levels < 1)
    {
        TBOX_ERROR(d_object_name << ":  "
                   << "Key data `max_levels' found in input is < 1.");
    }

    // Resize the vectors that are indexed by the level number.
    d_level_is_initialized.resize(d_max_levels,false);

    d_base_filename.resize(d_max_levels);
    d_data_sets.resize(d_max_levels);

    d_num_vertex.resize(d_max_levels);
    d_vertex_offset.resize(d_max_levels);
    d_vertex_posn.resize(d_max_levels);

    d_enable_springs.resize(d_max_levels);
    d_spring_edge_map.resize(d_max_levels);
    d_spring_spec_data.resize(d_max_levels);
    d_using_uniform_spring_stiffness.resize(d_max_levels);
    d_uniform_spring_stiffness.resize(d_max_levels);
    d_using_uniform_spring_rest_length.resize(d_max_levels);
    d_uniform_spring_rest_length.resize(d_max_levels);
    d_using_uniform_spring_force_fcn_idx.resize(d_max_levels);
    d_uniform_spring_force_fcn_idx.resize(d_max_levels);
#if ENABLE_SUBDOMAIN_INDICES
    d_using_uniform_spring_subdomain_idx.resize(d_max_levels);
    d_uniform_spring_subdomain_idx.resize(d_max_levels);
#endif

    d_enable_beams.resize(d_max_levels);
    d_beam_spec_data.resize(d_max_levels);
    d_using_uniform_beam_bend_rigidity.resize(d_max_levels);
    d_uniform_beam_bend_rigidity.resize(d_max_levels);
    d_using_uniform_beam_curvature.resize(d_max_levels);
    d_uniform_beam_curvature.resize(d_max_levels);
#if ENABLE_SUBDOMAIN_INDICES
    d_using_uniform_beam_subdomain_idx.resize(d_max_levels);
    d_uniform_beam_subdomain_idx.resize(d_max_levels);
#endif

    d_enable_rods.resize(d_max_levels);
    d_rod_edge_map.resize(d_max_levels);
    d_rod_spec_data.resize(d_max_levels);
    d_using_uniform_rod_properties.resize(d_max_levels);
    d_uniform_rod_properties.resize(d_max_levels);
#if ENABLE_SUBDOMAIN_INDICES
    d_using_uniform_rod_subdomain_idx.resize(d_max_levels);
    d_uniform_rod_subdomain_idx.resize(d_max_levels);
#endif

    d_enable_target_points.resize(d_max_levels);
    d_target_spec_data.resize(d_max_levels);
    d_using_uniform_target_stiffness.resize(d_max_levels);
    d_uniform_target_stiffness.resize(d_max_levels);
    d_using_uniform_target_damping.resize(d_max_levels);
    d_uniform_target_damping.resize(d_max_levels);
#if ENABLE_SUBDOMAIN_INDICES
    d_using_uniform_target_subdomain_idx.resize(d_max_levels);
    d_uniform_target_subdomain_idx.resize(d_max_levels);
#endif

    d_enable_anchor_points.resize(d_max_levels);
    d_anchor_spec_data.resize(d_max_levels);
#if ENABLE_SUBDOMAIN_INDICES
    d_using_uniform_anchor_subdomain_idx.resize(d_max_levels);
    d_uniform_anchor_subdomain_idx.resize(d_max_levels);
#endif

    d_enable_bdry_mass.resize(d_max_levels);
    d_bdry_mass_spec_data.resize(d_max_levels);
    d_using_uniform_bdry_mass.resize(d_max_levels);
    d_uniform_bdry_mass.resize(d_max_levels);
    d_using_uniform_bdry_mass_stiffness.resize(d_max_levels);
    d_uniform_bdry_mass_stiffness.resize(d_max_levels);

    d_directors.resize(d_max_levels);

    d_enable_instrumentation.resize(d_max_levels);
    d_instrument_idx.resize(d_max_levels);

    d_enable_sources.resize(d_max_levels);
    d_source_idx.resize(d_max_levels);

    d_global_index_offset.resize(d_max_levels);

    // Determine the various input file names.
    //
    // Prefer to use the new ``structure_names'' key, but revert to the
    // level-by-level ``base_filenames'' keys if necessary.
    if (db->keyExists("structure_names"))
    {
        const int n_strcts = db->getArraySize("structure_names");
        std::vector<std::string> structure_names(n_strcts);
        db->getStringArray("structure_names", &structure_names[0], n_strcts);
        for (int n = 0; n < n_strcts; ++n)
        {
            const std::string& strct_name = structure_names[n];
            if (db->keyExists(strct_name))
            {
                Pointer<Database> sub_db = db->getDatabase(strct_name);
                if (sub_db->keyExists("level_number"))
                {
                    const int ln = sub_db->getInteger("level_number");                    
                    if (ln < 0)
                    {
                        TBOX_ERROR(d_object_name << ":  "
                                   << "Key data `level_number' associated with structure `" << strct_name << "' is negative.");
                    }
                    else if (ln > d_max_levels)
                    {
                        TBOX_ERROR(d_object_name << ":  "
                                   << "Key data `level_number' associated with structure `" << strct_name << "' is greater than the expected maximum level number " << d_max_levels << ".");
                    }                    
                    d_data_sets[ln].push_back(data);
                    d_base_filename[ln].push_back(strct_name);
                }
                else
                {
                    TBOX_ERROR(d_object_name << ":  "
                               << "Key data `level_number' not found in structure `" << strct_name << "' input.");
                }
            }
            else
            {
                TBOX_ERROR(d_object_name << ":  "
                           << "Key data `" << strct_name << "' not found in input.");
            }
        }
    }
    else
    {
        for (int ln = 0; ln < d_max_levels; ++ln)
        {
            std::ostringstream db_key_name_stream;
            db_key_name_stream << "base_filenames_" << ln;
            const std::string db_key_name = db_key_name_stream.str();
            if (db->keyExists(db_key_name))
            {
                const int n_files = db->getArraySize(db_key_name);
                d_base_filename[ln].resize(n_files);
                db->getStringArray(db_key_name, &d_base_filename[ln][0], n_files);
            }
            else
            {
                TBOX_WARNING(d_object_name << ":  "
                             << "Key data `" << db_key_name << "' not found in input.");
            }
        }
    }

    // Read in any shift and scale factors.
    if (db->keyExists("length_scale_factor"))
    {
        d_length_scale_factor = db->getDouble("length_scale_factor");
    }

    if (db->keyExists("posn_shift"))
    {
        db->getDoubleArray("posn_shift", &d_posn_shift[0], NDIM);
    }

    // Read in any sub-databases associated with the input file names.
    for (int ln = 0; ln < d_max_levels; ++ln)
    {
        const unsigned int num_base_filename = d_base_filename[ln].size();

        d_enable_springs[ln].resize(num_base_filename,true);
        d_using_uniform_spring_stiffness[ln].resize(num_base_filename,false);
        d_uniform_spring_stiffness[ln].resize(num_base_filename,-1.0);
        d_using_uniform_spring_rest_length[ln].resize(num_base_filename,false);
        d_uniform_spring_rest_length[ln].resize(num_base_filename,-1.0);
        d_using_uniform_spring_force_fcn_idx[ln].resize(num_base_filename,false);
        d_uniform_spring_force_fcn_idx[ln].resize(num_base_filename,-1);
#if ENABLE_SUBDOMAIN_INDICES
        d_using_uniform_spring_subdomain_idx[ln].resize(num_base_filename,false);
        d_uniform_spring_subdomain_idx[ln].resize(num_base_filename,-1);
#endif

        d_enable_beams[ln].resize(num_base_filename,true);
        d_using_uniform_beam_bend_rigidity[ln].resize(num_base_filename,false);
        d_uniform_beam_bend_rigidity[ln].resize(num_base_filename,-1.0);
        d_using_uniform_beam_curvature[ln].resize(num_base_filename,false);
        d_uniform_beam_curvature[ln].resize(num_base_filename,blitz::TinyVector<double,NDIM>(0.0));
#if ENABLE_SUBDOMAIN_INDICES
        d_using_uniform_beam_subdomain_idx[ln].resize(num_base_filename,false);
        d_uniform_beam_subdomain_idx[ln].resize(num_base_filename,-1);
#endif

        d_enable_rods[ln].resize(num_base_filename,true);
        d_using_uniform_rod_properties[ln].resize(num_base_filename,false);
        d_uniform_rod_properties[ln].resize(num_base_filename,blitz::TinyVector<double,IBRodForceSpec::NUM_MATERIAL_PARAMS>(0.0));
#if ENABLE_SUBDOMAIN_INDICES
        d_using_uniform_rod_subdomain_idx[ln].resize(num_base_filename,false);
        d_uniform_rod_subdomain_idx[ln].resize(num_base_filename,-1);
#endif

        d_enable_target_points[ln].resize(num_base_filename,true);
        d_using_uniform_target_stiffness[ln].resize(num_base_filename,false);
        d_uniform_target_stiffness[ln].resize(num_base_filename,-1.0);
        d_using_uniform_target_damping[ln].resize(num_base_filename,false);
        d_uniform_target_damping[ln].resize(num_base_filename,-1.0);
#if ENABLE_SUBDOMAIN_INDICES
        d_using_uniform_target_subdomain_idx[ln].resize(num_base_filename,false);
        d_uniform_target_subdomain_idx[ln].resize(num_base_filename,-1);
#endif

        d_enable_anchor_points[ln].resize(num_base_filename,true);
#if ENABLE_SUBDOMAIN_INDICES
        d_using_uniform_anchor_subdomain_idx[ln].resize(num_base_filename,false);
        d_uniform_anchor_subdomain_idx[ln].resize(num_base_filename,-1);
#endif

        d_enable_bdry_mass[ln].resize(num_base_filename,true);
        d_using_uniform_bdry_mass[ln].resize(num_base_filename,false);
        d_uniform_bdry_mass[ln].resize(num_base_filename,-1.0);
        d_using_uniform_bdry_mass_stiffness[ln].resize(num_base_filename,false);
        d_uniform_bdry_mass_stiffness[ln].resize(num_base_filename,-1.0);

        d_enable_instrumentation[ln].resize(num_base_filename,true);

        d_enable_sources[ln].resize(num_base_filename,true);

        for (unsigned int j = 0; j < num_base_filename; ++j)
        {
            const std::string& base_filename = d_base_filename[ln][j];
            if (db->isDatabase(base_filename))
            {
                Pointer<Database> sub_db = db->getDatabase(base_filename);

                // Determine whether to enable or disable any particular
                // features.
                if (sub_db->keyExists("enable_springs"))
                {
                    d_enable_springs[ln][j] = sub_db->getBool("enable_springs");
                }
                if (sub_db->keyExists("enable_beams"))
                {
                    d_enable_beams[ln][j] = sub_db->getBool("enable_beams");
                }
                if (sub_db->keyExists("enable_rods"))
                {
                    d_enable_rods[ln][j] = sub_db->getBool("enable_rods");
                }
                if (sub_db->keyExists("enable_target_points"))
                {
                    d_enable_target_points[ln][j] = sub_db->getBool("enable_target_points");
                }
                if (sub_db->keyExists("enable_anchor_points"))
                {
                    d_enable_anchor_points[ln][j] = sub_db->getBool("enable_anchor_points");
                }
                if (sub_db->keyExists("enable_bdry_mass"))
                {
                    d_enable_bdry_mass[ln][j] = sub_db->getBool("enable_bdry_mass");
                }
                if (sub_db->keyExists("enable_instrumentation"))
                {
                    d_enable_instrumentation[ln][j] = sub_db->getBool("enable_instrumentation");
                }
                if (sub_db->keyExists("enable_sources"))
                {
                    d_enable_sources[ln][j] = sub_db->getBool("enable_sources");
                }

                // Determine whether to use uniform values for any particular
                // structure attributes.
                if (sub_db->keyExists("uniform_spring_stiffness"))
                {
                    d_using_uniform_spring_stiffness[ln][j] = true;
                    d_uniform_spring_stiffness[ln][j] = sub_db->getDouble("uniform_spring_stiffness");
                    if (d_uniform_spring_stiffness[ln][j] < 0.0)
                    {
                        TBOX_ERROR(d_object_name << ":\n  Invalid entry for key `uniform_spring_stiffness' in database " << base_filename << std::endl
                                   << "  spring constant is negative" << std::endl);
                    }
                }
                if (sub_db->keyExists("uniform_spring_rest_length"))
                {
                    d_using_uniform_spring_rest_length[ln][j] = true;
                    d_uniform_spring_rest_length[ln][j] = sub_db->getDouble("uniform_spring_rest_length");
                    if (d_uniform_spring_rest_length[ln][j] < 0.0)
                    {
                        TBOX_ERROR(d_object_name << ":\n  Invalid entry for key `uniform_spring_rest_length' in database " << base_filename << std::endl
                                   << "  spring resting length is negative" << std::endl);
                    }
                }
                if (sub_db->keyExists("uniform_spring_force_fcn_idx"))
                {
                    d_using_uniform_spring_force_fcn_idx[ln][j] = true;
                    d_uniform_spring_force_fcn_idx[ln][j] = sub_db->getInteger("uniform_spring_force_fcn_idx");
                }
#if ENABLE_SUBDOMAIN_INDICES
                if (sub_db->keyExists("uniform_spring_subdomain_idx"))
                {
                    d_using_uniform_spring_subdomain_idx[ln][j] = true;
                    d_uniform_spring_subdomain_idx[ln][j] = sub_db->getInteger("uniform_spring_subdomain_idx");
                }
#endif

                if (sub_db->keyExists("uniform_beam_bend_rigidity"))
                {
                    d_using_uniform_beam_bend_rigidity[ln][j] = true;
                    d_uniform_beam_bend_rigidity[ln][j] = sub_db->getDouble("uniform_beam_bend_rigidity");
                    if (d_uniform_beam_bend_rigidity[ln][j] < 0.0)
                    {
                        TBOX_ERROR(d_object_name << ":\n  Invalid entry for key `uniform_beam_bend_rigidity' in database " << base_filename << std::endl
                                   << "  beam bending rigidity is negative" << std::endl);
                    }
                }
                if (sub_db->keyExists("uniform_beam_curvature"))
                {
                    d_using_uniform_beam_curvature[ln][j] = true;
                    sub_db->getDoubleArray("uniform_beam_curvature", d_uniform_beam_curvature[ln][j].data(), NDIM);
                }
#if ENABLE_SUBDOMAIN_INDICES
                if (sub_db->keyExists("uniform_beam_subdomain_idx"))
                {
                    d_using_uniform_beam_subdomain_idx[ln][j] = true;
                    d_uniform_beam_subdomain_idx[ln][j] = sub_db->getInteger("uniform_beam_subdomain_idx");
                }
#endif

                if (sub_db->keyExists("uniform_rod_properties"))
                {
                    d_using_uniform_rod_properties[ln][j] = true;
                    sub_db->getDoubleArray("uniform_rod_properties", &d_uniform_rod_properties[ln][j][0], IBRodForceSpec::NUM_MATERIAL_PARAMS);
                }
#if ENABLE_SUBDOMAIN_INDICES
                if (sub_db->keyExists("uniform_rod_subdomain_idx"))
                {
                    d_using_uniform_rod_subdomain_idx[ln][j] = true;
                    d_uniform_rod_subdomain_idx[ln][j] = sub_db->getInteger("uniform_rod_subdomain_idx");
                }
#endif

                if (sub_db->keyExists("uniform_target_stiffness"))
                {
                    d_using_uniform_target_stiffness[ln][j] = true;
                    d_uniform_target_stiffness[ln][j] = sub_db->getDouble("uniform_target_stiffness");
                    if (d_uniform_target_stiffness[ln][j] < 0.0)
                    {
                        TBOX_ERROR(d_object_name << ":\n  Invalid entry for key `uniform_target_stiffness' in database " << base_filename << std::endl
                                   << "  target point spring constant is negative" << std::endl);
                    }
                }
                if (sub_db->keyExists("uniform_target_damping"))
                {
                    d_using_uniform_target_damping[ln][j] = true;
                    d_uniform_target_damping[ln][j] = sub_db->getDouble("uniform_target_damping");
                    if (d_uniform_target_damping[ln][j] < 0.0)
                    {
                        TBOX_ERROR(d_object_name << ":\n  Invalid entry for key `uniform_target_damping' in database " << base_filename << std::endl
                                   << "  target point spring constant is negative" << std::endl);
                    }
                }
#if ENABLE_SUBDOMAIN_INDICES
                if (sub_db->keyExists("uniform_target_subdomain_idx"))
                {
                    d_using_uniform_target_subdomain_idx[ln][j] = true;
                    d_uniform_target_subdomain_idx[ln][j] = sub_db->getInteger("uniform_target_subdomain_idx");
                }
#endif

#if ENABLE_SUBDOMAIN_INDICES
                if (sub_db->keyExists("uniform_anchor_subdomain_idx"))
                {
                    d_using_uniform_anchor_subdomain_idx[ln][j] = true;
                    d_uniform_anchor_subdomain_idx[ln][j] = sub_db->getInteger("uniform_anchor_subdomain_idx");
                }
#endif

                if (sub_db->keyExists("uniform_bdry_mass"))
                {
                    d_using_uniform_bdry_mass[ln][j] = true;
                    d_uniform_bdry_mass[ln][j] = sub_db->getDouble("uniform_bdry_mass");

                    if (d_uniform_bdry_mass[ln][j] < 0.0)
                    {
                        TBOX_ERROR(d_object_name << ":\n  Invalid entry for key `uniform_bdry_mass' in database " << base_filename << std::endl
                                   << "  boundary mass is negative" << std::endl);
                    }
                }
                if (sub_db->keyExists("uniform_bdry_mass_stiffness"))
                {
                    d_using_uniform_bdry_mass_stiffness[ln][j] = true;
                    d_uniform_bdry_mass_stiffness[ln][j] = sub_db->getDouble("uniform_bdry_mass_stiffness");

                    if (d_uniform_bdry_mass_stiffness[ln][j] < 0.0)
                    {
                        TBOX_ERROR(d_object_name << ":\n  Invalid entry for key `uniform_bdry_mass_stiffness' in database " << base_filename << std::endl
                                   << "  boundary mass spring constant is negative" << std::endl);
                    }
                }
            }
        }
    }

    // Output the names of the input files to be read along with additional
    // debugging information.
    pout << d_object_name << ":  Reading from datasets.\n";
    for (int ln = 0; ln < d_max_levels; ++ln)
    {
        const unsigned int num_base_filename = d_base_filename[ln].size();
        for (unsigned int j = 0; j < num_base_filename; ++j)
        {
            const std::string& base_filename = d_base_filename[ln][j];
            pout << "  dataset: " << base_filename << "\n"
                 << "  assigned to level " << ln << " of the Cartesian grid patch hierarchy\n";
            if (!d_enable_springs[ln][j])
            {
                pout << "  NOTE: spring forces are DISABLED for " << base_filename << "\n";
            }
            else
            {
                if (d_using_uniform_spring_stiffness[ln][j])
                {
                    pout << "  NOTE: uniform spring stiffnesses are being employed for the structure named " << base_filename << "\n"
                         << "        any stiffness information in optional file " << base_filename << ".spring will be IGNORED\n";
                }
                if (d_using_uniform_spring_rest_length[ln][j])
                {
                    pout << "  NOTE: uniform spring resting lengths are being employed for the structure named " << base_filename << "\n"
                         << "        any resting length information in optional file " << base_filename << ".spring will be IGNORED\n";
                }
                if (d_using_uniform_spring_force_fcn_idx[ln][j])
                {
                    pout << "  NOTE: uniform spring force functions are being employed for the structure named " << base_filename << "\n"
                         << "        any force function index information in optional file " << base_filename << ".spring will be IGNORED\n";
                }
#if ENABLE_SUBDOMAIN_INDICES
                if (d_using_uniform_spring_subdomain_idx[ln][j])
                {
                    pout << "  NOTE: uniform spring subdomain indicies are being employed for the structure named " << base_filename << "\n"
                         << "        any subdomain index information in optional file " << base_filename << ".spring will be IGNORED\n";
                }
#endif
            }

            if (!d_enable_beams[ln][j])
            {
                pout << "  NOTE: beam forces are DISABLED for " << base_filename << "\n";
            }
            else
            {
                if (d_using_uniform_beam_bend_rigidity[ln][j])
                {
                    pout << "  NOTE: uniform beam bending rigidities are being employed for the structure named " << base_filename << "\n"
                         << "        any stiffness information in optional file " << base_filename << ".beam will be IGNORED\n";
                }
                if (d_using_uniform_beam_curvature[ln][j])
                {
                    pout << "  NOTE: uniform beam curvatures are being employed for the structure named " << base_filename << "\n"
                         << "        any curvature information in optional file " << base_filename << ".beam will be IGNORED\n";
                }
#if ENABLE_SUBDOMAIN_INDICES
                if (d_using_uniform_beam_subdomain_idx[ln][j])
                {
                    pout << "  NOTE: uniform beam subdomain indicies are being employed for the structure named " << base_filename << "\n"
                         << "        any subdomain index information in optional file " << base_filename << ".beam will be IGNORED\n";
                }
#endif
            }

            if (!d_enable_rods[ln][j])
            {
                pout << "  NOTE: rod forces are DISABLED for " << base_filename << "\n";
            }
            else
            {
                if (d_using_uniform_rod_properties[ln][j])
                {
                    pout << "  NOTE: uniform rod material properties are being employed for the structure named " << base_filename << "\n"
                         << "        any material property information in optional file " << base_filename << ".rod will be IGNORED\n";
                }
#if ENABLE_SUBDOMAIN_INDICES
                if (d_using_uniform_rod_subdomain_idx[ln][j])
                {
                    pout << "  NOTE: uniform rod subdomain indicies are being employed for the structure named " << base_filename << "\n"
                         << "        any subdomain index information in optional file " << base_filename << ".rod will be IGNORED\n";
                }
#endif
            }

            if (!d_enable_target_points[ln][j])
            {
                pout << "  NOTE: target point penalty forces are DISABLED for " << base_filename << "\n";
            }
            else
            {
                if (d_using_uniform_target_stiffness[ln][j])
                {
                    pout << "  NOTE: uniform target point stiffnesses are being employed for the structure named " << base_filename << "\n"
                         << "        any target point stiffness information in optional file " << base_filename << ".target will be IGNORED\n";
                }
                if (d_using_uniform_target_damping[ln][j])
                {
                    pout << "  NOTE: uniform target point damping factors are being employed for the structure named " << base_filename << "\n"
                         << "        any target point damping factor information in optional file " << base_filename << ".target will be IGNORED\n";
                }
#if ENABLE_SUBDOMAIN_INDICES
                if (d_using_uniform_target_subdomain_idx[ln][j])
                {
                    pout << "  NOTE: uniform target point subdomain indicies are being employed for the structure named " << base_filename << "\n"
                         << "        any subdomain index information in optional file " << base_filename << ".target will be IGNORED\n";
                }
#endif
            }

            if (!d_enable_anchor_points[ln][j])
            {
                pout << "  NOTE: anchor points are DISABLED for " << base_filename << "\n";
            }
            else
            {
#if ENABLE_SUBDOMAIN_INDICES
                if (d_using_uniform_anchor_subdomain_idx[ln][j])
                {
                    pout << "  NOTE: uniform anchor point subdomain indicies are being employed for the structure named " << base_filename << "\n"
                         << "        any subdomain index information in optional file " << base_filename << ".anchor will be IGNORED\n";
                }
#endif
            }

            if (!d_enable_bdry_mass[ln][j])
            {
                pout << "  NOTE: massive boundary points are DISABLED for " << base_filename << "\n";
            }
            else
            {
                if (d_using_uniform_bdry_mass[ln][j])
                {
                    pout << "  NOTE: uniform boundary point masses are being employed for the structure named " << base_filename << "\n"
                         << "        any boundary point mass information in optional file " << base_filename << ".mass will be IGNORED\n";
                }
                if (d_using_uniform_bdry_mass_stiffness[ln][j])
                {
                    pout << "  NOTE: uniform massive boundary point stiffnesses are being employed for the structure named " << base_filename << "\n"
                         << "        any massive boundary point stiffness information in optional file " << base_filename << ".mass will be IGNORED\n";
                }
            }

            if (!d_enable_instrumentation[ln][j])
            {
                pout << "  NOTE: instrumentation is DISABLED for " << base_filename << "\n";
            }

            if (!d_enable_sources[ln][j])
            {
                pout << "  NOTE: sources/sinks are DISABLED for " << base_filename << "\n";
            }

            pout << "\n";
        }
    }
    return;
}// getFromInput

/////////////////////////////// NAMESPACE ////////////////////////////////////

}// namespace IBAMR

//////////////////////////////////////////////////////////////////////////////
