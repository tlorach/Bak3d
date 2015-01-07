/*-----------------------------------------------------------------------
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
*/ //--------------------------------------------------------------------
#ifndef __CURVEREADER__
#define __CURVEREADER__

#include "curveEngine.h"
#pragma warning(disable:4201 ) //C4201: nonstandard extension used : nameless struct/union

using namespace std;

class CurvePool;
class CurveReader;

enum EtTangentType 
{
	kTangentGlobal = 0,
	kTangentFixed,
	kTangentLinear,
	kTangentFlat,
	kTangentSmooth,
	kTangentStep,
	kTangentSlow,
	kTangentFast,
	kTangentClamped,
	kTangentPlateau,
    kTangentStepNext
};
typedef enum EtTangentType EtTangentType;

struct ReadKey
{
	float			time;
	float			value;
	EtTangentType	inTangentType;
	EtTangentType	outTangentType;
	float			inAngle;
	float			inWeight;
	float			outAngle;
	float			outWeight;
};
/*----------------------------------------------------------------------------------*/ /**

We are grouping curves so that we may have a vector from 1 to 4 dimensions.
This is interesting for connections with another plug which would be a vector

**/ //----------------------------------------------------------------------------------
class CurveVector
{
protected:
	union 
	{
		CurveReader *m_cvs[4]; ///< the 1 to 4 curves
		struct
		{
			CurveReader *m_cvx;
			CurveReader *m_cvy;
			CurveReader *m_cvz;
			CurveReader *m_cvw;
		};
	};
	int m_dim; ///< dimension of the 'vector'
	char * m_name; ///< the name. Pointed by CurvePool::CurveMapType
	float m_time, m_cvvals[4], m_cvvalsBase[4];
    float* m_bindPtr[4]; // Pointer to destinations we want the Vector to update directly
    unsigned char*   m_pDirty; // pointer to the 'dirty' flag of the element that own the bindPtr stuff.
public:
	CurveVector(const char * name, int dim);
	~CurveVector();
	/// \name almost the same methods of Curve class but for a 'vector'
	//@{
	bool find(float time, int *indexx, int *indexy, int *indexz, int *indexw);
    void bindPtrs(float *v1=NULL, float *v2 = NULL, float *v3 = NULL, float *v4 = NULL, unsigned char *pDirty=NULL);
    void setBaseValues(float v1, float v2, float v3, float v4) { m_cvvalsBase[0]=v1; m_cvvalsBase[1]=v2; m_cvvalsBase[2]=v3; m_cvvalsBase[3]=v4; };
    void unbindPtrs();
	float evaluate(float time, float *v1=NULL, float *v2 = NULL, float *v3 = NULL, float *v4 = NULL);
    float getClosestKeyTime(float time, bool backward=false);
	float evaluateInfinities(float time, bool evalPre, float *v1 = NULL, float *v2 = NULL, float *v3 = NULL, float *v4 = NULL);
    void  startKeySetup(bool inputIsTime, bool outputIsAngular, bool isWeighted,
			  EtInfinityType preinftype, EtInfinityType postinftype);
	void addKey(float frame, float x, float y, float z, float w, 
				EtTangentType inTangentType=kTangentSmooth, EtTangentType outTangentType=kTangentSmooth, 
				float inAngle=0, float inWeight=0, float outAngle=0, float outWeight=0);
	void addKeyHere(EtTangentType inTangentType=kTangentSmooth, EtTangentType outTangentType=kTangentSmooth, 
				float inAngle=0, float inWeight=0, float outAngle=0, float outWeight=0);
	//@}
	virtual CurveReader *getCurve(int n);
	virtual void clear(int n=-1);

    const char* getName() { return m_name; }

    bool    m_ON;           ///<to make it active or not active

    friend CurvePool;
};

class CurveQuat
{
public:
    struct Key {
        float time;
        float value[4];
    };
protected:
	char *  m_name; ///< the name. Pointed by CurvePool::CurveMapType
	float   m_time, m_cvvals[4];
    float*  m_bindPtr; // Pointer to destinations we want the Vector to update directly
    unsigned char*   m_pDirty; // pointer to the 'dirty' flag of the element that own the bindPtr stuff.

	int		m_numKeys;	///< The number of keys in the anim curve			
	Key *	m_lastKey;	///< lastKey evaluated							
	int		m_lastIndex;	///< last index evaluated							
	int		m_lastInterval;	///< last interval evaluated					
	bool	m_isStep;		///< whether or not this interval is a step interval 

	std::vector<Key>	m_keys;
    bool    find (float time, int *index);
public:
	CurveQuat(const char * name);
	~CurveQuat();
	/// \name almost the same methods of Curve class but for a 'vector'
	//@{
    void    bindPtr(float *v=NULL, unsigned char *pDirty=NULL);
    void    unbindPtrs();
	void    evaluate(float time, float *v=NULL);
    float   getClosestKeyTime(float time, bool backward=false);
    void    startKeySetup();
	void    endKeySetup();
	void    addKey(float frame, float *q);
	void    addKeyHere(float *q);
	//@}
	virtual void clear(int n=-1);

    const char* getName() { return m_name; }

    bool    m_ON;           ///<to make it active or not active

    friend CurvePool;
};
/*----------------------------------------------------------------------------------*/ /**

- contains a pool of allocated curves
.

**/ //----------------------------------------------------------------------------------
class CurvePool
{
public:
	~CurvePool();
	virtual void clear();

	CurveQuat   *getQuatCV(const char *  name);
    CurveQuat   *getQuatCVByIndex(int i) {return ( i < (int)m_quatcurves.size()) ? m_quatcurves[i] : 0; };
	CurveVector *getCV(const char *  name);
    CurveVector *getCVByIndex(int i) {return ( i < (int)m_curvesVec.size()) ? m_curvesVec[i] : 0; };
	int          getNumQuatCV() {return (int)m_quatcurves.size(); };
	int          getNumCV() {return (int)m_curves.size(); };
    CurveQuat   *newQuatCV(const char *  name);
    CurveVector *newCV(const char *  name, int dim);
	CurveVector *newCVFromFile(const char *  fname, char *  overloadname = NULL);

    void updateAll(float time);
	void updateAllInfinities(float time, bool evalPre);
    void updateToClosestKey(bool backward=false);
    float getCurTime() { return curTime; }
private:
    float curTime;
	struct ltstr
	{
	  bool operator()(const char * s1, const char * s2) const
	  {
		return strcmp(s1, s2) < 0;
	  }
	};

    typedef std::map<const char *, CurveVector*, ltstr> CurveMapType;
	CurveMapType m_curves;
    std::vector<CurveVector*> m_curvesVec;

	typedef std::map<const char *, CurveQuat*, ltstr> CurveQuatMapType;
	CurveQuatMapType m_mapquatcurves;
    std::vector<CurveQuat*> m_quatcurves;
};


class CurveReader : public Curve
{
public:
	CurveReader();
	~CurveReader() {};
protected:
	//EtTangentType AsTangentType (const char *str);
	bool assembleAnimCurve(vector<ReadKey> &keys, bool isWeighted, bool useOldSmooth);

	vector<ReadKey> m_keys; ///< keys which define the curve
	float			m_unitConversion;
	float			m_frameRate;
	
public:
	virtual void setName(const char *  name) {}
	virtual void startKeySetup(bool inputIsTime=true, bool outputIsAngular=false, bool isWeighted=false,
						EtInfinityType preinftype=kInfinityConstant, EtInfinityType postinftype=kInfinityConstant);
	virtual void getKeySetup(bool &inputIsTime, bool &outputIsAngular, bool &isWeighted,
						EtInfinityType &preinftype, EtInfinityType &postinftype);
	virtual void addKey(float frame, float val, 
				EtTangentType inTangentType=kTangentSmooth, EtTangentType outTangentType=kTangentSmooth, 
				float inAngle=0, float inWeight=0, float outAngle=0, float outWeight=0);
	virtual void endKeySetup();

	virtual int getNumKeys() { return (int)m_keys.size(); };
	bool getKey(int n, ReadKey &k);
	virtual void clear();
	virtual bool delkey(int nkey);

	friend CurveVector;
};
#endif
