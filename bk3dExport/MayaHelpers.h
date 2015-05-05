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



