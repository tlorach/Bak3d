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
#pragma warning(disable: 4786)
#include <map>
#pragma warning(disable: 4786)
#include <algorithm>

#  pragma warning(disable:4786)
#include <algorithm>

#include <maya/MPxCommand.h>

#include "bk3dExport.h"
#include "MayaHelpers.h"
#include "MiscHelpers.h"

#ifdef DOCURVETEXTEXPORT

bk3dlib::CurveVec::EtInfinityType getInfinityType(MFnAnimCurve::InfinityType t)
{
    switch(t)
    {
    case MFnAnimCurve::kConstant:
        return bk3dlib::CurveVec::kInfinityConstant;
    case MFnAnimCurve::kLinear:
        return bk3dlib::CurveVec::kInfinityLinear;
    case MFnAnimCurve::kCycle:
        return bk3dlib::CurveVec::kInfinityCycle;
    case MFnAnimCurve::kCycleRelative:
        return bk3dlib::CurveVec::kInfinityCycleRelative;
    case MFnAnimCurve::kOscillate:
        return bk3dlib::CurveVec::kInfinityOscillate;
    }
    return bk3dlib::CurveVec::kInfinityEnd;
}

bk3dlib::CurveVec::EtTangentType getTangentType(MFnAnimCurve::TangentType t)
{
    switch(t)
    {
    case MFnAnimCurve::kTangentGlobal:
        return bk3dlib::CurveVec::kTangentGlobal;
    case MFnAnimCurve::kTangentFixed:
        return bk3dlib::CurveVec::kTangentFixed;
    case MFnAnimCurve::kTangentLinear:
        return bk3dlib::CurveVec::kTangentLinear;
    case MFnAnimCurve::kTangentFlat:
        return bk3dlib::CurveVec::kTangentFlat;
    case MFnAnimCurve::kTangentSmooth:
        return bk3dlib::CurveVec::kTangentSmooth;
    case MFnAnimCurve::kTangentStep:
        return bk3dlib::CurveVec::kTangentStep;
    case MFnAnimCurve::kTangentSlow:
        return bk3dlib::CurveVec::kTangentSlow;
    case MFnAnimCurve::kTangentFast:
        return bk3dlib::CurveVec::kTangentFast;
    case MFnAnimCurve::kTangentClamped:
        return bk3dlib::CurveVec::kTangentClamped;
    case MFnAnimCurve::kTangentPlateau:
        return bk3dlib::CurveVec::kTangentPlateau;
    case MFnAnimCurve::kTangentStepNext:
        return bk3dlib::CurveVec::kTangentStepNext;
    //case MFnAnimCurve::kTangentEnd:
    //    return bk3dlib::CurveVec::kTangentEnd;
    };
    return bk3dlib::CurveVec::kTangentEnd;
}
//-------------------------------------------------------------------------
///	Class animUnitNames
class animUnitNames
{
public:
	animUnitNames();
	virtual ~animUnitNames();

	static void setToLongName(const MAngle::Unit&, MString&);
	static void setToShortName(const MAngle::Unit&, MString&);

	static void setToLongName(const MDistance::Unit&, MString&);
	static void setToShortName(const MDistance::Unit&, MString&);

	static void setToLongName(const MTime::Unit&, MString&);
	static void setToShortName(const MTime::Unit&, MString&);

	static bool setFromName(const MString&, MAngle::Unit&);
	static bool setFromName(const MString&, MDistance::Unit&);
	static bool setFromName(const MString&, MTime::Unit&);
};

//	String names for units.
//
const char *kMmString = 		"mm";
const char *kCmString =			"cm";
const char *kMString =			"m";
const char *kKmString =			"km";
const char *kInString =			"in";
const char *kFtString =			"ft";
const char *kYdString =			"yd";
const char *kMiString =			"mi";

const char *kMmLString =		"millimeter";
const char *kCmLString =		"centimeter";
const char *kMLString =			"meter";
const char *kKmLString =		"kilometer";
const char *kInLString =		"inch";
const char *kFtLString =		"foot";
const char *kYdLString =		"yard";
const char *kMiLString =		"mile";

const char *kRadString =		"rad";
const char *kDegString =		"deg";
const char *kMinString =		"min";
const char *kSecString =		"sec";

const char *kRadLString =		"radian";
const char *kDegLString =		"degree";
const char *kMinLString =		"minute";
const char *kSecLString =		"second";

const char *kHourTString =		"hour";
const char *kMinTString =		"min";
const char *kSecTString =		"sec";
const char *kMillisecTString =	"millisec";

const char *kGameTString =		"game";
const char *kFileTString =		"film";
const char *kPalTString =		"pal";
const char *kNtscTString =		"ntsc";
const char *kShowTString =		"show";
const char *kPalFTString =		"palf";
const char *kNtscFTString =		"ntscf";

const char *kUnknownTimeString =	"Unknown Time Unit";
const char *kUnknownAngularString =	"Unknown Angular Unit";
const char *kUnknownLinearString = 	"Unknown Linear Unit";

animUnitNames::animUnitNames()
//
//	Description:
//		Class constructor.
//
{
}

animUnitNames::~animUnitNames()
//
//	Description:
//		Class destructor.
//
{
}

/* static */
void animUnitNames::setToLongName(const MAngle::Unit& unit, MString& name)
//
//	Description:
//		Sets the string with the long text name of the angle unit.
//
{
	switch(unit) {
		case MAngle::kDegrees:
			name.set(kDegLString);
			break;
		case MAngle::kRadians:
			name.set(kRadLString);
			break;
		case MAngle::kAngMinutes:
			name.set(kMinLString);
			break;
		case MAngle::kAngSeconds:
			name.set(kSecLString);
			break;
		default:
			name.set(kUnknownAngularString);
			break;
	}
}

/* static */
void animUnitNames::setToShortName(const MAngle::Unit& unit, MString& name)
//
//	Description:
//		Sets the string with the short text name of the angle unit.
//
{
	switch(unit) {
		case MAngle::kDegrees:
			name.set(kDegString);
			break;
		case MAngle::kRadians:
			name.set(kRadString);
			break;
		case MAngle::kAngMinutes:
			name.set(kMinString);
			break;
		case MAngle::kAngSeconds:
			name.set(kSecString);
			break;
		default:
			name.set(kUnknownAngularString);
			break;
	}
}

/* static */
void animUnitNames::setToLongName(const MDistance::Unit& unit, MString& name)
//
//	Description:
//		Sets the string with the long text name of the distance unit.
//
{
	switch(unit) {
		case MDistance::kInches:
			name.set(kInLString);
			break;
		case MDistance::kFeet:
			name.set(kFtLString);
			break;
		case MDistance::kYards:
			name.set(kYdLString);
			break;
		case MDistance::kMiles:
			name.set(kMiLString);
			break;
		case MDistance::kMillimeters:
			name.set(kMmLString);
			break;
		case MDistance::kCentimeters:
			name.set(kCmLString);
			break;
		case MDistance::kKilometers:
			name.set(kKmLString);
			break;
		case MDistance::kMeters:
			name.set(kMLString);
			break;
		default:
			name.set(kUnknownLinearString);
			break;
	}
}

/* static */
void animUnitNames::setToShortName(const MDistance::Unit& unit, MString& name)
//
//	Description:
//		Sets the string with the short text name of the distance unit.
//
{
	switch(unit) {
		case MDistance::kInches:
			name.set(kInString);
			break;
		case MDistance::kFeet:
			name.set(kFtString);
			break;
		case MDistance::kYards:
			name.set(kYdString);
			break;
		case MDistance::kMiles:
			name.set(kMiString);
			break;
		case MDistance::kMillimeters:
			name.set(kMmString);
			break;
		case MDistance::kCentimeters:
			name.set(kCmString);
			break;
		case MDistance::kKilometers:
			name.set(kKmString);
			break;
		case MDistance::kMeters:
			name.set(kMString);
			break;
		default:
			name.set(kUnknownLinearString);
			break;
	}
}

/* static */
void animUnitNames::setToLongName(const MTime::Unit &unit, MString &name)
//
//	Description:
//		Sets the string with the long text name of the time unit.
//
{
	switch(unit) {
		case MTime::kHours:
			name.set(kHourTString);
			break;
		case MTime::kMinutes:
			name.set(kMinTString);
			break;
		case MTime::kSeconds:
			name.set(kSecTString);
			break;
		case MTime::kMilliseconds:
			name.set(kMillisecTString);
			break;
		case MTime::kGames:
			name.set(kGameTString);
			break;
		case MTime::kFilm:
			name.set(kFileTString);
			break;
		case MTime::kPALFrame:
			name.set(kPalTString);
			break;
		case MTime::kNTSCFrame:
			name.set(kNtscTString);
			break;
		case MTime::kShowScan:
			name.set(kShowTString);
			break;
		case MTime::kPALField:
			name.set(kPalFTString);
			break;
		case MTime::kNTSCField:
			name.set(kNtscFTString);
			break;
		default:
			name.set(kUnknownTimeString);
			break;
	}
}

/* static */
void animUnitNames::setToShortName(const MTime::Unit &unit, MString &name)
//
//	Description:
//		Sets the string with the short text name of the time unit.
//
{
	setToLongName(unit, name);
}

/* static */
bool animUnitNames::setFromName(const MString &str, MAngle::Unit &unit)
//
//	Description:
//		The angle unit is set based on the passed string. If the string
//		is not recognized, the angle unit is set to MAngle::kInvalid.
//
{
	bool state = true;

	const char *name = str.asChar();

	if ((strcmp(name, kDegString) == 0) || 
		(strcmp(name, kDegLString) == 0)) {
		unit = MAngle::kDegrees;
	} else if (	(strcmp(name, kRadString) == 0) ||
				(strcmp(name, kRadLString) == 0)) {
		unit = MAngle::kRadians;
	} else if (	(strcmp(name, kMinString) == 0) ||
				(strcmp(name, kMinLString) == 0)) {
		unit = MAngle::kAngMinutes;
	} else if (	(strcmp(name, kSecString) == 0) ||
				(strcmp(name, kSecLString) == 0)) {
		unit = MAngle::kAngSeconds;
	} else {
		//	This is not a recognized angular unit.
		//
		unit = MAngle::kInvalid;
		MString errorStr("'");
		errorStr += str;
		errorStr += "' is not a valid angular unit. ";
		errorStr += "Use rad|deg|min|sec instead.";
		MGlobal::displayError(errorStr);
		state = false;
	}

	return state;
}

/* static */
bool animUnitNames::setFromName(const MString &str, MDistance::Unit &unit)
//
//	Description:
//		The distance unit is set based on the passed string. If the string
//		is not recognized, the distance unit is set to MDistance::kInvalid.
//
{
	bool state = true;

	const char *name = str.asChar();

	if ((strcmp(name, kInString) == 0) ||
		(strcmp(name, kInLString) == 0)) {
		unit = MDistance::kInches;
	} else if (	(strcmp(name, kFtString) == 0) ||
				(strcmp(name, kFtLString) == 0)) {
		unit = MDistance::kFeet;
	} else if (	(strcmp(name, kYdString) == 0) ||
				(strcmp(name, kYdLString) == 0)) {
		unit = MDistance::kYards;
	} else if (	(strcmp(name, kMiString) == 0) ||
				(strcmp(name, kMiLString) == 0)) {
		unit = MDistance::kMiles;
	} else if (	(strcmp(name, kMmString) == 0) ||
				(strcmp(name, kMmLString) == 0)) {
		unit = MDistance::kMillimeters;
	} else if (	(strcmp(name, kCmString) == 0) ||
				(strcmp(name, kCmLString) == 0)) {
		unit = MDistance::kCentimeters;
	} else if (	(strcmp(name, kKmString) == 0) ||
				(strcmp(name, kKmLString) == 0)) {
		unit = MDistance::kKilometers;
	} else if (	(strcmp(name, kMString) == 0) ||
				(strcmp(name, kMLString) == 0)) {
		unit = MDistance::kMeters;
	} else {
		//  This is not a recognized distance unit.
		//
		state = false;
		MString errorStr("'");
		errorStr += str;
		errorStr += "' is not a valid linear unit. ";
		errorStr += "Use mm|cm|m|km|in|ft|yd|mi instead.";
		MGlobal::displayError(errorStr);
		unit = MDistance::kInvalid;
	}

	return state;
}

/* static */
bool animUnitNames::setFromName(const MString &str, MTime::Unit &unit)
//
//	Description:
//		The time unit is set based on the passed string. If the string
//		is not recognized, the time unit is set to MTime::kInvalid.
//
{
	bool state = true;
	const char *name = str.asChar();

	if (strcmp(name, kHourTString) == 0) {
		unit = MTime::kHours;
	} else if (strcmp(name, kMinTString) == 0) {
		unit = MTime::kMinutes;
	} else if (strcmp(name, kSecTString) == 0) {
		unit = MTime::kSeconds;
	} else if (strcmp(name, kMillisecTString) == 0) {
		unit = MTime::kMilliseconds;
	} else if (strcmp(name, kGameTString) == 0) {
		unit = MTime::kGames;
	} else if (strcmp(name, kFileTString) == 0) {
		unit = MTime::kFilm;
	} else if (strcmp(name, kPalTString) == 0) {
		unit = MTime::kPALFrame;
	} else if (strcmp(name, kNtscTString) == 0) {
		unit = MTime::kNTSCFrame;
	} else if (strcmp(name, kShowTString) == 0) {
		unit = MTime::kShowScan;
	} else if (strcmp(name, kPalFTString) == 0) {
		unit = MTime::kPALField;
	} else if (strcmp(name, kNtscFTString) == 0) {
		unit = MTime::kNTSCField;
	} else {
		//  This is not a recognized time unit.
		//
		unit = MTime::kInvalid;
		MString errorStr("'");
		errorStr += str;
		errorStr += "' is not a valid time unit. ";
		errorStr += 
			"Use game|film|pal|ntsc|show|palf|ntscf|hour|min|sec|millisec";
		errorStr += " instead.";
		MGlobal::displayError(errorStr);
		state = false;
	}

	return state;
}


// Tangent type words
//
const char *kWordTangentGlobal = "global";
const char *kWordTangentFixed = "fixed";
const char *kWordTangentLinear = "linear";
const char *kWordTangentFlat = "flat";
const char *kWordTangentSmooth = "smooth";
const char *kWordTangentStep = "step";
const char *kWordTangentSlow = "slow";
const char *kWordTangentFast = "fast";
const char *kWordTangentClamped = "clamped";

// Infinity type words
//
const char *kWordConstant = "constant";
const char *kWordLinear = "linear";
const char *kWordCycle = "cycle";
const char *kWordCycleRelative = "cycleRelative";
const char *kWordOscillate = "oscillate";

//	Param Curve types
//
const char *kWordTypeUnknown = "unknown";
const char *kWordTypeLinear = "linear";
const char *kWordTypeAngular = "angular";
const char *kWordTypeTime = "time";
const char *kWordTypeUnitless = "unitless";

/***************************************************************************/
/***************************************************************************/
const char *
bk3dTranslator::tangentTypeAsWord(MFnAnimCurve::TangentType type)
//
//	Description:
//		Returns a string with a test based desription of the passed
//		MFnAnimCurve::TangentType. 
//
{
	switch (type) {
		case MFnAnimCurve::kTangentGlobal:
			return (kWordTangentGlobal);
		case MFnAnimCurve::kTangentFixed:
			return (kWordTangentFixed);
		case MFnAnimCurve::kTangentLinear:
			return (kWordTangentLinear);
		case MFnAnimCurve::kTangentFlat:
			return (kWordTangentFlat);
		case MFnAnimCurve::kTangentSmooth:
			return (kWordTangentSmooth);
		case MFnAnimCurve::kTangentStep:
			return (kWordTangentStep);
		case MFnAnimCurve::kTangentSlow:
			return (kWordTangentSlow);
		case MFnAnimCurve::kTangentFast:
			return (kWordTangentFast);
		case MFnAnimCurve::kTangentClamped:
			return (kWordTangentClamped);
		default:
			break;
	}
	return (kWordTangentGlobal);
}


const char *
bk3dTranslator::infinityTypeAsWord(MFnAnimCurve::InfinityType type)
//	
//	Description:
//		Returns a string containing the name of the passed 
//		MFnAnimCurve::InfinityType type.
//
{
	switch (type) {
		case MFnAnimCurve::kConstant:
			return (kWordConstant);
		case MFnAnimCurve::kLinear:
			return (kWordLinear);
		case MFnAnimCurve::kCycle:
			return (kWordCycle);
		case MFnAnimCurve::kCycleRelative:
			return (kWordCycleRelative);
		case MFnAnimCurve::kOscillate:
			return (kWordOscillate);
		default:
			break;
	}
	return (kWordConstant);
}

const char *
bk3dTranslator::outputTypeAsWord (MFnAnimCurve::AnimCurveType type)
//
//	Description:
//		Returns a string identifying the output type of the
//		passed MFnAnimCurve::AnimCurveType.
//
{
	switch (type) {
		case MFnAnimCurve::kAnimCurveTL:
		case MFnAnimCurve::kAnimCurveUL:
			return (kWordTypeLinear);
		case MFnAnimCurve::kAnimCurveTA:
		case MFnAnimCurve::kAnimCurveUA:
			return (kWordTypeAngular);
		case MFnAnimCurve::kAnimCurveTT:
		case MFnAnimCurve::kAnimCurveUT:
			return (kWordTypeTime);
		case MFnAnimCurve::kAnimCurveTU:
		case MFnAnimCurve::kAnimCurveUU:
			return (kWordTypeUnitless);
		case MFnAnimCurve::kAnimCurveUnknown:
			return (kWordTypeUnitless);
	}
	return (kWordTypeUnknown);
}

const char *
bk3dTranslator::boolInputTypeAsWord(bool isUnitless) 
//
//	Description:
//		Returns a string based on a bool. 
//
{
	if (isUnitless) {
		return (kWordTypeUnitless);
	} else {
		return (kWordTypeTime);
	}
}

MStatus bk3dTranslator::processAnimCurveAsText(MObject &animCurveObj, MObject &owner, MString &connectionName, bool verboseUnits)
{
	char *s;
	MFnAnimCurve::AnimCurveType type;
	MStatus status = MS::kSuccess;
	MString unitName;
	static MString tmpstringfp; // temporary strings for keys to store in the files
	static MString tmpstringfp2;
	static MString header1;
	static MString header21;
	static MString header22;
	static char tmpstr[1024];
	static int nkeys = 0;
	/*if (animCurveObj.isNull()) 
	{
		status.perror("no animCurveObj");
		return status;
	}*/
	MFnAnimCurve animCurve(animCurveObj, &status);
	type = animCurve.animCurveType();

	char cvname[200];
	char cvname2[200];
	int comp = 0;
	int dim = 1;
	bool bDoCompileKeys = true;
	std::string conname(connectionName.asChar());
	std::string objowner(MFnDependencyNode(owner).name().asChar());
	std::string objname(objowner + std::string("_") + conname);//MFnDependencyNode(animCurveObj).name().asChar());
	int pos = objname.find("rotate");
	if(pos > 0)
	{
		objname.replace(pos, 6, "rotation");
	}
    // lets take the name of the object to which the curve is connected...
	strcpy(cvname, objname.c_str());
	//
	// Try to find if this curve is a component of a vector.
	// HACK. TODO : find a better way
	//
	if(cvname[strlen(cvname)-1]=='X')
	{
		char *c = cvname + strlen(cvname)-1;
		*c = '\0';
		sprintf(cvname2, "%s[0]",cvname);
		bDoCompileKeys = true;
		dim = 1;
		comp=0;
	}
	else if((cvname[strlen(cvname)-1]=='Y'))
	{
		char *c = cvname + strlen(cvname)-1;
		*c = '\0';
		sprintf(cvname2, "%s[1]",cvname);
		bDoCompileKeys = false;
		dim = 2;
		comp=1;
	}
	else if(cvname[strlen(cvname)-1]=='Z')
	{
		char *c = cvname + strlen(cvname)-1;
		*c = '\0';
		sprintf(cvname2, "%s[2]",cvname);
		bDoCompileKeys = false;
		dim = 3;
		comp=2;
	}
	else if((s=strstr(cvname, "_input"))) // FOR the dynamic simulation case... Note that it is very limited technique. To improve.
	{
		char n=0;
		s--;
		if((*s >= '0') && (*s <= '9'))
		{
			n = (*s - '0');
			s--;
		}
		if(s[0] == 'x')
		{
			s[0] = '0' + n;
			s[1] = '\0';
			sprintf(cvname2, "%s[0]",cvname);
			bDoCompileKeys = true;
			dim = 1;
			comp=0;
		}
		else if(s[0] == 'y')
		{
			s[0] = '0' + n;
			s[1] = '\0';
			sprintf(cvname2, "%s[1]",cvname);
			bDoCompileKeys = false;
			dim = 2;
			comp=1;
		}
		else if(s[0] == 'z')
		{
			s[0] = '0' + n;
			s[1] = '\0';
			sprintf(cvname2, "%s[2]",cvname);
			bDoCompileKeys = false;
			dim = 3;
			comp=2;
		}
		else strcpy(cvname2, cvname);
	}
	else strcpy(cvname2, cvname);
	//
	// Find existing curve to add the new component
	// If doesn't exist : 
	//
	if(m_VectorsProcessed.find(MString(cvname)) == m_VectorsProcessed.end())
	{
		//
		// Write the last curve to file
		//
		if(	(header1.length() > 0 )
		  &&(nkeys > 0))
		{
			tmpstringfp.clear();
			if(fp2)
			{
				fprintf(fp2, "%s", header21.asChar());
				fprintf(fp2, "%d", nkeys);
				fprintf(fp2, "%s", header22.asChar());
				fprintf(fp2, "%s", tmpstringfp2.asChar());
				tmpstringfp2.clear();
			}
			nkeys = 0;
			header1.clear();
		}
		//
		// check if the animcurve was ok (status)
		//
		IFFAILUREMSG("Could not read the anim curve for export.");
		//
		// Add the new vector, to keep track of it
		//
		m_VectorsProcessed.insert(MString(cvname));
		bool btunit = ((type==MFnAnimCurve::kAnimCurveTA)
			||(type==MFnAnimCurve::kAnimCurveTL)
			||(type==MFnAnimCurve::kAnimCurveTT)
			||(type==MFnAnimCurve::kAnimCurveTU))? true : false ;
		sprintf(tmpstr,"e.newcv(\"%s\", %d)\n", cvname, dim); 
		header1 = tmpstr;
		sprintf(tmpstr,"%s.setup(%d,%d,%d,\"%s\",\"%s\")\n", cvname,
			btunit,
			((type==MFnAnimCurve::kAnimCurveTA)||(type==MFnAnimCurve::kAnimCurveUA)) ? 1:0,
			animCurve.isWeighted() ? 1 : 0,
			infinityTypeAsWord(animCurve.preInfinityType()),
			infinityTypeAsWord(animCurve.postInfinityType())
			);  
		header1 += tmpstr;
		if(fp2)
		{
            // we take the name of the object on which the curve is connected...
          	//char *cvnamecut = strchr(cvname, '_');
            //if(owner == animCurveObj)
			    sprintf(tmpstr,"name:%s keys:", cvname, nkeys); 
            /*else
			    sprintf(tmpstr,"name:%s%s keys:", objowner.c_str(), cvnamecut, nkeys); */
			header21 = tmpstr;
			sprintf(tmpstr," dim:%d\ninputistime:%d outputisangular:%d isweighted:%d preinftype:%s postinftype:%s\n",
				dim, btunit,
				((type==MFnAnimCurve::kAnimCurveTA)||(type==MFnAnimCurve::kAnimCurveUA)) ? 1:0,
				animCurve.isWeighted() ? 1 : 0,
				infinityTypeAsWord(animCurve.preInfinityType()),
				infinityTypeAsWord(animCurve.postInfinityType())
				); 
			header22 = tmpstr;
		}
		//
		// Keep track of the curve
		// because later we'll need to make connections
		// 
		m_CurvesProcessed.insert(make_pair(MString(cvname), cvprops(btunit, dim)));
	}
	IFFAILUREMSG("Could not read the anim curve for export.");
	//
	//	These units default to the units in the header of the file.
	//	
	if (verboseUnits) {
		tmpstringfp += "\tinputUnit=\"";
		if (animCurve.isTimeInput()) {
			animUnitNames::setToShortName(timeUnit, unitName);
			sprintf(tmpstr,"%s\"\n",unitName.asChar()); tmpstringfp += tmpstr;
		} else {
			//	The anim curve has unitless input.
			//
			tmpstringfp += "unitless\"\n";
		}
		tmpstringfp +="\toutputUnit=\"";
	}

	double conversion = 1.0;
	switch (type) {
		case MFnAnimCurve::kAnimCurveTA:
		case MFnAnimCurve::kAnimCurveUA:
			animUnitNames::setToShortName(angularUnit, unitName);
			if (verboseUnits) 
			{
				sprintf(tmpstr,"%s\"\n",unitName); tmpstringfp += tmpstr;
			}
			{
				MAngle angle(1.0);
				conversion = angle.as(angularUnit);
			}
			break;
		case MFnAnimCurve::kAnimCurveTL:
		case MFnAnimCurve::kAnimCurveUL:
			animUnitNames::setToShortName(linearUnit, unitName);
			if (verboseUnits) 
			{
				sprintf(tmpstr,"%s\"\n",unitName); tmpstringfp += tmpstr;
			}
			{
				MDistance distance(1.0);
				conversion = distance.as(linearUnit);
			}
			break;
		case MFnAnimCurve::kAnimCurveTT:
		case MFnAnimCurve::kAnimCurveUT:
			animUnitNames::setToShortName(timeUnit, unitName);
			if (verboseUnits) 
			{
				sprintf(tmpstr,"%s\"\n",unitName); tmpstringfp += tmpstr;
			}
			break;
		default:
			if (verboseUnits) 
			{
				sprintf(tmpstr,"unitless\"\n"); tmpstringfp += tmpstr;
			}
			break;
	}
	if (verboseUnits) 
	{
		MString angleUnitName;
		animUnitNames::setToShortName(angularUnit, angleUnitName);
		sprintf(tmpstr,"\ttangentAngleUnit=\"%s\"\n", angleUnitName.asChar()); tmpstringfp += tmpstr;
	}
	//
	// And then write out each keyframe
	//
	unsigned numKeys = animCurve.numKeyframes();
	for (unsigned i = 0; i < numKeys; i++) 
	{
		nkeys++;
		sprintf(tmpstr,"%s.addkey(",cvname2); tmpstringfp += tmpstr;
		if(fp2) sprintf(tmpstr,"comp:%d", comp); tmpstringfp2 += tmpstr;
		if (animCurve.isUnitlessInput()) 
		{
			sprintf(tmpstr,"%d,", animCurve.unitlessInput(i)); tmpstringfp += tmpstr;
			if(fp2) sprintf(tmpstr," frm:%d", animCurve.unitlessInput(i)); tmpstringfp2 += tmpstr;
		}
		else 
		{
			sprintf(tmpstr,"%f, ", animCurve.time(i).value()); tmpstringfp += tmpstr;
			if(fp2) sprintf(tmpstr," frm:%f", animCurve.time(i).value()); tmpstringfp2 += tmpstr;
		}

		sprintf(tmpstr,"%f, ", (float)(conversion*animCurve.value(i))); tmpstringfp += tmpstr;
		if(fp2) sprintf(tmpstr," val:%f", (float)(conversion*animCurve.value(i))); tmpstringfp2 += tmpstr;

		sprintf(tmpstr,"\"%s\", ", tangentTypeAsWord(animCurve.inTangentType(i))); tmpstringfp += tmpstr;
		if(fp2) sprintf(tmpstr," %s", tangentTypeAsWord(animCurve.inTangentType(i))); tmpstringfp2 += tmpstr;
		sprintf(tmpstr,"\"%s\", ", tangentTypeAsWord(animCurve.outTangentType(i))); tmpstringfp += tmpstr;
		if(fp2) sprintf(tmpstr," %s", tangentTypeAsWord(animCurve.outTangentType(i))); tmpstringfp2 += tmpstr;

		sprintf(tmpstr,"%d,%d,%d",
			(animCurve.tangentsLocked(i) ? 1 : 0),
			(animCurve.weightsLocked(i) ? 1 : 0),
			(animCurve.isBreakdown(i) ? 1 : 0)); tmpstringfp += tmpstr;
		if(fp2) sprintf(tmpstr," %d %d %d",
			(animCurve.tangentsLocked(i) ? 1 : 0),
			(animCurve.weightsLocked(i) ? 1 : 0),
			(animCurve.isBreakdown(i) ? 1 : 0)); tmpstringfp2 += tmpstr;

		//if(animCurve.inTangentType(i) == MFnAnimCurve::kTangentFixed) {
			MAngle angle;
			double weight;
			animCurve.getTangent(i, angle, weight, true);

			sprintf(tmpstr,", %f, %f", (float)angle.as(angularUnit), weight); tmpstringfp += tmpstr;
			if(fp2) sprintf(tmpstr," %f %f", (float)angle.as(angularUnit), weight); tmpstringfp2 += tmpstr;
		/*}
		else 
		{
			sprintf(tmpstr,", 0,0"); tmpstringfp += tmpstr;
			if(fp2) sprintf(tmpstr," 0 0"); tmpstringfp2 += tmpstr;
		}
		if (animCurve.outTangentType(i) == MFnAnimCurve::kTangentFixed) {
			MAngle angle;
			double weight;*/
			animCurve.getTangent(i, angle, weight, false);

			sprintf(tmpstr,", %f, %f",  (float)angle.as(angularUnit), weight); tmpstringfp += tmpstr;
			if(fp2) sprintf(tmpstr," %f %f",  (float)angle.as(angularUnit), weight); tmpstringfp2 += tmpstr;
		/*}
		else 
		{
			sprintf(tmpstr,", 0,0"); tmpstringfp += tmpstr;
			if(fp2) sprintf(tmpstr," 0 0"); tmpstringfp2 += tmpstr;
		}*/
		sprintf(tmpstr,")\n"); tmpstringfp += tmpstr;
		if(fp2) sprintf(tmpstr,"\n"); tmpstringfp2 += tmpstr;
	}
	if(bDoCompileKeys)
	{
		sprintf(tmpstr,"%s.compilekeys()\n",cvname); tmpstringfp += tmpstr;
	}
	return status;
}
#endif

void bk3dTranslator::resetUnits()
{
	timeUnit = MTime::uiUnit();
	linearUnit = MDistance::uiUnit();
	angularUnit = MAngle::kDegrees;//MAngle::uiUnit();
}
 /*******************************************************/ /**
	Gather the curves data and store them for later : when we'll
	have to write them in bk3d pool
 **/
MStatus bk3dTranslator::processAnimCurveAsBinary(MObject &animCurveObj, MObject &owner, MString &connectionName, bool verboseUnits)
{
	char *s;
	MFnAnimCurve::AnimCurveType type;
	MStatus status = MS::kSuccess;
	static int nkeys = 0;
	MFnAnimCurve animCurve(animCurveObj, &status);
	type = animCurve.animCurveType();

	char cvVectorName[200];
	char cvName[200];
	unsigned int comp = 0;
	int dim = 1;
	std::string conname(connectionName.asChar());
	std::string objowner(MFnDependencyNode(owner).name().asChar());
	std::string objname(objowner + std::string("_") + conname); //MFnDependencyNode(animCurveObj).name().asChar());
	int pos = objname.find("rotate");
	if(pos > 0)
	{
		objname.replace(pos, 6, "rotation");
	}
    // lets take the name of the object to which the curve is connected...
	strcpy(cvVectorName, objname.c_str());
	//
	// Try to find if this curve is a component of a vector.
	// HACK. TODO : find a better way
	//
	if(cvVectorName[strlen(cvVectorName)-1]=='X')
	{
		char *c = cvVectorName + strlen(cvVectorName)-1;
		*c = '\0';
		sprintf(cvName, "%s[0]",cvVectorName);
		comp=0;
	}
	else if((cvVectorName[strlen(cvVectorName)-1]=='Y'))
	{
		char *c = cvVectorName + strlen(cvVectorName)-1;
		*c = '\0';
		sprintf(cvName, "%s[1]",cvVectorName);
		comp=1;
	}
	else if(cvVectorName[strlen(cvVectorName)-1]=='Z')
	{
		char *c = cvVectorName + strlen(cvVectorName)-1;
		*c = '\0';
		sprintf(cvName, "%s[2]",cvVectorName);
		comp=2;
	}
	else if((s=strstr(cvVectorName, "_input"))) // FOR the dynamic simulation case... Note that it is very limited technique. To improve.
	{
		char n=0;
		s--;
		if((*s >= '0') && (*s <= '9'))
		{
			n = (*s - '0');
			s--;
		}
		if(s[0] == 'x')
		{
			s[0] = '0' + n;
			s[1] = '\0';
			sprintf(cvName, "%s[0]",cvVectorName);
			comp=0;
		}
		else if(s[0] == 'y')
		{
			s[0] = '0' + n;
			s[1] = '\0';
			sprintf(cvName, "%s[1]",cvVectorName);
			comp=1;
		}
		else if(s[0] == 'z')
		{
			s[0] = '0' + n;
			s[1] = '\0';
			sprintf(cvName, "%s[2]",cvVectorName);
			comp=2;
		}
		else strcpy(cvName, cvVectorName);
	}
	else strcpy(cvName, cvVectorName);
	//
	// Find/Create existing curve to add the new component
	//
    bk3dlib::PCurveVec pCvVec = m_bk3dHeader->GetCurveVec(cvVectorName);
    if(!pCvVec)
    {
        // comp should always be the highest because the Z component should alway arrive first,
        // given that we sorted the curves
        pCvVec = bk3dlib::CurveVec::Create(cvVectorName, comp+1);
        m_bk3dHeader->AttachCurveVec(pCvVec);
    }
	bool btunit = ((type==MFnAnimCurve::kAnimCurveTA)
		||(type==MFnAnimCurve::kAnimCurveTL)
		||(type==MFnAnimCurve::kAnimCurveTT)
		||(type==MFnAnimCurve::kAnimCurveTU))? true : false ;
    // NOTE: the same props are for all the components of the same vector...
    // This could lead to a problem if this is not the case from Maya.
    // Shall we check and raise a warning ?
    pCvVec->SetProps(getInfinityType(animCurve.preInfinityType()),getInfinityType(animCurve.postInfinityType())
        ,btunit
        ,((type==MFnAnimCurve::kAnimCurveTA)||(type==MFnAnimCurve::kAnimCurveUA)) ? true:false
        ,animCurve.isWeighted() ? true : false);

    //
    // Depending on type, setup the convertion
    //
	double conversion = 1.0;
	switch (type) {
		case MFnAnimCurve::kAnimCurveTA:
		case MFnAnimCurve::kAnimCurveUA:
			{
				MAngle angle(1.0);
				conversion = angle.as(angularUnit);
			}
			break;
		case MFnAnimCurve::kAnimCurveTL:
		case MFnAnimCurve::kAnimCurveUL:
			{
				MDistance distance(1.0);
				conversion = distance.as(linearUnit);
			}
			break;
		case MFnAnimCurve::kAnimCurveTT:
		case MFnAnimCurve::kAnimCurveUT:
			break;
		default:
			break;
	}
	//
	// And then write out each keyframe
	//
	unsigned numKeys = animCurve.numKeyframes();
	for (unsigned i = 0; i < numKeys; i++) 
	{
        float time;
		if (animCurve.isUnitlessInput()) 
			time = (float)animCurve.unitlessInput(i);
		else 
			time = (float)animCurve.time(i).value();

		float value = (float)(conversion*animCurve.value(i));
		bk3dlib::CurveVec::EtTangentType  inTT = getTangentType(animCurve.inTangentType(i));
		bk3dlib::CurveVec::EtTangentType outTT = getTangentType(animCurve.outTangentType(i));

		MAngle angle;
		double weight;
		animCurve.getTangent(i, angle, weight, true);
		float inAngle = (float)angle.as(angularUnit);
		float inWeight = (float)weight;

		animCurve.getTangent(i, angle, weight, false);
		float outAngle = (float)angle.as(angularUnit);
		float outWeight = (float)weight;
		//
		// store the key
		//
        pCvVec->AddKey(comp, time, value, inTT, outTT, inAngle, inWeight, outAngle, outWeight);
	}
	return status;
}

 /*******************************************************/ /**
	We need the connected curves to be sorted from Z to A
	so we can guess the size of the vector : if name ends with Z... 3D; Y : 2D...
 **/
MStatus bk3dTranslator::ProcessSortedCurves()
{
	PF2(("ProcessSortedCurves...%d curves\n", m_curvesSorted.size()));
	MStatus status;
	map<MString, CurveAndOwner , ltstr_inv>::iterator iIter;
	for (iIter = m_curvesSorted.begin() ; iIter != m_curvesSorted.end();)
	{
		CurveAndOwner cao = iIter->second;
        MString s(iIter->first);
#ifdef DOCURVETEXTEXPORT
		if(bAnimCVsAsText)
		{
			if(!(status = processAnimCurveAsText(cao.curve, cao.owner, cao.connectionName)))
			{
				status.perror("Warning : some curves could not be processed");
			}
		} else
#endif
		{
			if(!(status = processAnimCurveAsBinary(cao.curve, cao.owner, cao.connectionName)))
			{
				status.perror("Warning : some curves could not be processed");
			}
		}
		++iIter;
	}
	m_curvesSorted.clear();
	return status;
}
#if 0
...
if(!(status = ProcessDagNode(curobj, 0, true)))
		{
			status.perror("Warning : some Nodes could not be processed");
		}
	}

	// Selection list loop
	PRocessSortedCurves();
	//
	// to flush what's in the buffer
	//
	processAnimCurveAsText(MObject(), MObject());
	//
	// perform connections
	//
	fprintf(fp, "print \"Connections...\\n\"\n");
  if(fp3) 
    fprintf(fp3, "// Connections : you can add this into your code\n");
	map<MString, bk3dTranslator::cvprops, ltstr>::iterator iObj;
	iObj = m_CurvesProcessed.begin();
	while(iObj != m_CurvesProcessed.end())
	{
		char tmp[100], *c;
		strcpy(tmp, iObj->first.asChar());
		c = tmp;
		while(*c != '\0')
		{
			//
			// Here we make a connections with an object containing the plug.
			// the name of the object is the first part before '_'
			// then the plug name is the rest, after '_'
			// example: "obj_scaleplug" -> "obj.scaleplug"
			//			"obj_in_scaleplug" -> "obj.in_scaleplug"
			//
			if(*c == '_')
			{
				*c = '.';
				break;
			}
			c++;
		}
    //
    // Python connections
    //
    fprintf(fp,"%s.plugout.connectto(%s)\n", iObj->first.asChar(), tmp);
    if(iObj->second.bTimeIn)
    {
	    fprintf(fp,"timeline.connectto(%s.plugin)\n", iObj->first.asChar());
    }
    //
    // C++ connections
    //
    if(iObj->second.bTimeIn)
    {
      fprintf(fp3, "  CV_CONNECT_IN(\"%s\", &timeline);\n", iObj->first.asChar());
    }
    if(iObj->second.dim == 1)
      fprintf(fp3,"  CV_CONNECT_OUT1D(\"%s\", &%s);\n", iObj->first.asChar(), tmp);
    else
      fprintf(fp3,"  CV_CONNECT_OUT%dD(\"%s\", %s);\n", iObj->second.dim, iObj->first.asChar(), tmp);
    ++iObj;
	}
	fprintf(fp, "print \"Done\\n\"\n");
	return status;
}
#endif
/******************************************************************************************/ /**
	walk through the connections.
 **/
MStatus bk3dTranslator::WalkConnections(MObject &objectOwner)
{
	const char * nodename;
	MStatus status;
	MPlugArray plugarray;
	MFnDependencyNode depNode(objectOwner, &status);
	if(!status)
		return status;
    /* EASIER:
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
    // We found a skin cluster!
    MFnSkinCluster skinCluster(dgIter.thisNode(), &g_status); assert(g_status);
    */
	//
	// Now walk through its plugs
	//
	if(depNode.getConnections( plugarray ))
	{
		for(unsigned int i=0; i<plugarray.length(); i++)
		{
			MPlug plug = plugarray[i]; // here is the plug of the object
			MFnAttribute att1(plug.attribute());
			MPlugArray plugarray2;
			if(plug.connectedTo( plugarray2, true/*asdst*/, false/*assrc*/, &status))
			{
				for(unsigned int j=0; j<plugarray2.length(); j++)
				{
					MPlug plug2 = plugarray2[j]; // here is the other side of the connection
MString mmm = plug.name();
const char *mmmm = mmm.asChar();
mmm = plug2.name();
const char *mmmm2 = mmm.asChar();
MString curveName(plug.name());
const char *cvNameDBG = curveName.asChar();
const char *aaa = att1.name().asChar();
					MObject objectCurve = plug2.node();
					//
					// Check if already handled
					//
					MFnDependencyNode depNode2(objectCurve, &status);
					if(!status)
						continue;
					if(m_NodesProcessed.find(depNode2.name()) != m_NodesProcessed.end())
						continue;
					m_NodesProcessed.insert(depNode2.name());

					MFnAttribute att2(plug2.attribute());
					int logicalidx2 = plug2.logicalIndex();
					nodename = plug2.node().apiTypeStr();
					switch(objectCurve.apiType())
					{
					case MFn::kAnimCurve:
					case MFn::kAnimCurveTimeToAngular:
					case MFn::kAnimCurveTimeToDistance:
					case MFn::kAnimCurveTimeToTime:
					case MFn::kAnimCurveTimeToUnitless:
					case MFn::kAnimCurveUnitlessToAngular:
					case MFn::kAnimCurveUnitlessToDistance:
					case MFn::kAnimCurveUnitlessToTime:
					case MFn::kAnimCurveUnitlessToUnitless:
						{
						MString plugName = plug.partialName(false,false,false,false,false,true);
						const char *b = plugName.asChar();
						const char *curveName2 = MFnDependencyNode(objectCurve).name().asChar();
						DPF((">>>> Found curve %s (%s). But naming it as %s...\n",curveName2, nodename, plug.name().asChar()));
						m_curvesSorted[plug.name()/* MFnDependencyNode(objectCurve).name()*/] = CurveAndOwner(objectCurve, objectOwner, plugName);
						break;
						}
					//
					//----> connections to this connected object...
					//
					default:
						WalkConnections(objectCurve);
						break;
					}
				}
			}
		}
	}
	return status;
}


MStatus bk3dTranslator::exportCurve(MObject object, int level, bool bIntermediate)
{
	PF2(("exportCurve"));
	//
	//----> connections
	//
	WalkConnections(object);
	//
	//----> write the curves for this object and the connected ones
	//
	ProcessSortedCurves();

	return MStatus();
}

