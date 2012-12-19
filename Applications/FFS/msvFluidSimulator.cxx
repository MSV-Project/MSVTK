// PETSc headers
#include <petscsys.h>
#include <petscvec.h>

// SAMRAI headers
#include <BergerRigoutsos.h>
#include <CartesianGridGeometry.h>
#include <CartesianPatchGeometry.h>
#include <LoadBalancer.h>
#include <Patch.h>
#include <PatchHierarchy.h>
#include <SAMRAI_config.h>
#include <StandardTagAndInitialize.h>
#include <Variable.h>
#include <VariableDatabase.h>

// IBAMR headers
#include <ibtk/LNodeSetData.h>
#include <ibamr/IBStandardSourceGen.h>
#include <ibtk/muParserCartGridFunction.h>
#include <ibtk/muParserRobinBcCoefs.h>
#include <ibamr/IBMethod.h>
#include <ibamr/IBHierarchyIntegrator.h>
#include <ibamr/IBStandardForceGen.h>
#include <ibamr/INSStaggeredHierarchyIntegrator.h>
#include <ibamr/INSCollocatedHierarchyIntegrator.h>
#include <ibamr/app_namespaces.h>

// MSV headers
#include "msvFluidSimulator.h"
#include "msvIBInitializer.h"
#include <msvVTKIBSourceGen.h>
#include "msvAppInitializer.h"

// VTK headers
#include <vtkHierarchicalBoxDataSet.h>
#include <vtkAMRBox.h>
#include <vtkUniformGrid.h>
#include <vtkDoubleArray.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkFeatureEdges.h>
#include <vtkAppendPolyData.h>
#include <vtkCleanPolyData.h>
#include <vtkMath.h>
#include <vtkDelaunay2D.h>
#include <vtkTimerLog.h>
#include <vtkNew.h>

// -----------------------------------------------------------------------------
class msvFluidSimulator::vtkInternal
{
public:
  typedef INSHierarchyIntegrator      INSHierarchyIntegratorType;
  typedef StandardTagAndInitialize<3> StandardTagAndInitializeType;
  typedef BergerRigoutsos<3>          BergerRigoutsosType;
  typedef CartesianGridGeometry<3>    CartesianGridGeometryType;
  typedef PatchHierarchy<3>           PatchHierarchyType;
  typedef LoadBalancer<3>             LoadBalancerType;
  typedef GriddingAlgorithm<3>        GriddingAlgorithmType;
  typedef IBHierarchyIntegrator       IBHierarchyIntegratorType;
  typedef IBMethod                    IBMethodType;
  typedef IBStandardForceGen          IBStandardForceGenType;
  typedef msvVTKIBSourceGen           msvVTKIBSourceGenType;
  typedef msvIBInitializer            IBInitializerType;

  vtkInternal(msvFluidSimulator* external);
  ~vtkInternal();

  // Populate vtk AMR datasets
  void SetPointData(vtkPolyData *polydata);

  // Populate lagrangian datasets
  void GetLagrangianDataSet(vtkPolyData *polydata);

  // Initialize the grid
  void InitializeCartesianGrid(const int lower[3],const int upper[3],
                               const double x_lo[3],
                               const double x_up[3]);

  // Initialize the lagrangian points within the grid
  void InitializeImmersedBoundaryMethod(vtkPolyData *polydata);
  void InitializeBoundaryConditions();

  // Extract velocity and pressure data
  void SetPointData(vtkHierarchicalBoxDataSet *dataset);

  // Create grid vtk hierarchy from SAMRAI data structures
  void SetDataset(vtkHierarchicalBoxDataSet *dataset);

  void SetGridDatabase();
  void SetIBDatabase();
  void SetBCDatabase();

  void Clear();

  // Create a vtkAMRBox with parameters and return resultant
  // box (used to initialize AMR dataset)
  vtkAMRBox GetAMRBox(const int lo[3], const int up[3], const double x[3],
                      const double h[3]);

  Pointer<INSHierarchyIntegratorType>   NavierStokesIntegrator;
  Pointer<IBHierarchyIntegratorType>    TimeIntegrator;
  Pointer<CartesianGridGeometryType>    GridGeometry;
  Pointer<PatchHierarchyType>           HierarchyPatch;
  Pointer<StandardTagAndInitializeType> TagStrategy;
  Pointer<BergerRigoutsosType>          BoxGenerator;
  Pointer<LoadBalancerType>             AMRLoadBalancer;
  Pointer<GriddingAlgorithmType>        AMRGriding;
  Pointer<IBMethodType>                 IbMethod;
  Pointer<IBInitializerType>            IbInitializer;
  Pointer<IBStandardForceGenType>       IbForceFcn;
  Pointer<msvVTKIBSourceGenType>        IbSourceFcn;
  Pointer<MemoryDatabase>               GridDatabase;
  Pointer<MemoryDatabase>               IBDatabase;
  Pointer<MemoryDatabase>               BCDatabase;
  vtkSmartPointer<vtkPolyData>          LagrangianDataset;
  msvFluidSimulator*                    External;
  Vec                                   PetscPositionVector;
  Vec                                   PetscVelocityVector;
  double*                               X;
  double*                               V;
};

// -----------------------------------------------------------------------------
msvFluidSimulator::vtkInternal::vtkInternal(msvFluidSimulator* external)
{
  this->LagrangianDataset   = vtkSmartPointer<vtkPolyData>::New();
  this->External            = external;
  this->PetscPositionVector = NULL;
  this->PetscVelocityVector = NULL;
  this->X                   = NULL;
  this->V                   = NULL;
}


// -----------------------------------------------------------------------------
msvFluidSimulator::vtkInternal::~vtkInternal()
{
  this->Clear();
}

// -----------------------------------------------------------------------------
void msvFluidSimulator::vtkInternal::SetDataset(
  vtkHierarchicalBoxDataSet* dataset)
{
  int numLevels = this->HierarchyPatch->getFinestLevelNumber();
  dataset->SetNumberOfLevels(numLevels);

  Pointer<CartesianGridGeometryType> gridGeometry =
    this->HierarchyPatch->getGridGeometry();

  const double * X0 = gridGeometry->getXLower();

  for (int level_num = 0; level_num < numLevels; ++level_num)
    {
    Pointer<PatchLevel<3> > level = this->HierarchyPatch->getPatchLevel(
      level_num);
    dataset->SetRefinementRatio(level_num,4);

    for(PatchLevel<3>::Iterator p(level); p; p++)
      {
      Pointer<Patch<3> > patch = level->getPatch(
        p());
      Box<3>                              patch_box      = patch->getBox();
      Pointer<CartesianPatchGeometry<3> > patch_geometry =
        patch->getPatchGeometry();

      const double * dx  = patch_geometry->getDx();
      const int *    lo  = patch_box.lower();
      const int *    hi  = patch_box.upper();
      vtkAMRBox      box = this->GetAMRBox(lo,hi,X0,dx);


      vtkSmartPointer<vtkUniformGrid> grid =
        vtkSmartPointer<vtkUniformGrid>::New();
      grid->Initialize(&box);

      int id          = patch->getPatchNumber();
      int patch_level = patch->getPatchLevelNumber();
      dataset->SetDataSet(patch_level,id,box,grid);
      }
    }
}

// -----------------------------------------------------------------------------
void msvFluidSimulator::vtkInternal::SetPointData(
  vtkHierarchicalBoxDataSet* dataset)
{
  if(this->HierarchyPatch.isNull() || this->NavierStokesIntegrator.isNull() ||
     this->GridGeometry.isNull() || !dataset)
    {
    return;
    }

  typedef VariableDatabase<3> VariableDatabase;
  typedef Variable<3>         Variable;

  VariableDatabase * variableDatabase = VariableDatabase::getDatabase();

  Pointer<Variable> velocityVariable =
    this->NavierStokesIntegrator->getVelocityVariable();
  Pointer<Variable> pressureVariable =
    this->NavierStokesIntegrator->getPressureVariable();

  int velocityIdx =
    variableDatabase->mapVariableAndContextToIndex(velocityVariable,
      this->NavierStokesIntegrator->getCurrentContext());
  int pressureIdx =
    variableDatabase->mapVariableAndContextToIndex(pressureVariable,
      this->NavierStokesIntegrator->getCurrentContext());

  int num_levels = this->HierarchyPatch->getFinestLevelNumber();

  vtkCompositeDataIterator *datasetIterator = dataset->NewIterator();
  for (int level_num = 0; level_num < num_levels; ++level_num)
    {
    Pointer<PatchLevel<3> > level = HierarchyPatch->getPatchLevel(level_num);
    for(PatchLevel<3>::Iterator p(level); p; p++)
      {
      Pointer<Patch<3> > patch = level->getPatch(p());

      Box<3>                              patch_box      = patch->getBox();
      Pointer<CartesianPatchGeometry<3> > patch_geometry =
        patch->getPatchGeometry();

      vtkAMRBox        box;
      vtkUniformGrid * grid =
        dataset->GetDataSet(level,patch->getPatchNumber(),box);

      ArrayData<3,double> &velocity_data =
        static_cast<CellData<3,
                             double>*>(patch->getPatchData(velocityIdx).
                                         getPointer())->getArrayData();
      ArrayData<3,double> &pressure_data =
        static_cast<CellData<3,
                             double>*>(patch->getPatchData(pressureIdx).
                                         getPointer())->getArrayData();

      // Set pressure array
      vtkNew<vtkDoubleArray> pressure;
      pressure->SetArray(pressure_data.getPointer(),
        pressure_data.getDepth()*pressure_data.getOffset(),1);
      pressure->SetNumberOfComponents(pressure_data.getDepth());
      pressure->SetName("pressure");
      grid->GetPointData()->AddArray(pressure.GetPointer());

      // Set velocity arrays
      const int ncomponents = velocity_data.getDepth();

      std::string name[] =
        {
        "velocity_x",
        "velocity_y",
        "velocity_z"
        };

      for (int i = 0; i < ncomponents; ++i)
        {
        int                    offset = velocity_data.getOffset();
        vtkNew<vtkDoubleArray> velocities;
        velocities->SetArray(velocity_data.getPointer(), offset,1);
        velocities->SetNumberOfComponents(ncomponents);
        velocities->SetName(name[i].c_str());
        grid->GetPointData()->AddArray(velocities.GetPointer());
        }

      }
    }
}


// -----------------------------------------------------------------------------
void msvFluidSimulator::vtkInternal::Clear()
{

}

// -----------------------------------------------------------------------------
vtkAMRBox msvFluidSimulator::vtkInternal::GetAMRBox(const int    lo[3],
                                                    const int    hi[3],
                                                    const double x[3],
                                                    const double h[3])
{
  vtkAMRBox box(lo,hi);
  box.SetDataSetOrigin(x);
  box.SetGridSpacing(h);
  return box;
}

// -----------------------------------------------------------------------------
void msvFluidSimulator::vtkInternal::SetPointData(vtkPolyData *polydata)
{
  if(this->HierarchyPatch.isNull() || this->GridGeometry.isNull() || !polydata)
    {
    return;
    }

  const double *X0         = this->GridGeometry->getXLower();
  int           num_levels = this->HierarchyPatch->getFinestLevelNumber();

  this->External->AMRDataset->SetNumberOfLevels(num_levels);

  for (int level_num = 0; level_num < num_levels; ++level_num)
    {
    Pointer<PatchLevel<3> > level =
      HierarchyPatch->getPatchLevel(level_num);
    this->External->AMRDataset->SetRefinementRatio(level_num,4);

    for(PatchLevel<3>::Iterator p(level); p; p++)
      {
      Pointer<Patch<3> > patch = level->getPatch(p());

      Pointer<CartesianPatchGeometry<3> > patch_geometry =
        patch->getPatchGeometry();
      Box<3> patch_box = patch->getBox();

      const double * dx  = patch_geometry->getDx();
      const int *    lo  = patch_box.lower();
      const int *    hi  = patch_box.upper();
      vtkAMRBox      box = this->GetAMRBox(lo,hi,X0,dx);

      vtkNew<vtkUniformGrid> grid;
      grid->Initialize(&box);
      int id          = patch->getPatchNumber();
      int patch_level = patch->getPatchLevelNumber();
      this->External->AMRDataset->SetDataSet(patch_level,id,box,
        grid.GetPointer());
      }
    }
  this->GetLagrangianDataSet(polydata);
}


// -----------------------------------------------------------------------------
void msvFluidSimulator::vtkInternal::GetLagrangianDataSet(
  vtkPolyData *polydata)
{
  if(this->HierarchyPatch.isNull() || this->IbMethod.isNull() || !polydata)
    {
    return;
    }
  int size;
  int num_levels = this->HierarchyPatch->getFinestLevelNumber();

  LDataManager * lagrangianDataManager = this->IbMethod->getLDataManager();

  Pointer<LData> X_data = lagrangianDataManager->getLData("X", num_levels);
  Pointer<LData> V_data = lagrangianDataManager->getLData("U", num_levels);

  Vec X_petsc_vec = X_data->getVec();
  Vec V_petsc_vec = V_data->getVec();

  VecDuplicate(X_petsc_vec, &this->PetscPositionVector);
  VecDuplicate(V_petsc_vec, &this->PetscVelocityVector);

  lagrangianDataManager->scatterPETScToLagrangian(X_petsc_vec,
    this->PetscPositionVector,
    num_levels);
  lagrangianDataManager->scatterPETScToLagrangian(V_petsc_vec,
    this->PetscVelocityVector,
    num_levels);

  VecGetArray(this->PetscPositionVector,&X);
  VecGetArray(this->PetscVelocityVector,&V);
  VecGetSize(this->PetscPositionVector,&size);

  vtkSmartPointer<vtkDoubleArray> positions =
    vtkSmartPointer<vtkDoubleArray>::New();
  vtkSmartPointer<vtkDoubleArray> velocity =
    vtkSmartPointer<vtkDoubleArray>::New();
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

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


// -----------------------------------------------------------------------------
void msvFluidSimulator::vtkInternal::InitializeCartesianGrid(
  const int    lower[3],
  const int    upper[3],
  const double x_lo[3],
  const double x_up[3])
{
  if(this->GridDatabase.isNull() || this->TimeIntegrator.isNull())
    {
    return;
    }
  IntVector<3> lo(lower[0],lower[1],lower[2]);
  IntVector<3> up(upper[0],upper[1],upper[2]);
  IntVector<3> periodic_dimension(0);
  BoxList<3>   domain(Box<3>(lo,up));

  this->GridGeometry = new CartesianGridGeometryType("CartesianGeometry",x_lo,
    x_up,domain);
  this->GridGeometry->initializePeriodicShift(periodic_dimension);

  this->HierarchyPatch = new PatchHierarchyType("HierarchyPatch",
    this->GridGeometry);
  this->BoxGenerator = new BergerRigoutsosType();

  Pointer<Database> tagStrategyDB = this->GridDatabase->getDatabase(
    "StandardTagAndInitialize");
  Pointer<Database> griddingDB = this->GridDatabase->getDatabase(
    "GriddingAlgorithm");
  Pointer<Database> loadBalancerDB = this->GridDatabase->getDatabase(
    "LoadBalancer");

  this->AMRLoadBalancer = new LoadBalancerType("LoadBalancer",loadBalancerDB);
  this->TagStrategy     = new StandardTagAndInitializeType(
    "StandardTagAndInitialize",this->TimeIntegrator,tagStrategyDB);
  this->AMRGriding = new GriddingAlgorithmType("AMRGriding",
    griddingDB,
    this->TagStrategy,
    this->BoxGenerator,
    this->AMRLoadBalancer);

  vtkSmartPointer<vtkTimerLog> timer = vtkSmartPointer<vtkTimerLog>::New();
  //   Initialize hierarchy configuration and data on all patches.
  timer->StartTimer();
  this->TimeIntegrator->initializePatchHierarchy(this->HierarchyPatch,
    this->AMRGriding);
  timer->StopTimer();
  std::cout << "init = " << timer->GetElapsedTime() << std::endl;
//   timer->StartTimer();
//   this->TimeIntegrator->regridHierarchy();
//   timer->StopTimer();
//   std::cout << "regrid = " << timer->GetElapsedTime() << std::endl;
}

// -----------------------------------------------------------------------------
void msvFluidSimulator::vtkInternal::InitializeBoundaryConditions()
{
  if(this->BCDatabase.isNull() || this->GridGeometry.isNull() ||
     this->NavierStokesIntegrator.isNull())
    {
    return;
    }
  // Create Eulerian boundary condition specification objects (when necessary).
  const IntVector<3>& periodic_shift =
    this->GridGeometry->getPeriodicShift();
  TinyVector<RobinBcCoefStrategy<3>*,3> u_bc_coefs;
  if (periodic_shift.min() > 0)
    {
    for (unsigned int d = 0; d < 3; ++d)
      {
      u_bc_coefs[d] = NULL;
      }
    }
  else
    {
    for (unsigned int d = 0; d < 3; ++d)
      {
      ostringstream bc_coefs_name_stream;
      bc_coefs_name_stream << "u_bc_coefs_" << d;
      const string bc_coefs_name = bc_coefs_name_stream.str();

      ostringstream bc_coefs_db_name_stream;
      bc_coefs_db_name_stream << "VelocityBcCoefs_" << d;
      const string bc_coefs_db_name = bc_coefs_db_name_stream.str();

      u_bc_coefs[d] = new muParserRobinBcCoefs(bc_coefs_name,
        this->BCDatabase->getDatabase(bc_coefs_db_name), this->GridGeometry);
      }
    this->NavierStokesIntegrator->registerPhysicalBoundaryConditions(u_bc_coefs);
    }
}

// -----------------------------------------------------------------------------
void msvFluidSimulator::vtkInternal::InitializeImmersedBoundaryMethod(
  vtkPolyData *polydata)
{
  if(this->IBDatabase.isNull())
    {
    return;
    }

  this->IbMethod =
    new IBMethodType("IBMethod", this->IBDatabase->getDatabase("IBMethod"));

  this->IbInitializer = new msvIBInitializer("msvIBInitializer",
    this->IBDatabase->getDatabase("msvIBInitializer"),polydata);

  this->IbMethod->registerLInitStrategy(this->IbInitializer);

  this->IbSourceFcn = new msvVTKIBSourceGenType;
  this->IbMethod->registerIBLagrangianSourceFunction(this->IbSourceFcn);
  this->IbForceFcn = new IBStandardForceGen(true);
  this->IbMethod->registerIBLagrangianForceFunction(this->IbForceFcn);
}

// -----------------------------------------------------------------------------
void msvFluidSimulator::vtkInternal::SetGridDatabase()
{
  if(this->GridDatabase.isNull())
    {
    this->GridDatabase = new MemoryDatabase("GridDatabase");
    }
  this->GridDatabase->putDatabase("StandardTagAndInitialize");
  this->GridDatabase->putDatabase("LoadBalancer");
  this->GridDatabase->putDatabase("GriddingAlgorithm");

  // Tagging stategy used by the gridding algorithm
  Pointer<Database> tagStrategyDB = this->GridDatabase->getDatabase(
    "StandardTagAndInitialize");
  tagStrategyDB->putString("tagging_method","GRADIENT_DETECTOR");

  // LoadBalancer
  Pointer<Database> loadBalancerDB = this->GridDatabase->getDatabase(
    "LoadBalancer");
  int max_workload_factor = 1;
  loadBalancerDB->putString("bin_pack_method","SPATIAL");
  loadBalancerDB->putInteger("max_workload_factor",max_workload_factor);

  // GriddingAlgorithm
  Pointer<Database> griddingDB = this->GridDatabase->getDatabase(
    "GriddingAlgorithm");
  griddingDB->putInteger("max_levels",this->External->MaxLevels);
  griddingDB->putDatabase("ratio_to_coarser");
  griddingDB->putDatabase("largest_patch_size");
  griddingDB->putDatabase("smallest_patch_size");
  griddingDB->putDouble("efficiency_tolerance",0.5e0);
  griddingDB->putDouble("combine_efficiency",0.5e0);

  Pointer<Database> griddingRatiosDB = griddingDB->getDatabase(
    "ratio_to_coarser");

  int levelRefineamentRatio[3] =
    {
    this->External->RefinamentRatio,
    this->External->RefinamentRatio,
    this->External->RefinamentRatio
    };
  for(int i = 1; i < this->External->MaxLevels; ++i)
    {
    std::stringstream level;
    level << "level_" << i;
    griddingRatiosDB->putIntegerArray(
      level.str().c_str(),levelRefineamentRatio,3);
    }

  Pointer<Database> largestPatchDB = griddingDB->getDatabase(
    "largest_patch_size");
  largestPatchDB->putIntegerArray("level_0",this->External->LargestPatch,3);
  // all finer levels will use same values as level_0...

  Pointer<Database> smallestPatchDB = griddingDB->getDatabase(
    "smallest_patch_size");
  smallestPatchDB->putIntegerArray("level_0",this->External->SmallestPatch,3);
  // all finer levels will use same values as level_0...
}

// -----------------------------------------------------------------------------
void msvFluidSimulator::vtkInternal::SetIBDatabase()
{
  if(this->IBDatabase.isNull())
    {
    this->IBDatabase = new MemoryDatabase("IBDatabase");
    }
  IBDatabase->putDatabase("IBMethod");
  IBDatabase->putDatabase("msvIBInitializer");

  Pointer<Database> IBMethodDB = IBDatabase->getDatabase("IBMethod");

  IBMethodDB->putString("delta_fcn","IB_4");
  IBMethodDB->putBool("enable_logging", true);

  Pointer<Database> IBInitializerDB =
    IBDatabase->getDatabase("msvIBInitializer");

  std::string structureName = "aneurysm";
  IBInitializerDB->putStringArray("structure_names",&structureName,1);
  IBInitializerDB->putInteger("max_levels",this->External->MaxLevels);
  IBInitializerDB->putDatabase("aneurysm");

  Pointer<Database> AneurysmDB = IBInitializerDB->getDatabase("aneurysm");

  AneurysmDB->putInteger("level_number",this->External->DataLevel);
  AneurysmDB->putBool("enable_springs",false);
  AneurysmDB->putBool("enable_beams",false);
  AneurysmDB->putBool("enable_rods",false);
  AneurysmDB->putBool("enable_target_points",false);
  AneurysmDB->putBool("enable_anchor_points",false);
  AneurysmDB->putBool("enable_bdry_mass",false);
  AneurysmDB->putBool("enable_instrumentation",false);
  AneurysmDB->putBool("enable_sources",false);
}

// -----------------------------------------------------------------------------
void msvFluidSimulator::vtkInternal::SetBCDatabase()
{
  if(this->BCDatabase.isNull())
    {
    this->BCDatabase = new MemoryDatabase("BCDatabase");
    }
  // Set a Dirichlet boundary conditions for the pressure solver
  double VelocityBcCoefs[3][18] =
    {
         {1.0,1.0,1.0,1.0,1.0,1.0,.0,.0,.0,.0,.0,.0,.0,.0,.0,.0,.0,.0},
         {1.0,1.0,1.0,1.0,1.0,1.0,.0,.0,.0,.0,.0,.0,.0,.0,.0,.0,.0,.0},
         {1.0,1.0,1.0,1.0,1.0,1.0,.0,.0,.0,.0,.0,.0,.0,.0,.0,.0,.0,.0}
    };
  BCDatabase->putDatabase("VelocityBcCoefs_0");
  BCDatabase->putDatabase("VelocityBcCoefs_1");
  BCDatabase->putDatabase("VelocityBcCoefs_2");
  // X coeff
  Pointer<Database> VelocityBcCoefsX = BCDatabase->getDatabase(
    "VelocityBcCoefs_0");
  VelocityBcCoefsX->putDouble("acoef_function_0",VelocityBcCoefs[0][0]);
  VelocityBcCoefsX->putDouble("acoef_function_0",VelocityBcCoefs[0][1]);
  VelocityBcCoefsX->putDouble("acoef_function_0",VelocityBcCoefs[0][2]);
  VelocityBcCoefsX->putDouble("acoef_function_0",VelocityBcCoefs[0][3]);
  VelocityBcCoefsX->putDouble("acoef_function_0",VelocityBcCoefs[0][4]);
  VelocityBcCoefsX->putDouble("acoef_function_0",VelocityBcCoefs[0][5]);
  VelocityBcCoefsX->putDouble("bcoef_function_0",VelocityBcCoefs[0][6]);
  VelocityBcCoefsX->putDouble("bcoef_function_0",VelocityBcCoefs[0][7]);
  VelocityBcCoefsX->putDouble("bcoef_function_0",VelocityBcCoefs[0][8]);
  VelocityBcCoefsX->putDouble("bcoef_function_0",VelocityBcCoefs[0][9]);
  VelocityBcCoefsX->putDouble("bcoef_function_0",VelocityBcCoefs[0][10]);
  VelocityBcCoefsX->putDouble("bcoef_function_0",VelocityBcCoefs[0][11]);
  VelocityBcCoefsX->putDouble("gcoef_function_0",VelocityBcCoefs[0][12]);
  VelocityBcCoefsX->putDouble("gcoef_function_0",VelocityBcCoefs[0][13]);
  VelocityBcCoefsX->putDouble("gcoef_function_0",VelocityBcCoefs[0][14]);
  VelocityBcCoefsX->putDouble("gcoef_function_0",VelocityBcCoefs[0][15]);
  VelocityBcCoefsX->putDouble("gcoef_function_0",VelocityBcCoefs[0][16]);
  VelocityBcCoefsX->putDouble("gcoef_function_0",VelocityBcCoefs[0][17]);
  // Y coeff
  Pointer<Database> VelocityBcCoefsY = BCDatabase->getDatabase(
    "VelocityBcCoefs_1");
  VelocityBcCoefsY->putDouble("acoef_function_1",VelocityBcCoefs[1][0]);
  VelocityBcCoefsY->putDouble("acoef_function_1",VelocityBcCoefs[1][1]);
  VelocityBcCoefsY->putDouble("acoef_function_1",VelocityBcCoefs[1][2]);
  VelocityBcCoefsY->putDouble("acoef_function_1",VelocityBcCoefs[1][3]);
  VelocityBcCoefsY->putDouble("acoef_function_1",VelocityBcCoefs[1][4]);
  VelocityBcCoefsY->putDouble("acoef_function_1",VelocityBcCoefs[1][5]);
  VelocityBcCoefsY->putDouble("bcoef_function_1",VelocityBcCoefs[1][6]);
  VelocityBcCoefsY->putDouble("bcoef_function_1",VelocityBcCoefs[1][7]);
  VelocityBcCoefsY->putDouble("bcoef_function_1",VelocityBcCoefs[1][8]);
  VelocityBcCoefsY->putDouble("bcoef_function_1",VelocityBcCoefs[1][9]);
  VelocityBcCoefsY->putDouble("bcoef_function_1",VelocityBcCoefs[1][10]);
  VelocityBcCoefsY->putDouble("bcoef_function_1",VelocityBcCoefs[1][11]);
  VelocityBcCoefsY->putDouble("gcoef_function_1",VelocityBcCoefs[1][12]);
  VelocityBcCoefsY->putDouble("gcoef_function_1",VelocityBcCoefs[1][13]);
  VelocityBcCoefsY->putDouble("gcoef_function_1",VelocityBcCoefs[1][14]);
  VelocityBcCoefsY->putDouble("gcoef_function_1",VelocityBcCoefs[1][15]);
  VelocityBcCoefsY->putDouble("gcoef_function_1",VelocityBcCoefs[1][16]);
  VelocityBcCoefsY->putDouble("gcoef_function_1",VelocityBcCoefs[1][17]);
  // Z coeff
  Pointer<Database> VelocityBcCoefsZ = BCDatabase->getDatabase(
    "VelocityBcCoefs_2");
  VelocityBcCoefsZ->putDouble("acoef_function_2",VelocityBcCoefs[2][0]);
  VelocityBcCoefsZ->putDouble("acoef_function_2",VelocityBcCoefs[2][1]);
  VelocityBcCoefsZ->putDouble("acoef_function_2",VelocityBcCoefs[2][2]);
  VelocityBcCoefsZ->putDouble("acoef_function_2",VelocityBcCoefs[2][3]);
  VelocityBcCoefsZ->putDouble("acoef_function_2",VelocityBcCoefs[2][4]);
  VelocityBcCoefsZ->putDouble("acoef_function_2",VelocityBcCoefs[2][5]);
  VelocityBcCoefsZ->putDouble("bcoef_function_2",VelocityBcCoefs[2][6]);
  VelocityBcCoefsZ->putDouble("bcoef_function_2",VelocityBcCoefs[2][7]);
  VelocityBcCoefsZ->putDouble("bcoef_function_2",VelocityBcCoefs[2][8]);
  VelocityBcCoefsZ->putDouble("bcoef_function_2",VelocityBcCoefs[2][9]);
  VelocityBcCoefsZ->putDouble("bcoef_function_2",VelocityBcCoefs[2][10]);
  VelocityBcCoefsZ->putDouble("bcoef_function_2",VelocityBcCoefs[2][11]);
  VelocityBcCoefsZ->putDouble("gcoef_function_2",VelocityBcCoefs[2][12]);
  VelocityBcCoefsZ->putDouble("gcoef_function_2",VelocityBcCoefs[2][13]);
  VelocityBcCoefsZ->putDouble("gcoef_function_2",VelocityBcCoefs[2][14]);
  VelocityBcCoefsZ->putDouble("gcoef_function_2",VelocityBcCoefs[2][15]);
  VelocityBcCoefsZ->putDouble("gcoef_function_2",VelocityBcCoefs[2][16]);
  VelocityBcCoefsZ->putDouble("gcoef_function_2",VelocityBcCoefs[2][17]);
}

//------------------------------------------------------------------------------
//
vtkStandardNewMacro(msvFluidSimulator);

//------------------------------------------------------------------------------
vtkSetObjectImplementationMacro(msvFluidSimulator,AMRDataset,
  vtkHierarchicalBoxDataSet);

// -----------------------------------------------------------------------------
msvFluidSimulator::msvFluidSimulator()
{
  this->AMRDataset          = vtkHierarchicalBoxDataSet::New();
  this->CoarsestGridSpacing = 8;
  this->MaxLevels           = 5;
  this->RefinamentRatio     = 2;
  this->FinestGridSpacing   = this->CoarsestGridSpacing << (this->MaxLevels-1);
  this->DataLevel           = this->MaxLevels-1;
  this->FluidDensity        = 1.0;
  this->FluidViscosity      = 0.005;
  this->CFLCondition        = 0.975;

  PetscInitializeNoArguments();
  SAMRAI_MPI::setCommunicator(PETSC_COMM_WORLD);
  SAMRAI_MPI::setCallAbortInSerialInsteadOfExit();
  SAMRAIManager::startup();
  this->Internal = new vtkInternal(this);

  this->InitFile         = NULL;
  this->SmallestPatch[0] = this->SmallestPatch[1] = this->SmallestPatch[2] = 0;
  this->LargestPatch[0]  = this->LargestPatch[1] = this->LargestPatch[2] = 512;
}

// -----------------------------------------------------------------------------
msvFluidSimulator::~msvFluidSimulator()
{
  SAMRAIManager::shutdown();
  PetscFinalize();
  this->AMRDataset->Delete();
  delete this->Internal;
}

// -----------------------------------------------------------------------------
void msvFluidSimulator::Init(vtkPolyData *polydata)
{
  if(this->MaxLevels < this->DataLevel)
    {
    vtkErrorMacro(<< "Data level is bigger then the number of levels.");
    return;
    }

  if(!this->InitFile)
    {
    vtkErrorMacro(<< "Missing fluid solver configuration file.");
    return;
    }

  Pointer<msvAppInitializer> app_initializer = new msvAppInitializer(
    this->InitFile);

  Pointer<Database> fluidSolverDB = app_initializer->getComponentDatabase(
    "INSStaggeredHierarchyIntegrator");

  fluidSolverDB->putDouble("cfl",this->CFLCondition);
  fluidSolverDB->putDouble("mu",this->FluidViscosity);
  fluidSolverDB->putDouble("rho",this->FluidDensity);

  // Create a Navier-Stokes solver on a staggered grid hierachy
  this->Internal->NavierStokesIntegrator
    = new INSStaggeredHierarchyIntegrator("INSStaggeredHierarchyIntegrator",
    fluidSolverDB);

  // Initialize immersed boundary
  this->Internal->SetIBDatabase();
  this->Internal->InitializeImmersedBoundaryMethod(polydata);

  // Create a time-stepping method for the immersed boundary
  this->Internal->TimeIntegrator
    = new IBHierarchyIntegrator("IBHierarchyIntegrator",
    app_initializer->getComponentDatabase("IBHierarchyIntegrator"),
    this->Internal->IbMethod,
    this->Internal->NavierStokesIntegrator);

  int    lower[3] = {0}, upper[3] = {0};
  double x_lo[3]  = {0}, x_up[3] = {0};
  double extent[6];

  upper[0] = upper[1] = upper[2] = this->CoarsestGridSpacing-1;

  polydata->GetBounds(extent);

  x_lo[0] = extent[0]-1;
  x_lo[1] = extent[2]-1;
  x_lo[2] = extent[4]-1;
  x_up[0] = extent[1]+1;
  x_up[1] = extent[3]+1;
  x_up[2] = extent[5]+1;

  // Initialize Cartesian Geometry
  this->Internal->SetGridDatabase();
  this->Internal->InitializeCartesianGrid(lower,upper,x_lo,x_up);

  this->Internal->SetBCDatabase();
  this->Internal->InitializeBoundaryConditions();
}

// -----------------------------------------------------------------------------
void msvFluidSimulator::SetCoarsestGridSpacing(int spacing)
{
  if(this->CoarsestGridSpacing != spacing)
  {
    this->CoarsestGridSpacing = spacing;
    this->FinestGridSpacing   = this->CoarsestGridSpacing << (this->MaxLevels-1);
    this->Modified();
  }
}

// -----------------------------------------------------------------------------
void msvFluidSimulator::Run()
{
  double dt = this->Internal->TimeIntegrator->getTimeStepSize();
  this->Internal->TimeIntegrator->advanceHierarchy(dt);
}

// -----------------------------------------------------------------------------
void msvFluidSimulator::SetDataSet()
{
  this->Internal->SetDataset(this->AMRDataset);
}

// -----------------------------------------------------------------------------
void msvFluidSimulator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "InitFile: " << this->InitFile << "\n";
  os << indent << "CoarsestGridSpacing: " << this->CoarsestGridSpacing << "\n";
  os << indent << "MaxLevels: " << this->MaxLevels << "\n";
  os << indent << "AMRDataset: " << this->AMRDataset << "\n";
}
