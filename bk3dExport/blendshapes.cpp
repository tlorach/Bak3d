/*
    Copyright (c) 2013, Tristan Lorach. All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Neither the name of its contributors may be used to endorse 
       or promote products derived from this software without specific
       prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
    OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    feedback to lorachnroll@gmail.com (Tristan Lorach)
*/
#  pragma warning(disable:4786)
#include <algorithm>
#include <list>

#include <maya/MPxCommand.h>

#include "bk3dExport.h"
#include "MayaHelpers.h"
#include "MiscHelpers.h"

/*----------------------------------------------------------------------------------*/ /**


**/ //----------------------------------------------------------------------------------
MStatus bk3dTranslator::ManageBlendshapes(MDagPath &dagPath, bk3dlib::PMesh bk3dMesh)
{
	PF2(("ManageBlendShapes\n"));
  MStatus status;
  MPointArray vertexArray0;
  MFloatVectorArray normals0, tangents0, binormals0;

  // Get the Mesh
  MFnMesh mesh0(dagPath.node());

  //m_blendShapeNames.clear();
  // Get the verices : pos, normal, tangent / binormal
  status = mesh0.getPoints( vertexArray0, MSpace::kObject );
  status = mesh0.getNormals ( normals0, MSpace::kObject );
  MStringArray  uvsets0;
  mesh0.getUVSetNames(uvsets0);
  status = mesh0.getTangents(  tangents0,  MSpace::kObject, &(uvsets0[0]) ); // take the first uvset
  status = mesh0.getBinormals(  binormals0,  MSpace::kObject, &(uvsets0[0]) ); // take the first uvset
  //
  // Find the Blendshapes connected to this mesh
  //
  MObject blendshape = getConnectedObjectType(dagPath.node(), "inMesh", MFn::kBlendShape, true, false, 4, &status);
  MFnBlendShapeDeformer FnBlendshape(blendshape, &status);
  if(!status)
  {
    DPF2(("No Blendshapes found\n"));
    //m_inattr_blendshapes = 0;
    return status;
  }
  //m_inattr_blendshapes = m_inattr.size();
  MIntArray wIndexList;
  // From the doc : "Return the array index numbers corresponding to the targets. 
  // The resulting index list will be the length of MFnBlendShape::numWeights. 
  // This method exists because the indices of the targets can be sparse. For example, if a target has been removed using Deform -> Edit BlendShape -> Remove."
  status = FnBlendshape.weightIndexList(wIndexList);
  /*DPF2(("%d weight index list\n", wIndexList.length()));
  for(unsigned int i=0; i<wIndexList.length(); i++)
  {
    DPF2(("%d : target %d \n", i, wIndexList[i]));
  }*/

  int numweights = FnBlendshape.numWeights();
  PF2(("Num BS Weights : %d\n", numweights));

  MObjectArray baseobjects;
  status = FnBlendshape.getBaseObjects(baseobjects);
  DPF2(("%d BaseObjects\n", baseobjects.length()));

  int bol = baseobjects.length();
  //
  // allocate the Buffer pointers for all the Blendshapes
  //
  if(!bsBuffers.empty())
  {
      assert(!"TODO: Free the pointers of buffers");
      bsBuffers.clear();
  }

  for(unsigned int i=0; i<bol; i++)
  {
    //DPF2(("BaseObject %s : ", baseobjects[i].apiTypeStr()));
    if(baseobjects[i].apiType() == MFn::kMesh)
    {
      MFnMesh mesh1(baseobjects[i]);
      int n = mesh1.numVertices();
      //DPF2(("%s : %d Vtx\n", mesh1.name().asChar(), n));
      if(mesh1.name() == mesh0.name())
      {
        DPF2(("found Mesh %s\n", mesh0.name().asChar()));
        MObjectArray targetObjects;
        for(int k=0; k<(int)wIndexList.length();k++)
        {
#pragma message(__FILE__"("S__LINE__"):>>>>>>>TODO - Connect Blendshape weight curves if necessary")
          BS_Buffers buffers;
          status = FnBlendshape.getTargets(baseobjects[i], wIndexList[k], targetObjects);
          DPF2(("\t%d target objects\n", targetObjects.length()));
if(targetObjects.length() != 1) 
assert(!"targetObjects.length() is not 1... TODO : see how to do.");
          for(unsigned int j=0; j<targetObjects.length(); j++)
          {
            AttributeInfo *attr;
            MFnDependencyNode fnDepNode(targetObjects[j]);
			MString bsname(fnDepNode.name());
			PF2(("\tweight %d : %s\n", k, fnDepNode.name().asChar()));
            if(targetObjects[j].apiType() == MFn::kMesh)
            {
              MFloatVectorArray normals, tangents, binormals;
              MFnMesh mesh2(targetObjects[j]);
              m_bsnames.push_back(fnDepNode.name());
              DPF2(("\t\t%s : %d Vtx %d polys\n", mesh2.name().asChar(), mesh2.numVertices(), mesh2.numPolygons()));
              assert(mesh2.numVertices() == mesh0.numVertices());
              //
              // Create the Vertex array
              //
                attr = findAttrInfo(Position, true);
                MPointArray vertexArray;
                status = mesh2.getPoints( vertexArray, MSpace::kObject );
                if(!bMultiIndex)
                {
                    buffers.bk3dBufferPosition_SIB = bk3dlib::Buffer::CreateVtxBuffer(MESH_POSITION, 3, attr->slot, bk3dlib::FLOAT32);
                    bk3dMesh->AttachVtxBuffer(buffers.bk3dBufferPosition_SIB, true/*isBlendShape*/);
                }
                buffers.bk3dBufferPosition = bk3dlib::Buffer::CreateVtxBuffer(MESH_POSITION, 3, attr->slot, bk3dlib::FLOAT32);
                bk3dMesh->AttachVtxBuffer(buffers.bk3dBufferPosition, true/*isBlendShape*/);
                status = extractPointTable(buffers.bk3dBufferPosition, vertexArray, MESH_POSITION,k, &vertexArray0);
              //status = extractPointTable(vertexArray, "bs_position", k, &vertexArray0);
              //
              // VertexID TABLE
              //
              //status = extractVertexIDs("bs_vertexid", k);
              //IFFAILURE();
              //
              // PER FACE NORMALS
              //
                attr = findAttrInfo(FaceNormal, true);
                if(attr)
                {
                    if(!bMultiIndex) {
                        buffers.bk3dBufferNormal_SIB = bk3dlib::Buffer::CreateVtxBuffer(MESH_FACENORMAL, 3, attr->slot, bk3dlib::FLOAT32);
                        bk3dMesh->AttachVtxBuffer(buffers.bk3dBufferNormal_SIB, true/*isBlendShape*/);
                    }
                    buffers.bk3dBufferNormal = bk3dlib::Buffer::CreateVtxBuffer(MESH_FACENORMAL, 3, attr->slot, bk3dlib::FLOAT32);
                    bk3dMesh->AttachVtxBuffer(buffers.bk3dBufferNormal, true/*isBlendShape*/);
                    status = mesh2.getNormals( normals, MSpace::kObject );
                    // NOTE : For some strange reason, the Normals from Blendshapes aren't good
                    // WARNING : BUG in Blendshapes normals : aren't good for now. Switching with reference...
                    status = extractNormalsTable(buffers.bk3dBufferNormal, normals, MESH_FACENORMAL, k, &normals0);
                    IFFAILURE();
                }
              //
              // VERTEX NORMALS TABLE
              //
                attr = findAttrInfo(Normal, true);
                if(attr)
                {
                    if(!bMultiIndex) {
                        buffers.bk3dBufferVertexNormal_SIB = bk3dlib::Buffer::CreateVtxBuffer(MESH_NORMAL, 3, attr->slot, bk3dlib::FLOAT32);
                        bk3dMesh->AttachVtxBuffer(buffers.bk3dBufferVertexNormal_SIB, true/*isBlendShape*/);
                    }
                    buffers.bk3dBufferVertexNormal = bk3dlib::Buffer::CreateVtxBuffer(MESH_NORMAL, 3, attr->slot, bk3dlib::FLOAT32);
                    bk3dMesh->AttachVtxBuffer(buffers.bk3dBufferVertexNormal, true/*isBlendShape*/);
                    status = extractVertexNormalsTable(buffers.bk3dBufferVertexNormal, mesh2, MESH_NORMAL, k, dagPath.node());
                    IFFAILURE();
                }
              //
              // TANGENTS TABLE
              //
                if(findAttrInfo(Tangent, true) || findAttrInfo(Binormal, true))
                {
                    if(mesh2.numUVSets() > 0)
                    {
                      MStringArray  uvsets;
                      mesh2.getUVSetNames(uvsets);
                      status = mesh2.getTangents(  tangents,  MSpace::kObject, &(uvsets[0]) ); // take the first uvset
                      if(status)
                      {
                          attr = findAttrInfo(Tangent, true);
                          if(attr)
                          {
                              if(!bMultiIndex) {
                                  buffers.bk3dBufferTangent_SIB = bk3dlib::Buffer::CreateVtxBuffer(MESH_TANGENT, 3, attr->slot, bk3dlib::FLOAT32);
                                  bk3dMesh->AttachVtxBuffer(buffers.bk3dBufferTangent_SIB, true/*isBlendShape*/);
                              }
                              buffers.bk3dBufferTangent = bk3dlib::Buffer::CreateVtxBuffer(MESH_TANGENT, 3, attr->slot, bk3dlib::FLOAT32);
                              bk3dMesh->AttachVtxBuffer(buffers.bk3dBufferTangent, true/*isBlendShape*/);
                              status = extractTanBinormTables(buffers.bk3dBufferTangent, tangents, MESH_TANGENT, k, &tangents0);
                          }
                          attr = findAttrInfo(Binormal, true);
                          if(attr)
                          {
                              if(!bMultiIndex) {
                                  buffers.bk3dBufferBinormal_SIB = bk3dlib::Buffer::CreateVtxBuffer(MESH_BINORMAL, 3, attr->slot, bk3dlib::FLOAT32);
                                  bk3dMesh->AttachVtxBuffer(buffers.bk3dBufferBinormal_SIB, true/*isBlendShape*/);
                              }
                              buffers.bk3dBufferBinormal = bk3dlib::Buffer::CreateVtxBuffer(MESH_BINORMAL, 3, attr->slot, bk3dlib::FLOAT32);
                              bk3dMesh->AttachVtxBuffer(buffers.bk3dBufferBinormal, true/*isBlendShape*/);
                              status = mesh2.getBinormals( binormals,  MSpace::kObject, &(uvsets[0]) ); // take the first uvset
                              status = extractTanBinormTables(buffers.bk3dBufferBinormal, binormals, MESH_BINORMAL, k, &binormals0);
                              IFFAILURE();
                          }
                      }
                      else DPF(("Failed at mesh2.getTangents()"));
                    }
                    else EPF(("WARNING : no UVSets. Cannot compute Tangents\n"));
                }
              //m_blendShapeNames.push_back(bsname);
            }
          } // for(unsigned int j=0; j<targetObjects.length(); j++)
          bsBuffers.push_back(buffers);
        } // for(int k=0; k<(int)wIndexList.length();k++)
        break;
      } // if(mesh1.name() == mesh0.name())
    }
  }

  return status;
}
