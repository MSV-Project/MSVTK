// Filename: msvIBInitializer.h
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

#ifndef included_msvIBInitializer
#define included_msvIBInitializer

/////////////////////////////// INCLUDES /////////////////////////////////////

#ifndef included_IBAMR_prefix_config
// #include <IBAMR_prefix_config.h>
#define included_IBAMR_prefix_config
#endif

// IBAMR INCLUDES
#include <ibamr/IBRodForceSpec.h>

// IBTK INCLUDES
#include <ibtk/LInitStrategy.h>
#include <ibtk/LSiloDataWriter.h>
#include <ibtk/Streamable.h>

// C++ STDLIB INCLUDES
#include <map>
#include <vector>
#include <vtkSmartPointer.h>

class vtkDataSet;

/////////////////////////////// CLASS DEFINITION /////////////////////////////

namespace IBAMR
{
/*!
 * \brief Class msvIBInitializer is a concrete LInitStrategy that
 * initializes the configuration of one or more Lagrangian structures from vtk
 * datasets.
 *
 *
*/
class msvIBInitializer
    : public IBTK::LInitStrategy
{
public:
    /*!
     * \brief Constructor.
     */
    msvIBInitializer(
        const std::string& object_name,
        SAMRAI::tbox::Pointer<SAMRAI::tbox::Database> input_db,
        vtkSmartPointer<vtkDataSet> data
                    );

    /*!
     * \brief Destructor.
     */
    ~msvIBInitializer();

    /*!
     * \brief Register a Silo data writer with the IB initializer object.
     */
    void
    registerLSiloDataWriter(
        SAMRAI::tbox::Pointer<IBTK::LSiloDataWriter> silo_writer);

    /*!
     * \brief Determine whether there are any Lagrangian nodes on the specified
     * patch level.
     *
     * \return A boolean value indicating whether Lagrangian data is associated
     * with the given level in the patch hierarchy.
     */
    bool
    getLevelHasLagrangianData(
        int level_number,
        bool can_be_refined) const;

    /*!
     * \brief Determine the number of local nodes on the specified patch level.
     *
     * \return The number of local nodes on the specified level.
     */
    unsigned int
    computeLocalNodeCountOnPatchLevel(
        SAMRAI::tbox::Pointer<SAMRAI::hier::PatchHierarchy<NDIM> > hierarchy,
        int level_number,
        double init_data_time,
        bool can_be_refined,
        bool initial_time);

    /*!
     * \brief Initialize the structure indexing information on the patch level.
     */
    void
    initializeStructureIndexingOnPatchLevel(
        std::map<int,std::string>& strct_id_to_strct_name_map,
        std::map<int,std::pair<int,int> >& strct_id_to_lag_idx_range_map,
        int level_number,
        double init_data_time,
        bool can_be_refined,
        bool initial_time,
        IBTK::LDataManager* l_data_manager);

    /*!
     * \brief Initialize the LNode and LData data needed to specify the
     * configuration of the curvilinear mesh on the patch level.
     *
     * \return The number of local nodes initialized on the patch level.
     */
    unsigned int
    initializeDataOnPatchLevel(
        int lag_node_index_idx,
        unsigned int global_index_offset,
        unsigned int local_index_offset,
        SAMRAI::tbox::Pointer<IBTK::LData> X_data,
        SAMRAI::tbox::Pointer<IBTK::LData> U_data,
        SAMRAI::tbox::Pointer<SAMRAI::hier::PatchHierarchy<NDIM> > hierarchy,
        int level_number,
        double init_data_time,
        bool can_be_refined,
        bool initial_time,
        IBTK::LDataManager* l_data_manager);

    /*!
     * \brief Initialize the LData needed to specify the mass and spring
     * constant data required by the penalty IB method.
     *
     * \return The number of local nodes initialized on the patch level.
     */
    unsigned int
    initializeMassDataOnPatchLevel(
        unsigned int global_index_offset,
        unsigned int local_index_offset,
        SAMRAI::tbox::Pointer<IBTK::LData> M_data,
        SAMRAI::tbox::Pointer<IBTK::LData> K_data,
        SAMRAI::tbox::Pointer<SAMRAI::hier::PatchHierarchy<NDIM> > hierarchy,
        int level_number,
        double init_data_time,
        bool can_be_refined,
        bool initial_time,
        IBTK::LDataManager* l_data_manager);

    /*!
     * \brief Initialize the LNode data needed to specify director vectors
     * required by some material models.
     *
     * \return The number of local nodes initialized on the patch level.
     */
    unsigned int
    initializeDirectorDataOnPatchLevel(
        unsigned int global_index_offset,
        unsigned int local_index_offset,
        SAMRAI::tbox::Pointer<IBTK::LData> D_data,
        SAMRAI::tbox::Pointer<SAMRAI::hier::PatchHierarchy<NDIM> > hierarchy,
        int level_number,
        double init_data_time,
        bool can_be_refined,
        bool initial_time,
        IBTK::LDataManager* l_data_manager);

    /*!
     * \brief Tag cells for initial refinement.
     *
     * When the patch hierarchy is being constructed at the initial simulation
     * time, it is necessary to instruct the gridding algorithm where to place
     * local refinement in order to accommodate portions of the curvilinear mesh
     * that will reside in any yet-to-be-constructed level(s) of the patch
     * hierarchy.
     */
    void
    tagCellsForInitialRefinement(
        SAMRAI::tbox::Pointer<SAMRAI::hier::PatchHierarchy<NDIM> > hierarchy,
        int level_number,
        double error_data_time,
        int tag_index);

protected:

private:
    /*!
     * \brief Default constructor.
     *
     * \note This constructor is not implemented and should not be used.
     */
    msvIBInitializer();

    /*!
     * \brief Copy constructor.
     *
     * \note This constructor is not implemented and should not be used.
     *
     * \param from The value to copy to this object.
     */
    msvIBInitializer(
        const msvIBInitializer& from);

    /*!
     * \brief Assignment operator.
     *
     * \note This constructor is not implemented and should not be used.
     *
     * \param that The value to assign to this object.
     *
     * \return A reference to this object.
     */
    msvIBInitializer&
    operator=(
        const msvIBInitializer& that);

    /*!
     * \brief Configure the Lagrangian Silo data writer to plot the data
     * associated with the specified level of the locally refined Cartesian
     * grid.
     */
    void
    initializeLSiloDataWriter(
        int level_number);

    /*!
     * \brief Read the vertex data from one or more datasets.
     */
    void
    readVertexDataset();

    /*!
     * \brief Read the spring data from one or more input files.
     */
    void
    readSpringDataset();

    /*!
     * \brief Read the spring data from one or more vtk datasets.
     */
    void
    AddPolyDataSet(int ln, vtkDataSet *data_set);
    
//     /*!
//      * \brief Read the beam data from one or more input files.
//      */
//     void
//     readBeamFiles();
// 
//     /*!
//      * \brief Read the rod data from one or more input files.
//      */
//     void
//     readRodFiles();
// 
    /*!
     * \brief Read the target point data from one or more input files.
     */
    void
    readTargetPointDataset();
// 
//     /*!
//      * \brief Read the anchor point data from one or more input files.
//      */
//     void
//     readAnchorPointFiles();
// 
//     /*!
//      * \brief Read the boundary mass data from one or more input files.
//      */
//     void
//     readBoundaryMassFiles();
// 
//     /*!
//      * \brief Read the director data from one or more input files.
//      */
//     void
//     readDirectorFiles();
// 
//     /*!
//      * \brief Read the instrumentation data from one or more input files.
//      */
//     void
//     readInstrumentationFiles();
// 
    /*!
     * \brief Read the source/sink data from one or more input datasets.
     */
    void
    readSourceDatasets();

    /*!
     * \brief Determine the indices of any vertices initially located within the
     * specified patch.
     */
    void
    getPatchVertices(
        std::vector<std::pair<int,int> >& point_indices,
        SAMRAI::tbox::Pointer<SAMRAI::hier::Patch<NDIM> > patch,
        int level_number,
        bool can_be_refined) const;

    /*!
     * \return The canonical Lagrangian index of the specified vertex.
     */
    int
    getCanonicalLagrangianIndex(
        const std::pair<int,int>& point_index,
        int level_number) const;

    /*!
     * \return The initial position of the specified vertex.
     */
    blitz::TinyVector<double,NDIM>
    getVertexPosn(
        const std::pair<int,int>& point_index,
        int level_number) const;

    /*!
     * \return The target point specifications associated with a particular
     * node.
     */
    struct TargetSpec;
    const TargetSpec&
    getVertexTargetSpec(
        const std::pair<int,int>& point_index,
        int level_number) const;

    /*!
     * \return The anchor point specifications associated with a particular
     * node.
     */
    struct AnchorSpec;
    const AnchorSpec&
    getVertexAnchorSpec(
        const std::pair<int,int>& point_index,
        int level_number) const;

    /*!
     * \return The massive boundary point specifications associated with a
     * particular node.
     */
    struct BdryMassSpec;
    const BdryMassSpec&
    getVertexBdryMassSpec(
        const std::pair<int,int>& point_index,
        int level_number) const;

    /*!
     * \return The directors associated with a particular node.
     */
    const std::vector<double>&
    getVertexDirectors(
        const std::pair<int,int>& point_index,
        int level_number) const;

    /*!
     * \return The instrumentation indices associated with a particular node (or
     * std::make_pair(-1,-1) if there is no instrumentation data associated with
     * that node).
     */
    std::pair<int,int>
    getVertexInstrumentationIndices(
        const std::pair<int,int>& point_index,
        int level_number) const;

    /*!
     * \return The source indices associated with a particular node (or -1 if
     * there is no source data associated with that node).
     */
    int
    getVertexSourceIndices(
        const std::pair<int,int>& point_index,
        int level_number) const;

    /*!
     * \return The specification objects associated with the specified vertex.
     */
    std::vector<SAMRAI::tbox::Pointer<IBTK::Streamable> >
    initializeSpecs(
        const std::pair<int,int>& point_index,
        unsigned int global_index_offset,
        int level_number) const;

    /*!
     * Read input values, indicated above, from given database.
     *
     * When assertion checking is active, the database pointer must be non-null.
     */
    void
    getFromInput(
        SAMRAI::tbox::Pointer<SAMRAI::tbox::Database> db,
        vtkDataSet* data
                );

    /*
     * The object name is used as a handle to databases stored in restart files
     * and for error reporting purposes.
     */
    std::string d_object_name;

    /*
     * The boolean value determines whether file read batons are employed to
     * prevent multiple MPI processes from accessing the same input files
     * simultaneously.
     */
    bool d_use_file_batons;

    /*
     * The maximum number of levels in the Cartesian grid patch hierarchy and a
     * vector of boolean values indicating whether a particular level has been
     * initialized yet.
     */
    int d_max_levels;
    std::vector<bool> d_level_is_initialized;

    /*
     * An (optional) Lagrangian Silo data writer.
     */
    SAMRAI::tbox::Pointer<IBTK::LSiloDataWriter> d_silo_writer;

    /*
     * The base filenames of the structures are used to generate unique names
     * when registering data with the Silo data writer.
     */
    std::vector<std::vector<std::string> > d_base_filename;
    
    /*
     * Pointers to vtk datasets
     */
    std::vector<std::vector<vtkSmartPointer<vtkDataSet> > > d_data_sets;
    
    /*
     * Optional shift and scale factors.
     *
     * \note These shift and scale factors are applied to ALL structures read in
     * by this reader.
     *
     * \note The scale factor is applied both to positions and to spring rest
     * lengths.
     *
     * \note The shift factor should have the same units as the positions in the
     * input files, i.e., X_final = scale*(X_initial + shift).
     */
    double d_length_scale_factor;
    blitz::TinyVector<double,NDIM> d_posn_shift;

    /*
     * Vertex information.
     */
    std::vector<std::vector<int> > d_num_vertex, d_vertex_offset;
    std::vector<std::vector<std::vector<blitz::TinyVector<double,NDIM> > > > d_vertex_posn;

    /*
     * Edge data structures.
     */
    typedef std::pair<int,int> Edge;
    struct EdgeComp
        : public std::binary_function<Edge,Edge,bool>
    {
        inline bool
        operator()(
            const Edge& e1,
            const Edge& e2) const
            {
                return (e1.first < e2.first) || (e1.first == e2.first && e1.second < e2.second);
            }
    };

    /*
     * Spring information.
     */
    std::vector<std::vector<bool> > d_enable_springs;

    std::vector<std::vector<std::multimap<int,Edge> > > d_spring_edge_map;

    struct SpringSpec
    {
        double stiffness, rest_length;
        int force_fcn_idx;
#if ENABLE_SUBDOMAIN_INDICES
        int subdomain_idx;
#endif
    };
    std::vector<std::vector<std::map<Edge,SpringSpec,EdgeComp> > > d_spring_spec_data;

    std::vector<std::vector<bool> > d_using_uniform_spring_stiffness;
    std::vector<std::vector<double> > d_uniform_spring_stiffness;

    std::vector<std::vector<bool> > d_using_uniform_spring_rest_length;
    std::vector<std::vector<double> > d_uniform_spring_rest_length;

    std::vector<std::vector<bool> > d_using_uniform_spring_force_fcn_idx;
    std::vector<std::vector<int> > d_uniform_spring_force_fcn_idx;

#if ENABLE_SUBDOMAIN_INDICES
    std::vector<std::vector<bool> > d_using_uniform_spring_subdomain_idx;
    std::vector<std::vector<int> > d_uniform_spring_subdomain_idx;
#endif

    /*
     * Beam information.
     */
    std::vector<std::vector<bool> > d_enable_beams;

    struct BeamSpec
    {
        std::pair<int,int> neighbor_idxs;
        double bend_rigidity;
        blitz::TinyVector<double,NDIM> curvature;
#if ENABLE_SUBDOMAIN_INDICES
        int subdomain_idx;
#endif
    };
    std::vector<std::vector<std::multimap<int,BeamSpec> > > d_beam_spec_data;

    std::vector<std::vector<bool> > d_using_uniform_beam_bend_rigidity;
    std::vector<std::vector<double> > d_uniform_beam_bend_rigidity;

    std::vector<std::vector<bool> > d_using_uniform_beam_curvature;
    std::vector<std::vector<blitz::TinyVector<double,NDIM> > > d_uniform_beam_curvature;

#if ENABLE_SUBDOMAIN_INDICES
    std::vector<std::vector<bool> > d_using_uniform_beam_subdomain_idx;
    std::vector<std::vector<int> > d_uniform_beam_subdomain_idx;
#endif

    /*
     * Rod information.
     */
    std::vector<std::vector<bool> > d_enable_rods;

    std::vector<std::vector<std::multimap<int,Edge> > > d_rod_edge_map;

    struct RodSpec
    {
        blitz::TinyVector<double,IBRodForceSpec::NUM_MATERIAL_PARAMS> properties;
#if ENABLE_SUBDOMAIN_INDICES
        int subdomain_idx;
#endif
    };
    std::vector<std::vector<std::map<Edge,RodSpec,EdgeComp> > > d_rod_spec_data;

    std::vector<std::vector<bool> > d_using_uniform_rod_properties;
    std::vector<std::vector<blitz::TinyVector<double,IBRodForceSpec::NUM_MATERIAL_PARAMS> > > d_uniform_rod_properties;

#if ENABLE_SUBDOMAIN_INDICES
    std::vector<std::vector<bool> > d_using_uniform_rod_subdomain_idx;
    std::vector<std::vector<int> > d_uniform_rod_subdomain_idx;
#endif

    /*
     * Target point information.
     */
    std::vector<std::vector<bool> > d_enable_target_points;

    struct TargetSpec
    {
        double stiffness, damping;
#if ENABLE_SUBDOMAIN_INDICES
        int subdomain_idx;
#endif
    };
    std::vector<std::vector<std::vector<TargetSpec> > > d_target_spec_data;

    std::vector<std::vector<bool> > d_using_uniform_target_stiffness;
    std::vector<std::vector<double> > d_uniform_target_stiffness;

    std::vector<std::vector<bool> > d_using_uniform_target_damping;
    std::vector<std::vector<double> > d_uniform_target_damping;

#if ENABLE_SUBDOMAIN_INDICES
    std::vector<std::vector<bool> > d_using_uniform_target_subdomain_idx;
    std::vector<std::vector<int> > d_uniform_target_subdomain_idx;
#endif

    /*
     * Anchor point information.
     */
    std::vector<std::vector<bool> > d_enable_anchor_points;

    struct AnchorSpec
    {
        bool is_anchor_point;
#if ENABLE_SUBDOMAIN_INDICES
        int subdomain_idx;
#endif
    };
    std::vector<std::vector<std::vector<AnchorSpec> > > d_anchor_spec_data;

#if ENABLE_SUBDOMAIN_INDICES
    std::vector<std::vector<bool> > d_using_uniform_anchor_subdomain_idx;
    std::vector<std::vector<int> > d_uniform_anchor_subdomain_idx;
#endif

    /*
     * Mass information for the pIB method.
     */
    std::vector<std::vector<bool> > d_enable_bdry_mass;

    struct BdryMassSpec
    {
        double bdry_mass, stiffness;
    };
    std::vector<std::vector<std::vector<BdryMassSpec> > > d_bdry_mass_spec_data;

    std::vector<std::vector<bool> > d_using_uniform_bdry_mass;
    std::vector<std::vector<double> > d_uniform_bdry_mass;

    std::vector<std::vector<bool> > d_using_uniform_bdry_mass_stiffness;
    std::vector<std::vector<double> > d_uniform_bdry_mass_stiffness;

    /*
     * Orthonormal directors for the generalized IB method.
     */
    std::vector<std::vector<std::vector<std::vector<double> > > > d_directors;

    /*
     * Instrumentation information.
     */
    std::vector<std::vector<bool> > d_enable_instrumentation;
    std::vector<std::vector<std::map<int,std::pair<int,int> > > > d_instrument_idx;

    /*
     * Source information.
     */
    std::vector<std::vector<bool> > d_enable_sources;
    std::vector<std::vector<std::map<int,int> > > d_source_idx;

    /*
     * Data required to specify connectivity information for visualization
     * purposes.
     */
    std::vector<unsigned int> d_global_index_offset;
};
}// namespace IBAMR

/////////////////////////////// INLINE ///////////////////////////////////////

//#include "msvIBInitializer.I"

//////////////////////////////////////////////////////////////////////////////

#endif //#ifndef included_msvIBInitializer
