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
class IBStandardInitializer;
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
  msvFluidSimulator(const std::string &file_name);
  msvFluidSimulator();
  ~msvFluidSimulator();
  void run();
  void setAMRData();
  void setLagrangianData();
  void msvInitializeCartesianGridGeometry(std::vector<int> &lower, std::vector<int> &upper, std::vector<double> &x_lo, std::vector<double> &x_up, int max_levels);
  void msvInitializeImmersedBoundaryMethod(const std::string &filename);
  void msvInitializeImmersedBoundaryMethod(int max_levels, vtkSmartPointer<vtkPolyData> data);
  void msvInitializeAMR(const std::string &init_file, int coarsest_grid_spacing, const std::string &lagrangian_points);
  void msvInitializeAMR(const std::string &init_file, int coarsest_grid_spacing, int max_levels, vtkSmartPointer<vtkPolyData> data);
  void msvInitializeBoundaryConditions(double VelocityBcCoefs[NDIM][18]);
  vtkSmartPointer<vtkHierarchicalBoxDataSet> getAMRDataSet();
  vtkSmartPointer<vtkPolyData> getPolyDataSet();
  Vec petsc_position_vector, petsc_velocity_vector;
  double *X, *V;

private:  
  vtkAMRBox getAMRBox(const int lo[3],const int up[3],const double x[3],const double h[3]);
  
private:
  SAMRAI::tbox::Pointer<IBAMR::INSHierarchyIntegrator>                  navier_stokes_integrator;
  SAMRAI::tbox::Pointer<IBAMR::IBHierarchyIntegrator>                   time_integrator;
  SAMRAI::tbox::Pointer<SAMRAI::geom::CartesianGridGeometry<NDIM> >     grid_geometry;
  SAMRAI::tbox::Pointer<SAMRAI::hier::PatchHierarchy<NDIM> >            patch_hierarchy;
  SAMRAI::tbox::Pointer<SAMRAI::mesh::StandardTagAndInitialize<NDIM> >  error_detector;
  SAMRAI::tbox::Pointer<SAMRAI::mesh::BergerRigoutsos<NDIM> >           box_generator;
  SAMRAI::tbox::Pointer<SAMRAI::mesh::LoadBalancer<NDIM> >              load_balancer;
  SAMRAI::tbox::Pointer<SAMRAI::mesh::GriddingAlgorithm<NDIM> >         gridding_algorithm; 
  SAMRAI::tbox::Pointer<IBAMR::IBMethod>                                ib_method_ops;
  SAMRAI::tbox::Pointer<IBAMR::msvIBInitializer>                        ib_initializer;
  SAMRAI::tbox::Pointer<IBAMR::IBStandardInitializer>                   ib_stinitializer;
  SAMRAI::tbox::Pointer<IBAMR::IBStandardForceGen>                      ib_force_fcn;
  
  vtkSmartPointer<vtkHierarchicalBoxDataSet>                            vtk_amr_dataset;
  vtkSmartPointer<vtkPolyData>                                          vtk_lagrangian_dataset;
};
