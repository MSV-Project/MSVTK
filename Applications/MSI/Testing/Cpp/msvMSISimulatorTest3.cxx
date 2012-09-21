
#include <SAMRAI_config.h>

// Headers for basic PETSc functions
#include <petscsys.h>

// Headers for basic SAMRAI objects
#include <BergerRigoutsos.h>
#include <CartesianGridGeometry.h>
#include <LoadBalancer.h>
#include <StandardTagAndInitialize.h>

// Headers for application-specific algorithm/data structure objects
#include <ibamr/IBHierarchyIntegrator.h>
#include <ibamr/IBMethod.h>
#include <ibamr/IBStandardForceGen.h>
#include <ibamr/IBStandardInitializer.h>
#include <ibamr/INSStaggeredHierarchyIntegrator.h>
#include <ibamr/app_namespaces.h>
#include <ibtk/AppInitializer.h>

// VTK headers
#include <vtkSmartPointer.h>
#include <vtkHierarchicalBoxDataSet.h>
#include <vtkUniformGrid.h>
#include <vtkAMRBox.h>
#include<vtkXMLHierarchicalBoxDataWriter.h>
#include <vtkTestUtilities.h>


// Function prototypes
vtkAMRBox getAMRBox(const int lo[3], const int hi[3], const double x[3], const double h[3]);
void amrToVTK(Pointer<PatchHierarchy<NDIM> > patch_hierarchy, 
              vtkSmartPointer<vtkHierarchicalBoxDataSet> dataset);
Pointer<IBMethod> getIBMethod(int max_levels, int on_level);
Pointer<GriddingAlgorithm<NDIM> > getGriddingAlgorithm(Pointer<IBHierarchyIntegrator> time_integrator, 
                                                       std::vector<blitz::TinyVector<int,3> > &ratios);
Pointer<PatchHierarchy<NDIM> > getPatchHierarchy(std::vector<int> &lower, 
                                                 std::vector<int> &upper, 
                                                 std::vector<double> &x_lo, 
                                                 std::vector<double> &x_up,
                                                 int max_levels);
/*******************************************************************************
 * For each run, the input filename and restart information (if needed) must   *
 * be given on the command line.  For non-restarted case, command line is:     *
 *                                                                             *
 *    executable <input file name>                                             *
 *******************************************************************************/
int msvMSISimulatorTest3(
    int argc,
    char* argv[])
{
    // Initialize PETSc, MPI, and SAMRAI.
    PetscInitialize(&argc,&argv,PETSC_NULL,PETSC_NULL);
    SAMRAI_MPI::setCommunicator(PETSC_COMM_WORLD);
    SAMRAI_MPI::setCallAbortInSerialInsteadOfExit();
    SAMRAIManager::startup();
    
    vtkSmartPointer<vtkXMLHierarchicalBoxDataWriter> writer = vtkSmartPointer<vtkXMLHierarchicalBoxDataWriter>::New(); 
    vtkSmartPointer<vtkHierarchicalBoxDataSet> dataset = vtkSmartPointer<vtkHierarchicalBoxDataSet>::New();   
    std::string out_file = "amr_grid_dataset.";
    {// cleanup dynamically allocated objects prior to shutdown

        // Parse command line options, set some standard options from the input
        // file, initialize the restart database (if this is a restarted run),
        // and enable file logging.
        Pointer<AppInitializer> app_initializer = new AppInitializer(argc, argv, "IB.log");
        Pointer<Database> eulerian_solver_db    = app_initializer->getComponentDatabase("INSStaggeredHierarchyIntegrator");
        Pointer<Database> lagrangian_solver_db  = app_initializer->getComponentDatabase("IBHierarchyIntegrator");
        
        // Create major algorithm and data objects that comprise the
        // application.  These objects are configured from the input database
        // and, if this is a restarted run, from the restart database.
        Pointer<INSHierarchyIntegrator> navier_stokes_integrator;
        Pointer<IBHierarchyIntegrator> time_integrator;
        navier_stokes_integrator = new INSStaggeredHierarchyIntegrator("INSStaggeredHierarchyIntegrator",eulerian_solver_db);

        const int max_levels = 10;
        const int coarsest_grid_spacing = 8;
        Pointer<IBMethod> ib_method_ops = getIBMethod(max_levels,6);
        time_integrator = new IBHierarchyIntegrator("IBHierarchyIntegrator", lagrangian_solver_db, ib_method_ops, navier_stokes_integrator);
        
        std::vector<int> lower(3,0), upper(3,coarsest_grid_spacing-1);
        std::vector<double> x_lo, x_up;
        
        x_lo.push_back(-250);
        x_lo.push_back(-250);
        x_lo.push_back(-250);
        
        x_up.push_back(0);
        x_up.push_back(0);
        x_up.push_back(0);
        
        std::vector<blitz::TinyVector<int,3> > ratios;
        for(int i = 0; i < max_levels; ++i)
          ratios.push_back(blitz::TinyVector<int,3>(4));
        
        Pointer<PatchHierarchy<NDIM> > patch_hierarchy       = getPatchHierarchy(lower,upper,x_lo,x_up,max_levels);
        Pointer<GriddingAlgorithm<NDIM> > gridding_algorithm = getGriddingAlgorithm(time_integrator,ratios);

        // Initialize hierarchy configuration and data on all patches.
        time_integrator->initializePatchHierarchy(patch_hierarchy, gridding_algorithm);

        // Deallocate initialization objects.
        ib_method_ops->freeLInitStrategy();
        app_initializer.setNull();

        // Print the input database contents to the log file.
        plog << "Input database:\n";
        
        // VTK writer
        amrToVTK(patch_hierarchy,dataset);
        writer->SetInput(dataset);
        writer->SetDataModeToBinary();
        out_file += writer->GetDefaultFileExtension();
        writer->SetFileName(out_file.c_str());
        writer->Write();   
        
    }// cleanup dynamically allocated objects prior to shutdown

    SAMRAIManager::shutdown();
    PetscFinalize();
    return 0;
}// main

// -----------------------------------------------------------------------------
Pointer<IBMethod> getIBMethod(int max_levels, int on_level)
{
  Pointer<MemoryDatabase> IBDatabase = new MemoryDatabase("IBDatabase");
  IBDatabase->putDatabase("IBMethod");
  IBDatabase->putDatabase("IBInitializer");
  IBDatabase->getDatabase("IBMethod")->putString("delta_fcn","IB_4");
  IBDatabase->getDatabase("IBMethod")->putBool("enable_logging", true);
  Pointer<IBMethod> ib_method_ops  = new IBMethod("IBMethod", IBDatabase->getDatabase("IBMethod"));
  
  IBDatabase->getDatabase("IBInitializer")->putInteger("max_levels",max_levels);
  std::vector<std::string> structure_names;
  structure_names.push_back("data_points");
  IBDatabase->getDatabase("IBInitializer")->putDatabase("data_points");
  IBDatabase->getDatabase("IBInitializer")->getDatabase("data_points")->putInteger("level_number",on_level);

  IBDatabase->getDatabase("IBInitializer")->putStringArray("structure_names",&structure_names[0],structure_names.size());
  
  Pointer<IBStandardInitializer> ib_initializer = new IBStandardInitializer("IBInitializer", 
                                  IBDatabase->getDatabase("IBInitializer"));
  
  ib_method_ops->registerLInitStrategy(ib_initializer);
  ib_initializer.setNull();

  Pointer<IBStandardForceGen> ib_force_fcn = new IBStandardForceGen(true);
  ib_method_ops->registerIBLagrangianForceFunction(ib_force_fcn);
  return ib_method_ops;
}

// -----------------------------------------------------------------------------
Pointer<PatchHierarchy<NDIM> > getPatchHierarchy(std::vector<int> &lower, 
                                                 std::vector<int> &upper, 
                                                 std::vector<double> &x_lo, 
                                                 std::vector<double> &x_up,
                                                 int max_levels)
{
  IntVector<NDIM> lo(lower[0],lower[1],lower[2]);
  IntVector<NDIM> up(upper[0],upper[1],upper[2]);
  IntVector<NDIM> periodic_dimension(0);
  BoxList<NDIM> domain(Box<NDIM>(lo,up));
  
  Pointer<CartesianGridGeometry<NDIM> > grid_geometry = new CartesianGridGeometry<NDIM>("CartesianGeometry",&x_lo[0],&x_up[0],domain);
  grid_geometry->initializePeriodicShift(periodic_dimension);
  return new PatchHierarchy<NDIM>("PatchHierarchy", grid_geometry);
}
// -----------------------------------------------------------------------------
Pointer<GriddingAlgorithm<NDIM> > getGriddingAlgorithm(Pointer<IBHierarchyIntegrator> time_integrator, std::vector<blitz::TinyVector<int,3> > &ratios)
{
  Pointer<MemoryDatabase> GridDatabase = new MemoryDatabase("GridDatabase");
  
  GridDatabase->putDatabase("StandardTagAndInitialize");
  GridDatabase->putDatabase("LoadBalancer");
  GridDatabase->putDatabase("GriddingAlgorithm");
  
  GridDatabase->getDatabase("StandardTagAndInitialize")->putString("tagging_method","GRADIENT_DETECTOR");
  
  // load_balancer
  int max_workload_factor = 1;
  GridDatabase->getDatabase("LoadBalancer")->putString("bin_pack_method","SPATIAL");
  GridDatabase->getDatabase("LoadBalancer")->putInteger("max_workload_factor",max_workload_factor);
  
  // gridding_algorithm
  GridDatabase->getDatabase("GriddingAlgorithm")->putInteger("max_levels",ratios.size());
  GridDatabase->getDatabase("GriddingAlgorithm")->putDatabase("ratio_to_coarser");
  GridDatabase->getDatabase("GriddingAlgorithm")->putDatabase("largest_patch_size");
  GridDatabase->getDatabase("GriddingAlgorithm")->putDatabase("smallest_patch_size");
  
  std::vector<blitz::TinyVector<int,3> > level;
  for(int i = 0; i < ratios.size(); ++i)
  {    
    std::stringstream level; 
    level << "level_" << i;
    GridDatabase->getDatabase("GriddingAlgorithm")
      ->getDatabase("ratio_to_coarser")->putIntegerArray(level.str(),ratios[i].data(),3);
  }

  blitz::TinyVector<int,3> level_0(4);
  GridDatabase->getDatabase("GriddingAlgorithm")->getDatabase("smallest_patch_size")->putIntegerArray("level_0",level_0.data(),3);
  level_0[0] = level_0[1] = level_0[2] = 512;
  GridDatabase->getDatabase("GriddingAlgorithm")->getDatabase("largest_patch_size")->putIntegerArray("level_0",level_0.data(),3);
  
  GridDatabase->getDatabase("GriddingAlgorithm")->putDouble("efficiency_tolerance",0.85e0);
  GridDatabase->getDatabase("GriddingAlgorithm")->putDouble("combine_efficiency",0.85e0);
  
  Pointer<StandardTagAndInitialize<NDIM> > tag_init = new StandardTagAndInitialize<NDIM> ("StandardTagAndInitialize", 
                                                          time_integrator, 
                                                          GridDatabase->getDatabase("StandardTagAndInitialize"));
  Pointer<BergerRigoutsos<NDIM> > box_generator     = new BergerRigoutsos<NDIM> ();
  Pointer<LoadBalancer<NDIM> > load_balancer        = new LoadBalancer<NDIM> ("LoadBalancer", GridDatabase->getDatabase("LoadBalancer"));
  return new GriddingAlgorithm<NDIM> ("GriddingAlgorithm", 
                               GridDatabase->getDatabase("GriddingAlgorithm"), 
                               tag_init, 
                               box_generator, 
                               load_balancer);
}

// -----------------------------------------------------------------------------
void amrToVTK(Pointer<PatchHierarchy<NDIM> > patch_hierarchy, 
              vtkSmartPointer<vtkHierarchicalBoxDataSet> dataset)
{
  int num_levels = patch_hierarchy->getFinestLevelNumber();
  dataset->SetNumberOfLevels(num_levels);
  
  Pointer<CartesianGridGeometry<NDIM> > grid_geometry = patch_hierarchy->getGridGeometry();
  const double *X0 = grid_geometry->getXLower();
  
  for (int level_num = 0; level_num < num_levels; ++level_num)
  {
    Pointer<PatchLevel<NDIM> > level = patch_hierarchy->getPatchLevel(level_num);
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
vtkAMRBox getAMRBox(const int lo[3], const int hi[3], const double x[3], const double h[3])
{
  vtkAMRBox box(lo,hi);
  box.SetDataSetOrigin(x);
  box.SetGridSpacing(h);
  return box;  
}
