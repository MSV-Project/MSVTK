
#include "msvFluidSimulator.h"

// Config files
#include <SAMRAI_config.h>

// Headers for basic PETSc functions
#include <petscsys.h>
#include <petscvec.h>

// Headers for basic SAMRAI objects
#include <BergerRigoutsos.h>
#include <LoadBalancer.h>
#include <StandardTagAndInitialize.h>
#include <VariableDatabase.h>
#include <CartesianGridGeometry.h>

// Headers for application-specific algorithm/data structure objects
#include <ibtk/LNodeSetData.h>
#include <ibtk/muParserCartGridFunction.h>
#include <ibtk/muParserRobinBcCoefs.h>
#include <ibamr/IBMethod.h>
#include <ibamr/IBHierarchyIntegrator.h>
#include <ibamr/IBStandardForceGen.h>
#include <ibamr/INSStaggeredHierarchyIntegrator.h>
#include <ibamr/INSCollocatedHierarchyIntegrator.h>
#include <ibamr/app_namespaces.h>

#include "msvIBInitializer.h"
#include "msvAppInitializer.h"

// VTK headers
#include <vtkHierarchicalBoxDataSet.h>
#include <vtkAMRBox.h>
#include <vtkUniformGrid.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkFeatureEdges.h>
#include <vtkAppendPolyData.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkCleanPolyData.h>
#include <vtkMath.h>
#include <vtkDelaunay2D.h>
#include <vtkTimerLog.h>


// -----------------------------------------------------------------------------
msvFluidSimulator::msvFluidSimulator() 
{
  this->vtk_amr_dataset = vtkSmartPointer<vtkHierarchicalBoxDataSet>::New();
}

// -----------------------------------------------------------------------------
msvFluidSimulator::~msvFluidSimulator()
{
  SAMRAIManager::shutdown();
}

// -----------------------------------------------------------------------------
void msvFluidSimulator::msvInitializeAMR(const std::string &conf_file, 
                                         int coarsest_grid_spacing, 
                                         int max_levels, 
                                         vtkSmartPointer<vtkPolyData> polydata)
{
  SAMRAI_MPI::setCommunicator(PETSC_COMM_WORLD);
  SAMRAI_MPI::setCallAbortInSerialInsteadOfExit();
  SAMRAIManager::startup();
  Pointer<msvAppInitializer> app_initializer = new msvAppInitializer(conf_file);
  
  // Create a Navier-Stokes solver on a staggered grid hierachy 
  this->navier_stokes_integrator 
    = new INSStaggeredHierarchyIntegrator("INSStaggeredHierarchyIntegrator", 
                                           app_initializer->getComponentDatabase("INSStaggeredHierarchyIntegrator"));
  
  // Initialize immersed boundary
  this->msvInitializeImmersedBoundaryMethod(max_levels,polydata);
  
  // Create a time-stepping method for the immersed boundary
  this->time_integrator 
    = new IBHierarchyIntegrator("IBHierarchyIntegrator", 
                                app_initializer->getComponentDatabase("IBHierarchyIntegrator"), 
                                this->ib_method_ops, 
                                this->navier_stokes_integrator);
  
  std::vector<int> lower(3,0), upper(3,coarsest_grid_spacing-1);
  std::vector<double> x_lo(3,0), x_up(3,0);
  double extent[6];
  polydata->GetBounds(extent);
  
  x_lo[0] = extent[0]-1;
  x_lo[1] = extent[2]-1;
  x_lo[2] = extent[4]-1;
  x_up[0] = extent[1]+1;
  x_up[1] = extent[3]+1;
  x_up[2] = extent[5]+1;
  
  // Initialize Cartesian Geometry
  this->msvInitializeCartesianGrid(lower,upper,x_lo,x_up,max_levels);
  
  // Set a Dirichlet boundary conditions for the pressure solver
  double VelocityBC[3][18] = 
  {
    {1.0,1.0,1.0,1.0,1.0,1.0,.0,.0,.0,.0,.0,.0,.0,.0,.0,.0,.0,.0},
    {1.0,1.0,1.0,1.0,1.0,1.0,.0,.0,.0,.0,.0,.0,.0,.0,.0,.0,.0,.0},
    {1.0,1.0,1.0,1.0,1.0,1.0,.0,.0,.0,.0,.0,.0,.0,.0,.0,.0,.0,.0}
  };
  
  this->msvInitializeBoundaryConditions(VelocityBC);      
//   this->setAMRData(polydata);
}

// -----------------------------------------------------------------------------
void msvFluidSimulator::msvInitializeImmersedBoundaryMethod(int max_levels, 
                                                            vtkSmartPointer<vtkPolyData> polydata)
{
  Pointer<MemoryDatabase> IBDatabase = new MemoryDatabase("IBDatabase");
  IBDatabase->putDatabase("IBMethod");
  IBDatabase->putDatabase("msvIBInitializer");
  
  IBDatabase->getDatabase("IBMethod")->putString("delta_fcn","IB_4");
  IBDatabase->getDatabase("IBMethod")->putBool("enable_logging", true);
  this->ib_method_ops  = new IBMethod("IBMethod", IBDatabase->getDatabase("IBMethod"));
  
  IBDatabase->getDatabase("msvIBInitializer")->putInteger("max_levels",max_levels);
  std::vector<std::string> structure_names;
  structure_names.push_back("aneurysm");
  IBDatabase->getDatabase("msvIBInitializer")->putDatabase("aneurysm");
  IBDatabase->getDatabase("msvIBInitializer")->getDatabase("aneurysm")->putInteger("level_number",4);
  IBDatabase->getDatabase("msvIBInitializer")->getDatabase("aneurysm")->putBool("enable_springs",false);
  IBDatabase->getDatabase("msvIBInitializer")->getDatabase("aneurysm")->putBool("enable_beams",false);
  IBDatabase->getDatabase("msvIBInitializer")->getDatabase("aneurysm")->putBool("enable_rods",false);
  IBDatabase->getDatabase("msvIBInitializer")->getDatabase("aneurysm")->putBool("enable_target_points",false);
  IBDatabase->getDatabase("msvIBInitializer")->getDatabase("aneurysm")->putBool("enable_anchor_points",false);
  IBDatabase->getDatabase("msvIBInitializer")->getDatabase("aneurysm")->putBool("enable_bdry_mass",false);
  IBDatabase->getDatabase("msvIBInitializer")->getDatabase("aneurysm")->putBool("enable_instrumentation",false);
  IBDatabase->getDatabase("msvIBInitializer")->getDatabase("aneurysm")->putBool("enable_sources",false);
  IBDatabase->getDatabase("msvIBInitializer")->putStringArray("structure_names",&structure_names[0],structure_names.size());
  
  this->ib_initializer = new msvIBInitializer("msvIBInitializer", 
                                              IBDatabase->getDatabase("msvIBInitializer"),polydata);

  this->ib_method_ops->registerLInitStrategy(this->ib_initializer);
//   this->ib_method_ops->IBMethod::registerIBLagrangianSourceFunction();
  this->ib_force_fcn = new IBStandardForceGen(true);
  this->ib_method_ops->registerIBLagrangianForceFunction(this->ib_force_fcn);
}

// -----------------------------------------------------------------------------
void msvFluidSimulator::msvInitializeBoundaryConditions(double VelocityBcCoefs[NDIM][18])
{
  MemoryDatabase BCDatabase("BCDatabase");
  BCDatabase.putDatabase("VelocityBcCoefs_0");
  BCDatabase.putDatabase("VelocityBcCoefs_1");
  BCDatabase.putDatabase("VelocityBcCoefs_2");
   
  BCDatabase.getDatabase("VelocityBcCoefs_0")->putDouble("acoef_function_0",VelocityBcCoefs[0][0]);
  BCDatabase.getDatabase("VelocityBcCoefs_0")->putDouble("acoef_function_0",VelocityBcCoefs[0][1]);
  BCDatabase.getDatabase("VelocityBcCoefs_0")->putDouble("acoef_function_0",VelocityBcCoefs[0][2]);
  BCDatabase.getDatabase("VelocityBcCoefs_0")->putDouble("acoef_function_0",VelocityBcCoefs[0][3]);
  BCDatabase.getDatabase("VelocityBcCoefs_0")->putDouble("acoef_function_0",VelocityBcCoefs[0][4]);
  BCDatabase.getDatabase("VelocityBcCoefs_0")->putDouble("acoef_function_0",VelocityBcCoefs[0][5]);
  BCDatabase.getDatabase("VelocityBcCoefs_0")->putDouble("bcoef_function_0",VelocityBcCoefs[0][6]);
  BCDatabase.getDatabase("VelocityBcCoefs_0")->putDouble("bcoef_function_0",VelocityBcCoefs[0][7]);
  BCDatabase.getDatabase("VelocityBcCoefs_0")->putDouble("bcoef_function_0",VelocityBcCoefs[0][8]);
  BCDatabase.getDatabase("VelocityBcCoefs_0")->putDouble("bcoef_function_0",VelocityBcCoefs[0][9]);
  BCDatabase.getDatabase("VelocityBcCoefs_0")->putDouble("bcoef_function_0",VelocityBcCoefs[0][10]);
  BCDatabase.getDatabase("VelocityBcCoefs_0")->putDouble("bcoef_function_0",VelocityBcCoefs[0][11]);
  BCDatabase.getDatabase("VelocityBcCoefs_0")->putDouble("gcoef_function_0",VelocityBcCoefs[0][12]);
  BCDatabase.getDatabase("VelocityBcCoefs_0")->putDouble("gcoef_function_0",VelocityBcCoefs[0][13]);
  BCDatabase.getDatabase("VelocityBcCoefs_0")->putDouble("gcoef_function_0",VelocityBcCoefs[0][14]);
  BCDatabase.getDatabase("VelocityBcCoefs_0")->putDouble("gcoef_function_0",VelocityBcCoefs[0][15]);
  BCDatabase.getDatabase("VelocityBcCoefs_0")->putDouble("gcoef_function_0",VelocityBcCoefs[0][16]);
  BCDatabase.getDatabase("VelocityBcCoefs_0")->putDouble("gcoef_function_0",VelocityBcCoefs[0][17]);
  
  BCDatabase.getDatabase("VelocityBcCoefs_1")->putDouble("acoef_function_1",VelocityBcCoefs[1][0]);
  BCDatabase.getDatabase("VelocityBcCoefs_1")->putDouble("acoef_function_1",VelocityBcCoefs[1][1]);
  BCDatabase.getDatabase("VelocityBcCoefs_1")->putDouble("acoef_function_1",VelocityBcCoefs[1][2]);
  BCDatabase.getDatabase("VelocityBcCoefs_1")->putDouble("acoef_function_1",VelocityBcCoefs[1][3]);
  BCDatabase.getDatabase("VelocityBcCoefs_1")->putDouble("acoef_function_1",VelocityBcCoefs[1][4]);
  BCDatabase.getDatabase("VelocityBcCoefs_1")->putDouble("acoef_function_1",VelocityBcCoefs[1][5]);
  BCDatabase.getDatabase("VelocityBcCoefs_1")->putDouble("bcoef_function_1",VelocityBcCoefs[1][6]);
  BCDatabase.getDatabase("VelocityBcCoefs_1")->putDouble("bcoef_function_1",VelocityBcCoefs[1][7]);
  BCDatabase.getDatabase("VelocityBcCoefs_1")->putDouble("bcoef_function_1",VelocityBcCoefs[1][8]);
  BCDatabase.getDatabase("VelocityBcCoefs_1")->putDouble("bcoef_function_1",VelocityBcCoefs[1][9]);
  BCDatabase.getDatabase("VelocityBcCoefs_1")->putDouble("bcoef_function_1",VelocityBcCoefs[1][10]);
  BCDatabase.getDatabase("VelocityBcCoefs_1")->putDouble("bcoef_function_1",VelocityBcCoefs[1][11]);
  BCDatabase.getDatabase("VelocityBcCoefs_1")->putDouble("gcoef_function_1",VelocityBcCoefs[1][12]);
  BCDatabase.getDatabase("VelocityBcCoefs_1")->putDouble("gcoef_function_1",VelocityBcCoefs[1][13]);
  BCDatabase.getDatabase("VelocityBcCoefs_1")->putDouble("gcoef_function_1",VelocityBcCoefs[1][14]);
  BCDatabase.getDatabase("VelocityBcCoefs_1")->putDouble("gcoef_function_1",VelocityBcCoefs[1][15]);
  BCDatabase.getDatabase("VelocityBcCoefs_1")->putDouble("gcoef_function_1",VelocityBcCoefs[1][16]);
  BCDatabase.getDatabase("VelocityBcCoefs_1")->putDouble("gcoef_function_1",VelocityBcCoefs[1][17]);
  
  BCDatabase.getDatabase("VelocityBcCoefs_2")->putDouble("acoef_function_2",VelocityBcCoefs[2][0]);
  BCDatabase.getDatabase("VelocityBcCoefs_2")->putDouble("acoef_function_2",VelocityBcCoefs[2][1]);
  BCDatabase.getDatabase("VelocityBcCoefs_2")->putDouble("acoef_function_2",VelocityBcCoefs[2][2]);
  BCDatabase.getDatabase("VelocityBcCoefs_2")->putDouble("acoef_function_2",VelocityBcCoefs[2][3]);
  BCDatabase.getDatabase("VelocityBcCoefs_2")->putDouble("acoef_function_2",VelocityBcCoefs[2][4]);
  BCDatabase.getDatabase("VelocityBcCoefs_2")->putDouble("acoef_function_2",VelocityBcCoefs[2][5]);
  BCDatabase.getDatabase("VelocityBcCoefs_2")->putDouble("bcoef_function_2",VelocityBcCoefs[2][6]);
  BCDatabase.getDatabase("VelocityBcCoefs_2")->putDouble("bcoef_function_2",VelocityBcCoefs[2][7]);
  BCDatabase.getDatabase("VelocityBcCoefs_2")->putDouble("bcoef_function_2",VelocityBcCoefs[2][8]);
  BCDatabase.getDatabase("VelocityBcCoefs_2")->putDouble("bcoef_function_2",VelocityBcCoefs[2][9]);
  BCDatabase.getDatabase("VelocityBcCoefs_2")->putDouble("bcoef_function_2",VelocityBcCoefs[2][10]);
  BCDatabase.getDatabase("VelocityBcCoefs_2")->putDouble("bcoef_function_2",VelocityBcCoefs[2][11]);
  BCDatabase.getDatabase("VelocityBcCoefs_2")->putDouble("gcoef_function_2",VelocityBcCoefs[2][12]);
  BCDatabase.getDatabase("VelocityBcCoefs_2")->putDouble("gcoef_function_2",VelocityBcCoefs[2][13]);
  BCDatabase.getDatabase("VelocityBcCoefs_2")->putDouble("gcoef_function_2",VelocityBcCoefs[2][14]);
  BCDatabase.getDatabase("VelocityBcCoefs_2")->putDouble("gcoef_function_2",VelocityBcCoefs[2][15]);
  BCDatabase.getDatabase("VelocityBcCoefs_2")->putDouble("gcoef_function_2",VelocityBcCoefs[2][16]);
  BCDatabase.getDatabase("VelocityBcCoefs_2")->putDouble("gcoef_function_2",VelocityBcCoefs[2][17]);
  
  // Create Eulerian boundary condition specification objects (when necessary).
  const IntVector<NDIM>& periodic_shift = grid_geometry->getPeriodicShift();
  TinyVector<RobinBcCoefStrategy<NDIM>*,NDIM> u_bc_coefs;
  if (periodic_shift.min() > 0)
  {
    for (unsigned int d = 0; d < NDIM; ++d)
    {
      u_bc_coefs[d] = NULL;
    }
  }
  else
  {
    for (unsigned int d = 0; d < NDIM; ++d)
    {
      ostringstream bc_coefs_name_stream;
      bc_coefs_name_stream << "u_bc_coefs_" << d;
      const string bc_coefs_name = bc_coefs_name_stream.str();
      
      ostringstream bc_coefs_db_name_stream;
      bc_coefs_db_name_stream << "VelocityBcCoefs_" << d;
      const string bc_coefs_db_name = bc_coefs_db_name_stream.str();
      
      u_bc_coefs[d] = new muParserRobinBcCoefs(bc_coefs_name, BCDatabase.getDatabase(bc_coefs_db_name), grid_geometry);
    }
    this->navier_stokes_integrator->registerPhysicalBoundaryConditions(u_bc_coefs);
  }
}

// -----------------------------------------------------------------------------
void msvFluidSimulator::msvInitializeCartesianGrid(std::vector<int> &lower, 
                                                   std::vector<int> &upper, 
                                                   std::vector<double> &x_lo, 
                                                   std::vector<double> &x_up,
                                                   int max_levels
                                                   )
{
  Pointer<MemoryDatabase> GridDatabase = new MemoryDatabase("GridDatabase");
  
  GridDatabase->putDatabase("StandardTagAndInitialize");
  GridDatabase->putDatabase("LoadBalancer");
  GridDatabase->putDatabase("GriddingAlgorithm");
  
  IntVector<NDIM> lo(lower[0],lower[1],lower[2]);
  IntVector<NDIM> up(upper[0],upper[1],upper[2]);
  IntVector<NDIM> periodic_dimension(0);
  BoxList<NDIM> domain(Box<NDIM>(lo,up));

  this->grid_geometry = new CartesianGridGeometry("CartesianGeometry",&x_lo[0],&x_up[0],domain);
  this->grid_geometry->initializePeriodicShift(periodic_dimension);
  this->patch_hierarchy = new PatchHierarchy("PatchHierarchy", this->grid_geometry);
  
  // error_detector
  GridDatabase->getDatabase("StandardTagAndInitialize")->putString("tagging_method","GRADIENT_DETECTOR");
  
  // load_balancer
  int max_workload_factor = 1;
  GridDatabase->getDatabase("LoadBalancer")->putString("bin_pack_method","SPATIAL");
  GridDatabase->getDatabase("LoadBalancer")->putInteger("max_workload_factor",max_workload_factor);
  
  // gridding_algorithm
  GridDatabase->getDatabase("GriddingAlgorithm")->putInteger("max_levels",max_levels);
  GridDatabase->getDatabase("GriddingAlgorithm")->putDatabase("ratio_to_coarser");
  GridDatabase->getDatabase("GriddingAlgorithm")->putDatabase("largest_patch_size");
  GridDatabase->getDatabase("GriddingAlgorithm")->putDatabase("smallest_patch_size");
    
  std::vector<blitz::TinyVector<int,3> > level;
  for(int i = 0; i < max_levels; ++i)
    level.push_back(blitz::TinyVector<int,3>(4));
  
  GridDatabase->getDatabase("GriddingAlgorithm")->getDatabase("ratio_to_coarser")->putIntegerArray("level_1",level[0].data(),3);
  GridDatabase->getDatabase("GriddingAlgorithm")->getDatabase("ratio_to_coarser")->putIntegerArray("level_2",level[1].data(),3);
  GridDatabase->getDatabase("GriddingAlgorithm")->getDatabase("ratio_to_coarser")->putIntegerArray("level_3",level[2].data(),3);
  GridDatabase->getDatabase("GriddingAlgorithm")->getDatabase("ratio_to_coarser")->putIntegerArray("level_4",level[3].data(),3);
  GridDatabase->getDatabase("GriddingAlgorithm")->getDatabase("ratio_to_coarser")->putIntegerArray("level_5",level[4].data(),3);
  
  blitz::TinyVector<int,3> level_0(4);
  GridDatabase->getDatabase("GriddingAlgorithm")->getDatabase("smallest_patch_size")->putIntegerArray("level_0",level_0.data(),3);
  level_0[0] = level_0[1] = level_0[2] = 512;
  GridDatabase->getDatabase("GriddingAlgorithm")->getDatabase("largest_patch_size")->putIntegerArray("level_0",level_0.data(),3);
  
  GridDatabase->getDatabase("GriddingAlgorithm")->putDouble("efficiency_tolerance",0.85e0);
  GridDatabase->getDatabase("GriddingAlgorithm")->putDouble("combine_efficiency",0.85e0);
  
  this->error_detector     = new StandardTagAndInitialize("StandardTagAndInitialize", 
                                                          this->time_integrator, 
                                                          GridDatabase->getDatabase("StandardTagAndInitialize"));
  this->box_generator      = new BergerRigoutsos();
  this->load_balancer      = new LoadBalancer("LoadBalancer", GridDatabase->getDatabase("LoadBalancer"));
  this->gridding_algorithm = new GriddingAlgorithm("GriddingAlgorithm", 
                                                         GridDatabase->getDatabase("GriddingAlgorithm"), 
                                                         this->error_detector, 
                                                         this->box_generator, 
                                                         this->load_balancer); 
  
  vtkSmartPointer<vtkTimerLog> timer = vtkSmartPointer<vtkTimerLog>::New();
  //   Initialize hierarchy configuration and data on all patches.
  timer->StartTimer();
  this->time_integrator->initializePatchHierarchy(this->patch_hierarchy, this->gridding_algorithm);  
  timer->StopTimer();
  std::cout << "init = " << timer->GetElapsedTime() << std::endl;
  timer->StartTimer();
  this->time_integrator->regridHierarchy(); 
  timer->StopTimer();
  std::cout << "regrid = " << timer->GetElapsedTime() << std::endl;
}

// -----------------------------------------------------------------------------
void msvFluidSimulator::setAMRData(vtkSmartPointer<vtkPolyData> polydata)
{
  VariableDatabase<NDIM> *variable_database = VariableDatabase<NDIM>::getDatabase();
  Pointer<Variable<NDIM> > velocity_variable = this->navier_stokes_integrator->getVelocityVariable();
  Pointer<Variable<NDIM> > pressure_variable = this->navier_stokes_integrator->getPressureVariable();
  int velocity_idx = variable_database->mapVariableAndContextToIndex(velocity_variable, this->navier_stokes_integrator->getCurrentContext());
  int pressure_idx = variable_database->mapVariableAndContextToIndex(pressure_variable, this->navier_stokes_integrator->getCurrentContext());  

  int num_levels = this->patch_hierarchy->getFinestLevelNumber();
  vtk_amr_dataset->SetNumberOfLevels(num_levels);
  
  const double *X0 = this->grid_geometry->getXLower();
  
  for (int level_num = 0; level_num < num_levels; ++level_num)
  {
    Pointer<PatchLevel<NDIM> > level = patch_hierarchy->getPatchLevel(level_num);
    vtk_amr_dataset->SetRefinementRatio(level_num,4);

    for(PatchLevel<NDIM>::Iterator p(level); p; p++)
    {
      Pointer<Patch<NDIM> > patch = level->getPatch(p());
      Box<NDIM> patch_box = patch->getBox();      
      Pointer<CartesianPatchGeometry<NDIM> > patch_geometry = patch->getPatchGeometry();
      const double *dx = patch_geometry->getDx();
      
      const int *lo = patch_box.lower();
      const int *hi = patch_box.upper();
      vtkAMRBox box = getAMRBox(lo,hi,X0,dx);
      
      vtkSmartPointer<vtkUniformGrid> grid = vtkSmartPointer<vtkUniformGrid>::New();
      
//       vtkSmartPointer<vtkDoubleArray> pressure  = vtkSmartPointer<vtkDoubleArray>::New();
//             
//       ArrayData<NDIM,double> &velocity_data 
//         = static_cast<Pointer<CellData<NDIM,double> > >(patch->getPatchData(velocity_idx))->getArrayData();
//       ArrayData<NDIM,double> &pressure_data 
//         = static_cast<Pointer<CellData<NDIM,double> > >(patch->getPatchData(pressure_idx))->getArrayData();
//       
//       double *velocity_array, *pressure_array;
      
      // Set pressure array
//       pressure_array = pressure_data.getPointer();
//       pressure->SetArray(pressure_array, pressure_data.getDepth()* pressure_data.getOffset(),1);
//       pressure->SetNumberOfComponents(pressure_data.getDepth());
//       pressure->SetName("pressure");
//       grid->GetPointData()->AddArray(pressure);
            
      // Set velocity arrays
//       const int ncomponents = velocity_data.getDepth();
//       std::string name[] = {"velocity_x","velocity_y","velocity_z"};
//       vtkSmartPointer<vtkDoubleArray> velocities[ncomponents];
//       for (int i = 0; i < ncomponents; ++i)
//       {
//         int offset = velocity_data.getOffset();
//         velocity_array = velocity_data.getPointer(offset*i);
//         velocities[i] = vtkSmartPointer<vtkDoubleArray>::New();
//         velocities[i]->SetArray(velocity_array, offset,1);
//         velocities[i]->SetNumberOfComponents(ncomponents);        
//         velocities[i]->SetName(name[i].c_str());        
//         grid->GetPointData()->AddArray(velocities[i]);
//       }
      
      grid->Initialize(&box);
      int id = patch->getPatchNumber();
      int patch_level = patch->getPatchLevelNumber();
      vtk_amr_dataset->SetDataSet(patch_level,id,box,grid);
    }
  }    
  this->setLagrangianData(polydata);
}

// -----------------------------------------------------------------------------
void msvFluidSimulator::extractAMRData(vtkHierarchicalBoxDataSet *dataset)
{
  VariableDatabase<NDIM> *variable_database = VariableDatabase<NDIM>::getDatabase();
  Pointer<Variable<NDIM> > velocity_variable = this->navier_stokes_integrator->getVelocityVariable();
  Pointer<Variable<NDIM> > pressure_variable = this->navier_stokes_integrator->getPressureVariable();
  int velocity_idx = variable_database->mapVariableAndContextToIndex(velocity_variable, this->navier_stokes_integrator->getCurrentContext());
  int pressure_idx = variable_database->mapVariableAndContextToIndex(pressure_variable, this->navier_stokes_integrator->getCurrentContext());  

  int num_levels = this->patch_hierarchy->getFinestLevelNumber();
  
  const double *X0 = this->grid_geometry->getXLower();
  
  for (int level_num = 0; level_num < num_levels; ++level_num)
  {
    Pointer<PatchLevel<NDIM> > level = patch_hierarchy->getPatchLevel(level_num);

    for(PatchLevel<NDIM>::Iterator p(level); p; p++)
    {
      Pointer<Patch<NDIM> > patch = level->getPatch(p());
      Box<NDIM> patch_box = patch->getBox();      
      Pointer<CartesianPatchGeometry<NDIM> > patch_geometry = patch->getPatchGeometry();
      const double *dx = patch_geometry->getDx();
      
      const int *lo = patch_box.lower();
      const int *hi = patch_box.upper();
      vtkAMRBox box = getAMRBox(lo,hi,X0,dx);
      
      vtkUniformGrid *grid = dataset->GetDataSet(level,patch->getPatchNumber(),box);
      
      vtkSmartPointer<vtkDoubleArray> pressure  = vtkSmartPointer<vtkDoubleArray>::New();
            
      ArrayData<NDIM,double> &velocity_data 
        = static_cast<Pointer<CellData<NDIM,double> > >(patch->getPatchData(velocity_idx))->getArrayData();
      ArrayData<NDIM,double> &pressure_data 
        = static_cast<Pointer<CellData<NDIM,double> > >(patch->getPatchData(pressure_idx))->getArrayData();
      
      double *velocity_array, *pressure_array;
      
      // Set pressure array
      pressure_array = pressure_data.getPointer();
      pressure->SetArray(pressure_array, pressure_data.getDepth()* pressure_data.getOffset(),1);
      pressure->SetNumberOfComponents(pressure_data.getDepth());
      pressure->SetName("pressure");
      grid->GetPointData()->AddArray(pressure);
            
      // Set velocity arrays
      const int ncomponents = velocity_data.getDepth();
      std::string name[] = {"velocity_x","velocity_y","velocity_z"};
      vtkSmartPointer<vtkDoubleArray> velocities[ncomponents];
      for (int i = 0; i < ncomponents; ++i)
      {
        int offset = velocity_data.getOffset();
        velocity_array = velocity_data.getPointer(offset*i);
        velocities[i] = vtkSmartPointer<vtkDoubleArray>::New();
        velocities[i]->SetArray(velocity_array, offset,1);
        velocities[i]->SetNumberOfComponents(ncomponents);        
        velocities[i]->SetName(name[i].c_str());        
        grid->GetPointData()->AddArray(velocities[i]);
      }

    }
  }    
}

// -----------------------------------------------------------------------------
void msvFluidSimulator::amrToVTK(vtkHierarchicalBoxDataSet *dataset)
{
  int num_levels = this->patch_hierarchy->getFinestLevelNumber();
  dataset->SetNumberOfLevels(num_levels);
  
  Pointer<CartesianGridGeometry> grid_geometry = this->patch_hierarchy->getGridGeometry();
  const double *X0 = grid_geometry->getXLower();
  
  for (int level_num = 0; level_num < num_levels; ++level_num)
  {
    Pointer<PatchLevel<NDIM> > level = this->patch_hierarchy->getPatchLevel(level_num);
    dataset->SetRefinementRatio(level_num,4);

    for(PatchLevel<NDIM>::Iterator p(level); p; p++)
    {
      Pointer<Patch<NDIM> > patch = level->getPatch(p());
      Box<NDIM> patch_box = patch->getBox();      
      Pointer<CartesianPatchGeometry<NDIM> > patch_geometry = patch->getPatchGeometry();
      const double *dx = patch_geometry->getDx();
      
      const int *lo = patch_box.lower();
      const int *hi = patch_box.upper();
      vtkAMRBox box = getAMRBox(lo,hi,X0,dx);
      
      vtkSmartPointer<vtkUniformGrid> grid = vtkSmartPointer<vtkUniformGrid>::New();
      
      grid->Initialize(&box);
      int id = patch->getPatchNumber();
      int patch_level = patch->getPatchLevelNumber();
      dataset->SetDataSet(patch_level,id,box,grid);
    }
  }    
}

// -----------------------------------------------------------------------------
void msvFluidSimulator::setLagrangianData(vtkSmartPointer<vtkPolyData> polydata)
{
  int size;
  int num_levels = this->patch_hierarchy->getFinestLevelNumber();
  LDataManager *lagrangian_data_manager = this->ib_method_ops->getLDataManager();
  Pointer<LData> X_data = lagrangian_data_manager->getLData("X", num_levels);
  Pointer<LData> V_data = lagrangian_data_manager->getLData("U", num_levels);
  Vec X_petsc_vec = X_data->getVec();
  Vec V_petsc_vec = V_data->getVec();
  
  VecDuplicate(X_petsc_vec, &petsc_position_vector);
  VecDuplicate(V_petsc_vec, &petsc_velocity_vector);
  lagrangian_data_manager->scatterPETScToLagrangian(X_petsc_vec, petsc_position_vector, num_levels);
  lagrangian_data_manager->scatterPETScToLagrangian(V_petsc_vec, petsc_velocity_vector, num_levels);
  VecGetArray(petsc_position_vector,&X);
  VecGetArray(petsc_velocity_vector,&V);
  VecGetSize(petsc_position_vector,&size);
  
  vtkSmartPointer<vtkDoubleArray> positions = vtkSmartPointer<vtkDoubleArray>::New();
  vtkSmartPointer<vtkDoubleArray> velocity = vtkSmartPointer<vtkDoubleArray>::New();
  vtkSmartPointer<vtkPoints>      points = vtkSmartPointer<vtkPoints>::New();
  
  positions->SetArray(X,size,1);
  positions->SetName("positions");
  positions->SetNumberOfComponents(3);
  
  velocity->SetArray(V,size,1);
  velocity->SetName("velocity");
  velocity->SetNumberOfComponents(3);
  
  points->SetData(positions);
  polydata->SetPoints(points);  
  polydata->GetPointData()->AddArray(velocity);  
}

int msvFluidSimulator::setBoundaryEdges(vtkPolyData *dataset, int numSubdivisions, double Tol)
{
  int numRegions;
  double center[3] = {0};
  double bounds[6] = {0};
  double length;
  this->vtk_boundary_edges = vtkPolyData::New();
  
  // Extract boundary edges
  vtkSmartPointer<vtkFeatureEdges> boundaryEdges 
    = vtkSmartPointer<vtkFeatureEdges>::New();
  boundaryEdges->SetInput ( dataset );
  boundaryEdges->FeatureEdgesOff();
  boundaryEdges->NonManifoldEdgesOff();
  
  // Extract individual connected regions
  vtkSmartPointer<vtkPolyDataConnectivityFilter> boundaryEdgesLoops
    = vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
  boundaryEdgesLoops->SetInput( boundaryEdges->GetOutput() );
  boundaryEdgesLoops->SetExtractionModeToSpecifiedRegions();
  boundaryEdgesLoops->Update();
  
  // Triangulate each region
  vtkSmartPointer<vtkDelaunay2D> capTriangulator 
    = vtkSmartPointer<vtkDelaunay2D>::New();
  capTriangulator->SetTolerance(Tol);
  
  capTriangulator->BoundingTriangulationOff();
  
  vtkSmartPointer<vtkMath> math 
    = vtkSmartPointer<vtkMath>::New();
    
  vtkSmartPointer<vtkCleanPolyData> region 
    = vtkSmartPointer<vtkCleanPolyData>::New();
  
  // Loop over connected regions  
  region->CreateDefaultLocator();
  numRegions = boundaryEdgesLoops->GetNumberOfExtractedRegions();
  std::vector<vtkSmartPointer<vtkPolyData> > boundaryRegions;
  
  for ( int i = 0; i < numRegions; ++i )
  {      
    boundaryEdgesLoops->AddSpecifiedRegion(i);
    boundaryEdgesLoops->Update();
    
    // Get rid of orphan nodes
    region->SetInputConnection(boundaryEdgesLoops->GetOutputPort());
    region->Update();
    
    vtkPoints *boundaryPoints = region->GetOutput()->GetPoints();
    
    vtkIdType numPoints = region->GetOutput()->GetNumberOfPoints();
    vtkIdType numCells = region->GetOutput()->GetNumberOfCells();
        
    boundaryPoints->GetBounds(bounds);
    
    center[0] = .5*(bounds[0]+bounds[1]);
    center[1] = .5*(bounds[2]+bounds[3]);
    center[2] = .5*(bounds[4]+bounds[5]);
    
    for (vtkIdType j = 0; j < numPoints;  j+=5)  
    {
      double v[3] = {0};
      boundaryPoints->GetPoint(j,v);
      v[0] -= center[0];
      v[1] -= center[1];
      v[2] -= center[2];
      
      length = math->Norm(v);
      
      v[0] /= length;
      v[1] /= length;
      v[2] /= length;
      
      double dx = length/(numSubdivisions+1);
      
      for(int k = 1; k <= numSubdivisions; ++k)
      {
        double s = k*dx;
        double p[3] = {center[0]+s*v[0],center[1]+s*v[1],center[2]+s*v[2]};
        boundaryPoints->InsertNextPoint(p);
      }
    }
    boundaryPoints->InsertNextPoint(center);
    boundaryPoints->Squeeze();
        
    capTriangulator->SetInputConnection(region->GetOutputPort());
    capTriangulator->SetSourceConnection(region->GetOutputPort());
    capTriangulator->Update();
    
    boundaryRegions.push_back(vtkSmartPointer<vtkPolyData>::New());
    boundaryRegions.back()->CopyStructure(capTriangulator->GetOutput());
    
    boundaryEdgesLoops->DeleteSpecifiedRegion(i);        
  }
  
  // Add regions back together
  vtkSmartPointer<vtkAppendPolyData> appendBoundaryEdges 
    = vtkSmartPointer<vtkAppendPolyData>::New();
    
  for(int i = 0; i < boundaryRegions.size(); ++i)
    appendBoundaryEdges->AddInput(boundaryRegions[i]);
  appendBoundaryEdges->Update();
  
  this->vtk_boundary_edges->CopyStructure(appendBoundaryEdges->GetOutput());
  
}

// -----------------------------------------------------------------------------
vtkAMRBox msvFluidSimulator::getAMRBox(const int lo[3], const int hi[3], const double x[3], const double h[3])
{
  vtkAMRBox box(lo,hi);
  box.SetDataSetOrigin(x);
  box.SetGridSpacing(h);
  return box;  
}

// -----------------------------------------------------------------------------
vtkSmartPointer<vtkHierarchicalBoxDataSet> msvFluidSimulator::getAMRDataSet()
{
  return vtk_amr_dataset;
}

// -----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> msvFluidSimulator::getDataCaps()
{
  return vtk_boundary_edges;
}

// -----------------------------------------------------------------------------
void msvFluidSimulator::run()
{
  double dt = this->time_integrator->getTimeStepSize();
  this->time_integrator->advanceHierarchy(dt); 
}
