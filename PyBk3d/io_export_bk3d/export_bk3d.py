#!BPY
# ##### BEGIN GPL LICENSE BLOCK #####
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software Foundation,
#  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
# ##### END GPL LICENSE BLOCK #####

# <pep8-80 compliant>

import struct
import bk3d
import bpy

triangulate = False
materials = dict()

#==========================================================================
#==========================================================================
class Topology:
    UNKNOWN_TOPOLOGY =0
    POINTS =1
    LINES =2
    LINE_LOOP =3
    LINE_STRIP =4
    TRIANGLES =5
    TRIANGLE_STRIP =6
    TRIANGLE_FAN =7
    QUADS =8
    QUAD_STRIP =9
    FAN =10
    LINES_ADJ =11
    LINE_STRIP_ADJ =12
    TRIANGLES_ADJ =13
    TRIANGLE_STRIP_ADJ =14
    # Use Topology as a way to tell how many vertices in the patch... avoid additional API func()
    PATCHES0 =20 # Only to be used as a base (say, PATCHES0 + i)
    PATCHES1 = 21
    PATCHES2 = 22
    PATCHES3 = 23
    PATCHES4 = 24
    PATCHES5 = 25
    PATCHES6 = 26
    PATCHES7 = 27
    PATCHES8 = 28
    PATCHES9 = 29
    PATCHES10= 30
    PATCHES11= 31
    PATCHES12= 32
    PATCHES13= 33
    PATCHES14= 34
    PATCHES15= 35
    PATCHES16= 36
    PATCHES17= 37
    PATCHES18= 38
    PATCHES19= 39
    PATCHES20= 40
    PATCHES21= 41
    PATCHES22= 42
    PATCHES23= 43
    PATCHES24= 44
    PATCHES25= 45
    PATCHES26= 46
    PATCHES27= 47
    PATCHES28= 48
    PATCHES29= 49
    PATCHES30= 50
    PATCHES31= 51
    PATCHES32= 52

dictPrimTopology = dict([(2,Topology.LINES), (3,Topology.TRIANGLES), (4,Topology.QUADS)])

class DataType:
    FLOAT32=0
    FLOAT16=1
    UINT32=2
    UINT16=3
    UINT8=4
    UNKNOWN=5

class Attribute:
    POSITION =              "position"
    VERTEXID =              "vertexid"
    COLOR =                  "color"
    NORMAL =                    "normal"
    TANGENT =                "tangent"
    BINORMAL =              "binormal"
    VTXNORMAL =          "vtxnormal"
    TEXCOORD0 =          "texcoord0"
    TEXCOORD1 =          "texcoord1"
    TEXCOORD2 =          "texcoord2"
    TEXCOORD3 =          "texcoord3"
    BLIND0 =                    "blind0"
    BLIND1 =                    "blind1"
    BLIND2 =                    "blind2"
    BLIND3 =                    "blind3"
    BONESOFFSETS =      "bonesoffsets"
    BONESWEIGHTS =      "bonesweights"

#==========================================================================
#==========================================================================
# http://www.blender.org/documentation/246PythonDoc/

#==========================================================================
# o : the Blender Object that owns the Mesh
# head : the bk3d 'header' object
#
def process_objectTransform(o, head):
    # try to find it first with its name
    bk3dTr = head.getTransform(o.name)
    if bk3dTr != None:
        return bk3dTr
    print("creating Transformation ", o.name)
    # if not found, create it
    bk3dTr = bk3d.TransformType(o.name)
    head.addTransform(bk3dTr)
    bk3dTr.name = o.name
    localPos = o.matrix_parent_inverse * o.location.to_4d()
    bk3dTr.pos = [localPos.x, localPos.y, localPos.z]
    bk3dTr.scale = [o.scale.x, o.scale.y, o.scale.z]
    # TODO: setup the order... Assuming XYZ for now
    bk3dTr.rotation = o.rotation_euler
    if o.parent != None:
        bk3dTr.parent = process_objectTransform(o.parent, head)
    bk3dTr.computeMatrix()
    return bk3dTr

#==========================================================================
def newFilename(ext):
    return '.'.join(Get('filename').split('.')[:-1] + [ext])

#==========================================================================
# NOTE: As opposed to MAYA, Bones in an Armature cannot be used as any other
# transformation. For example we can't set a Bone as the parent of an object
# In Maya, this is possible.
# NOTE: Matrix transformations are done here in the DirectX Order : v' = v.Mobj.Mworld (for example)
def gather_armature(mResolved, oArm, head):
    print("parsing Armature %s" % oArm.name)
    result = []
    # gather the transformation of this object
    # in mode of the cases, this object is the parent of the Mesh object that is
    # using it as a modifier. So the returned bk3d transformation was already there
    bk3dTr = process_objectTransform(oArm, head)
    armature = oArm.getData()
    # Now we need to add the bones to the transformation list :
    # in our bk3d model, bones and transformations are the same...
    matWorldArm = oArm.matrix
    #take pose information
    pose = oArm.getPose()
    poseBonesAsList = pose.bones.values()
    # loop into bones
    # Note: we assume the posebones are the same as the bones in the modifier...
    for poseBone in poseBonesAsList:
        print('processing ', poseBone.name)
        bone = armature.bones[poseBone.name]
        #NOTE: the current approach I took in bk3d is a simplified one :
        # - bindpose is taking the whole hierarchy of transforms : Armature+Object hierarchy
        # - Abs Matrix are expressed in World space
        # Blender takes the Object as the basis and makes a difference 
        # between Armature-bone transfs and the Object transfs...
        # GOOD thing : vertex position at rest pose are in Object-space
        # in my case, vertex pos at rest pose are in World-space :-(
        # TODO: Make vertex rest-pos in Object space like here...
        matWorldBone = bone.matrix['ARMATURESPACE'] * matWorldArm
        matObjSpaceBone = bone.matrix['ARMATURESPACE']
        matBone = bone.matrix['BONESPACE']
        matworldPose= poseBone.poseMatrix * matWorldArm
        matObjSpacePose= poseBone.poseMatrix
        # try to find it if already there
        bk3dTrBone = head.getTransform(poseBone.name)
        if bk3dTrBone == None:
            print("creating Bone Transformation ", poseBone.name)
            # if not found, create it
            bk3dTrBone = bk3d.TransformType(poseBone.name)
            head.addTransform(bk3dTrBone)
            #bk3dTr.name = poseBone.name
            bk3dTrBone.pos = [poseBone.loc.x, poseBone.loc.y, poseBone.loc.z]
            bk3dTrBone.scale = [poseBone.size[0],poseBone.size[1],poseBone.size[2]]
            # Transform to Euler, too...
            r = poseBone.quat.toEuler()
            bk3dTrBone.rotation = [r.x, r.y, r.z]
            #bk3dTrBone.quat = [poseBone.quat[0], poseBone.quat[1], poseBone.quat[2], poseBone.quat[3]]
            #Matrices are already computed... no need to ask for it. Just copy them
            #Note: Blender matrix is not a known object for my bk3d module
            # The bindpose is expressed in my bk3d as a "Object-space-to-Local-Space"
            mbp = Blender.Mathutils.Matrix(matObjSpaceBone) #matworldPose)
            mbp.invert()
            bk3dTrBone.matrixbindpose = [
             [mbp[0][0], mbp[0][1], mbp[0][2], mbp[0][3]],
             [mbp[1][0], mbp[1][1], mbp[1][2], mbp[1][3]],
             [mbp[2][0], mbp[2][1], mbp[2][2], mbp[2][3]],
             [mbp[3][0], mbp[3][1], mbp[3][2], mbp[3][3]]]
            m = matBone
            bk3dTrBone.matrix = [
             [m[0][0], m[0][1], m[0][2], 0],#3x3 matrix !!!
             [m[1][0], m[1][1], m[1][2], 0],
             [m[2][0], m[2][1], m[2][2], 0],
             [0,        0,       0,        1]]
            # absolute matrix of bones contains the influence of bindpose inverted
            # NOTE: Blender Matrix ops is DX-oriented : p'= p.M
            # NOTE: we could multiply by mbp this abs matrix... to save some ops...
            # But let's not do it to prevent some mistakes
            m = matworldPose #mbp * matworldPose
            bk3dTrBone.matrixabs = [
             [m[0][0], m[0][1], m[0][2], m[0][3]],
             [m[1][0], m[1][1], m[1][2], m[1][3]],
             [m[2][0], m[2][1], m[2][2], m[2][3]],
             [m[3][0], m[3][1], m[3][2], m[3][3]]]
            #TODO: other interesting components are available
            # we should also export them. Could be cool for some effects
        # store it to return the list
        result.append(bk3dTrBone)
    # connect parents/children
    for bone in armature.bones.values():
        # We MUST find them back. Or it is a bug
        print('looking for parent for ', bone.name)
        bk3dTrBone = head.getTransform(bone.name)
        if bone.hasParent():
            bk3dTrParent = head.getTransform(bone.parent.name)
            bk3dTrBone.parent = bk3dTrParent
        else:
            # NOTE: here again in our case : NO limit between Bone and other transforms
            # Thus the Bone's parent is a regular transform
            bk3dTrBone.parent = bk3dTr
    return result


#-----------------------------------------------------------------------
# Copy the mesh and related object
#
def copyMesh(osrc, suffix=str()):
    name = osrc.name + suffix
    # http://www.blender.org/documentation/blender_python_api_2_65_3/bpy.types.Object.html?highlight=to_mesh
    # 'RENDER' or 'PREVIEW'
    mdst = osrc.to_mesh(bpy.context.scene, True, 'PREVIEW') #bpy.data.meshes.new(name)
    #old mdst.getFromObject(osrc.name)    # Get the object's mesh data
    return mdst

#==========================================================================
def process_material(mat, head):
    bk3dmat = head.getMaterial(mat.name)
    if bk3dmat != None:
        #print(" -Bk3d name is ", bk3dmat.getName())
        return bk3dmat
    print("Processing Material ", mat.name, "mat.diffuse_color", mat.diffuse_color)
    bk3dmat = bk3d.MaterialType(mat.name)
    bk3dmat.diffuse = mat.diffuse_color
    bk3dmat.specular = mat.specular_color
    amb = mat.diffuse_color
    amb[0] *= mat.ambient
    amb[1] *= mat.ambient
    amb[2] *= mat.ambient
    bk3dmat.ambient = amb
    #bk3dmat.transparency = mat.getAlpha()
    #bk3dmat.reflectivity        = mat.get()
    #bk3dmat.specexp             = mat.hard
    #bk3dmat.translucency        = mat.get()
    #bk3dmat.shader              = mat.get()
    #bk3dmat.technique           = mat.get()
    #bk3dmat.textureDiffuse      = mat.get()
    #bk3dmat.specexptexture      = mat.get()
    #bk3dmat.ambienttexture      = mat.get()
    #bk3dmat.reflectivitytexture = mat.get()
    #bk3dmat.transparencytexture = mat.get()
    #bk3dmat.transluencytexture  = mat.get()
    #bk3dmat.speculartexture     = mat.get()
    head.addMaterial(bk3dmat)
    #print(" Bk3d name is ", bk3dmat.getName())
    return bk3dmat

#==========================================================================
# faces is a list of [face,singleFaceIdx]
# because we recomputed the idx, rather than taking the original from face
def createPrimGroup(bk3dmesh, mesh, mat,prim,smooth, faces, bk3dmat):
    if len(faces) == 0:
        print('Face group empty...')
        return -1
    try:
        idxBufName = 'IdxBuf_'+mesh.materials[mat].name+'_p'+str(prim)+'_s'+str(smooth)
    except:
        idxBufName = 'IdxBuf_p'+str(prim)+'_s'+str(smooth)
    idx = bk3d.BufferType(idxBufName)
    idx.dataType = DataType.UINT32
    idx.components = 1
    for face in faces: # face is [Blender face, [list of single idx]]
        for sidx in face[1]:
            #print("vtx index = %d" % sidx)
            idx.addData(sidx)
    try:
        primGroupName = mesh.materials[mat].name+'_p'+str(prim)+'_s'+str(smooth)
    except:
        primGroupName = 'PG_p'+str(prim)+'_s'+str(smooth)
    print("Prim ", dictPrimTopology[prim])
    primId = bk3dmesh.createPrimGroup(primGroupName, idx, dictPrimTopology[prim], bk3dmat)
    print('Created Prim Group : ', primGroupName, ' prim ID = ', primId)
    return primId

#==========================================================================
#
def veckey2d(v):
    return round(v[0], 6), round(v[1], 6)
#==========================================================================
# To return the current index of a UV coord and create it if not there, yet
# d (dict) : dictionnaries in which we maintain the UV indices
# uvTable (list): the table in which we put the UVs
# uv (Blender Vector 2D): the 2 components used as keys
#
# return : the index of the UV stored in uvTable
def addUVInTable(uv_dict, uvTable, uv):
    uvkey = veckey2d(uv)
    try:
        #print("found ", uvkey)
        return uv_dict[uvkey]
    except:
        i = uv_dict[uvkey] = len(uv_dict)
        uvTable.append(uv)
        #print('vt %.6f %.6f\n' % uv[:])
        return i

def faceToTriangles(face):
    if (len(face) == 4) and triangulate:
        triangles = [[face[0], face[1], face[2]],[face[2], face[3], face[0]]]
    else:
        triangles = [face]
    return triangles

#==========================================================================
# Generates uv tables and the indices pointing the right UVs in the faces
# Blender does not provide these arrays. So we need to build them by
# trying to group UVs when they are the same
# m: mesh
# http://gamedev.stackexchange.com/questions/30368/exporting-uv-coords-from-blender
def createTempUVTables(m):
    # pairs of [uv table , vertex face indices]. Now, only used for active TCs
    # we could avoid it but let's keep it so that we could later export more than the active TCs
    uvTables = list() 
    if len(m.uv_textures)==0:
        return uvTables
    # we might want to do a 'for' loop in the layers/textures instead of taking only active
    uv_layer=m.tessface_uv_textures.active.data
    uvDict = dict()
    uvTable = list()
    faces = list()
    # http://www.blender.org/documentation/blender_python_api_2_63_2/info_gotcha.html#ngons-and-tessellation-faces
    m.calc_tessface()
    for fidx, f in enumerate(m.tessfaces):
        faceIndices = list()
        uv=uv_layer[fidx]
        uv=uv.uv1, uv.uv2, uv.uv3, uv.uv4
        for i,v in enumerate(f.vertices):
            uvIdx = addUVInTable(uvDict, uvTable, uv[i])
            faceIndices.append(uvIdx)
        faces.extend(faceToTriangles(faceIndices))
    uvTables.append([uvTable,faces])
    return uvTables

#==========================================================================
# generates tables of weights and offsets
# the heaviers weights are put first
# returns a tuple: (skinWeightLists, skinOffsetLists, ninfluences)
# the idea is to use these tables to generate vertex attributes
# in a (xyzw) attribute, we can put 4 weights and in another, 4 offsets
# if more than 4 needed, we need to add another set of 4d attributes...
def createTempSkinTables(mResolved, oArm):
    armature = oArm.getData()
    # keep track of bone Ids in the order in which we stored them as bk3d trasnforms
    bones = armature.bones.values()
    offset = 0
    DOffset = dict()
    for bone in bones:
        DOffset[bone.name] = offset
        offset += 1
    # create separate lists of weights and offsets
    skinWeightLists = [ [] for i in range(0,offset)]
    skinOffsetLists = [ [] for i in range(0,offset)]
    maxInfluences = 0
    for vtx in mResolved.verts:
        vinfluences = mResolved.getVertexInfluences(vtx.index)
        i = 0
        for vinfl in vinfluences:
            if vinfl[1] > 0.0:
                skinOffsetLists[i].append(DOffset[vinfl[0]])
                skinWeightLists[i].append(vinfl[1])
                i += 1
        maxInfluences = max(maxInfluences, i)
        for j in range(i, offset):
            skinOffsetLists[j].append(0)
            skinWeightLists[j].append(0.0)
    print('max influences = ', maxInfluences)
    print('# of bones for skinning = ', offset)
    return (skinWeightLists, skinOffsetLists, maxInfluences)

#==========================================================================
# o : the Blender Object that owns the Mesh
# head : the bk3d 'header' object
# applyArmature : True to export the mesh transformed by Armature modifier
# applySubdiv=True : True to export the result of subdiv modifier(s)
# applyOtherModifiers : True to apply any other modifiers
#
def process_objectMesh(o, head, 
    applyArmature=False, 
    applySubdiv=True, 
    applyOtherModifiers=True,
    mergeTransfWithParentArmature=True, # to remove the object transforms and only use the parent's Armature transform
    worldSpaceVertex=False):
    m = o.data
    if len(m.vertices) == 0:
        return
    print("---------------------------------------------------")
    print("Mesh object ", m.name)
    # TODO - we must check if the mesh was *already referenced* earlier
    # by a previous object. If so, we must compare the Modifier list:
    # - if the modifiers are the same, we can share the mesh with the previous
    #   object. Only transformation is different : just add this one to the previously
    #   created mesh.
    # - if the object just has different shapekeys, We can share the mesh... and add these shapekeys
    # - if the modifier has different Armature : we could still keep the same mesh
    #   a) but we need to add one more "slot" for the additional Weight/offsets
    #   b) we also need to add the base tranform + list of Tranf Bones in the previous list of transforms
    # - if the Modifier list has other modifiers than ShapeKey or Armature, we need to duplicate it
    # - NOTE: for subdiv : if we export the patches, we could keep the same mesh...
    # look for Armature data only if we need to export it as a deformer
    #walk through modifiers and see if we have an Armature
    # TODO: export Vertex Groups for Shapekeys and/or Armatures
    # this could be good for when we do the computation on the CPU, on SPU or in CUDA
    # while the solution of writing data for each vertex in Slots is good for Gfx pipeline
    # We can use SLOTS and MESH_VERTEXID to create subsets of data...
    # Add this as an option to have one or the other or both solutions
    #Condition is that Armature be the first in the list
    modArmature = None
    modSubdiv = None
    # Now we can find it easily :
    oArm = o.find_armature()
    '''if len(o.modifiers) > 0:
        #if o.modifiers[0].type != Modifier.Types.ARMATURE:
        #    Draw.PupMenu('First modifier should be Armature')
        # TODO TODO TODO TODO TODO TODO TODO TODO
        for mod in o.modifiers:
            if mod.type == Blender.Modifier.Types.ARMATURE:
                if(applyArmature == False):
                    modArmature = mod
                    oArm = mod[Blender.Modifier.Settings.OBJECT]
                    print('Modifier : Armature ', oArm.name)
                    #cancel the action of the this modifier
                    print("temporary Canceling Armature Modifier")
                    prevArmREALTIME = mod[Blender.Modifier.Settings.REALTIME]
                    mod[Blender.Modifier.Settings.REALTIME]  = False
            elif mod.type == Blender.Modifier.Types.SUBSURF:
                print('Modifier : Subsurface')
                if(applySubdiv == False):
                    modSubdiv = mod
                    print("Canceling Subsurface")
                    prevSubdREALTIME = mod[Blender.Modifier.Settings.REALTIME]
                    mod[Blender.Modifier.Settings.REALTIME]  = False'''
    # Check if we can do the mergeTransfWithParentArmature option
    # for this, we want oArm to exist and to be the parent !
    # later we will see for other cases
    oMatrix = None
    if oArm != None:
        if(mergeTransfWithParentArmature == True):
            if oArm == o.parent:
                print("Using parent Armature ", oArm.name, " as the Mesh main transform")
                oMatrix = o.matrix_world
            else:
                print("Armature is NOT the parent of the object. Impossible for now to convert object transform to Armature")
    # process the transformations
    # Starting with this object which owns the first transformation
    # which will become a transformation
    # return the bk3dtransform
    if oMatrix == None:
        bk3dtransf = process_objectTransform(o, head) # register the object's transform
    else:
        bk3dtransf = process_objectTransform(oArm, head) # bypass object Transform and go to parent (later vtx will be converted)
    # update the mesh : changing modifiers can modify vertices
    m.update()
    m.calc_normals()
    #walk through the Shapekeys and make temporary resolved copy
    #Note that only the vtx/normals (+tangents) are modified... UVs don't
    #we must walk through shapekeys BUT also "resolve" them
    #witn copyMesh(o) so that we have the same modifiers applied
    # rather than creating new temp lists... let's just create temp
    mResolvedSKs = list()
    #let's start with SK 1 : #0 is supposed to be "No shapekey" and #1 the basis
    try:
        for k in range(1, len(m.shape_keys)):
            print("Shape Key" + m.shape_keys[k].name)
            o.active_shape_key_index = k+1
            m.update()
            m.calc_normals()
            mResolvedSKs.append(copyMesh(o, 'ShapeKey'+str(k)))
    except:
        print('No Shapekeys')
    #back to "No Shapekey"
    o.active_shape_key_index = 0
    m.update()
    m.calc_normals()
    #make a temp copy of the mesh for modifiers to be resolved
    #we will use this copy to gather the vertices/faces
    mResolved = copyMesh(o)
    # we need to link it to the object o. Later we will link-back the original one
    mesh_original = o.data
    o.data = mResolved
    # gather the materials that are referenced by the mesh
    global materials
    for mat in m.materials:
        try:
            bk3dmat = materials[mat.name]
            print("Found material ", mat.name)
        except:
            bk3dmat = process_material(mat, head)
            materials[mat.name] = bk3dmat
    #build temporary UV tables and indices for faces
    #Blender keeps UVs at face level rather than maintaining multi-index tables
    #Multi-index table is the way we want things
    # As a consequence, we will use the multi-index system of Bk3d (TODO)
    uvTableAndFacesList = createTempUVTables(mResolved)
    # If we have oArm, gather armature data
    bk3dBones, skinWeightLists, skinOffsetLists = None, None, None
    maxinfluences = 0
    # TODO TODO TODO TODO TODO
    '''if oArm!=None:
        bk3dBones = gather_armature(mResolved, oArm, head)
        #[( [Wlist1, Wlist2,...], [OffsList1, OffsList2,...] )
        (skinWeightLists, skinOffsetLists, maxinfluences) = createTempSkinTables(mResolved, oArm)
    ''' 
    # Create buffers for position and normal
    bvtxSrc = bk3d.BufferType("positionO")
    bvtxSrc.dataType = DataType.FLOAT32
    bvtxSrc.slot = 0;
    bvtxSrc.components = 3
    bvtx = bk3d.BufferType("position")
    bvtx.dataType = DataType.FLOAT32
    bvtx.slot = 0;
    bvtx.components = 3
    print("Vtx pos : ", len(mResolved.vertices))
    for vertex in mResolved.vertices:
        co = vertex.co.to_4d()
        if oMatrix != None:
            co = co * oMatrix
        bvtxSrc.addData(co.to_3d())
    print("Vtx pos : ", bvtxSrc.numItems)
    #normal buffer
    bvtxnSrc = bk3d.BufferType("normalO")
    bvtxnSrc.dataType = DataType.FLOAT32
    bvtxnSrc.slot = 0;
    bvtxnSrc.components = 3
    bvtxn = bk3d.BufferType("normal")
    bvtxn.dataType = DataType.FLOAT32
    bvtxn.slot = 0;
    bvtxn.components = 3
    print("Vtx normals : ", len(mResolved.vertices), '')
    for vertex in mResolved.vertices:
        #TODO: transform if needed
        bvtxnSrc.addData(vertex.normal)
    #TC buffers
    # TODO  TODO TODO TODO TODO TODO
    bvtxUVs = list()
    for i,uvTableAndFaces in enumerate(uvTableAndFacesList):
        bvtxUVSrc = bk3d.BufferType("texcoordO"+str(i))
        bvtxUVSrc.dataType = DataType.FLOAT32
        bvtxUVSrc.slot = 0;
        bvtxUVSrc.components = 2
        bvtxUV = bk3d.BufferType("texcoord"+str(i))
        bvtxUV.dataType = DataType.FLOAT32
        bvtxUV.slot = 0;
        bvtxUV.components = 2
        print("Vtx TEXCOORD"+str(i)+" : ", len(uvTableAndFaces[0]))
        for uv in uvTableAndFaces[0]:
          bvtxUVSrc.addData(uv)
        bvtxUVs.append([bvtxUVSrc, bvtxUV])
    # Create BlenderShape (Shapekey) tables
    # WARNING: in this mode we use the whole mesh, regardless of which Vertex Group is related to the Shapekey
    # Another mode should be made to that we export the vertex group as index (either as attribute, or as regular index)
    # we assume that the copies of Shapekey poses (mResolvedSKs) are coherent with the mResolved mesh !!
    #TODO: add tangent if nececssary
    '''bvtxSKs = list() # will contain the 4 created buffers : 2 for pos and 2 for normal
    for mResolvedSK in mResolvedSKs:
        print("making Shapekey buffers ", mResolvedSK.name)
        mResolvedSK.update()
        mResolvedSK.calc_normals()
        # Create buffers for position and normal of the Shapekey
        bvtxSKSrc = bk3d.BufferType("DeltaPositionO"+str(len(bvtxSKs)))
        bvtxSKSrc.dataType = DataType.FLOAT32
        bvtxSKSrc.slot = len(bvtxSKs); # slots when in ShapeKey, is a way to separate them : one slot for one Shapekey
        bvtxSKSrc.components = 3
        bvtxSK = bk3d.BufferType("position"+str(len(bvtxSKs)))
        bvtxSK.dataType = DataType.FLOAT32
        bvtxSK.slot = len(bvtxSKs);
        bvtxSK.components = 3
        print("SK Vtx pos : ", len(mResolved.verts), '')
        i = 0
        for vertex in mResolvedSK.verts:
            co = vertex.co.copy().resize4D()
            if oMatrix != None:
                co = co * oMatrix
            co2 = mResolved.verts[i].co.copy().resize4D()
            if oMatrix != None:
                co2 = co2 * oMatrix
            bvtxSKSrc.addData((co - co2).resize3D())
            i+=1
        #normal buffer
        bvtxnSKSrc = bk3d.BufferType("normalO"+str(len(bvtxSKs)))
        bvtxnSKSrc.dataType = DataType.FLOAT32
        bvtxnSKSrc.slot = len(bvtxSKs); # slots when in ShapeKey, is a way to separate them : one slot for one Shapekey
        bvtxnSKSrc.components = 3
        bvtxnSK = bk3d.BufferType("normal"+str(len(bvtxSKs)))
        bvtxnSK.dataType = DataType.FLOAT32
        bvtxnSK.slot = 0;
        bvtxnSK.components = 3
        print("SK Vtx normals : ", len(mResolved.verts), '')
        i = 0
        for vertex in mResolvedSK.verts:
            # TODO: transform Normals with Inverse-Transpose
            bvtxnSKSrc.addData(vertex.no - mResolved.verts[i].no)
            i+=1
        bvtxSKs.append([bvtxSKSrc, bvtxSK, bvtxnSKSrc, bvtxnSK])
        '''
    # Now create a single index buffer built on the multi index buffers
    bk3dSingleIdxBuffer = bk3d.BufferType('idx_' + o.name)
    bk3dSingleIdxBuffer.dataType = DataType.UINT32
    bk3dSingleIdxBuffer.components = 1
    # create the buffers for vtx, normals, texcoords and attach them to bk3dSingleIdxBuffer
    bk3dIdxBuf_Pos = bk3d.BufferType('idxPos_' + o.name)
    bk3dIdxBuf_Pos.dataType = DataType.UINT32
    bk3dIdxBuf_Pos.components = 1
    # attach it to bk3dSingleIdxBuffer
    bk3dSingleIdxBuffer.SIB_AddBuffers(bk3dIdxBuf_Pos, bvtxSrc, bvtx)
    bk3dIdxBuf_N = bk3d.BufferType('idxN_' + o.name)
    bk3dIdxBuf_N.dataType = DataType.UINT32
    bk3dIdxBuf_N.components = 1
    # attach it to bk3dSingleIdxBuffer
    bk3dSingleIdxBuffer.SIB_AddBuffers(bk3dIdxBuf_N, bvtxnSrc, bvtxn)
    # Skinning data
    #(skinWeightLists, skinOffsetLists, maxinfluences)
    # TODO: when more than 4 !!
    '''if maxinfluences > 4:
        print('warning : skinning weights > 4 : ', maxinfluences)
    if maxinfluences > 0:
        bvtxSkinWSrc = bk3d.BufferType("bonesweightsO")
        bvtxSkinWSrc.dataType = DataType.FLOAT32
        bvtxSkinWSrc.slot = 0;
        bvtxSkinWSrc.components = 4
        bvtxSkinW = bk3d.BufferType("bonesweights")
        bvtxSkinW.dataType = DataType.FLOAT32
        bvtxSkinW.slot = 0;
        bvtxSkinW.components = 4
        print("Vtx bonesweights : ", len(mResolved.verts))
        bvtxSkinOSrc = bk3d.BufferType("bonesoffsetsO")
        bvtxSkinOSrc.dataType = DataType.FLOAT32
        bvtxSkinOSrc.slot = 0;
        bvtxSkinOSrc.components = 4
        bvtxSkinO = bk3d.BufferType("bonesoffsets")
        bvtxSkinO.dataType = DataType.FLOAT32
        bvtxSkinO.slot = 0;
        bvtxSkinO.components = 4
        print("Vtx bonesOffsets : ", len(mResolved.verts))
        i = 0
        for w0 in skinWeightLists[0]:
            weights4 = [w0, 0,0,0]
            for j in range(1, maxinfluences):
                weights4[j] = skinWeightLists[j][i]
            bvtxSkinWSrc.addData(weights4)
            offsets4 = [0,0,0,0]
            for j in range(0, maxinfluences):
                offsets4[j] = skinOffsetLists[j][i]
            bvtxSkinOSrc.addData(offsets4)
            i += 1
        # attach to bk3dSingleIdxBuffer using the same idxbuffer than pos
        bk3dSingleIdxBuffer.SIB_AddBuffers(bk3dIdxBuf_Pos, bvtxSkinWSrc, bvtxSkinW)
        bk3dSingleIdxBuffer.SIB_AddBuffers(bk3dIdxBuf_Pos, bvtxSkinOSrc, bvtxSkinO)
        '''
    # Create the idx buffers for Blendshapes and associate for SIB compilation
    # WARNING: we re-use the buffers from regular pos and normal... we assume they match !
    '''for bvtxSK in bvtxSKs:
        # create the buffers for vtx, normals, texcoords and attach them to bk3dSingleIdxBuffer
        #bk3dIdxBuf_Pos = bk3d.BufferType('idxPos_' + o.name)
        #bk3dIdxBuf_Pos.dataType = DataType.UINT32
        #bk3dIdxBuf_Pos.components = 1
        # attach it to bk3dSingleIdxBuffer
        bk3dSingleIdxBuffer.SIB_AddBuffers(bk3dIdxBuf_Pos, bvtxSK[0], bvtxSK[1])

        #bk3dIdxBuf_N = bk3d.BufferType('idxN_' + o.name)
        #bk3dIdxBuf_N.dataType = DataType.UINT32
        #bk3dIdxBuf_N.components = 1
        # attach it to bk3dSingleIdxBuffer
        bk3dSingleIdxBuffer.SIB_AddBuffers(bk3dIdxBuf_N, bvtxSK[2], bvtxSK[3])
        '''
    # TC
    bk3dIdxBuf_TCs = list()
    for i,uvTableAndFaces in enumerate(uvTableAndFacesList):
        bk3dIdxBuf_TC = bk3d.BufferType('idxTC_'+str(len(bk3dIdxBuf_TCs))+'_'+o.name)
        bk3dIdxBuf_TC.dataType = DataType.UINT32
        bk3dIdxBuf_TC.components = 1
        bk3dIdxBuf_TCs.append(bk3dIdxBuf_TC)
        # attach it to the single idx buffer
        bk3dSingleIdxBuffer.SIB_AddBuffers(bk3dIdxBuf_TC, bvtxUVs[i][0], bvtxUVs[i][1])
        i=i+1
    #populate these idx buffers
    mResolved.calc_tessface()
    for i,face in enumerate(mResolved.tessfaces):
        for j,vertex in enumerate(face.vertices):
            bk3dIdxBuf_Pos.addData(vertex)
            if face.use_smooth == True:
                bk3dIdxBuf_N.addData(vertex)
            else:
                # we use a normal from the face : add it to our existing list
                #TODO: transform if needed
                bvtxnSrc.addData(face.normal)
                bk3dIdxBuf_N.addData(bvtxnSrc.numItems-1)
            for k,uvTableAndFaces in enumerate(uvTableAndFacesList):
                # uvTableAndFaces[1] is for face vertex indices ([0] for tcs)
                bk3dIdxBuf_TCs[k].addData(uvTableAndFaces[1][i][j])
    #debug info
    print("Index Pos table : ", bk3dIdxBuf_Pos.numItems)
    #k=0
    #for uvTableAndFaces in uvTableAndFacesList:
    #    print("Index UV"+str(i)+" table : ", bk3dIdxBuf_TCs[k].numItems)
    #    k+=1
    #Compile the single index buffer
    bk3dSingleIdxBuffer.SIB_Compile(0)
    bk3dSingleIdxBuffer.SIB_ClearBuffers()
    #get back the indices newly computed as a list()
    singleIdxList = bk3dSingleIdxBuffer.dataAsList()
    print("Single Index table : ", bk3dSingleIdxBuffer.numItems)
    print("new Vtx table : ", bvtx.numItems)
    print("new Normal table : ", bvtxn.numItems)
    #i=0
    #for bvtxUV in bvtxUVs:
    #    print("new UV"+str(i)+" : ", bvtxUV[1].numItems)
    #    i+=1
    #i=0
    #for bvtxSK in bvtxSKs:
    #    print("new SK"+str(i)+" : ", bvtxSK[1].numItems)
    #    i+=1
    # Create the bk3d Mesh
    mesh = bk3d.MeshType(name = m.name)
    # attach the main transformation, if exists
    if(bk3dtransf != None):
        mesh.addTransformReference(bk3dtransf)
    if(bk3dBones != None):
        for bk3dBone in bk3dBones:
            #additional transformation references are for the skinning of the mesh
            mesh.addTransformReference(bk3dBone)
    # add the buffers
    mesh.addVtxBuffer(bvtx)
    mesh.addVtxBuffer(bvtxn)
    '''if maxinfluences > 0:
        mesh.addVtxBuffer(bvtxSkinW)
        mesh.addVtxBuffer(bvtxSkinO)'''
    for bvtxUV in bvtxUVs:
        #print(bvtxUV[1])
        mesh.addVtxBuffer(bvtxUV[1])
    '''for bvtxSK in bvtxSKs:
        mesh.addVtxBuffer(bvtxSK[1], True)
        mesh.addVtxBuffer(bvtxSK[3], True)
        '''
    # Primitive Group creation:
    #sort faces according to 
    # - materials
    #   - primitive type : line (2), triangles (3), quads(4)
    #     - smooth on/off
    #key is simple the number of the material in the list
    if(len(mResolved.materials) > 0):
        sorted_faces = dict([
            (id, dict([
            (2,dict([(0,[]),(1,[])])),
            (3,dict([(0,[]),(1,[])])),
            (4,dict([(0,[]),(1,[])]))
            ])) for id in range(0, len(mResolved.materials))])
    else:
        sorted_faces = dict([
            (0, dict([
            (2,dict([(0,[]),(1,[])])),
            (3,dict([(0,[]),(1,[])])),
            (4,dict([(0,[]),(1,[])]))
            ])) ])
    #Walk through faces and put them in whatever slot they correspond
    i=0
    for f in mResolved.tessfaces:
        l=len(f.vertices)
        #if l == 4: # TODO Only do this if we asked for triangles only
        #    l = 6
        singleFaceIdx = singleIdxList[i:i+l]
        #print("face : l=", l, singleFaceIdx)
        sorted_faces[f.material_index][l][f.use_smooth].append([f,singleFaceIdx]) # append() belongs to the list object in dictionnary
        #increment i to reach the next 'single' indices we computed earlier
        i=i+l
    # Walk through sorted faces
    print("===============", materials)
    for mat, prim_faces in sorted_faces.items():
        try:
            bk3dmat = materials[m.materials[mat].name] # get the bk3d material from the indexed name
            print("using material ", bk3dmat.getName())
        except:
            bk3dmat = None
            print("NOT using any material ")
        for prim, smooth_faces in prim_faces.items():
            for smooth, faces in smooth_faces.items():
                #faces is a list of [face,singleFaceIdx]
                print('Prim Group for : mat ', mat, ' prim ', prim, ' smooth ', smooth, ' faces: ', len(faces))
                if bk3dmat is None:
                    print("No material for this prim group...")
                    createPrimGroup(mesh, m, mat,prim,smooth, faces, None)
                else:
                    createPrimGroup(mesh, m, mat,prim,smooth, faces, bk3dmat)
    mesh.computeBoundingVolumes(bvtx)
    head.addMesh(mesh)
    # link again the object with its original mesh
    o.data = mesh_original
    #settings : back to original values
    '''if modArmature != None:      
        modArmature[Blender.Modifier.Settings.REALTIME] = prevArmREALTIME
    if modSubdiv != None:
        modSubdiv[Blender.Modifier.Settings.REALTIME] = prevSubdREALTIME'''


#==========================================================================

def process_selected(filename):
    #global materials = dict() #we will store materials that are referenced by what is selected
    head = bk3d.HeaderType("name1")
    scene = bpy.context.scene
    #bpy.types.Mesh.update (calc_tessface=True)
    for o in bpy.context.selected_objects:
        if o.type == 'MESH':
            process_objectMesh(o, head)
    print("Cooking and Saving to " + filename)
    head.save(filename)

#print("Testing")
#process_selected("c:/tmp/simple_test.bk3d")
# execute the simple viewer to look at the result
#import os
#os.system("\"C:\\p4\\GitHub\\Bak3d\\cmake_built\\bin\\Debug\\simpleOpenGL.exe\"")