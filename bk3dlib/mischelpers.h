
inline int numPrimitivesDX9(unsigned int nvtx, bk3dlib::Topology topo)
{
    switch(topo)
    {
        case bk3dlib::POINTS: return nvtx;
        case bk3dlib::LINES: return nvtx/2;
        case bk3dlib::LINE_STRIP: return nvtx - 1;
        case bk3dlib::TRIANGLES: return nvtx/3;
        case bk3dlib::TRIANGLE_STRIP: return nvtx - 2;
        case bk3dlib::TRIANGLE_FAN: return nvtx-1;
        // Should not happen:
        case bk3dlib::QUADS: return nvtx/4;
        case bk3dlib::QUAD_STRIP: return nvtx -2;
        case bk3dlib::FAN: return nvtx-2;
        //LINES_ADJ
        //LINE_STRIP_ADJ
        //TRIANGLES_ADJ
        //TRIANGLE_STRIP_ADJ
    }
    int p = (int)topo - (int)bk3dlib::PATCHES0;
    if(p > 0)
    {
        return nvtx /= p;
    }
    return 0;
}

inline D3D11_PRIMITIVE_TOPOLOGY TopologyDXGI(bk3dlib::Topology t)
{
    if( ((int)t >= (int)bk3dlib::PATCHES1) && ( (int)t <= (int)bk3dlib::PATCHES32) )
    {
        return (D3D11_PRIMITIVE_TOPOLOGY)(
            (int)D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST 
            + ((int)t - (int)bk3dlib::PATCHES1) );
    }
	else switch(t)
	{
	case bk3dlib::QUADS:
	case bk3dlib::QUAD_STRIP:
	case bk3dlib::POINTS:
		return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
	case bk3dlib::LINES:
		return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	case bk3dlib::LINE_LOOP:
	case bk3dlib::LINE_STRIP:
		return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
	case bk3dlib::TRIANGLES:
		return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	case bk3dlib::TRIANGLE_STRIP:
		return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	case bk3dlib::LINES_ADJ:
		return D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
	case bk3dlib::LINE_STRIP_ADJ :
		return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
	case bk3dlib::TRIANGLES_ADJ:
		return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
	case bk3dlib::TRIANGLE_STRIP_ADJ:
		return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
	case bk3dlib::FAN:
		return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
	default:
		return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
	}
}

inline D3DPRIMITIVETYPE TopologyDX9(bk3dlib::Topology t)
{
	switch(t)
	{
	case bk3dlib::QUADS:
	case bk3dlib::QUAD_STRIP:
	case bk3dlib::POINTS:
		return D3DPT_POINTLIST;
	case bk3dlib::LINES:
		return D3DPT_LINELIST;
	case bk3dlib::LINE_LOOP:
	case bk3dlib::LINE_STRIP:
		return D3DPT_LINESTRIP;
	case bk3dlib::TRIANGLES:
		return D3DPT_TRIANGLELIST;
	case bk3dlib::TRIANGLE_STRIP:
		return D3DPT_TRIANGLESTRIP;
	case bk3dlib::LINES_ADJ:
		return D3DPT_POINTLIST;
	case bk3dlib::LINE_STRIP_ADJ :
		return D3DPT_POINTLIST;
	case bk3dlib::TRIANGLES_ADJ:
		return D3DPT_POINTLIST;
	case bk3dlib::TRIANGLE_STRIP_ADJ:
		return D3DPT_POINTLIST;
	case bk3dlib::FAN:
		return D3DPT_TRIANGLEFAN;
	default:
		return D3DPT_UNDEFINED;
	}
}

inline GLenum TopologyOpenGL(bk3dlib::Topology t)
{
    if( ((int)t >= (int)bk3dlib::PATCHES0) && ((int)t <= (int)bk3dlib::PATCHES32) )
    {
        return GL_PATCHES;
    }
	else switch(t)
	{
	case bk3dlib::QUADS:
		return GL_QUADS;
	case bk3dlib::QUAD_STRIP:
		return GL_QUAD_STRIP;
	case bk3dlib::POINTS:
		return GL_POINTS;
	case bk3dlib::LINES:
		return GL_LINES;
	case bk3dlib::LINE_LOOP:
		return GL_LINE_LOOP;
	case bk3dlib::LINE_STRIP:
		return GL_LINE_STRIP;
	case bk3dlib::TRIANGLES:
		return GL_TRIANGLES;
	case bk3dlib::TRIANGLE_STRIP:
		return GL_TRIANGLE_STRIP;
	case bk3dlib::LINES_ADJ:
		return GL_POINTS;
	case bk3dlib::LINE_STRIP_ADJ :
		return GL_POINTS;
	case bk3dlib::TRIANGLES_ADJ:
		return GL_POINTS;
	case bk3dlib::TRIANGLE_STRIP_ADJ:
		return GL_POINTS;
    case bk3dlib::TRIANGLE_FAN:
	case bk3dlib::FAN:
		return GL_TRIANGLE_FAN;
	default:
		return GL_POINTS;
	}
}

inline void formatDX(int ncomp/*, MString typeName*/, DXGI_FORMAT &dxgi, D3DDECLTYPE &dx9) // SEE LATER
{
  switch(ncomp)
  {
    case 1:
    dxgi = DXGI_FORMAT_R32_FLOAT;
    dx9 =  D3DDECLTYPE_FLOAT1;
    case 2:
    dxgi = DXGI_FORMAT_R32G32_FLOAT;
    dx9 =  D3DDECLTYPE_FLOAT2;
    case 3:
    dxgi = DXGI_FORMAT_R32G32B32_FLOAT;
    dx9 =  D3DDECLTYPE_FLOAT3;
    case 4:
    dxgi = DXGI_FORMAT_R32G32B32A32_FLOAT;
    dx9 =  D3DDECLTYPE_FLOAT4;
  }
  dx9  = D3DDECLTYPE_UNDEF;
  dxgi = DXGI_FORMAT_UNKNOWN;
}
inline int sizeofDXGIFormat(DXGI_FORMAT fmt)
{
  switch(fmt)
  {
	case DXGI_FORMAT_R16_UINT:
    return 2;
    case DXGI_FORMAT_R32_FLOAT:
    case DXGI_FORMAT_R32_UINT:
    return 4;
	case DXGI_FORMAT_R8G8_UINT:
    return 2;
    case DXGI_FORMAT_R32G32_UINT:
    case DXGI_FORMAT_R32G32_FLOAT:
    return 2*4;
    case DXGI_FORMAT_R32G32B32_FLOAT:
    case DXGI_FORMAT_R32G32B32_UINT:
    return 3*4;
	case DXGI_FORMAT_R8G8B8A8_UINT:
    return 4;
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
    case DXGI_FORMAT_R32G32B32A32_UINT:
    return 4*4;
  }
  assert(!"sizeofDXGIFormat(DXGI_FORMAT fmt) failed");
  throw("sizeofDXGIFormat(DXGI_FORMAT fmt) failed");
  return 0;
}
inline bool isFormatFloat(DXGI_FORMAT fmt)
{
  switch(fmt)
  {
    case DXGI_FORMAT_R32_FLOAT:
    case DXGI_FORMAT_R32G32_FLOAT:
    case DXGI_FORMAT_R32G32B32_FLOAT:
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
    return true;
  }
  return false;
}
inline int numcompofDXGIFormat(DXGI_FORMAT fmt)
{
  switch(fmt)
  {
    case DXGI_FORMAT_R32_FLOAT:
	case DXGI_FORMAT_R16_UINT:
    case DXGI_FORMAT_R32_UINT:
    return 1;
    case DXGI_FORMAT_R32G32_FLOAT:
	case DXGI_FORMAT_R8G8_UINT:
    case DXGI_FORMAT_R32G32_UINT:
    return 2;
    case DXGI_FORMAT_R32G32B32_FLOAT:
    case DXGI_FORMAT_R32G32B32_UINT:
    return 3;
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
	case DXGI_FORMAT_R8G8B8A8_UINT:
    case DXGI_FORMAT_R32G32B32A32_UINT:
    return 4;
  }
  return 0;
}
/*inline MString RemovePath(MString &name)
{
    MStringArray array;
    MString result;
    name.split('/', array);
    result = array[array.length()-1];
    return result;
}

inline MString AddSuffix(MString &name, MString meshname)
{
    MStatus    status;
    MStringArray array;
    MString result;
    name.split('.', array);
    if(array.length() == 1)
    {
        result = name + "_" + meshname;
        result += ".bk3d";
        return result;
    }
    result = array[0];
    for(unsigned int i=1; i< array.length()-1; i++)
    {
        result += ".";
        result += array[i];
    }
    result += "_";
    result += meshname;
    result += ".";
    result += array[array.length()-1];

    return result;
}
*/
