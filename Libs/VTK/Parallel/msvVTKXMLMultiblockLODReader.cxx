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

// VTK includes
#include "vtkCompositeDataPipeline.h"
#include "vtkCompositeDataSet.h"
#include "vtkDataArraySelection.h"
#include "vtkDataSet.h"
#include "vtkDemandDrivenPipeline.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkInstantiator.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkMultiPieceDataSet.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkXMLDataElement.h"
#include "vtkXMLDataParser.h"

#include "vtkXMLPolyDataReader.h"
#include "vtkXMLUnstructuredGridReader.h"
#include "vtkXMLRectilinearGridReader.h"
#include "vtkXMLStructuredGridReader.h"

#include <vtksys/SystemTools.hxx>

// STD includes
#include <assert.h>
#include <map>
#include <set>
#include <vector>

// MSVTK includes
#include "msvVTKXMLMultiblockLODReader.h"
#include "msvVTKPolyDataFileSeriesReader.h"

struct vtkXMLCompositeDataReaderEntry
{
  const char* extension;
  const char* name;
};

//------------------------------------------------------------------------------
class msvVTKXMLMultiblockLODReaderInternal
{
public:
  msvVTKXMLMultiblockLODReaderInternal();
  ~msvVTKXMLMultiblockLODReaderInternal();

  void InitListUpdateNodes(vtkXMLDataElement* rootElement);
  void InitListUpdateNodes(vtkXMLDataElement* root,
                           unsigned int lodTreeLevel,
                           unsigned int defaultLOD,
                           int parent = 0);

  void UpdateNodesFromDefaultLOD(vtkXMLDataElement* root,
                                 unsigned int LODLevel,
                                 unsigned int lod,
                                 int pieceIndex = -1);

  static unsigned int CountNumberOfNodes(vtkXMLDataElement* root);
  static int GetLevel(vtkXMLDataElement* node);
  int GetFatherIndex(unsigned int nodeIndex);
  int GetFatherLOD(unsigned int nodeIndex);

  // Use to know if a child of an LOD has to be considered or not
  // bool ShouldReadNode(vtkXMLDataElement* node, int CurrentLODTreeLevel);

  vtkXMLReader* GetReaderOfType(const char* type);

  // The information is stocked in this vector is as the follow:
  // The index of the vector corresponds to the flatIndex of the tree.
  // The int information is described as the follow:
  // - If the node is a composite which is the piece which has the LODs
  //   (At level-1), the it is the LOD set
  // - If the node is a leaf, then the number correspond to his fatherIndex
  struct NodeInfos
  {
    int MixedIndexFatherLOD;
    vtkSmartPointer<vtkXMLReader> Reader;
  };
  typedef std::vector<NodeInfos> NodesInfosType;
  NodesInfosType NodesInfos;

  bool RequestUpdateInformation;
  unsigned int CurrentFlatIndex;

  int DefaultLOD;          // The level of detail by default.
  int CurrentLODTreeLevel; // At which level the node are considered as LOD.
};

//------------------------------------------------------------------------------
int msvVTKXMLMultiblockLODReaderInternal::GetFatherIndex(unsigned int nodeIndex)
{
  assert(nodeIndex < this->NodesInfos.size());
  return this->NodesInfos[nodeIndex].MixedIndexFatherLOD;
}

//------------------------------------------------------------------------------
int msvVTKXMLMultiblockLODReaderInternal::GetFatherLOD(unsigned int nodeIndex)
{
  assert(nodeIndex < this->NodesInfos.size());
  return this->NodesInfos[
    this->NodesInfos[nodeIndex].MixedIndexFatherLOD].MixedIndexFatherLOD;
}

//------------------------------------------------------------------------------
// msvVTKXMLMultiblockLODReaderInternal methods

//------------------------------------------------------------------------------
msvVTKXMLMultiblockLODReaderInternal::
msvVTKXMLMultiblockLODReaderInternal()
{
  this->RequestUpdateInformation = true;
  this->CurrentFlatIndex = 0;

  // Read Only the low level of resolution by default
  this->DefaultLOD = 0;
  // By default the LOD definition are in the second level of the tree
  this->CurrentLODTreeLevel = 2;
}

//------------------------------------------------------------------------------
msvVTKXMLMultiblockLODReaderInternal::
~msvVTKXMLMultiblockLODReaderInternal()
{

}

//------------------------------------------------------------------------------
void msvVTKXMLMultiblockLODReaderInternal::InitListUpdateNodes(vtkXMLDataElement* primaryElement)
{
  unsigned int numberOfNodes = msvVTKXMLMultiblockLODReaderInternal::
    CountNumberOfNodes(primaryElement);

  // Default values
  msvVTKXMLMultiblockLODReaderInternal::NodeInfos nodeInfos = {-1,0};
  this->NodesInfos.assign(numberOfNodes, nodeInfos);
  this->CurrentFlatIndex = 0; // We reset the flatIndex
  this->InitListUpdateNodes(primaryElement,
                            this->CurrentLODTreeLevel,
                            this->DefaultLOD);
  this->RequestUpdateInformation = false;
}

//------------------------------------------------------------------------------
void msvVTKXMLMultiblockLODReaderInternal::InitListUpdateNodes(
  vtkXMLDataElement* element,
  unsigned int lodTreeLevel,
  unsigned int defaultLOD,
  int parent)
{
  if (element == 0)
    {
    return;
    }

  const char* tag = element->GetName();
  if (strcmp(tag, "vtkMultiBlockDataSet") != 0 &&
        strcmp(tag, "Block") != 0 && strcmp(tag, "Piece") != 0)
    {
    return;
    }

  unsigned int maxElems = element->GetNumberOfNestedElements();
  for (unsigned int cc=0; cc < maxElems; ++cc)
    {
    vtkXMLDataElement* childXML = element->GetNestedElement(cc);
    if (!childXML ||
        (strcmp(childXML->GetName(), "DataSet") != 0 &&
         strcmp(childXML->GetName(), "Block") != 0 &&
         strcmp(childXML->GetName(), "Piece") != 0))
      {
      continue;
      }

    this->CurrentFlatIndex++;

    // Father LOD level
    if (msvVTKXMLMultiblockLODReaderInternal::GetLevel(childXML) ==
        static_cast<int>(lodTreeLevel)-1)
      {
      // Clamp the LOD required to the available one
      unsigned int clampLOD = std::min(defaultLOD,
        static_cast<unsigned int>(childXML->GetNumberOfNestedElements())-1);

      this->NodesInfos[this->CurrentFlatIndex].MixedIndexFatherLOD = clampLOD;
      }
    // The node is not at the level just above the LODs
    else
      {
      this->NodesInfos[this->CurrentFlatIndex].MixedIndexFatherLOD = parent;
      }

    // Child is a block or a multipiece
    const char* tagName = childXML->GetName();
    if (strcmp(tagName, "Block") == 0 || strcmp(tagName, "Piece") == 0)
      {
      this->InitListUpdateNodes(childXML,
                                lodTreeLevel,
                                defaultLOD,
                                this->CurrentFlatIndex);
      }
    }
}

//------------------------------------------------------------------------------
void msvVTKXMLMultiblockLODReaderInternal::UpdateNodesFromDefaultLOD(
  vtkXMLDataElement* element,
  unsigned int lodTreeLevel,
  unsigned int lod,
  int pieceIndex)
{
  if (element == 0)
    {
    return;
    }
  const char* tag = element->GetName();
  if (strcmp(tag, "vtkMultiBlockDataSet") != 0 &&
        strcmp(tag, "Block") != 0 && strcmp(tag, "Piece") != 0)
    {
    return;
    }

  unsigned int maxElems = element->GetNumberOfNestedElements();
  for (unsigned int cc=0; cc < maxElems; ++cc)
    {
    vtkXMLDataElement* childXML = element->GetNestedElement(cc);
    if (!childXML ||
        (strcmp(childXML->GetName(), "DataSet") != 0 &&
         strcmp(childXML->GetName(), "Block") != 0 &&
         strcmp(childXML->GetName(), "Piece") != 0))
      {
      continue;
      }

    this->CurrentFlatIndex++;

    // Parents of the LOD definitions - We set the information
    // If there is a specific piece which need to change but not the current
    // one we just continue to go through the tree
    if (msvVTKXMLMultiblockLODReaderInternal::GetLevel(childXML) ==
        static_cast<int>(lodTreeLevel)-1 &&
        ((pieceIndex >= 0 && static_cast<unsigned int>(pieceIndex) == cc) ||
         pieceIndex == -1))
      {
      // Clamp the LOD required to the available one
      unsigned int clampLOD = std::min(lod,
        static_cast<unsigned int>(childXML->GetNumberOfNestedElements())-1);

      this->NodesInfos[this->CurrentFlatIndex].MixedIndexFatherLOD = clampLOD;
      }

    // Child is a multiblock dataset itself or a multipiece
    const char* tagName = childXML->GetName();
    if (strcmp(tagName, "Block") == 0 || strcmp(tagName, "Piece") == 0)
      {
      this->UpdateNodesFromDefaultLOD(childXML, lodTreeLevel, lod);
      }
    }
}

//------------------------------------------------------------------------------
unsigned int msvVTKXMLMultiblockLODReaderInternal::
CountNumberOfNodes(vtkXMLDataElement* node)
{
  if (node == 0)
    {
    return 0;
    }

  const char* tagName = node->GetName();
  if (strcmp(tagName, "DataSet") == 0)
    {
    return 1;
    }

  // Neither a Block, a Piece nor a MultiBlock, we ignore the node
  else if (strcmp(tagName, "vtkMultiBlockDataSet") != 0 &&
      strcmp(tagName, "Block") != 0 && strcmp(tagName, "Piece") != 0)
    {
    return 0;
    }

  unsigned int numberOfSubNodes = 1;
  unsigned int maxElems = node->GetNumberOfNestedElements();
  for (unsigned int cc=0; cc < maxElems; ++cc)
    {
    vtkXMLDataElement* childXML = node->GetNestedElement(cc);
    if (!childXML)
      {
      continue;
      }

    numberOfSubNodes +=
      msvVTKXMLMultiblockLODReaderInternal::CountNumberOfNodes(childXML);
    }

  return numberOfSubNodes;
}

//------------------------------------------------------------------------------
int msvVTKXMLMultiblockLODReaderInternal::
GetLevel(vtkXMLDataElement* node)
{
  if (!node)
    {
    return -1;
    }

  int level = 0;
  const char* tagName = node->GetName();

  // vtkMultiBlockDataSet must be the root of our composite dataset
  while (node && strcmp(tagName, "vtkMultiBlockDataSet") != 0)
    {
    node = node->GetParent();
    if (node)
      {
      tagName = node->GetName();
      }

    ++level;
    }

  return level;
}

//------------------------------------------------------------------------------
// msvVTKXMLMultiblockLODReader methods

//------------------------------------------------------------------------------
vtkStandardNewMacro(msvVTKXMLMultiblockLODReader);

//------------------------------------------------------------------------------
msvVTKXMLMultiblockLODReader::msvVTKXMLMultiblockLODReader()
{
  this->Internal = new msvVTKXMLMultiblockLODReaderInternal;
}

//------------------------------------------------------------------------------
msvVTKXMLMultiblockLODReader::~msvVTKXMLMultiblockLODReader()
{

}

//------------------------------------------------------------------------------
int msvVTKXMLMultiblockLODReader
::RequestInformation(vtkInformation *request,
                     vtkInformationVector **vtkNotUsed(inputVector),
                     vtkInformationVector *outputVector)
{
  return Superclass::RequestInformation(request, 0, outputVector);
}

//------------------------------------------------------------------------------
int msvVTKXMLMultiblockLODReader::
RequestData(vtkInformation* request,
            vtkInformationVector** inputVector,
            vtkInformationVector* outputVector)
{
  // Use if the XML is modified outside of the application or the first time.
  // TODO: set RequestUpdateInformation to true when modified
  if (this->Internal->RequestUpdateInformation ||
      this->Internal->NodesInfos.size() == 0)
    {
    this->Internal->InitListUpdateNodes(this->GetPrimaryElement());
    }

  this->Internal->CurrentFlatIndex = 0;
  return Superclass::RequestData(request, inputVector, outputVector);
}

//------------------------------------------------------------------------------
void msvVTKXMLMultiblockLODReader::
ReadComposite(vtkXMLDataElement* element,
              vtkCompositeDataSet* composite,
              const char* filePath,
              unsigned int &dataSetIndex)
{
  vtkMultiBlockDataSet* mblock = vtkMultiBlockDataSet::SafeDownCast(composite);
  vtkMultiPieceDataSet* mpiece = vtkMultiPieceDataSet::SafeDownCast(composite);
  if (!mblock && !mpiece)
    {
    vtkErrorMacro("Unsuported composite dataset.");
    return;
    }

  if (this->GetFileMajorVersion() < 1)
    {
    // Read legacy file.
    std::cout << "FileMajorVersion < 1" << std::endl;
    this->ReadVersion0(element, composite, filePath, dataSetIndex);
    return;
    }

  unsigned int maxElems = element->GetNumberOfNestedElements();
  for (unsigned int cc=0; cc < maxElems; ++cc)
    {
    vtkXMLDataElement* childXML = element->GetNestedElement(cc);
    if (!childXML || !childXML->GetName())
      {
      continue;
      }

    int index = 0;
    if (!childXML->GetScalarAttribute("index", index))
    // if index not in the structure file, then
    // set up to add at the end
      {
      if (mblock)
        {
        index = mblock->GetNumberOfBlocks();
        }
      else if (mpiece)
        {
        index = mpiece->GetNumberOfPieces();
        }
      }

    // child is a leaf node, read and insert.
    const char* tagName = childXML->GetName();
    if (strcmp(tagName, "DataSet") == 0)
      {
      this->Internal->CurrentFlatIndex++;

      vtkSmartPointer<vtkDataSet> childDS;
      const char* name = 0;

      if (this->ShouldGetDataSet(dataSetIndex, childXML))
        {
        childDS.TakeReference(this->ReadDataset(childXML, filePath));
        name = childXML->GetAttribute("name");
        }
      else
        {
        // We clean its reader if the node got one;
        // We should put the function within the set but would break the optimization -- Check
        if (this->Internal->NodesInfos[this->Internal->CurrentFlatIndex].Reader)
          {
          this->Internal->NodesInfos[this->Internal->CurrentFlatIndex].Reader = 0;
          }
        }

      // insert
      if (mblock)
        {
        mblock->SetBlock(index, childDS);
        mblock->GetMetaData(index)->Set(vtkCompositeDataSet::NAME(), name);
        }
      else if (mpiece)
        {
        mpiece->SetPiece(index, childDS);
        mpiece->GetMetaData(index)->Set(vtkCompositeDataSet::NAME(), name);
        }

      dataSetIndex++;
      }

    // Child is a multiblock dataset itself. Create it.
    else if (mblock != 0 && strcmp(tagName, "Block") == 0)
      {
      this->Internal->CurrentFlatIndex++;

      vtkMultiBlockDataSet* childDS = vtkMultiBlockDataSet::New();
      this->ReadComposite(childXML, childDS, filePath, dataSetIndex);
      const char* name = childXML->GetAttribute("name");
      mblock->SetBlock(index, childDS);
      mblock->GetMetaData(index)->Set(vtkCompositeDataSet::NAME(), name);
      childDS->Delete();
      }

    // Child is a multipiece dataset. Create it.
    else if (mblock!=0 && strcmp(tagName, "Piece") == 0)
      {
      this->Internal->CurrentFlatIndex++;

      vtkMultiPieceDataSet* childDS = vtkMultiPieceDataSet::New();
      this->ReadComposite(childXML, childDS, filePath, dataSetIndex);
      const char* name = childXML->GetAttribute("name");
      mblock->SetBlock(index, childDS);
      mblock->GetMetaData(index)->Set(vtkCompositeDataSet::NAME(), name);
      childDS->Delete();
      }
    else
      {
      vtkErrorMacro("Syntax error in file.");
      return;
      }
    }
}

//------------------------------------------------------------------------------
vtkXMLReader* msvVTKXMLMultiblockLODReader::GetReaderOfType(const char* type)
{
  vtkXMLReader* reader = 0;

  if (strcmp(type,"vtkXMLPolyDataReader") == 0)
    {
    reader = vtkXMLPolyDataReader::SafeDownCast(
      this->Internal->NodesInfos[this->Internal->CurrentFlatIndex].Reader);

    if (!reader)
      {
      reader = vtkXMLPolyDataReader::New();
      this->Internal->NodesInfos[this->Internal->CurrentFlatIndex].Reader =
        reader;
      }
    }
  else if (strcmp(type,"vtkXMLRectilinearGridReader") == 0)
    {
    reader = vtkXMLRectilinearGridReader::SafeDownCast(
      this->Internal->NodesInfos[this->Internal->CurrentFlatIndex].Reader);

    if (!reader)
      {
      reader = vtkXMLRectilinearGridReader::New();
      this->Internal->NodesInfos[this->Internal->CurrentFlatIndex].Reader =
        reader;
      }
    }
  else if (strcmp(type,"vtkXMLStructuredGridReader") == 0)
    {
    reader = vtkXMLStructuredGridReader::SafeDownCast(
      this->Internal->NodesInfos[this->Internal->CurrentFlatIndex].Reader);

    if (!reader)
      {
      reader = vtkXMLStructuredGridReader::New();
      this->Internal->NodesInfos[this->Internal->CurrentFlatIndex].Reader = reader;
      }
    }
  if (!reader)
    {
    // If all fails, Use the instantiator to create the reader.
    reader = vtkXMLReader::SafeDownCast(vtkInstantiator::CreateInstance(type));
    }

  return reader;
}

//------------------------------------------------------------------------------
bool msvVTKXMLMultiblockLODReader::ShouldGetDataSet(int dataSetIndex,
                                                   vtkXMLDataElement* node)
{
  int nodeLevel = msvVTKXMLMultiblockLODReaderInternal::GetLevel(node);

  // If we should not read the data or if the parent should not be get
  // DataSetIndex is set to -1 when the data is a composite node.
  if (!this->Superclass::ShouldReadDataSet(dataSetIndex)
      // || Write a should use method
      )
    {
    return false;
    }

  // Otherwise, we should get the dataset if the we are not at the LOD level
  // Or if the node corresponds to the LOD set by its parent.
  vtkXMLDataElement* father = node->GetParent();
  int currentLOD =
      this->Internal->GetFatherLOD(this->Internal->CurrentFlatIndex);
  if (nodeLevel != this->Internal->CurrentLODTreeLevel ||
      node == father->GetNestedElement(currentLOD))
    {
    return true;
    }

  return false;
}

//------------------------------------------------------------------------------
void msvVTKXMLMultiblockLODReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  std::vector<msvVTKXMLMultiblockLODReaderInternal::NodeInfos>::iterator
    itC = this->Internal->NodesInfos.begin();

  int index = 0;
  for (;itC!=this->Internal->NodesInfos.end();++itC)
    {
    os << indent << "FlatIndex: " << index++
       << indent << " | Father/LOD: " << itC->MixedIndexFatherLOD
       << indent << " | Reader: " << itC->Reader.GetPointer();

    if (itC->Reader.GetPointer())
      os << indent << " | NumberOfCell: "
         << itC->Reader->GetOutputAsDataSet()->GetNumberOfPoints();

    os << "\n" << indent;
    }
}

//------------------------------------------------------------------------------
unsigned int  msvVTKXMLMultiblockLODReader::GetDefaultLOD()
{
  return this->Internal->DefaultLOD;
}

//------------------------------------------------------------------------------
void msvVTKXMLMultiblockLODReader::SetDefaultLOD(unsigned int lod)
{
  this->Internal->CurrentFlatIndex = 0;
  this->Internal->UpdateNodesFromDefaultLOD(this->GetPrimaryElement(),
                                            this->Internal->CurrentLODTreeLevel, lod);

  this->Internal->DefaultLOD = lod;
  this->Modified();
}

//------------------------------------------------------------------------------
void msvVTKXMLMultiblockLODReader::SetPieceLOD(int pieceIndex, unsigned int lod)
{
  this->Internal->CurrentFlatIndex = 0;
  this->Internal->UpdateNodesFromDefaultLOD(this->GetPrimaryElement(),
                                            this->Internal->CurrentLODTreeLevel,
                                            lod,
                                            pieceIndex);

  this->Modified();
}

//------------------------------------------------------------------------------
void msvVTKXMLMultiblockLODReader::RestoreDefaultLOD()
{
  this->SetDefaultLOD(this->Internal->DefaultLOD);
}
