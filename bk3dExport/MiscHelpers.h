//
// Shall we take into account some special compressed cases ?
// Yes... later
//
inline int getAttrSizeBytes(AttributeType attrid)
{
    switch(attrid)
    {
    case Position:	return 3 * sizeof(float);
	case Color:		return 4 * sizeof(float);
	case FaceNormal: return 3 * sizeof(float);
	case Tangent:	return 3 * sizeof(float);
	case Binormal:	return 3 * sizeof(float);
	case Normal: return 3 * sizeof(float);
	case TexCoord0:	return 2 * sizeof(float);
	case TexCoord1:	return 2 * sizeof(float);
	case TexCoord2:	return 2 * sizeof(float);
	case TexCoord3:	return 2 * sizeof(float);
	case BonesOffsets: // TODO: this is wrong : can be different
		//assert(!"change this");
		return 4 * sizeof(float);
	case BonesWeights: // TODO: this is wrong : can be different
		//assert(!"change this");
		return 4 * sizeof(float);
	case VertexID:	return sizeof(unsigned int);
	case Blind0:	return 3 * sizeof(float);
	case Blind1:	return 3 * sizeof(float);
	case Blind2:	return 3 * sizeof(float);
	case Blind3:	return 3 * sizeof(float);
	}
	return 0;
}

inline MString RemovePath(MString &name)
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

extern MObject getConnectedObjectFromAttribute(MObject osrc, MString attrname, MStatus &status);
