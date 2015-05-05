#  pragma warning(disable:4786)
#include <algorithm>

#include <maya/MPxCommand.h>

#include "bk3dExport.h"
#include "MayaHelpers.h"
#include "MiscHelpers.h"

MObject getConnectedObject(MObject objsrc, const char *connectionName, bool asDst, bool asSrc, MStatus *pStatus)
{
  MStatus status;
	MPlugArray plugarray1;
	MPlugArray plugarray2;
	MFnDependencyNode depNode(objsrc, &status);
  if(!status)
  {
    if(pStatus) *pStatus = status;
    return MObject();
  }

	if(depNode.getConnections( plugarray1 ))
	{
		for(unsigned int i=0; i<plugarray1.length(); i++)
		{
			MPlug plug = plugarray1[i]; // here is the plug of the object
      MString name(plug.partialName(false,false,false,false,false,true));
      //DPF(("\tplug name = %s\n", name.asChar()));
      if(!(name == connectionName))
        continue;
			if(plug.connectedTo( plugarray2, asDst, asSrc, &status))
			{
				/*for(int j=0; j<plugarray2.length(); j++)
				{
          MString name2(plugarray2[j].partialName(false,false,false,false,false,true));
          DPF(("\t\tplug name2 = %s ; ", name2.asChar()));
					MPlug plug2 = plugarray2[j];
					MObject object2 = plug2.node();
          DPF(("\t\tparent node = %s\n", object2.apiTypeStr()));
				}*/
		if(plugarray2.length() >=1 )
		{
		  if(pStatus) *pStatus = status;
		  return plugarray2[0].node();
		}
			}
		}
  } 
  if(pStatus) *pStatus = MStatus(MStatus::kNotFound);
  return MObject();
}
//
// Look for the first object of type MFn::Type we can find
//
/* SO SIMPLE:
    MItDependencyGraph dgIter(m_mesh.object(),
                              MFn::kSkinClusterFilter,
                              MItDependencyGraph::kUpstream,
                              MItDependencyGraph::kBreadthFirst,
                              MItDependencyGraph::kNodeLevel,
                              &g_status);

    if(dgIter.isDone())
    {        
        return false;
    }

*/
MObject getConnectedObjectType(MObject objsrc, const char *connectionName, MFn::Type type, bool bFromInput, bool bFromOutput, int maxdepth, MStatus *pStatus)
{
  MStatus status;
	MPlugArray plugarray1;
	MPlugArray plugarray2;
	MFnDependencyNode depNode(objsrc, &status);
    //DPF(("maxdepth %d\n", maxdepth));
  if((!status)||(maxdepth == 0))
  {
    if(pStatus) *pStatus = status;
    return MObject();
  }
//const char *dbgstr;
	if(depNode.getConnections( plugarray1 ))
	{
		for(unsigned int i=0; i<plugarray1.length(); i++)
		{
			MPlug plug = plugarray1[i]; // here is the plug of the object
			MString name(plug.partialName(false,false,false,false,false,true));
			if(connectionName && (name != connectionName))
				continue;
//dbgstr = name.asChar();
		    //DPF(("\tplug name = %s\n", name.asChar()));
			if(plug.connectedTo( plugarray2, bFromInput/*asdst*/, bFromOutput/*assrc*/, &status))
			{
				//for(int j=0; j<plugarray2.length(); j++)
				int j=0; if(plugarray2.length() > 0)// do we really care of reading all the connected parts ?
				{
                  MString name2(plugarray2[j].partialName(false,false,false,false,false,true));
//dbgstr = name2.asChar();
                  //DPF(("\t\tplug name2 = %s ; \n", name2.asChar()));
					        MPlug plug2 = plugarray2[j];
					        MObject object2 = plug2.node();
                            const char *typestr = object2.apiTypeStr();
                  //DPF(("\t\t\tparent node = %s\n", typestr));
                  //
                  // found the type of object
                  //
                  if(object2.apiType() == type)
                    return object2;
                  //
                  // no inspect connected object
                  //
                  MObject o = getConnectedObjectType(object2, NULL, type, bFromInput, bFromOutput, maxdepth-1, pStatus);
                  if(o.apiType() == type)
                    return o;
				}
			}
		}
  } 
  if(pStatus) *pStatus = MStatus(MStatus::kNotFound);
  return MObject();
}

/*
// Build adjacent triangle index list
// a 6 indices



*/
MStatus buildAdjTriangleIndices( MObject mesh, MIntArray faceIndices )
{
  MStatus status;
  faceIndices.clear();

  MItMeshPolygon faceIter( mesh, &status );
  MCheckStatus( status, ("MItMeshPolygon constructor failed") );
  MFnMesh meshFn( mesh, &status );
  MCheckStatus( status, ("MFnMesh constructor failed") );
  MItMeshEdge edgeIter( mesh, &status );
  MCheckStatus( status, ("MItMeshEdge constructor failed") );

  int numtriangles = meshFn.numPolygons(); // again we assume we are working with triangles
  faceIndices.setLength(numtriangles * 6);
  for(int i=0 ;i<numtriangles; i++)
  {
    MIntArray  vertexList;
    status = meshFn.getPolygonVertices ( i, vertexList );
    faceIndices[i*6 + 0] = vertexList[0];
    faceIndices[i*6 + 1] = vertexList[1];
    faceIndices[i*6 + 2] = vertexList[2];
    faceIndices[i*6 + 3] = -1;
    faceIndices[i*6 + 4] = -1;
    faceIndices[i*6 + 5] = -1;
  }

  for( ; !edgeIter.isDone(); edgeIter.next() )
  {
    //MIntArray faceEdges;
    //status = faceIter.getEdges( faceEdges );
    //MCheckStatus( status, ("failed to get edge") );

    // retrieve the triangles using this edge
    MIntArray faces;
    edgeIter.getConnectedFaces(faces,  &status);
    // get the index for the edge
    //getEdgeVertices ( int edgeId, int2 & vertexList )
    int idx0 = edgeIter.index ( 0,  &status );
    int idx1 = edgeIter.index ( 1,  &status );
    // for connected faces, provide the adjacent index
    // basic case only : 2 faces connected
    if(faces.length() != 2)
    {
      DPF(("edge %d is having %d faces... skipping", edgeIter.count(), faces.length()));
      continue;
    }
    for(int p1=0, p2=1; p1<2; p1++, p2--)
    {
      MIntArray  vertexList1;
      MIntArray  vertexList2;
      status = meshFn.getPolygonVertices ( faces[p1], vertexList1 );
      status = meshFn.getPolygonVertices ( faces[p2], vertexList2 );
      if(vertexList1.length() != 3)
        EPF(("Warning : polygon is not a triangle\n"));
      // write the triangle indices.
      // On this face get the 3rd vertices that isn't par of the edge
      for(int i=0; i<3; i++)
      {
        if((vertexList1[i] != idx0) || (vertexList1[i] != idx1))
        {
          // this vertex can be the adjacent one for the other triangle
          // find which offset to apply in the 6 idx area
          int offs = 0;
          for(int j=0; j<3; i++)
            if((vertexList2[j] != idx0) || (vertexList2[j] != idx1))
            { offs = j; break;}
          faceIndices[faces[p2]*6 + 3 + offs] = vertexList1[i];
        }
      }
    }
  }

   return status;
}




MStatus getFaceEdges( MObject mesh,
                      int faceId,
                      MIntArray faceEdges )
{
   MStatus status;

   // Reset the faceEdges array
   //
   faceEdges.clear();

   // Initialize a face iterator and function set
   //
   MItMeshPolygon faceIter( mesh, &status );
   MCheckStatus( status, ("MItMeshPolygon constructor failed") );
   MFnMesh meshFn( mesh, &status );
   MCheckStatus( status, ("MFnMesh constructor failed") );

   // Check to make sure that the faceId passed in is valid
   //
   if( faceId >= meshFn.numPolygons() || faceId < 0 )
   {
      status = MS::kFailure;
   }
   else
   {
      // Now parse the mesh for the given face and
      // return the edges
      //
      for( ; !faceIter.isDone(); faceIter.next() )
      {
         // If we find the matching face, retrieve the
         // edge indices
         //
         if( faceIter.index() == faceId )
         {
            faceIter.getEdges( faceEdges );
            break;
         }
      }
   }

   return status;
}


MStatus getEdgeStartVertices( MObject mesh,
                              MPointArray& pointArray )
{
   MStatus status;

   // Clear the output array
   //
   pointArray.clear();

   // Initialize our iterator
   //
   MItMeshEdge edgeIter( mesh, &status );
   MCheckStatus( status, ("MItMeshEdge constructor failed") );

   // Now parse the mesh
   //
   for( ; !edgeIter.isDone(); edgeIter.next() )
   {
      // Retrieve the start vertex of each edge and append it to
      // our point array. Use the default object coordinate
      // system for our space
      //
      pointArray.append( edgeIter.point(0, MSpace::kObject) );
   }

   return status;
}

void getOrderString(MTransformationMatrix::RotationOrder ro, char *str)
{
    switch(ro)
    {
    case MTransformationMatrix::kXYZ:
        strcpy_s(str, 4, "xyz");
        break;
    case MTransformationMatrix::kYZX:
        strcpy_s(str, 4, "yzx");
        break;
    case MTransformationMatrix::kZXY:
        strcpy_s(str, 4, "zxy");
        break;
    case MTransformationMatrix::kXZY:
        strcpy_s(str, 4, "xzy");
        break;
    case MTransformationMatrix::kYXZ:
        strcpy_s(str, 4, "yxz");
        break;
    case MTransformationMatrix::kZYX:
        strcpy_s(str, 4, "zyx");
        break;
    }
}

bool GetFloat(const MFnDependencyNode &dgNode, 
              const char *attributeName, 
              float &value)
{
  MStatus status;
  if(dgNode.object().isNull())
    return false;
  
  // Try to find the attribute
  MObject attribute = dgNode.attribute(attributeName);
  if (attribute.isNull())
    return false;
  
  // Get the plug and then the sub-plugs that hold the components
  MPlug plug(dgNode.object(), attribute);
  status = plug.getValue(value);
  if(!status)   
    return false;

  // success
  return true;
}

bool GetString(const MFnDependencyNode &dgNode, 
               const char *attributeName, 
               std::string &value)
{
  MStatus status;

  if(dgNode.object().isNull())
    return false;

  // Try to find the attribute
  MObject attribute = dgNode.attribute(attributeName);
  if (attribute.isNull())
    return false;

  // Extract the value fromt the plug
  MString mayaString;
  MPlug plug(dgNode.object(), attribute);
  status = plug.getValue(mayaString);
  
  if(!status)   
    return false;

  // tuck into our string type
  value = mayaString.asChar();

  // success
  return true;
}

MObject getConnectedObjectFromAttribute(MObject osrc, MString attrname, MStatus &status)
{
	MFnDependencyNode dn(osrc, &status);
	MObject o = dn.attribute(attrname, &status);
	if(status)
	{
	  MPlug plug(osrc, o);
	  MPlugArray plugarray2;
	  const char * nnn = plug.name().asChar();
	  if(plug.connectedTo( plugarray2, true/*asdst*/, false/*assrc*/, &status))
	  {
		  int l = plugarray2.length();
        if(l > 0)
        {
          return plugarray2[0].node();
        }
	  }
	}
	return MObject();
}



