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
#ifndef __CURVEENGINE__
#define __CURVEENGINE__

#pragma warning(disable: 4786)
#include <vector>
#include <map>
#include <assert.h>
#include <math.h>

using namespace std;

enum EtInfinityType 
{
	kInfinityConstant=0,
	kInfinityLinear=1,
	kInfinityCycle=3,
	kInfinityCycleRelative=4,
	kInfinityOscillate=5
};
/**
	The Curve class
 **/
class Curve
{
public:
	typedef struct ag_polynomial 
	{
		float *p;
		int deg;
	} AG_POLYNOMIAL;
	struct Key 
	{
		float	time;			/* key time (in seconds)						*/
		float	value;			/* key value (in internal units)				*/
		float	inTanX;			/* key in-tangent x value						*/
		float	inTanY;			/* key in-tangent y value						*/
		float	outTanX;		/* key out-tangent x value						*/
		float	outTanY;		/* key out-tangent y value						*/
	};
protected:
	/// \name the curve data
	//@{
	int		m_numKeys;	///< The number of keys in the anim curve			
	bool	m_isWeighted;	///< whether or not this curve has weighted tangents 
	bool	m_isStatic;	///< whether or not all the keys have the same value 
	bool	m_inputistime; ///< if true, the input do not need Plugs to increase
	EtInfinityType m_preInfinity;		///< how to evaluate pre-infinity			
	EtInfinityType m_postInfinity;	///< how to evaluate post-infinity		
	// evaluate cache
	Key *		m_lastKey;	///< lastKey evaluated							
	int		m_lastIndex;	///< last index evaluated							
	int		m_lastInterval;	///< last interval evaluated					
	bool	m_isStep;		///< whether or not this interval is a step interval 
	bool	m_isLinear;	///< whether or not this interval is linear		
	float		m_fX1;		///< start x of the segment						
	float		m_fX4;		///< end x of the segment							
	float		m_fCoeff[4];	///< bezier x parameters (only used for weighted curves 
	float		m_fPolyY[4];	///< bezier y parameters							

	vector<Key>	m_keyList;
	//@}

	static float sMachineTolerance;

	int	dbl_gt(float *a, float *b)
	{
		return (*a > *b ? 1 : 0);
	}
	void	dbl_mult(float *a, float *b, float *atimesb)
	{
		float product = (*a) * (*b);
		*atimesb = product;
	}
	void	dbl_add(float *a, float *b, float *aplusb)
	{
		float sum = (*a) + (*b);
		*aplusb = sum;
	}
	void	init_tolerance();
	void	constrainInsideBounds(float *x1, float *x2);
	void	checkMonotonic(float *x1, float *x2);
	void	bezierToPower(	float a1, float b1, float c1, float d1, 	float *a2, float *b2, float *c2, float *d2);
	float	ag_horner1(float P[], int deg, float s);
	float	ag_zeroin2(float a, float b, float fa, float fb, float tol, AG_POLYNOMIAL *pars);
	float	ag_zeroin(float a, float b, float tol, AG_POLYNOMIAL *pars);
	int	polyZeroes(float Poly[], int deg, float a, int a_closed, float b, int b_closed, float Roots[]);
	void	engineHermiteCreate(float x[4], float y[4]);
	float	engineHermiteEvaluate(float time);
	void	engineBezierCreate(float x[4], float y[4]);
	float	engineBezierEvaluate(float time);
public:
	Curve();
	~Curve();
	bool find(float time, int *index);
	float evaluate(float time);
    float getClosestKeyTime(float time, bool backward=false);
	float evaluateInfinities(float time, bool evalPre);
};

/* local constants */
#define kDegRad 0.0174532925199432958
#define kFourThirds (4.0 / 3.0)
#define kTwoThirds (2.0 / 3.0)
#define kOneThird (1.0 / 3.0)
#define kMaxTan 5729577.9485111479





#endif