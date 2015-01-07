// obj2bk3d.cpp : Defines the entry point for the console application.
//

#pragma warning(disable:4312)

#ifndef _WIN32_WINNT        // Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501    // Change this to the appropriate value to target other versions of Windows.
#endif                        

#include <stdio.h>
#include <tchar.h>

#include <windows.h>

#include "assert.h"

#include <vector>
#include <string>
#include <set>
#include <map>

#include "bk3dlib.h"
#include "bk3dEx.h"
#include "bk3dDbgFuncs.inl" // additional functions for debug purpose : to print out the binary data
#include "nvModel.h"

#include "nv_math.h"
using namespace nv_math;

#include "table.h"

extern bool verbose;

class PMD
{
private:
    //------------------------------------------------------------------------
    // See http://mikumikudance.wikia.com/wiki/MMD:Polygon_Model_Data
    //------------------------------------------------------------------------
    #pragma pack(push, 1)
    struct Header {
        char pmd[3];
        char version[4];
        char name[20];
        char comments[256];
    };
    struct Vertex {
        vec3f Pos;
        vec3f Normal;
        vec2f TC;
        unsigned short boneID[2];
        char bone0Weight;
        char edgeFlag;
    };

    struct Material {
        vec3f            diffuse;
        float           alpha;
        float           shininess;
        vec3f            spec;
        vec3f            ambient;
        unsigned char   toon;
        unsigned char   edgeFlag;
        unsigned int    indices;
        char            name[20]; // The file name corresponding to the texture applied to this material. May be left blank. If material has a sphere map, the texture file and sphere map file are separated by a "*". Example: "tex0.bmp*sphere01.spa"
    };
    enum BoneType {
        // TODO: Add a flag to tell it can only rotate ??
        // will lead to a hemispheric DOF
        Bone_Rotate = 0,
        // typical bone: 'Root'
        // could lead to a hemispheric DOF... TODO: maybe we should not put any DOF...
        Bone_RotateMove = 1,
        // to be translated as "handlePos" in IK
        // leads to no DOF: Handle's transform
        Bone_IK = 2,
        NOTUSED = 3,
        // A Bone considered as to be influenced by IK: any bone in a IK-chain
        // Maybe we should tag the bk3d transformation as being Ik-dependent...
        // Not sure how we can guess its original DOF complexity...
        // hemisphere DOF will be the default
        Bone_IKRotateInfluence = 4,
        // the bone gets the rotation from bone.ikIndex
        // leads to a IK Handle for the purpose of copying bone.ikIndex
        // typical case: eyes L and R copying rotation from a transform's Handle
        Bone_RotateInfluence = 5,
        // to be translated as "effectorPos" in IK
        // doesn't lead to any DOF
        Bone_IKTarget = 6,
        // Transformation useless...
        Bone_Invisible = 7,
        // the Bone is limited in rolling only (?)
        // Arm twist, wrist twist...
        // will lead to special limited DOF on the rolling axis
        Bone_Rolling = 8,
        // take the boneTail and constraints the rotation with it
        // use percentage value copyRotateCoef to change the influence
        // ((copyRotateCoef * 0.01) to weight the boneTail rotation)
        // tweak doesn't require any DOF: the calculation of those ones
        // depends on the spread of the IK Handle's transform rotation
        // typical bone: intermediate arm/wrist bones for skinning
        Bone_Tweak = 9,
    };

    struct Bone {
        char            name[20];
        short           parent;
        short           tail;
        unsigned char   type;
        union {
            short       ikIndex;//boneHandle; // if type 4 : bone ID that this bone is affected by
            short       copyRotateCoef; // percentage value (0-100) when influenced by another Bone rotation
        };
        vec3f            pos;
    };

    struct IK {
        short           target;
        short           endEffector;
        unsigned char   nLinks;
        unsigned short  maxIter;
        float           weight;
        short           linkList[1]; // nLinks items
    };

    struct Morph {
        char            name[20];
        long            morphSize;
        unsigned char   type;
        struct Idx {
            unsigned long   idx;
            vec3f            pos;
        };
        Idx             vtx[1]; // has morphSize items
    };

    typedef char Name50[50];
    typedef char Name100[100];

    struct DisplayBone
    {
        short           boneID;
        unsigned char   groupID;
    };

    struct RigidBody
    {
        char name[20];
        unsigned short  bone_index;
        unsigned char   collision_group;
        short           collision_group_mask;
        unsigned char   shape_type; //0: sphere; 1: box; 2:capsule
        vec3f            shape_size; // width: for all; heigh: for sphere/capsules; depth: box shapes
        vec3f            shape_position;
        vec3f            shape_rotation;
        float           mass;
        float           linear_dampening;
        float           angular_dampening;
        float           restitution;
        float           friction;
        unsigned char   mode;
    };
    struct Joint
    {
        char    name[20];
        long    rigidbody_index_a;
        long    rigidbody_index_b;
        vec3f    position;
        vec3f    rotation;
        vec3f    translation_limit_min;
        vec3f    translation_limit_max;
        vec3f    rotation_limit_min;
        vec3f    rotation_limit_max;
        vec3f    spring_constant_translation;
        vec3f    spring_constant_rotation;
    };
    #pragma pack(pop)

    //------------------------------------------------------------------------
    //
    //------------------------------------------------------------------------
    Header          header;
    FILE* fd;
    unsigned long   nVtx;
    Vertex*         vertices;
    unsigned long   nIdx;
    unsigned short* indices;
    unsigned long   nMaterials;
    Material*       materials;
    unsigned short  nBones;
    Bone*           bones;
    unsigned short  nIKs;
    IK**            iks;
    unsigned short  nMorphs;
    Morph**         morphs;
    unsigned char   nMorphIndices;
    unsigned short* morphIndices;
    unsigned char   nBoneGroups;
    Name50 *        boneGroups;
    unsigned long   nDisplayBones;
    DisplayBone *   displayBones;
    Name100         toonTextures[10];
    unsigned long   nRigidBodies;
    RigidBody *     rigidBodies;
    unsigned long   nJoints;
    Joint *         joints;

    //------------------------------------------------------------------------
    //
    //------------------------------------------------------------------------
    bool readVertices()
    {
        if(fread(&nVtx, sizeof(long), 1, fd) == EOF)
            return false;
        vertices = new Vertex[nVtx];
        if(fread(vertices, sizeof(Vertex), nVtx, fd) == EOF)
                return false;

        return true;
    }
    bool readIndices()
    {
        if(fread(&nIdx, sizeof(long), 1, fd) == EOF)
            return false;
        indices = new unsigned short[nIdx];
        if(fread(indices, sizeof(short), nIdx, fd) == EOF)
                return false;

        return true;
    }
    bool readMaterials()
    {
        nMaterials = 0;
        if(fread(&nMaterials, sizeof(long), 1, fd) == EOF)
            return false;
        materials = new Material[nMaterials];
        if(fread(materials, sizeof(Material), nMaterials, fd) == EOF)
            return false;

        return true;
    }
    bool readBones()
    {
        if(fread(&nBones, sizeof(short), 1, fd) == EOF)
            return false;
        bones = new Bone[nBones];
        if(fread(bones, sizeof(Bone), nBones, fd) == EOF)
            return false;
        for(int i=0; i<nBones; i++)
        {
            convertJtoEn(boneTable_table, bones[i].name);
            printf("Reading Bone %s\n", bones[i].name);
        }
        return true;
    }
    bool readIK()
    {
        if(fread(&nIKs, sizeof(short), 1, fd) == EOF)
            return false;
        iks = new IK*[nIKs];
        for(int i=0; i<nIKs; i++)
        {
            IK ik;
            if(fread(&ik, sizeof(IK), 1, fd) == EOF)
                return false;
            iks[i] = (IK*)malloc(sizeof(IK)+sizeof(short)*(ik.nLinks-1));
            memcpy(iks[i], &ik, sizeof(IK));
            if(ik.nLinks-1 > 0)
                if(fread(iks[i]->linkList+1, sizeof(short), ik.nLinks-1, fd) == EOF)
                    return false;
        }
        return true;
    }
    bool readMorphs()
    {
        if(fread(&nMorphs, sizeof(short), 1, fd) == EOF)
            return false;
        morphs = new Morph*[nMorphs];
        for(int i=0; i<nMorphs; i++)
        {
            Morph morph;
            if(fread(&morph, sizeof(Morph), 1, fd) == EOF)
                return false;
            morphs[i] = (Morph*)malloc(sizeof(Morph)+sizeof(Morph::Idx)*(morph.morphSize-1));
            memcpy(morphs[i], &morph, sizeof(Morph));
            if(morph.morphSize-1 > 0)
                if(fread(morphs[i]->vtx+1, sizeof(Morph::Idx), morph.morphSize-1, fd) == EOF)
                    return false;
            // this is how MMD is done... morph 0 is the list of absolute vertices
            // that are touched by all the morphs. Then subsequence morphs are the real
            // deformers, indexing to this morph 0 (!!)
            if(i>0)
                for(int v=0; v<morphs[i]->morphSize; v++)
                {
                    morphs[i]->vtx[v].idx = morphs[0]->vtx[ morphs[i]->vtx[v].idx ].idx;
                }
        }

        return true;
    }
    bool readMorphIndices()
    {
        if(fread(&nMorphIndices, sizeof(nMorphIndices), 1, fd) == EOF)
            return false;
        morphIndices = new unsigned short[nMorphIndices];
        if(fread(morphIndices, sizeof(unsigned short), nMorphIndices, fd) == EOF)
            return false;

        return true;
    }
    bool readBoneGroupList()
    {
        if(fread(&nBoneGroups, sizeof(nBoneGroups), 1, fd) == EOF)
            return false;
        boneGroups = new Name50[nBoneGroups];
        if(fread(boneGroups, sizeof(Name50), nBoneGroups, fd) == EOF)
            return false;

        return true;
    }
    bool readBoneDisplayList()
    {
        if(fread(&nDisplayBones, sizeof(nDisplayBones), 1, fd) == EOF)
            return false;
        displayBones = new DisplayBone[nDisplayBones];
        if(fread(displayBones, sizeof(DisplayBone), nDisplayBones, fd) == EOF)
            return false;

        return true;
    }

    bool readEnglish()
    {
        char tmp[50];
        int i;
        if(fread(header.name, 20, 1, fd) == EOF)
            return false;
        if(fread(header.comments, 256, 1, fd) == EOF)
            return false;
        for(i=0; i<nBones; i++) {
            //if(fread(bones[i].name, 1, 20, fd) == EOF)
            if(fread(tmp, 1, 20, fd) == EOF)
                return false;
            //convertJtoEn(boneTable_table, bones[i].name);
        }
        int eyebrow = 0;
        int eye = 0;
        int lip = 0;
        int other = 0;
        for(i=0; i<nMorphs; i++) {
            if(!strcmp(morphs[i]->name, "base"))
                continue;
            if(fread(morphs[i]->name, 1, 20, fd) == EOF)
                return false;
            switch(morphs[i]->type)
            {
            case 1:
                sprintf_s(morphs[i]->name, 20, "EBrow%d", eyebrow++);
                break;
            case 2:
                sprintf_s(morphs[i]->name, 20, "Eye%d", eye++);
                break;
            case 3:
                sprintf_s(morphs[i]->name, 20, "Lip%d", lip++);
                break;
            default:
            case 0:
                sprintf_s(morphs[i]->name, 20, "Other%d", other++);
                break;
            }
        }
        for(i=0; i<nBoneGroups; i++)
            if(fread(boneGroups, 1, 50, fd) == EOF)
                return false;
        return true;
    }
    
    bool readRigidBodies()
    {
        if(fread(&nRigidBodies, sizeof(unsigned long), 1, fd) == EOF)
            return false;
        rigidBodies = new RigidBody[nRigidBodies];
        if(fread(rigidBodies, sizeof(RigidBody), nRigidBodies, fd) == EOF)
            return false;
        // hack for names: replace the japanese name with arbitrary name+counter
        for(int i=0;i<nRigidBodies; i++)
            sprintf(rigidBodies[i].name, "RigidBody_%d", i);
        return true;
    }
    bool readJoints()
    {
        if(fread(&nJoints, sizeof(unsigned long), 1, fd) == EOF)
            return false;
        joints = new Joint[nJoints];
        if(fread(joints, sizeof(Joint), nJoints, fd) == EOF)
            return false;
        // hack for names: replace the japanese name with arbitrary name+counter
        for(int i=0;i<nJoints; i++)
            sprintf(joints[i].name, "Constraint_%d", i);
        return true;
    }
public:
    PMD()
    {
        fd = NULL;
        nVtx = 0;
        vertices = NULL;
        nIdx = 0;
        indices = NULL;
        nMaterials = 0;
        materials = NULL;
        nBones = 0;
        bones = NULL;
        nIKs = 0;
        iks = NULL;
        nMorphs = 0;
        morphs = NULL;
        nMorphIndices = 0;
        morphIndices = NULL;
        nBoneGroups = 0;
        boneGroups = NULL;
        nDisplayBones = 0;
        displayBones = NULL;
    }
    ~PMD()
    {
        if(fd) fclose(fd);
        delete [] vertices;
        delete [] indices;
        delete [] materials;
        delete [] bones;
        for(int i=0; i<nIKs; i++)
        {
            free(iks[i]);
        }
        delete [] iks;
        for(int i=0; i<nMorphs; i++)
        {
            free(morphs[i]);
        }
        delete [] morphs;
        delete [] morphIndices;
        delete [] boneGroups;
        delete [] displayBones;
    }
    //------------------------------------------------------------------------
    //
    //------------------------------------------------------------------------
    bool read(const char* fullInStr)
    {
        if(verbose) printf( "loading file...\n");
        fd = fopen(fullInStr, "rb");
        if(!fd)
        {
            if(verbose) printf( "failed loading file...\n");
            return false;
        }
        if(EOF == fread(&header, sizeof(Header), 1, fd))
            goto failed;
		if(strcmp(header.pmd, "Pmd"))
			goto failed;
        readVertices();
        readIndices();
        readMaterials();
        readBones();
        readIK();
        readMorphs();
        readMorphIndices();
        readBoneGroupList();
        readBoneDisplayList();
        // Extra #1 : English information
        char code;
        if(EOF == fread(&code, 1, 1, fd))
            goto done;
        if(code == 1)
            readEnglish();
        // Extra #2 : Toon Textures
        if(EOF == fread(toonTextures, 100, 10, fd))
            goto done;
        // Extra #3 : Physics
        if(!readRigidBodies())
            goto done;
        if(!readJoints())
            goto done;
    done:
        fclose(fd);
        return true;
    failed:
        fclose(fd);
        return false;
    }

    //------------------------------------------------------------------------
    // 0: not influencing; 1:rotate influence; 2: tweak Influence
    //------------------------------------------------------------------------
    int checkIfUsedAsInfluence(int boneInfluence)
    {
        for(int b=0; b<nBones; b++)
        {
            Bone& bone = bones[b];
            switch(bone.type)
            {
            case Bone_RotateInfluence://5
                // the bone gets the rotation from this one:
                if(bone.ikIndex == boneInfluence)
                    return 1;
                break;
            case Bone_Tweak://9
                if(bone.tail == boneInfluence)
                    return 2;
                break;
            }
        }
            return 0;
    }
    //------------------------------------------------------------------------
    //
    //------------------------------------------------------------------------
    bool buildbk3d(const char* singleName, bk3dlib::PFileHeader fileHeader)
    {
        int i;
        bool bForceDDS = true;
        if(vertices == NULL)
            return false;
        // Create a Mesh (in our case, only one mesh)
        bk3dlib::PMesh mesh = bk3dlib::Mesh::Create(singleName);
        // we attach it to the main body of the file we want to create
        fileHeader->AttachMesh(mesh);

        // mark which transform IDs are being used by skinning
        std::set<int> usedSkinIDs;
        for(i=0; i<(int)nVtx; i++)
        {
            int bid0 = vertices[i].boneID[0];
            int bid1 = vertices[i].boneID[1];
            usedSkinIDs.emplace(bid0);
            usedSkinIDs.emplace(bid1);
        }
        // mark the ones used by tweak
        for(int b=0; b<nBones; b++)
        {
            Bone& bone = bones[b];
            switch(bone.type)
            {
            case Bone_Tweak://9
                usedSkinIDs.emplace(bone.tail);
                break;
            case Bone_RotateInfluence:
                usedSkinIDs.emplace(bone.ikIndex);
                break;
            }
        }
        // calculate the shift in indices after removing some of them
        std::vector<int> boneIDShift;
        int shiftVal = 0;
        for(int b=0; b<nBones; b++)
        {
            Bone& bone = bones[b];
            switch(bone.type)
            {
            case NOTUSED:
            case Bone_Invisible:
                if(usedSkinIDs.find(b) == usedSkinIDs.end()) // not found... discard
                {
                    shiftVal++;
                    boneIDShift.push_back(-1); // -1 for debug: should never be referenced in skinning
                    continue;
                }
                break;
            }
            // store the shifting value that we must apply to correct skinning Bone refs
            boneIDShift.push_back(shiftVal);
        }
        //
        // reorder the walk-though bones to process IKHandles at the end
        //
        std::multimap<int, int> sortedBones;
        for(int b=0; b<nBones; b++)
        {
            Bone& bone = bones[b];
            if(bone.type == Bone_IK)
                sortedBones.insert(std::pair<int, int>(2, b));
            else if(checkIfUsedAsInfluence(b))
                sortedBones.insert(std::pair<int, int>(1, b));
            else
                sortedBones.insert(std::pair<int, int>(0, b));
        }
        std::multimap<int, int>::const_iterator iB = sortedBones.begin();
        // another table to do the opposite: find new position from original bone index
        std::vector<int> newBoneIndex;
        for(int bb=0; bb<nBones; bb++, iB++)
        {
            newBoneIndex.push_back(iB->second);
        }
        //
        // Transformations (Bones)
        // NOTE: MMD models seem to use Bones even for terminal parts of very last bones.
        // this is useless and only necessary for the tail to be computed.
        // we will bypass those transformations
        // NOTE2: some Bones are only used for IK. On my side, Ik targets are "IK-Handle"
        // we will remove those bones and replace them with Specific IK handle
        // NOTE3: any removed bone will lead to SHIFTING the skin bone references for each
        // vertex !! After this, we must adjust those bone ID references for each vertex.
        //
        Bone* boneRotInfluence = NULL;
        iB = sortedBones.begin();
        for(int bb=0; bb<nBones; bb++, iB++)
        {
            bk3dlib::Bone* pParent = NULL;
            Bone* boneTail = NULL;
            int b = bb;//iB->second;
            Bone& bone = bones[b];
            vec3f tailPos(0.0f,0.01f,0.0f);
            bk3dlib::Bone* pTransf = NULL;
            switch(bone.type)
            {
            default:
            case Bone_Rotate://0
                // TODO: Add a flag to tell it can only rotate ??
                break;
            case Bone_RotateMove://1
                // TODO: add a flag to tell is can be moved...??
                break;
            case Bone_IK://2
                {
                // to be translated as "handlePos" in IK
                bk3dlib::IKHandle* pIK = bk3dlib::IKHandle::Create(bone.name);
                pTransf = pIK->AsBone();
                fileHeader->AttachIKHandle(pIK);
                }
                break;
            case NOTUSED://3
                if(usedSkinIDs.find(b) == usedSkinIDs.end()) // not found... discard
                    continue;
                break;
            case Bone_IKRotateInfluence://4
                // Those are the Bones considered as to be influenced by IK
                break;
            case Bone_RotateInfluence://5
                // the bone gets the rotation from this one:
                boneRotInfluence = bones + bone.ikIndex;
                break;
            case Bone_IKTarget://6
                // to be translated as "effectorPos" in IK
                // BUT we will use the tailBone instead: they are the same (to be verified)
                break;
            case Bone_Invisible://7
                if(usedSkinIDs.find(b) == usedSkinIDs.end()) // not found... discard
                    continue;
                break;
            case Bone_Rolling://8
                {
                    // the Bone is limited in rolling only (?)
                }
                break;
            case Bone_Tweak://9
                {
                    // take the boneTail and constraints the rotation with it
                    // use copyRotateCoef * 0.01 to change the influence
                }
                break;
            }

            if(pTransf == NULL)
            {
                // The transformation can have another role on top of being a transform:
                // it can be a target for other ones. Therefore it is a Handle, too.
                switch(checkIfUsedAsInfluence(b))
                {
                default:
                case 0:
                    pTransf = bk3dlib::Bone::Create(bone.name);
                    break;
                case 1: // as Rotate Influence
                        pTransf = bk3dlib::IKHandle::CreateRotateInfluence(bone.name);
                    break;
                case 2: // as Roll Influence
                        pTransf = bk3dlib::IKHandle::CreateRollInfluence(bone.name);
                    break;
                }
                if(fileHeader->AttachTransform(pTransf) == false)
                {
                    printf("Error: a bone name '%s' was already found!\n", bone.name);
                    DebugBreak();
                }
                //
                // add the Bone transformations that are involved in skinning to the Mesh (AddTransformReference)
                // this later will allow to take this subset as the array of transformations to send to the shader for skinning
                // Note that there is already a list of transformations in the FileHeader. But it is a list for all the meshes.
                // in the case where many meshes (many characters for example... not the case here) are in the same file, it makes
                // sense to take the list from the mesh instead of the one from the FileHeader
                //
                mesh->AddTransformReference(pTransf, -1);
            }
            // we assume the parent is always created prior to being referenced!
            if(bone.parent >= 0)
            {
                pParent = fileHeader->GetTransform(bones[bone.parent].name);
                if(pParent)
                    pTransf->SetParent(pParent);
                else
                    printf("Warning: we should be able to find the parent\n");
            }
            // Pos : Bone only have pos offset, while their scale/rotations
            // are at identity in rest-pose
            // Note: Blender has a 'Roll' quite useful to set the main rotation axis (elbow...)
            // Not sure how to work with that in this format
            vec3f pp(0,0,0);
            vec3f p(bone.pos);
            if(pParent) {
                // trivial case where nothing else but translation in transformations
                //float m[16];
                //pParent->GetMatrix_Abs(m);
                //pp = vec3f(m+12); // take abs translation only
                pp = bones[bone.parent].pos; // take abs translation only
            }
            p -= pp;
            // Bone_Tweak doesn't neet the tail: the direction of the Roll comes from the bone taht influences them
            if((bone.tail > 0)&&(bone.type != Bone_Tweak)&&(bone.type != Bone_IK))
            {
                tailPos = bones[bone.tail].pos;
                tailPos -= bone.pos;
                pTransf->SetTailPos(tailPos.x, tailPos.y, tailPos.z);
            }
            // Maybe we need to subtract the parent...
            pTransf->SetPos(p.x, p.y, p.z);
            pTransf->ComputeMatrix(true);

        } // for(nBones)
        //
        // DOF transformations for some transformations
        // Done *after* the previous list of transformations so that we don't disturb
        // the index of original transformations for skinning
        //
        for(int b=0; b<nBones; b++)
        {
            bk3dlib::Bone* pParent = NULL;
            Bone& bone = bones[b];
            switch(bone.type)
            {
            default:
            case Bone_Rotate://0
                // TODO: Add a flag to tell it can only rotate ??
                // will lead to a hemispheric DOF
                break;
            case Bone_RotateMove://1
                // typical bone: 'Root'
                // could lead to a hemispheric DOF... TODO: maybe we should not put any DOF...
                break;
            case Bone_IK://2
                // to be translated as "handlePos" in IK
                // leads to no DOF: Handle's transform
                continue;
            case NOTUSED://3
                continue;
            case Bone_IKRotateInfluence://4
                // A Bone considered as to be influenced by IK: any bone in a IK-chain
                // Maybe we should tag the bk3d transformation as being Ik-dependent...
                // Not sure how we can guess its original DOF complexity...
                // hemisphere DOF will be the default
                break;
            case Bone_RotateInfluence://5
                // the bone gets the rotation from bone.ikIndex
                // leads to a IK Handle for the purpose of copying bone.ikIndex
                // typical case: eyes L and R copying rotation from a transform's Handle
                break;
            case Bone_IKTarget://6
                // to be translated as "effectorPos" in IK
                // doesn't lead to any DOF
                continue;
            case Bone_Invisible://7
                // Transformation useless...
                continue;
            case Bone_Rolling://8
                // the Bone is limited in rolling only (?)
                // Arm twist, wrist twist...
                // will lead to special limited DOF on the rolling axis
                break;
            case Bone_Tweak://9
                // take the boneTail and constraints the rotation with it
                // use percentage value copyRotateCoef to change the influence
                // ((copyRotateCoef * 0.01) to weight the boneTail rotation)
                // tweak doesn't require any DOF: the calculation of those ones
                // depends on the spread of the IK Handle's transform rotation
                // typical bone: intermediate arm/wrist bones for skinning
                continue;
            }
            // find back the corresponding transformations we created just before
            bk3dlib::Bone* pTransf = fileHeader->GetTransform(bone.name);
            assert(pTransf);
            bk3dlib::TransformDOF* pTransfDOF = pTransf->CreateDOF();
            // set DOF data
            float DOFAlpha = 50.0f;
            float AxisLimitStart = -45.0;
            float AxisLimitRange = 90.0;
            vec3f axis1(pTransf->GetTailPos());
            axis1.normalize();
            vec3f axis2(1.0,0.0,0.0);
            float dotval = dot(axis1, axis2);
            if(dotval < 0.99f)
            {
                vec3f crossvec;
                cross(crossvec, axis2, axis1);
                float angle = asinf(1.0/crossvec.normalize());
                if(dotval<0.0f)
                    angle = nv_pi-angle;
                quatf qaxis(crossvec, angle);
                pTransfDOF->SetQuaternion(qaxis.x, qaxis.x, qaxis.z, qaxis.w);
            }
            pTransfDOF->SetDOFValues(bk3dlib::DOF_CONE, &DOFAlpha, &AxisLimitStart, &AxisLimitRange);
        } // for(nBones)

        // find back the parents... No need if done above
        //for(int b=0; b<nBones; b++)
        //{
        //    Bone& bone = bones[b];
        //    bk3dlib::Bone* pTransf = fileHeader->GetTransform(bone[b].name);
        //    if(bone.parent >= 0)
        //    {
        //        bk3dlib::Bone* pParent = fileHeader->GetTransform(bone.parent);
        //        assert(pParent);
        //        pTransf->SetParent(pParent);
        //    }
        //}

        //
        // IK defined by MMD
        //
        for(int i=0; i<nIKs; i++)
        {
            IK& ik = *(iks[i]);
            Bone& boneTarget = bones[ik.target];
            Bone& boneEndEffector = bones[ik.endEffector];
            bk3dlib::PBone pTransfEndEffector = fileHeader->GetTransform(boneEndEffector.name);
            bk3dlib::PBone pTransfTarget = fileHeader->GetTransform(boneTarget.name);
            assert(pTransfTarget);
            assert(pTransfEndEffector);
            float x,y,z;
            //float xe,ye,ze;
            x=y=z=0.0f;
            // a Bone for IK that doesn't have any children doesn't have any reason to be kept
// Cancel this optimization for now: put it back as soon as we also connect anim curves
// to the right target: on the Handle-pos, too... TODO
#if 0
            if(pTransfTarget->GetChild(0) == NULL)
            {
                // keep the pos offset for the handle
                pTransfTarget->GetPos(x,y,z);
                // NOTE: we should normally recompute the shifting of transformation indices
                // for skinning. But let's assume IK transforms are *after* any skinning transforms
                // In fact it would make sense: se we wouldn't have to send useless transforms
                // to the shaders for skinning.
                // TODO: check if IK transforms are after skin transforms. If not, reorganize...
                pTransfTarget->Destroy();
                pTransfTarget = NULL;
            } else {
                // TODO: the Tail of the transform for IK's could be changed to a more friendly looking way...
            }
#endif
            // Create the IK Handle
            // ...
            // Now already done
                //char tmpstr[40];
                //sprintf(tmpstr, "IKHandle%d_%s", i, boneTarget.name);
                //bk3dlib::PIKHandle pIKHandle = bk3dlib::IKHandle::Create(tmpstr);
                //fileHeader->AttachIKHandle(pIKHandle);
                //pIKHandle->setParent(pTransfTarget);
            // the transform target IS the IKHandle in our case
            bk3dlib::PIKHandle pIKHandle = pTransfTarget->AsIKHandle();
            //pTransfEndEffector->GetPos(xe,ye,ze);
            //pIKHandle->setEffectorPos(xe,ye,ze);
            // if we removed the transformation, let's keep its offset pos
            // otherwise, no offset: the matrix has it
            // canceled: the transform of IKHandle is already ok
            //pIKHandle->setHandlePos(x,y,z);

            pIKHandle->setWeight(ik.weight);
            pIKHandle->setMaxIter(ik.maxIter); // TODO

            // populate the chain
            // keep looping for consistency check
            Bone& boneIK = bones[ik.linkList[0]];
            bk3dlib::PBone pTransfCheck = fileHeader->GetTransform(boneIK.name);
            assert(pTransfCheck);
            // check to see if it is always true that the end-effector is same as tail
            assert(ik.endEffector == bones[bones[ik.endEffector].parent].tail);
            assert(ik.linkList[0] == bones[ik.endEffector].parent);
            for(int l=0; l<ik.nLinks; l++) // moves-up the chain
            {
                Bone& boneIK = bones[ik.linkList[l]];
                bk3dlib::PBone pTransf = fileHeader->GetTransform(boneIK.name);
                assert(pTransf);
                assert(pTransf == pTransfCheck);
                if(l == 0)
                    pIKHandle->setEffectorTransformStart(pTransf);
                if(l == (ik.nLinks-1))
                    pIKHandle->setEffectorTransformEnd(pTransf);
                pTransfCheck = pTransf->GetParent();
            }
        }//for(int i=0; i<nIKs; i++)

        //
        // IK for Bone_Tweak & Bone_RotateInfluence
        //
        for(int b=0; b<nBones; b++)
        {
            bk3dlib::Bone* pParent = NULL;
            Bone& bone = bones[b];
            if(bone.type == Bone_Tweak)
            {
                // a Bone Tweak uses the bone referenced by tail-end and applies a percentage
                // of its rotation to the roll-transform.
                // We must transform this into a rather different way: This bone will become part
                // of a chain of end-effectors in the IK-Handle. Each effector will have a weight
                // from copyRotateCoef %
                // 
                // find the transformation where the IKHandle must belong
                bk3dlib::Bone* pTransfHandle = fileHeader->GetTransform(bones[bone.tail].name);
                assert(pTransfHandle);
                // check if not already there
                bk3dlib::PIKHandle pIKHandle = fileHeader->GetIKHandle(bones[bone.tail].name);
                if(pIKHandle == NULL)
                {   // create it and link
                    assert(!"Should not happen: the transform should have be created before");
                    pIKHandle = bk3dlib::IKHandle::CreateRollInfluence(bones[bone.tail].name);
                    fileHeader->AttachIKHandle(pIKHandle);
                    pIKHandle->SetParent(pTransfHandle);
                }
                // Now add the effector with the coef
                bk3dlib::Bone* pTransf = fileHeader->GetTransform(bone.name);
                assert(pTransf);
                pIKHandle->addEffectorTransform(pTransf, (float)bone.copyRotateCoef/100.0f);
            }
            else if(bone.type == Bone_RotateInfluence)
            {
                // we will create (or find back) a special handle meant only
                // to copy the transformation of its parent transform
                // Meaning that this IKHandle will have many users, as opposed to the other cases

                // find back the corresponding transformation of the bone used for rotation
                bk3dlib::Bone* pTransf = fileHeader->GetTransform(bones[bone.ikIndex].name);
                assert(pTransf);
                // Create a IKHandle with this transformation as the parent
                bk3dlib::PIKHandle pIKHandle = fileHeader->GetIKHandle(bones[bone.ikIndex].name);
                if(pIKHandle == NULL)
                {
                    assert(!"Should not happen: the transform should have be created before");
                    pIKHandle = bk3dlib::IKHandle::CreateRotateInfluence(bones[bone.ikIndex].name);
                    fileHeader->AttachIKHandle(pIKHandle);
                    pIKHandle->SetParent(pTransf);
                }
                // TODO: tell the IKHandle it is a rotation IKHandle ?
                // Add the transform it will influence
                // here we don't use setEffectorTransformStart/End
                pTransf = fileHeader->GetTransform(bone.name);
                assert(pTransf);
                pIKHandle->addEffectorTransform(pTransf);
            }
        } //for(int b=0; b<nBones; b++)

        // Now correct the skinning information on Bones
        for(i=0; i<(int)nVtx; i++)
        {
            int bid0 = vertices[i].boneID[0];
            int bid1 = vertices[i].boneID[1];
            int shift = boneIDShift[bid0];
            assert(shift >= 0);
            vertices[i].boneID[0] -= shift;
            shift = boneIDShift[bid1];
            assert(shift >= 0);
            vertices[i].boneID[1] -= shift;
        }

        //
        // Physics
        //
        for(i=0; i<nRigidBodies; i++)
        {
            RigidBody &rb = rigidBodies[i];
            bk3dlib::PPhRigidBody pRB = bk3dlib::PhRigidBody::Create(rb.name);
            fileHeader->AttachPhRigidBody(pRB);
            pRB->SetPos(rb.shape_position[0], rb.shape_position[1], rb.shape_position[2]);
            pRB->SetQuaternionFromEulerXYZ(rb.shape_rotation[0], rb.shape_rotation[1], rb.shape_rotation[2]);
            pRB->setCollisionGroup(rb.collision_group, rb.collision_group_mask);
            pRB->setShapeType((bk3dlib::PhRigidBody::ShapeType)rb.shape_type);
            if(rb.shape_type == 0)
                rb.shape_size.y = 0; // cancel the capsule shift
            else if(rb.shape_type == 1)
                rb.shape_size.x = 0.01; // cancel the capsule shift
            pRB->setShapeDimensions(rb.shape_size.vec_array);
            pRB->setMode(rb.mode);
            // fix from MMD: if mode is kinematic, mass should be set to 0
            if(rb.mode == 0)
                rb.mass = 0;
            pRB->setAttributes(rb.mass, rb.linear_dampening, rb.angular_dampening, rb.restitution, rb.friction);
            // find back the corresponding transformation of the bone used for rotation
            bk3dlib::Bone* pBone = fileHeader->GetTransform(bones[rb.bone_index].name);
            assert(pBone);
            pRB->SetParent(pBone);
        }
        for(i=0; i<nJoints; i++)
        {
            Joint &j = joints[i];
            bk3dlib::PPhConstraint pC = bk3dlib::PhConstraint::Create(j.name);
            fileHeader->AttachPhConstraint(pC);
            pC->SetPos(j.position[0], j.position[1], j.position[2]);
            pC->SetQuaternionFromEulerXYZ(j.rotation[0], j.rotation[1], j.rotation[2]);
            // find back the corresponding transformation of the bone used for rotation
            bk3dlib::PhRigidBody* pRB1 = fileHeader->GetPhRigidBody(rigidBodies[j.rigidbody_index_a].name);
            bk3dlib::PhRigidBody* pRB2 = fileHeader->GetPhRigidBody(rigidBodies[j.rigidbody_index_b].name);
            assert(pRB1);
            assert(pRB2);
            pC->linkRigidBodies(pRB1, pRB2);
            pC->setTranslationLimits(j.translation_limit_min[0], j.translation_limit_min[1], j.translation_limit_min[2],
                                     j.translation_limit_max[0], j.translation_limit_max[1], j.translation_limit_max[2]);
            pC->setRotationLimits(j.rotation_limit_min[0], j.rotation_limit_min[1], j.rotation_limit_min[2],
                                  j.rotation_limit_max[0], j.rotation_limit_max[1], j.rotation_limit_max[2]);
            pC->setSpringConstantTranslation(j.spring_constant_translation[0], j.spring_constant_translation[1], j.spring_constant_translation[2]);
            pC->setSpringConstantRotation(j.spring_constant_rotation[0], j.spring_constant_rotation[1], j.spring_constant_rotation[2]);
        }

        // create the index buffer
        bk3dlib::PBuffer bufferIdx = bk3dlib::Buffer::CreateIdxBuffer("idx");
        // attach it to the mesh
        mesh->AttachIndexBuffer(bufferIdx);

        // basic interleaved slot name
        mesh->SetSlotName(0, "PosNormTexBoneW");
        // VERTEX: one buffer for one attribute
        // even if after all this might lead to some interleaving approach
        bk3dlib::PBuffer bufferVtx = bk3dlib::Buffer::CreateVtxBuffer("position", 3);
        mesh->AttachVtxBuffer(bufferVtx);
        // NORMAL: one buffer
        bk3dlib::PBuffer bufferNormals = bk3dlib::Buffer::CreateVtxBuffer("normal", 3);
        mesh->AttachVtxBuffer(bufferNormals);
        // Texcoords: one buffer again
        bk3dlib::PBuffer bufferTCs = bk3dlib::Buffer::CreateVtxBuffer("Texcoord", 2);
        mesh->AttachVtxBuffer(bufferTCs);
        // 2 Bones and 1 weight as bytes
        bk3dlib::PBuffer buffer2Bones2Weights = bk3dlib::Buffer::CreateVtxBuffer("2Bones2Weights", 4, 0, bk3dlib::UINT16);
        mesh->AttachVtxBuffer(buffer2Bones2Weights);

        // fill buffers
        for(i=0; i<(int)nVtx; i++)
        {
            bufferVtx->AddData(vertices[i].Pos.vec_array, 3);
            bufferNormals->AddData(vertices[i].Normal.vec_array,3);
            bufferTCs->AddData(vertices[i].TC.vec_array,2);
            unsigned short v2B2W[4] = { 
                vertices[i].boneID[0], vertices[i].boneID[1], 
                vertices[i].bone0Weight, 100-vertices[i].bone0Weight};
            buffer2Bones2Weights->AddData(v2B2W,4);
        }
        // TODO instead:
        //bufferVtx->AddData(vertices[i].Pos.vec_array, 3, nVtx, sizeof(Vertex));
        //bufferNormals->AddData(vertices[i].Normal.vec_array, 3, nVtx, sizeof(Vertex));
        //bufferTCs->AddData(vertices[i].TC.vec_array, 2, nVtx, sizeof(Vertex));
        
        //computing bounding volumes
        mesh->ComputeBoundingVolumes(bufferVtx);

        // create a single index buffer. But we will be able to pick groups into it
        // for Primitive groups (often sorted from their materials)
        bufferIdx->AddData(indices, nIdx);
        // Primitive groups : this one would be a prim group for ALL...
        //mesh->CreatePrimGroup("All", bufferIdx, bk3dlib::TRIANGLES);
        //
        // TODO: goodies like Tan/Binorm
        // MeshMender...
        //
        //bk3dlib::Buffer::computeNormals(bufferNormals, bufferIdxNormals, bufferVtx, bufferIdx);
        //bk3dlib::Buffer::computeTangents(bufferNormals, bufferIdxNormals, bufferVtx, bufferIdx);
        //bk3dlib::Buffer::computeBiTangents(bufferNormals, bufferIdxNormals, bufferVtx, bufferIdx);

        //
        // Blendshapes (Morphs)
        //
        int bs = 0;
        for(int m=0; m<nMorphs; m++)
        {
            Morph* pM = morphs[m];
            char tmp[20];
            // it seems like the "base" contains the regular absolute vertices... no need
            if(!strcmp(pM->name, "base") )
                continue;
            if(pM->name[0] == '\0')
                sprintf(tmp, "Morph_%d", bs);
            else
                strncpy(tmp, pM->name, 20);
            mesh->SetBSSlotName(bs, tmp);
            bk3dlib::PBuffer bufferMorphVtx = bk3dlib::Buffer::CreateVtxBuffer(MESH_POSITION, 3, bs, bk3dlib::FLOAT32);
            mesh->AttachVtxBuffer(bufferMorphVtx, true);
            bk3dlib::PBuffer bufferMorphIdVtx = bk3dlib::Buffer::CreateVtxBuffer(MESH_VERTEXID, 1, bs, bk3dlib::UINT32);
            mesh->AttachVtxBuffer(bufferMorphIdVtx, true);
            //bk3dlib::PBuffer bufferMorphNormals = bk3dlib::Buffer::CreateVtxBuffer(MESH_NORMAL, 3, m, bk3dlib::FLOAT32);
            //mesh->AttachVtxBuffer(bufferMorphNormals, true);
            for(int i=0; i<pM->morphSize; i++)
            {
                bufferMorphIdVtx->AddData((unsigned int)pM->vtx[i].idx);
                bufferMorphVtx->AddData(pM->vtx[i].pos.vec_array, 3);
                //bufferMorphNormals->AddData(pM->vtx[i]...
            }
            bs++;
        }


// TODO TODO TODO TODO TODO TODO TODO TODO
// SORT materials depending on the effect and techniques
// so there is less switch when later iterating through prim groups.
// TODO TODO TODO TODO TODO TODO TODO TODO

        // materials: lead to primitive groups
        // Note: seems like the materials are acting on a specific amount of indices
        // one after another. So they seem to share the the index buffer
        // by partitionning it. The offset is here to work on it
        unsigned long offset = 0;
        for(int m=0; m<(int)nMaterials; m++)
        {
            char matname[60];
            char str[60];
            char *pstr;
            bool bToon = false;
            bool bDiffTex = false;
            bool bSphTex = false;
            Material &matsrc = materials[m];
            sprintf(matname, "M%d %s", m, matsrc.name);
            bk3dlib::PMaterial matdst = bk3dlib::Material::Create(matname);
            // Toon information
            if(matsrc.toon != 255) {
                if(matsrc.toon < 10 && (toonTextures[matsrc.toon][0] != '\0')) {
                    // let's take reflectivity texture as the slot to tell about toon texture...
                    strcpy(str, toonTextures[matsrc.toon]);
                    if(bForceDDS && (pstr=strchr(str, '.')))
                        strcpy(pstr+1, "dds");
                    matdst->setSpecularTexture(str, str);
                } else {
                    sprintf(matname, "toon%2d.%s", matsrc.toon, bForceDDS ? "dds" : "bmp");
                    sprintf(str, "toon%2d", matsrc.toon);
                    matdst->setSpecularTexture(str, matname);
                }
                bToon = true;
            }
            // texture names : can have a sph texture reference
            char *sphname;
            char *diffname;
            if(sphname = strchr( matsrc.name, '*'))
                (*sphname++) = '\0';
            diffname = matsrc.name;
            if((strstr(matsrc.name, ".sp")||strstr(matsrc.name, "_s"))) { // maybe we need to invert names
                diffname = sphname;
                sphname = matsrc.name;
            }
            if(diffname && (strlen(diffname) > 0)) {
                strcpy(str, diffname);
                pstr=strchr(str, '.');
                if(pstr) // keep only if has a suffix
                {
                    if(bForceDDS)
                        strcpy(pstr+1, "dds");
                    matdst->setDiffuseTexture(str, str);
                    bDiffTex = true;
                }
            }
            if(sphname && (strlen(sphname) > 0)) {
                strcpy(str, sphname);
                pstr=strchr(str, '.');
                if(pstr) // keep only if has a suffix
                {
                    if(bForceDDS)
                        strcpy(pstr+1, "dds");
                    matdst->setReflectivityTexture(str, str);
                    bSphTex = true;
                }
            }
            // Now customize the effect/technique names depending on what will be needed
            if(bToon)
            {
                if(bSphTex && bDiffTex)
                    matdst->setShaderName("PMD", "Toon_TexDiffSph");
                else if(bSphTex)
                    matdst->setShaderName("PMD", "Toon_TexSph");
                else if(bDiffTex)
                    matdst->setShaderName("PMD", "Toon_TexDiff");
                else
                    matdst->setShaderName("PMD", "Toon_NoTex");
            } else {
                if(bSphTex && bDiffTex)
                    matdst->setShaderName("PMD", "Default_TexDiffSph");
                else if(bSphTex)
                    matdst->setShaderName("PMD", "Default_TexSph");
                else if(bDiffTex)
                    matdst->setShaderName("PMD", "Default_TexDiff");
                else
                    matdst->setShaderName("PMD", "Default_NoTex");
            }
            // the rest
            fileHeader->AttachMaterial(matdst);
            matdst->setDiffuse(matsrc.diffuse.x, matsrc.diffuse.y, matsrc.diffuse.z);
            matdst->setAmbient(matsrc.ambient.x, matsrc.ambient.y, matsrc.ambient.z);
            matdst->setSpecular(matsrc.spec.x, matsrc.spec.y, matsrc.spec.z);
            matdst->setTransparency(matsrc.alpha, matsrc.alpha, matsrc.alpha);
            matdst->setSpecexp((float)matsrc.shininess);
            // TODO: texture names are in the material name, separated by '*'
            // ...
            // Create the primitive group for this material
            // here we use the same index buffer for all so it is later possible
            // to bind it in the GPU once for all
            sprintf(matname, "PG%d %s", m, matsrc.name);
            mesh->CreatePrimGroup(matname, 
                bufferIdx, bk3dlib::TRIANGLES, 
                matdst, 
                offset, matsrc.indices);
            offset += matsrc.indices;
        } //for(int m=0; m<(int)nMaterials; m++)
        if(offset != nIdx)
            printf("problem: material groups don't match the right amount of vtx indices\n");
        return true;
    }

}; // Class PMD

bool readPMD(const char* fullInStr, const char* singleName, bk3dlib::PFileHeader fileHeader)
{
    PMD pmd;
    if(!pmd.read(fullInStr))
        return false;
    return pmd.buildbk3d(singleName, fileHeader);
}
