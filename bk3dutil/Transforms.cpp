#pragma warning(disable:4312)

#include "assert.h"

#include <vector>
#include <string>

#include "bk3dlib.h"
#define NOGZLIB
#include "bk3dEx.h"
#include "nvModel.h"

#include "nv_math.h"
using namespace nv_math;

bool ComputeMatrixSimple(bk3d::TransformSimple * tr, bool bComputeBindPose, bool forceDirty, int parentDirty);
bool ComputeMatrixBone(bk3d::Bone * tr, bool bComputeBindPose, bool forceDirty, int parentDirty);
//-----------------------------------------------------------------------------
// Computes the DOF quaternions
//-----------------------------------------------------------------------------
bool computeDOFQuat(bk3d::Bone * tr)
{
    if(tr->getTransformDOF() == NULL)
        return false;
    quatf qp(0,0,0,1);
    // the DOF is relative the the parent of the transformation that it is contraining
    // otherwise the DOF would rotate with the one that it must contraint...
    if(tr->getParent())
        qp = (quatf&)tr->getParent()->QuatAbs();
    quatf& q = (quatf&)tr->getTransformDOF()->Quat();
    quatf& qabs = (quatf&)tr->getTransformDOF()->QuatAbs();
    qabs = qp * q;
    qabs.normalize();
    return true;
}

//-----------------------------------------------------------------------------
// Compute or update all the components of the transformation
// If necessary, it will call the parent's update prior to finalizing the work
// If possible, it will use Quaternions but still maintain the matrices
// Note that the Quaternion case may not be 100% compatible with Maya complex transformations (Pivots...)
//-----------------------------------------------------------------------------
bool ComputeMatrix(bk3d::Transform * tr, bool bComputeBindPose, bool forceDirty, bool bParentChanged)
{
    mat4f m_matrix, m_abs_matrix, m_bindpose_matrix;
    bool bChanged = false;

    // copy the original matrix, if ever this matrix is already available and bDirty is false
    memcpy(m_matrix.mat_array, tr->Matrix(), sizeof(float)*16);
    if(forceDirty)
    {
        tr->ValidComps() &= ~(TRANSFCOMP_matrix_ready);
        tr->setDirty(true);
    }
    if((tr->getDirty() || forceDirty))
    {
        // case where the matrix is really the main source of transformation
        if(tr->ValidComps() & TRANSFCOMP_matrix)
        {
            //if(!(tr->getValidComps() & TRANSFCOMP_matrix_ready))
            {
                bChanged = true;
                tr->ValidComps() |= TRANSFCOMP_matrix_ready;
                tr->ValidComps() &= ~(TRANSFCOMP_abs_matrix_ready|TRANSFCOMP_abs_Quat_ready);

                // compute the quaternion rotation from it, for consistency
                // warning: scaling could be a problem
                quatf *Q = (quatf *)(float*)tr->Quat();
                Q->from_matrix(m_matrix);
                tr->ValidComps() |= TRANSFCOMP_Quat;
                memcpy(tr->Pos(), tr->Pos(), 3);
                tr->ValidComps() |= TRANSFCOMP_pos;

                Q->to_euler_xyz(tr->Rotation());
                tr->Rotation()[0] *= nv_to_deg;
                tr->Rotation()[1] *= nv_to_deg;
                tr->Rotation()[2] *= nv_to_deg;
            }
        } else
        {
            quatf Q(0.0f, 0.0f, 0.0f, 1.0f);
            // because some components are now used, turn the matrix to Identity and re-compute it from scratch
            m_matrix.identity();
            // Joint transformation order (OpenGL order) :
            // P2 = Mtranslate * MparentScaleInverse * MjointOrient * Mrotation * Mrotorientation * Mscale * PosVec
            // P2 = [T]        * [IS]                * [JO]         * [R]       * [RO]            * [S]    * P
            // from maya doc : matrix = [S] * [RO] * [R] * [JO] * [IS] * [T]
            // basic Transformation order :
            // P2 = Mtranslate * MrotPivotTransl * 
            //      MrotPivot * Mrotation * MrotOrient * MrotPivotInv * 
            //      MscalePivotTransl * MscalePivot * Mscale * MscalePivotInv * PosVec
            // In maya doc (DX matrix orientation) :
            //             -1                      -1
            // p' = p x [Sp]x[S]x[Sh]x[Sp]x[St]x[Rp]x[Ro]x[R]x[Rp]x[Rt]x[T]


            //translation (T)
            if(tr->ValidComps() & TRANSFCOMP_pos)
            {
                bChanged = true;
                m_matrix.translate(tr->Pos());
            }
            // parent scale inverse for JOINTs: TODO

            //rotation pivot translate (Rt)
            if(tr->ValidComps() & TRANSFCOMP_rotationPivotTranslate)
            {
                bChanged = true;
                m_matrix.translate(tr->RotationPivotTranslate());
            }
            //JOINT ONLY : orientation
            if(tr->ValidComps() & TRANSFCOMP_jointOrientation)
            {
                bChanged = true;
                Q = quatf(tr->JointOrientation());
                m_matrix.rotate(Q);
            }
	        //Rotation
            // rotation-pivot (Rp)
            if(tr->ValidComps() & TRANSFCOMP_rotationPivot)
            {
                bChanged = true;
                m_matrix.translate(vec3f(tr->RotationPivot()[0], tr->RotationPivot()[1], tr->RotationPivot()[2]));
            }
            {
                // if you want Quaternion at any cost, Euler must be canceled
	            //rotation from a Quaternion (==R)
                // NOTE: if we force the dirty flag, we'll give priority to Euler
                if((tr->ValidComps() & TRANSFCOMP_Quat)
                    ||((tr->ValidComps() & TRANSFCOMP_Quat_ready)&&(forceDirty == false)))
                {
                    bChanged = true;
                    quatf Qr(tr->Quat());
                    Q *= Qr;
	                m_matrix.rotate(Qr);
                    tr->ValidComps() |= TRANSFCOMP_Quat_ready;
                    if(tr->ValidComps() & TRANSFCOMP_rotation)
                    {
                        Qr.to_euler_xyz(tr->Rotation());
                        tr->Rotation()[0] *= nv_to_deg;
                        tr->Rotation()[1] *= nv_to_deg;
                        tr->Rotation()[2] *= nv_to_deg;
                    }
                }
	            // TODO: take the order into account (R)
                // Euler rotation was higher priority over Quaternion, but I changed it so that IK can work
                // when quaternions get updated without Euler...
                // Warning : when an anim curve is connected to Euler, we will have to cancel Quaternion every-time
                else if(tr->ValidComps() & TRANSFCOMP_rotation)
                {
                    bChanged = true;
                    // No need : use internal method from the quatf
                    //quatf qz(vec3f(0,0,1), tr->rotation[2] * nv_to_rad);
                    //quatf qy(vec3f(0,1,0), tr->rotation[1] * nv_to_rad);
                    //quatf qx(vec3f(1,0,0), tr->rotation[0] * nv_to_rad);
                    //qz *= qy;
                    //qz *= qx;
                    vec3f e(tr->Rotation());
                    e *= nv_to_rad;
                    quatf qz(e);
                    m_matrix.rotate(qz);
                    // make a copy of this rotation in the quaternion and now the quaternion part is valid
                    memcpy(tr->Quat(), qz.comp, sizeof(nv_scalar)*4);
                    tr->ValidComps() |= TRANSFCOMP_Quat_ready;
                    Q *= qz;
                } else {
                    // if no rotation in this transformation, just update the quaternion
                    bChanged = true;
                    tr->Quat().x = tr->Quat().y = tr->Quat().z = 0.0f;
                    tr->Quat().w = 1.0f;
                    tr->ValidComps() |= TRANSFCOMP_Quat_ready;
                }
	            //rotation orientation (Ro)
                if(tr->ValidComps() & TRANSFCOMP_rotationOrientation)
                {
                    bChanged = true;
                    quatf Qro(tr->RotationOrientation());
                    Q *= Qro;
	                m_matrix.rotate(Qro);
                }
            }
            // inverse rotation-pivot Inv(Rp)
            if(tr->ValidComps() & TRANSFCOMP_rotationPivot)
            {
                bChanged = true;
                mat4f mrp, mrpI;
                mrp.identity();
                mrp.set_translation(vec3f(tr->RotationPivot()[0], tr->RotationPivot()[1], tr->RotationPivot()[2]));
                invert(mrpI, mrp);
                m_matrix *= mrpI;
            }
            // scale-pivot translation (St) introduced to preserve existing scale transformations when moving pivot. 
            // This is used to prevent the object from moving when the objects pivot point is not at the origin 
            // and a non-unit scale is applied to the object 
            if(tr->ValidComps() & TRANSFCOMP_scalePivotTranslate)
            {
                bChanged = true;
                m_matrix.translate(vec3f(tr->ScalePivotTranslate()[0], tr->ScalePivotTranslate()[1], tr->ScalePivotTranslate()[2]));
            }

            // scale-pivot (Sp)
            if(tr->ValidComps() & TRANSFCOMP_scalePivot)
            {
                bChanged = true;
                m_matrix.translate(vec3f(tr->ScalePivot()[0], tr->ScalePivot()[1], tr->ScalePivot()[2]));
            }
            {
                // According to maya doc, there should be a "Shearing" matrix, here... TODO?
	            //scale (S)
                if(tr->ValidComps() & TRANSFCOMP_scale)
                {
                    bChanged = true;
	                m_matrix.scale(vec3f(tr->Scale()));
                }
            }
            // inverse scale-pivot Inv(Sp)
            if(tr->ValidComps() & TRANSFCOMP_scalePivot)
            {
                mat4f mscp, mscpI;
                mscp.identity();
                mscp.set_translation(vec3f(tr->ScalePivot()[0], tr->ScalePivot()[1], tr->ScalePivot()[2]));
                invert(mscpI, mscp);
                m_matrix *= mscpI;
            }
        } // else (if it has components ON)
    } //if(tr->getDirty() || forceDirty)
    // re-compute absolute values (and bindpose if necessary)
    if(bParentChanged || bChanged || (!(tr->ValidComps() & TRANSFCOMP_abs_matrix_ready)))
    {
        bChanged = true;
        // if we have the Quaternion ON, let's update the abs values, too
        if(tr->ValidComps() & (TRANSFCOMP_Quat|TRANSFCOMP_Quat_ready))
        {
            tr->ValidComps() |= TRANSFCOMP_abs_Quat_ready;
            //if(tr->ValidComps() & TRANSFCOMP_rotation) // If Euler is still there... careful !
            //{
            //    // Shall we re-compute the local quaternion from Euler ?
            //    assert(!"Warning : local quaternion overridden by Euler");
            //}
            // NOTE: the Quaternion case certainly missed details from the complex matrix chain from Maya
            // TODO: We totally ignore pivots in this case :-( Need to check what we can do
            //if((!(tr->ValidComps() & (TRANSFCOMP_scalePivot|TRANSFCOMP_scalePivotTranslate|TRANSFCOMP_rotationPivot|TRANSFCOMP_rotationPivotTranslate))))
            //{
            //  PRINTF(("Warning with tr->ValidComps()"));
            //}
            //vec3f VQpos(tr->pos);
            vec3f VQscale(tr->Scale());
            // we assume the whole chain of transformations are consistent : parent has valid Quaternions
            quatf Qabs;
            quatf Q(0.0f, 0.0f, 0.0f, 1.0f);
            if(tr->ValidComps() & TRANSFCOMP_jointOrientation)
            {
                Q = quatf(tr->JointOrientation());
                if(tr->ValidComps() & (TRANSFCOMP_Quat|TRANSFCOMP_Quat_ready))
                    Q *= quatf(tr->Quat());
            } else if(tr->ValidComps() & (TRANSFCOMP_Quat|TRANSFCOMP_Quat_ready))
                Q = quatf(tr->Quat());
            if(tr->ValidComps() & TRANSFCOMP_rotationOrientation)
                Q *= quatf(tr->RotationOrientation());
            if(tr->getParent())
            {
                assert(tr->ValidComps() & TRANSFCOMP_abs_Quat_ready);
                quatf Qparent(tr->getParent()->QuatAbs());
                Qabs = Qparent * Q;
                Qabs.normalize(); // maybe we could this only fewer times
                // correct the scale and pos according to parents
                //VQpos.rotateBy(Qparent);
                //VQpos += vec3f(tr->getParent()->abs_pos);
                if((tr->getParent()->nodeType == NODE_TRANSFORM)||(tr->getParent()->nodeType == NODE_TRANSFORMSIMPLE))
                    VQscale *= vec3f(((bk3d::TransformSimple*)tr->getParent())->ScaleAbs());
            } else {
                Qabs = Q;
            }
            memcpy(tr->QuatAbs(), Qabs.comp, sizeof(nv_scalar)*4);
            //memcpy(tr->abs_pos, VQpos.vec_array, sizeof(nv_scalar)*3);
            memcpy(tr->ScaleAbs(), VQscale.vec_array, sizeof(nv_scalar)*3);
        }
#if 1
        // OROGINAL VERSION
        if(tr->getParent())
            m_abs_matrix = mat4f(tr->getParent()->MatrixAbs()) * m_matrix;
        else
            m_abs_matrix = m_matrix;
#else
// =====================================================================
// TEMPORARY TEMPORARY TEMPORARY TEMPORARY TEMPORARY TEMPORARY TEMPORARY
// Temporary code to debug the Quaternions
        mat4f M;
#if 0
        quatf Q(tr->Quat());
        vec3f P(tr->pos);
        vec3f S(tr->scale);
        struct DODO {
            static void calc(bk3d::Bone *PT, quatf & Q, vec3f & P, vec3f & S)
            {
                quatf Q2;
                vec3f P2, S2;
                if(PT->getParent())
                {
                    DODO::calc(PT->getParent(), Q2, P2, S2);
                    Q = Q2 * quatf(PT->Quat());
                    memcpy(PT->QuatAbs(), Q.comp, sizeof(float)*4);
                    P = vec3f(PT->pos);
                    P.rotateBy(Q2);
                    P += P2;
                    memcpy(PT->abs_pos, P.vec_array, sizeof(float)*3);
                    S = vec3f(PT->scale);
                    S *= S2;
                    memcpy(PT->abs_scale, S.vec_array, sizeof(float)*3);
                } else {
                    Q = quatf(PT->Quat());
                    P = vec3f(PT->pos);
                    S = vec3f(PT->scale);
                }
            }
        };
        DODO::calc(tr, Q, P, S);
#endif
        // create the abs matrix from the quaternions
        quatf Qabs(tr->QuatAbs());
        vec3f Pabs(tr->abs_pos);
        vec3f Sabs(tr->abs_scale);
        M = mat4f(array16_id);
        M.as_translation(Pabs);
        M.rotate(Qabs);
        M.scale(Sabs);

        m_abs_matrix = M;
#endif
// TEMPORARY TEMPORARY TEMPORARY TEMPORARY TEMPORARY TEMPORARY TEMPORARY
// =====================================================================
    }
    // recompute the bindpose only if needed (i.e. the first time)
    if((tr->MatrixInvBindpose()[15] == 0.0)||bComputeBindPose)
    {
        tr->ValidComps() |= TRANSFCOMP_bindpose_matrix;
        invert(m_bindpose_matrix, m_abs_matrix);
        memcpy(tr->MatrixInvBindpose(), m_bindpose_matrix.mat_array, sizeof(float)*16);
    }
    if(bChanged)
    {
        tr->ValidComps() |= TRANSFCOMP_matrix_ready;
        tr->ValidComps() |= TRANSFCOMP_abs_matrix_ready;
        // Add the Quaternion data... if possible
        // Warning
        tr->ValidComps() |= TRANSFCOMP_abs_Quat_ready;
        memcpy(tr->MatrixAbs(), m_abs_matrix.mat_array, sizeof(float)*16);
        memcpy(tr->MatrixAbs(), m_abs_matrix.mat_array, sizeof(float)*16);
        memcpy(tr->Matrix(), m_matrix.mat_array, sizeof(float)*16);
    }
    // DOF quaternions
    if(bParentChanged)
        computeDOFQuat(tr);
    return bChanged;
}

//-----------------------------------------------------------------------------
// Compute or update all the components of the transformation
// If necessary, it will call the parent's update prior to finalizing the work
// If possible, it will use Quaternions but still maintain the matrices
// Note that the Quaternion case may not be 100% compatible with Maya complex transformations (Pivots...)
//-----------------------------------------------------------------------------
bool ComputeMatrixBone(bk3d::Bone * tr, bool bComputeBindPose, bool forceDirty, bool bParentChanged)
{
    //LOGI("Compute Bone :%s (%d)\n", tr->name, tr->ID);
    mat4f m_matrix, m_abs_matrix, m_bindpose_matrix;
    bool bChanged = false;

    // copy the original matrix, if ever this matrix is already available and bDirty is false
    memcpy(m_matrix.mat_array, tr->Matrix(), sizeof(float)*16);
    if(forceDirty)
    {
        tr->ValidComps() &= ~(TRANSFCOMP_matrix_ready);
        tr->setDirty(true);
    }
    if((tr->getDirty() || forceDirty))
    {
        // case where the matrix is really the main source of transformation
        if(tr->ValidComps() & TRANSFCOMP_matrix)
        {
            //if(!(tr->getValidComps() & TRANSFCOMP_matrix_ready))
            {
                bChanged = true;
                tr->ValidComps() |= TRANSFCOMP_matrix_ready;
                tr->ValidComps() &= ~(TRANSFCOMP_abs_matrix_ready|TRANSFCOMP_abs_Quat_ready);
                // compute the quaternion rotation from it, for consistency
                // warning: scaling could be a problem
                quatf *Q = (quatf *)(float*)tr->Quat();
                Q->from_matrix(m_matrix);
                //tr->getValidComps() |= TRANSFCOMP_Quat;
                //tr->getValidComps() |= TRANSFCOMP_pos;
            }
        } else
        {
            quatf Q(0.0f, 0.0f, 0.0f, 1.0f);
            // because some transformation components are now used, turn the matrix to Identity and re-compute it from scratch
            // HOWEVER: translation is already in this matrix. So don't touch it: 9x9 identity used instead
            if(tr->ValidComps() & TRANSFCOMP_pos)
                m_matrix.set_rot(*(mat3f*)(array9_id));
            else
                m_matrix.identity();

            // if you want Quaternion at any cost, Euler must be canceled
	        //rotation from a Quaternion (==R)
            // NOTE: if we force the dirty flag, we'll give priority to Euler
            if((tr->ValidComps() & TRANSFCOMP_Quat)
                ||((tr->ValidComps() & TRANSFCOMP_Quat_ready)&&(forceDirty == false)))
            {
                bChanged = true;
                quatf Qr(tr->Quat());
                Q *= Qr;
	            m_matrix.rotate(Qr);
                tr->ValidComps() |= TRANSFCOMP_Quat_ready;
            } else {
                // if no rotation in this transformation, just update the quaternion
                bChanged = true;
                tr->Quat().x = tr->Quat().y = tr->Quat().z = 0.0f;
                tr->Quat().w = 1.0f;
                tr->ValidComps() |= TRANSFCOMP_Quat_ready;
            }
        } // else (if it has components ON)
    } //if(tr->getDirty() || forceDirty)
    if(bParentChanged || bChanged || (!(tr->ValidComps() & TRANSFCOMP_abs_matrix_ready)))
    {
        bChanged = true;
        // if we have the Quaternion ON, let's update the abs values, too
        if(tr->ValidComps() & (TRANSFCOMP_Quat|TRANSFCOMP_Quat_ready))
        {
            tr->ValidComps() |= TRANSFCOMP_abs_Quat_ready;
            //if(tr->getValidComps() & TRANSFCOMP_rotation) // If Euler is still there... careful !
            //{
            //    // Shall we re-compute the local quaternion from Euler ?
            //    assert(!"Warning : local quaternion overridden by Euler");
            //}
            // NOTE: the Quaternion case certainly missed details from the complex matrix chain from Maya
            // TODO: We totally ignore pivots in this case :-( Need to check what we can do
            //if((!(tr->getValidComps() & (TRANSFCOMP_scalePivot|TRANSFCOMP_scalePivotTranslate|TRANSFCOMP_rotationPivot|TRANSFCOMP_rotationPivotTranslate))))
            //{
            //  PRINTF(("Warning with tr->getValidComps()"));
            //}
            //vec3f VQpos(tr->pos);
            // we assume the whole chain of transformations are consistent : parent has valid Quaternions
            quatf Qabs;
            quatf Q(tr->Quat());
            if(tr->getParent())
            {
                assert(tr->ValidComps() & TRANSFCOMP_abs_Quat_ready);
                quatf Qparent(tr->getParent()->QuatAbs());
                Qabs = Qparent * Q;
                Qabs.normalize(); // maybe we could this only fewer times
                // correct the scale and pos according to parents
                //VQpos.rotateBy(Qparent);
                //VQpos += vec3f(tr->getParent()->abs_pos);
            } else {
                Qabs = Q;
            }
            memcpy(tr->QuatAbs(), Qabs.comp, sizeof(nv_scalar)*4);
            //memcpy(tr->abs_pos, VQpos.vec_array, sizeof(nv_scalar)*3);
        }
        // OROGINAL VERSION
        if(tr->getParent())
            m_abs_matrix = mat4f(tr->getParent()->MatrixAbs()) * m_matrix;
        else
            m_abs_matrix = m_matrix;
    }
    // recompute the bindpose only if needed (i.e. the first time)
    if((tr->MatrixInvBindpose()[15] == 0.0)||bComputeBindPose)
    {
        tr->ValidComps() |= TRANSFCOMP_bindpose_matrix;
        invert(m_bindpose_matrix, m_abs_matrix);
        memcpy(tr->MatrixInvBindpose(), m_bindpose_matrix.mat_array, sizeof(float)*16);
    }
    if(bChanged)
    {
        tr->ValidComps() |= TRANSFCOMP_matrix_ready;
        tr->ValidComps() |= TRANSFCOMP_abs_matrix_ready;
        // Add the Quaternion data... if possible
        // Warning
        tr->ValidComps() |= TRANSFCOMP_abs_Quat_ready;
        memcpy(tr->MatrixAbs(), m_abs_matrix.mat_array, sizeof(float)*16);
        memcpy(tr->MatrixAbs(), m_abs_matrix.mat_array, sizeof(float)*16);
        memcpy(tr->Matrix(), m_matrix.mat_array, sizeof(float)*16);
    }
    // DOF quaternions
    if(bParentChanged)
        computeDOFQuat(tr);
    return bChanged;
}
//-----------------------------------------------------------------------------
// Compute or update all the components of the transformation
// If necessary, it will call the parent's update prior to finalizing the work
// If possible, it will use Quaternions but still maintain the matrices
// Note that the Quaternion case may not be 100% compatible with Maya complex transformations (Pivots...)
//-----------------------------------------------------------------------------
bool ComputeMatrixSimple(bk3d::TransformSimple * tr, bool bComputeBindPose, bool forceDirty, bool bParentChanged)
{
    mat4f m_matrix, m_abs_matrix, m_bindpose_matrix;
    bool bChanged = false;

    // copy the original matrix, if ever this matrix is already available and bDirty is false
    memcpy(m_matrix.mat_array, tr->Matrix(), sizeof(float)*16);
    if(forceDirty)
    {
        tr->ValidComps() &= ~(TRANSFCOMP_matrix_ready);
        tr->setDirty(true);
    }
    if((tr->getDirty() || forceDirty))
    {
        // case where the matrix is really the main source of transformation
        if(tr->ValidComps() & TRANSFCOMP_matrix)
        {
            if(!(tr->ValidComps() & TRANSFCOMP_matrix_ready))
            {
                bChanged = true;
                tr->ValidComps() |= TRANSFCOMP_matrix_ready;
                tr->ValidComps() &= ~(TRANSFCOMP_abs_matrix_ready|TRANSFCOMP_abs_Quat_ready);
            }
        } else
        {
            quatf Q(0.0f, 0.0f, 0.0f, 1.0f);
            // because some components are now used, turn the matrix to Identity and re-compute it from scratch
            m_matrix.identity();

            //translation (T)
            if(tr->ValidComps() & TRANSFCOMP_pos)
            {
                bChanged = true;
                m_matrix.translate(tr->Pos());
            }
            // if you want Quaternion at any cost, Euler must be canceled
	        //rotation from a Quaternion (==R)
            // NOTE: if we force the dirty flag, we'll give priority to Euler
            if((tr->ValidComps() & TRANSFCOMP_Quat)
                ||((tr->ValidComps() & TRANSFCOMP_Quat_ready)&&(forceDirty == false)))
            {
                bChanged = true;
                quatf Qr(tr->Quat());
                Q *= Qr;
	            m_matrix.rotate(Qr);
                tr->ValidComps() |= TRANSFCOMP_Quat_ready;
            } else {
                // if no rotation in this transformation, just update the quaternion
                bChanged = true;
                tr->Quat().x = tr->Quat().y = tr->Quat().z = 0.0f;
                tr->Quat().w = 1.0f;
                tr->ValidComps() |= TRANSFCOMP_Quat_ready;
            }
            {
                // According to maya doc, there should be a "Shearing" matrix, here... TODO?
	            //scale (S)
                if(tr->ValidComps() & TRANSFCOMP_scale)
                {
                    bChanged = true;
	                m_matrix.scale(vec3f(tr->ScaleAbs()));
                }
            }
        } // else (if it has components ON)
    } //if(tr->getDirty() || forceDirty)
    // re-compute absolute values (and bindpose if necessary)
    if(bParentChanged || bChanged || (!(tr->ValidComps() & TRANSFCOMP_abs_matrix_ready)))
    {
        bChanged = true;
        // if we have the Quaternion ON, let's update the abs values, too
        if(tr->ValidComps() & (TRANSFCOMP_Quat|TRANSFCOMP_Quat_ready))
        {
            tr->ValidComps() |= TRANSFCOMP_abs_Quat_ready;
            //if(tr->getValidComps() & TRANSFCOMP_rotation) // If Euler is still there... careful !
            //{
            //    // Shall we re-compute the local quaternion from Euler ?
            //    assert(!"Warning : local quaternion overridden by Euler");
            //}
            // NOTE: the Quaternion case certainly missed details from the complex matrix chain from Maya
            // TODO: We totally ignore pivots in this case :-( Need to check what we can do
            //if((!(tr->getValidComps() & (TRANSFCOMP_scalePivot|TRANSFCOMP_scalePivotTranslate|TRANSFCOMP_rotationPivot|TRANSFCOMP_rotationPivotTranslate))))
            //{
            //  PRINTF(("Warning with tr->getValidComps()"));
            //}
            //vec3f VQpos(tr->pos);
            vec3f VQscale(tr->ScaleAbs());
            // we assume the whole chain of transformations are consistent : parent has valid Quaternions
            quatf Qabs;
            quatf Q(tr->Quat());
            if(tr->getParent())
            {
                assert(tr->ValidComps() & TRANSFCOMP_abs_Quat_ready);
                quatf Qparent(tr->getParent()->QuatAbs());
                Qabs = Qparent * Q;
                Qabs.normalize(); // maybe we could this only fewer times
                // correct the scale and pos according to parents
                //VQpos.rotateBy(Qparent);
                //VQpos += vec3f(tr->getParent()->abs_pos);
                if((tr->getParent()->nodeType == NODE_TRANSFORM)||(tr->getParent()->nodeType == NODE_TRANSFORMSIMPLE))
                    VQscale *= vec3f(((bk3d::TransformSimple*)tr->getParent())->ScaleAbs());
            } else {
                Qabs = Q;
            }
            memcpy(tr->QuatAbs(), Qabs.comp, sizeof(nv_scalar)*4);
            //memcpy(tr->abs_pos, VQpos.vec_array, sizeof(nv_scalar)*3);
            memcpy(tr->ScaleAbs(), VQscale.vec_array, sizeof(nv_scalar)*3);
        }
        // OROGINAL VERSION
        if(tr->getParent())
            m_abs_matrix = mat4f(tr->getParent()->MatrixAbs()) * m_matrix;
        else
            m_abs_matrix = m_matrix;
    }
    // recompute the bindpose only if needed (i.e. the first time)
    if((tr->MatrixInvBindpose()[15] == 0.0)||bComputeBindPose)
    {
        tr->ValidComps() |= TRANSFCOMP_bindpose_matrix;
        invert(m_bindpose_matrix, m_abs_matrix);
        memcpy(tr->MatrixInvBindpose(), m_bindpose_matrix.mat_array, sizeof(float)*16);
    }
    if(bChanged)
    {
        tr->ValidComps() |= TRANSFCOMP_matrix_ready;
        tr->ValidComps() |= TRANSFCOMP_abs_matrix_ready;
        // Add the Quaternion data... if possible
        // Warning
        tr->ValidComps() |= TRANSFCOMP_abs_Quat_ready;
        memcpy(tr->MatrixAbs(), m_abs_matrix.mat_array, sizeof(float)*16);
        memcpy(tr->MatrixAbs(), m_abs_matrix.mat_array, sizeof(float)*16);
        memcpy(tr->Matrix(), m_matrix.mat_array, sizeof(float)*16);
    }
    // DOF quaternions
    if(bParentChanged)
        computeDOFQuat(tr);
    return bChanged;
}
//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
void recTransfUpdate(bk3d::Bone *tr, bool bComputeBindPose, bool forceDirty, bool bParentChanged)
{
    switch(tr->nodeType)
    {
    case NODE_RIGIDBODY:
    case NODE_CONSTRAINT:
    case NODE_IKHANDLE:
    case NODE_IKHANDLEROTATEINFLUENCE:
    case NODE_IKHANDLEROLLINFLUENCE:
    case NODE_BONE:
        ComputeMatrixBone(tr, bComputeBindPose, forceDirty, bParentChanged);
        break;
    case NODE_TRANSFORMSIMPLE:
        ComputeMatrixSimple(tr->asTransfSimple(), bComputeBindPose, forceDirty, bParentChanged);
        break;
    case NODE_TRANSFORM:
        ComputeMatrix(tr->asTransf(), bComputeBindPose, forceDirty, bParentChanged);
        break;
    }
    //if(tr->getParent())
    //    recTransfUpdate(tr->getParent(), bComputeBindPose, forceDirty, tr->getDirty()||forceDirty ? 1 : bParentChanged);
    //
    // Children
    //
    if(tr)
    {
        //LOGI("  Children (%d/%d)", tr->getNumChildren(), tr->pChildren ? tr->pChildren->n : 0);
        //for(int i=0; i<tr->getNumChildren(); i++)
        //{
        //    LOGI(" %d/%d", tr->getChild(i)->ID, tr->pChildren && (i<tr->pChildren->n) ? tr->pChildren->p[i]->ID : -1);
        //}
        //LOGI("\n");
        for(int i=0; i<tr->getNumChildren(); i++)
        {
            recTransfUpdate(tr->getChild(i), bComputeBindPose, forceDirty, tr->getDirty()||forceDirty ? 1 : bParentChanged);
        }
    }
    // now we can set the dirty flag to false...
    tr->setDirty(false);
}
//-----------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------
void updateTransforms(bk3d::FileHeader * bk3dHeader, bool bComputeBindPose, bool forceDirty)
{
    if(bk3dHeader && bk3dHeader->pTransforms)
        for(int i=0; i<bk3dHeader->pTransforms->nBones; i++)
        {
            if(bk3dHeader->pTransforms->pBones[i]->getParent() == NULL)
                recTransfUpdate(bk3dHeader->pTransforms->pBones[i], bComputeBindPose, forceDirty, 0);
        }
}


