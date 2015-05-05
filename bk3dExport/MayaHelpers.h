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
#define   MCheckStatus( status, msg )\
if(!status) {\
  printf msg;\
  return status;\
}

extern int VSPrintf(LPCSTR fmt, ...);

#if 1
# define   DPF( msg ) VSPrintf msg
#else
# define   DPF( msg )
#endif
#if 1
# define   DPF2( msg ) VSPrintf msg
#else
# define   DPF2( msg )
#endif
#if 1
# define   EPF( msg ) VSPrintf msg
#else
# define   EPF( msg )
#endif
#if 1
# define   PF( msg ) VSPrintf msg
#else
# define   PF( msg )
#endif
#if 1
# define   PF2( msg ) VSPrintf msg
#else
# define   PF2( msg )
#endif
extern MObject getConnectedObject(MObject objsrc, const char *connectionName, bool asDst=true, bool asSrc=true, MStatus *pStatus=NULL);
/* SO SIMPLE:
extern     MItDependencyGraph dgIter(m_mesh.object(),
                              MFn::kSkinClusterFilter,
                              MItDependencyGraph::kUpstream,
                              MItDependencyGraph::kBreadthFirst,
                              MItDependencyGraph::kNodeLevel,
                              &g_status);
*/
extern MObject getConnectedObjectType(MObject objsrc, const char *connectionName, MFn::Type type, bool bFromInput=true, bool bFromOutput=true, int maxdepth=10, MStatus *pStatus=NULL);
extern MStatus buildAdjTriangleIndices( MObject mesh, MIntArray faceIndices );
extern MStatus getFaceEdges( MObject mesh,
                      int faceId,
                      MIntArray faceEdges );
extern MStatus getEdgeStartVertices( MObject mesh,
                              MPointArray& pointArray );
extern void getOrderString(MTransformationMatrix::RotationOrder ro, char *str);

extern bool GetString(const MFnDependencyNode &dgNode, 
               const char * attributeName, 
               std::string &value);

extern bool GetFloat(const MFnDependencyNode &dgNode, 
              const char * attributeName, 
              float &value);

inline MStatus getAttribute(const MObject &obj, 
              const char *attributeName, 
              bool &value)
{
	MStatus status;
	MFnDependencyNode depNode(obj, &status);
	if(!status)
		return status;
	// Try to find the attribute
	MObject attribute = depNode.attribute(attributeName, &status);
	if (!status)
		return status;
	// Get the plug and then the sub-plugs that hold the components
	MPlug plug(obj, attribute);
	status = plug.getValue(value);
	if(!status)   
		return status;
	// success
	return status;
}
inline MStatus getAttributeFloat(const MObject &obj, 
              const char *attributeName, 
              float &value)
{
	MStatus status;
	MFnDependencyNode depNode(obj, &status);
	if(!status)
		return status;
	// Try to find the attribute
	MObject attribute = depNode.attribute(attributeName, &status);
	if (!status)
		return status;
	// Get the plug and then the sub-plugs that hold the components
	MPlug plug(obj, attribute);
	status = plug.getValue(value);
	if(!status)   
		return status;
	// success
	return status;
}



