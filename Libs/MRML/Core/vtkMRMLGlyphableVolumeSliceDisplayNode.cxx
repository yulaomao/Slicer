/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLGlyphableVolumeSliceDisplayNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/

#include "vtkAlgorithmOutput.h"
#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include "vtkTransform.h"
#include "vtkImageData.h"
#include "vtkPolyData.h"

#include "vtkTransformPolyDataFilter.h"

#include "vtkMRMLGlyphableVolumeSliceDisplayNode.h"
#include <sstream>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLGlyphableVolumeSliceDisplayNode);

//----------------------------------------------------------------------------
vtkMRMLGlyphableVolumeSliceDisplayNode::vtkMRMLGlyphableVolumeSliceDisplayNode()
{
  this->ColorMode = this->colorModeScalar;

#if (VTK_MAJOR_VERSION <= 5)
  this->SliceImage = NULL;
#else
  this->SliceImagePort = NULL;
#endif

  this->SliceToXYTransformer = vtkTransformPolyDataFilter::New();

  this->SliceToXYTransform = vtkTransform::New();

  this->SliceToXYMatrix = vtkMatrix4x4::New();
  this->SliceToXYMatrix->Identity();
  this->SliceToXYTransform->PreMultiply();
  this->SliceToXYTransform->SetMatrix(this->SliceToXYMatrix);

  //this->SliceToXYTransformer->SetInput(this->GlyphGlyphFilter->GetOutput());
  this->SliceToXYTransformer->SetTransform(this->SliceToXYTransform);

  // don't backface cull the glyphs - they may not be geometrically consistent
  // since they have been transformed in ways that may have flipped them.
  // See issue 1368
  this->BackfaceCulling = 0;
}


//----------------------------------------------------------------------------
vtkMRMLGlyphableVolumeSliceDisplayNode::~vtkMRMLGlyphableVolumeSliceDisplayNode()
{
  this->RemoveObservers ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
#if (VTK_MAJOR_VERSION <= 5)
  this->SetSliceImage(NULL);
#else
  this->SetSliceImagePort(NULL);
#endif
  this->SliceToXYMatrix->Delete();
  this->SliceToXYTransform->Delete();
  this->SliceToXYTransformer->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLGlyphableVolumeSliceDisplayNode::WriteXML(ostream& of, int nIndent)
{

  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << " colorMode =\"" << this->ColorMode << "\"";

}


//----------------------------------------------------------------------------
void vtkMRMLGlyphableVolumeSliceDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "colorMode"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> ColorMode;
      }

    }

  this->EndModify(disabledModify);

}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLGlyphableVolumeSliceDisplayNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLGlyphableVolumeSliceDisplayNode *node = (vtkMRMLGlyphableVolumeSliceDisplayNode *) anode;

  this->SetColorMode(node->ColorMode);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLGlyphableVolumeSliceDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
 //int idx;

  Superclass::PrintSelf(os,indent);
  os << indent << "ColorMode:             " << this->ColorMode << "\n";
}
//----------------------------------------------------------------------------
void vtkMRMLGlyphableVolumeSliceDisplayNode::SetSliceGlyphRotationMatrix(vtkMatrix4x4 *vtkNotUsed(matrix))
{
}

//----------------------------------------------------------------------------
void vtkMRMLGlyphableVolumeSliceDisplayNode::SetSlicePositionMatrix(vtkMatrix4x4 *matrix)
{
//  if (this->GlyphGlyphFilter)
//    {
//    this->GlyphGlyphFilter->SetVolumePositionMatrix(matrix);
//    }
  this->SliceToXYMatrix->DeepCopy(matrix);
  this->SliceToXYMatrix->Invert();
  if (this->SliceToXYTransform)
    {
    this->SliceToXYTransform->SetMatrix(this->SliceToXYMatrix);
    }
  this->Modified();
}

//----------------------------------------------------------------------------
#if (VTK_MAJOR_VERSION <= 5)
void vtkMRMLGlyphableVolumeSliceDisplayNode::SetSliceImage(vtkImageData *image)
{
   vtkSetObjectBodyMacro(SliceImage,vtkImageData,image);
}
#else
void vtkMRMLGlyphableVolumeSliceDisplayNode::SetSliceImagePort(vtkAlgorithmOutput *imagePort)
{
   vtkSetObjectBodyMacro(SliceImagePort,vtkAlgorithmOutput,imagePort);
}
#endif

//----------------------------------------------------------------------------
#if (VTK_MAJOR_VERSION <= 5)
void vtkMRMLGlyphableVolumeSliceDisplayNode
::SetInputToPolyDataPipeline(vtkPolyData *vtkNotUsed(glyphPolyData))
{
  vtkErrorMacro(<< this->GetClassName() <<" ("<<this
                    <<"): SetInputPolyData method should not be used");
}
#else
void vtkMRMLGlyphableVolumeSliceDisplayNode
::SetInputToPolyDataPipeline(vtkAlgorithmOutput *vtkNotUsed(glyphPolyData))
{
  vtkErrorMacro(<< this->GetClassName() <<" ("<<this
                    <<"): SetInputPolyData method should not be used");
}
#endif

//---------------------------------------------------------------------------
vtkPolyData* vtkMRMLGlyphableVolumeSliceDisplayNode::GetOutputPolyData()
{
  // Don't check input polydata as it is not used, but the image data instead.
#if (VTK_MAJOR_VERSION <= 5)
  if (!this->GetSliceImage())
#else
  if (!this->GetOutputPolyDataConnection())
#endif
    {
    return 0;
    }
  return vtkPolyData::SafeDownCast(
    this->GetOutputPolyDataConnection()->GetProducer()->GetOutputDataObject(
      this->GetOutputPolyDataConnection()->GetIndex()));
}
//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLGlyphableVolumeSliceDisplayNode
::GetOutputPolyDataConnection()
{
  return 0;
}

//----------------------------------------------------------------------------
void vtkMRMLGlyphableVolumeSliceDisplayNode::UpdatePolyDataPipeline()
{
  this->SliceToXYTransformer->SetInputConnection(
    this->GetOutputPolyDataConnection());
}

//---------------------------------------------------------------------------
vtkPolyData* vtkMRMLGlyphableVolumeSliceDisplayNode::GetSliceOutputPolyData()
{
  // Don't check input polydata as it is not used, but the image data instead.
#if (VTK_MAJOR_VERSION <= 5)
  if (!this->GetSliceImage())
#else
  if (!this->GetSliceOutputPort())
#endif
    {
    return 0;
    }
  return vtkPolyData::SafeDownCast(
    this->GetSliceOutputPort()->GetProducer()->GetOutputDataObject(
      this->GetSliceOutputPort()->GetIndex()));
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLGlyphableVolumeSliceDisplayNode::GetSliceOutputPort()
{
  return this->SliceToXYTransformer->GetOutputPort();
}

//---------------------------------------------------------------------------
void vtkMRMLGlyphableVolumeSliceDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  return;
}

//-----------------------------------------------------------
void vtkMRMLGlyphableVolumeSliceDisplayNode::UpdateScene(vtkMRMLScene *scene)
{
   Superclass::UpdateScene(scene);
}

//-----------------------------------------------------------
void vtkMRMLGlyphableVolumeSliceDisplayNode::UpdateReferences()
{
  Superclass::UpdateReferences();
}





