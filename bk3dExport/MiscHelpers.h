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
