/*==============================================================================
 Library: MSVTK                                                                
 
 Copyright (c) Kitware Inc.
 
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

#include<tbox/Pointer.h>
#include<vtkSmartPointer.h>
#include <petscvec.h>
#include <string>
#include <vector>

namespace IBTK {
class msvAppInitializer;

}
namespace IBAMR {
class INSHierarchyIntegrator; 
class IBHierarchyIntegrator;
class msvIBInitializer;
class IBMethod;
class IBStandardForceGen;
}
namespace SAMRAI {
  namespace tbox {
    class Database;
  }
  namespace geom {
    template<int DIM> class CartesianGridGeometry;
  }
  namespace hier {
    template<int DIM> class PatchHierarchy;
  }
  namespace mesh {
    template<int DIM> class StandardTagAndInitialize;
    template<int DIM> class LoadBalancer;
    template<int DIM> class GriddingAlgorithm;
    template<int DIM> class BergerRigoutsos;
  }
}

class vtkHierarchicalBoxDataSet;
class vtkPolyData;
class vtkAMRBox;

class msvFluidSimulator
{
public:
  typedef SAMRAI::geom::CartesianGridGeometry<NDIM> 	CartesianGridGeometry;
  typedef SAMRAI::hier::PatchHierarchy<NDIM> 		PatchHierarchy;
  typedef SAMRAI::mesh::StandardTagAndInitialize<NDIM> 	StandardTagAndInitialize;
  typedef SAMRAI::mesh::BergerRigoutsos<NDIM> 		BergerRigoutsos;
  typedef SAMRAI::mesh::LoadBalancer<NDIM> 		LoadBalancer;
  typedef SAMRAI::mesh::GriddingAlgorithm<NDIM> 		GriddingAlgorithm;
  typedef IBAMR::INSHierarchyIntegrator 			INSHierarchyIntegrator;
  typedef IBAMR::IBHierarchyIntegrator 			IBHierarchyIntegrator;
  typedef IBAMR::IBMethod 				IBMethod;
  typedef IBAMR::IBStandardForceGen 			IBStandardForceGen;
  typedef IBAMR::msvIBInitializer 			msvIBInitializer;
  
public:
  msvFluidSimulator(const std::string &file_name);
  msvFluidSimulator();
  ~msvFluidSimulator();
  
  // Initialize AMR data structure, solver and main algorithms
  void msvInitializeAMR(const std::string &init_file, int coarsest_grid_spacing, int max_levels, vtkSmartPointer<vtkPolyData> polydata);
  
  // Run one time-step of the fluid solver
  void run();
  
  // Populate vtk AMR datasets
  void setAMRData(vtkSmartPointer<vtkPolyData> polydata);
  
  // Populate lagrangian datasets
  void setLagrangianData(vtkSmartPointer<vtkPolyData> polydata);
  
  // Initialize the grid
  void msvInitializeCartesianGrid(std::vector<int> &lower, std::vector<int> &upper, std::vector<double> &x_lo, std::vector<double> &x_up, int max_levels);
  
  // Initialize the lagrangian points within the grid
  void msvInitializeImmersedBoundaryMethod(int max_levels, vtkSmartPointer<vtkPolyData> polydata);
  
  // Add boundary conditions
  void msvInitializeBoundaryConditions(double VelocityBcCoefs[NDIM][18]);
  
  // Extract velocity and pressure data
  void extractAMRData(vtkHierarchicalBoxDataSet *dataset);
  
  // Create grid hierarchy
  void amrToVTK(vtkHierarchicalBoxDataSet *dataset);
  
  // Return internal vtk AMR dataset
  vtkSmartPointer<vtkHierarchicalBoxDataSet> getAMRDataSet();
  
  // Set boundary edges of the vessel dataset
  int setBoundaryEdges(vtkPolyData* data, int numSubdivisions = 2, double Tol = 1e-2);
  
  vtkSmartPointer<vtkPolyData> getDataCaps();

private:  
  // Create a vtkAMRBox with parameters and return resultant 
  // box (used to initialize AMR dataset)
  vtkAMRBox getAMRBox(const int lo[3],const int up[3],const double x[3],const double h[3]);
  
private:  
  Vec petsc_position_vector, petsc_velocity_vector;
  double *X, *V;

private:
  SAMRAI::tbox::Pointer<INSHierarchyIntegrator>    navier_stokes_integrator;
  SAMRAI::tbox::Pointer<IBHierarchyIntegrator>     time_integrator;
  SAMRAI::tbox::Pointer<CartesianGridGeometry>     grid_geometry;
  SAMRAI::tbox::Pointer<PatchHierarchy>            patch_hierarchy;
  SAMRAI::tbox::Pointer<StandardTagAndInitialize>  error_detector;
  SAMRAI::tbox::Pointer<BergerRigoutsos>           box_generator;
  SAMRAI::tbox::Pointer<LoadBalancer>              load_balancer;
  SAMRAI::tbox::Pointer<GriddingAlgorithm>         gridding_algorithm; 
  SAMRAI::tbox::Pointer<IBMethod>                  ib_method_ops;
  SAMRAI::tbox::Pointer<msvIBInitializer>          ib_initializer;
  SAMRAI::tbox::Pointer<IBStandardForceGen>        ib_force_fcn;
  
  vtkSmartPointer<vtkHierarchicalBoxDataSet>       vtk_amr_dataset;
  vtkSmartPointer<vtkPolyData>                     vtk_lagrangian_dataset;
  vtkSmartPointer<vtkPolyData>                     vtk_boundary_edges;
};
