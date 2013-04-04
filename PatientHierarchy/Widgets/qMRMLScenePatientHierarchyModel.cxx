/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#include "qMRMLScenePatientHierarchyModel.h"

// SlicerRT includes
#include "SlicerRtCommon.h"
#include "vtkMRMLContourNode.h"

// Patient Hierarchy includes
#include "qMRMLScenePatientHierarchyModel_p.h"
#include "vtkSlicerPatientHierarchyModuleLogic.h"

// MRML includes
#include <vtkMRMLHierarchyNode.h>
#include <vtkMRMLDisplayNode.h>

//------------------------------------------------------------------------------
qMRMLScenePatientHierarchyModelPrivate::qMRMLScenePatientHierarchyModelPrivate(qMRMLScenePatientHierarchyModel& object)
: Superclass(object)
{
  this->NodeTypeColumn = -1;

  this->BeamIcon = QIcon(":Icons/Beam.png");
  this->ContourIcon = QIcon(":Icons/Contour.png");
  this->DoseVolumeIcon = QIcon(":Icons/DoseVolume.png");
  this->IsocenterIcon = QIcon(":Icons/Isocenter.png");
  this->PatientIcon = QIcon(":Icons/Patient.png");
  this->PlanIcon = QIcon(":Icons/Plan.png");
  this->ShowInViewersIcon = QIcon(":Icons/ShowInViewers.png");
  this->StructureSetIcon = QIcon(":Icons/StructureSet.png");
  this->StudyIcon = QIcon(":Icons/Study.png");
  this->VolumeIcon = QIcon(":Icons/Volume.png");
}

//------------------------------------------------------------------------------
void qMRMLScenePatientHierarchyModelPrivate::init()
{
  Q_Q(qMRMLScenePatientHierarchyModel);
  this->Superclass::init();

  q->setNameColumn(0);
  q->setNodeTypeColumn(q->nameColumn());
  q->setVisibilityColumn(1);
  q->setIDColumn(2);

  q->setHorizontalHeaderLabels(
    QStringList() << "Nodes" << "Vis" << "IDs");

  q->horizontalHeaderItem(0)->setToolTip(QObject::tr("Node name and type"));
  q->horizontalHeaderItem(1)->setToolTip(QObject::tr("Show/hide branch or node"));
  q->horizontalHeaderItem(2)->setToolTip(QObject::tr("Node ID"));
}


//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
qMRMLScenePatientHierarchyModel::qMRMLScenePatientHierarchyModel(QObject *vparent)
: Superclass(new qMRMLScenePatientHierarchyModelPrivate(*this), vparent)
{
  Q_D(qMRMLScenePatientHierarchyModel);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLScenePatientHierarchyModel::~qMRMLScenePatientHierarchyModel()
{
}

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLScenePatientHierarchyModel::parentNode(vtkMRMLNode* node)const
{
  return vtkMRMLHierarchyNode::SafeDownCast(
    this->Superclass::parentNode(node));
}

//------------------------------------------------------------------------------
bool qMRMLScenePatientHierarchyModel::canBeAChild(vtkMRMLNode* node)const
{
  if (!node)
    {
    return false;
    }
  return node->IsA("vtkMRMLNode");
}

//------------------------------------------------------------------------------
bool qMRMLScenePatientHierarchyModel::canBeAParent(vtkMRMLNode* node)const
{
  vtkMRMLHierarchyNode* hnode = vtkMRMLHierarchyNode::SafeDownCast(node);
  if ( hnode
    && SlicerRtCommon::IsPatientHierarchyNode(hnode)
    && hnode->GetAssociatedNodeID() == 0 )
    {
    return true;
    }
  return false;
}

//------------------------------------------------------------------------------
int qMRMLScenePatientHierarchyModel::nodeTypeColumn()const
{
  Q_D(const qMRMLScenePatientHierarchyModel);
  return d->NodeTypeColumn;
}

//------------------------------------------------------------------------------
void qMRMLScenePatientHierarchyModel::setNodeTypeColumn(int column)
{
  Q_D(qMRMLScenePatientHierarchyModel);
  d->NodeTypeColumn = column;
  this->updateColumnCount();
}

//------------------------------------------------------------------------------
int qMRMLScenePatientHierarchyModel::maxColumnId()const
{
  Q_D(const qMRMLScenePatientHierarchyModel);
  int maxId = this->Superclass::maxColumnId();
  maxId = qMax(maxId, d->VisibilityColumn);
  maxId = qMax(maxId, d->NodeTypeColumn);
  maxId = qMax(maxId, d->NameColumn);
  maxId = qMax(maxId, d->IDColumn);
  return maxId;
}

//------------------------------------------------------------------------------
void qMRMLScenePatientHierarchyModel::updateItemDataFromNode(QStandardItem* item, vtkMRMLNode* node, int column)
{
  Q_D(qMRMLScenePatientHierarchyModel);
  if (column == this->nameColumn())
  {
    item->setText(QString(node->GetName()));
    item->setToolTip(node->GetNodeTagName());
  }
  if (column == this->idColumn())
  {
    item->setText(QString(node->GetID()));
  }
  if (column == this->visibilityColumn())
  {
    int visible = -1;
    if (SlicerRtCommon::IsPatientHierarchyNode(node))
    {
      visible = vtkSlicerPatientHierarchyModuleLogic::GetBranchVisibility( vtkMRMLHierarchyNode::SafeDownCast(node) );
    }
    else if (node->IsA("vtkMRMLContourNode"))
    {
      vtkMRMLContourNode* contourNode = vtkMRMLContourNode::SafeDownCast(node);
      visible = contourNode->GetDisplayVisibility();
    }
    else if (node->IsA("vtkMRMLDisplayableNode"))
    {
      vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(node);
      visible = displayableNode->GetDisplayVisibility();
    }

    // Always set a different icon to volumes. If not a volume then set the appropriate eye icon
    if (node->IsA("vtkMRMLVolumeNode"))
    {
      item->setIcon(d->ShowInViewersIcon);
    }
    // It should be fine to set the icon even if it is the same, but due
    // to a bug in Qt (http://bugreports.qt.nokia.com/browse/QTBUG-20248),
    // it would fire a superflous itemChanged() signal.
    else if (item->data(VisibilityRole).isNull() || item->data(VisibilityRole).toInt() != visible)
    {
      item->setData(visible, VisibilityRole);
      switch (visible)
      {
      case 0:
        item->setIcon(d->HiddenIcon);
        break;
      case 1:
        item->setIcon(d->VisibleIcon);
        break;
      case 2:
        item->setIcon(d->PartiallyVisibleIcon);
        break;
      default:
        break;
      }
    }
  }
  if (column == this->nodeTypeColumn())
  {
    if (SlicerRtCommon::IsPatientHierarchyNode(node))
    {
      if ( vtkSlicerPatientHierarchyModuleLogic::IsDicomLevel(node,
        vtkSlicerPatientHierarchyModuleLogic::PATIENTHIERARCHY_LEVEL_PATIENT) )
      {
        item->setIcon(d->PatientIcon);
      }
      else if ( vtkSlicerPatientHierarchyModuleLogic::IsDicomLevel(node,
        vtkSlicerPatientHierarchyModuleLogic::PATIENTHIERARCHY_LEVEL_STUDY) )
      {
        item->setIcon(d->StudyIcon);
      }
      else if ( vtkSlicerPatientHierarchyModuleLogic::IsDicomLevel(node,
        vtkSlicerPatientHierarchyModuleLogic::PATIENTHIERARCHY_LEVEL_SERIES) )
      {
        if (node->IsA("vtkMRMLContourHierarchyNode"))
        {
          item->setIcon(d->StructureSetIcon);
        }
        //TODO: Set icon for plan
      }
      else if ( vtkSlicerPatientHierarchyModuleLogic::IsDicomLevel(node,
        vtkSlicerPatientHierarchyModuleLogic::PATIENTHIERARCHY_LEVEL_SUBSERIES) )
      {
        // Set icon for PH type subseries objects here
      }
      else
      {
        vtkWarningWithObjectMacro(node, "Invalid DICOM level found for node '" << node->GetName() << "'");
      }
    }
    else if (node->IsA("vtkMRMLVolumeNode"))
    {
      if (SlicerRtCommon::IsDoseVolumeNode(node))
      {
        item->setIcon(d->DoseVolumeIcon);
      }
      else
      {
        item->setIcon(d->VolumeIcon);
      }
    }
    else if (node->IsA("vtkMRMLContourNode"))
    {
      item->setIcon(d->ContourIcon);
    }
    else if (node->IsA("vtkMRMLAnnotationFiducialNode"))
    {
      // TODO: add check to make sure it is an actual isocenter
      item->setIcon(d->IsocenterIcon);
    }
    else if (node->IsA("vtkMRMLModelNode"))
    {
      // TODO: add check to make sure it is an actual beam
      item->setIcon(d->BeamIcon);
    }
  }
}

//------------------------------------------------------------------------------
void qMRMLScenePatientHierarchyModel::updateNodeFromItemData(vtkMRMLNode* node, QStandardItem* item)
{
  if ( item->column() == this->nameColumn() )
  {
    node->SetName(item->text().toLatin1());
  }
  if ( item->column() == this->visibilityColumn()
    && !item->data(VisibilityRole).isNull() )
  {
    int visible = item->data(VisibilityRole).toInt();
    if (visible > -1)
    {
      if (SlicerRtCommon::IsPatientHierarchyNode(node))
      {
        vtkSlicerPatientHierarchyModuleLogic::SetBranchVisibility( vtkMRMLHierarchyNode::SafeDownCast(node), visible );
      }
      else if (node->IsA("vtkMRMLContourNode"))
      {
        vtkMRMLContourNode* contourNode = vtkMRMLContourNode::SafeDownCast(node);
        contourNode->SetDisplayVisibility(visible);
      }
      else if (node->IsA("vtkMRMLDisplayableNode") && !node->IsA("vtkMRMLVolumeNode"))
      {
        vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(node);
        displayableNode->SetDisplayVisibility(visible);

        vtkMRMLDisplayNode* displayNode = displayableNode->GetDisplayNode();
        if (displayNode)
        {
          displayNode->SetSliceIntersectionVisibility(visible);
        }
      }
    }
  }
}