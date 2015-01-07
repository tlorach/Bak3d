import sys
import os
# PYTHONPATH seems wrong when running from Sublim Text 2
# Add the path where bk3d module is located by hand...
sys.path.append('C:\\p4\\GitHub\\Bak3d\\cmake_built\\lib\\Debug')
import bk3d

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

class DataType:
	FLOAT32=0
	FLOAT16=1
	UINT32=2
	UINT16=3
	UINT8=4
	UNKNOWN=5

class Attribute:
	POSITION =				"position"
	VERTEXID =				"vertexid"
	COLOR =					 "color"
	NORMAL =					"normal"
	TANGENT =				 "tangent"
	BINORMAL =				"binormal"
	VTXNORMAL =			 "vtxnormal"
	TEXCOORD0 =			 "texcoord0"
	TEXCOORD1 =			 "texcoord1"
	TEXCOORD2 =			 "texcoord2"
	TEXCOORD3 =			 "texcoord3"
	BLIND0 =					"blind0"
	BLIND1 =					"blind1"
	BLIND2 =					"blind2"
	BLIND3 =					"blind3"
	BONESOFFSETS =		"bonesoffsets"
	BONESWEIGHTS =		"bonesweights"



head = bk3d.HeaderType("name1")

bk3dTr = bk3d.TransformType("someTransf")
head.addTransform(bk3dTr)
bk3dTr.name = "someTransf"
bk3dTr.pos = [0,0,0]
bk3dTr.scale = [1,1,1]
#bk3dTr.quat (RW): [x,y,z,w] list for quaternion
bk3dTr.rotation = [1.5, 0, 0]

bk3dmat = bk3d.MaterialType("material0")
bk3dmat.diffuse = [0.4, 0.8, 0.4]
bk3dmat.specular = [0.8, 0.8, 0.8]
bk3dmat.ambient = [0.1, 0.1, 0.1]

# Create buffers for position and normal
bvtxSrc = bk3d.BufferType("positionO")
bvtxSrc.dataType = DataType.FLOAT32
bvtxSrc.slot = 0;
bvtxSrc.components = 3
bvtx = bk3d.BufferType("position")
bvtx.dataType = DataType.FLOAT32
bvtx.slot = 0;
bvtx.components = 3
# Cube
bvtxSrc.addData([ 1, 1, 1],
				[-1, 1, 1],
				[-1,-1, 1],
				[ 1,-1, 1],
				[ 1, 1,-1],
				[-1, 1,-1],
				[-1,-1,-1],
				[ 1,-1,-1])
#print "Vtx pos : ", bvtxSrc.numItems

#normal buffer
bvtxnSrc = bk3d.BufferType("normalO")
bvtxnSrc.dataType = DataType.FLOAT32
bvtxnSrc.slot = 0;
bvtxnSrc.components = 3
bvtxn = bk3d.BufferType("normal")
bvtxn.dataType = DataType.FLOAT32
bvtxn.slot = 0;
bvtxn.components = 3
bvtxnSrc.addData(	[ 0, 0, 1],
					[ 0, 0,-1],
					[ 1, 0, 0],
					[-1, 0, 0],
					[ 0, 1, 0],
					[ 0,-1, 0])
    
# Now create a single index buffer built on the multi index buffers
bk3dSingleIdxBuffer = bk3d.BufferType('idxBuf')
bk3dSingleIdxBuffer.dataType = DataType.UINT32
bk3dSingleIdxBuffer.components = 1

# create the buffers for vtx, normals, texcoords and attach them to bk3dSingleIdxBuffer
bk3dIdxBuf_Pos = bk3d.BufferType('idxPos')
bk3dIdxBuf_Pos.dataType = DataType.UINT32
bk3dIdxBuf_Pos.components = 1
# attach it to bk3dSingleIdxBuffer
bk3dSingleIdxBuffer.SIB_AddBuffers(bk3dIdxBuf_Pos, bvtxSrc, bvtx)

bk3dIdxBuf_N = bk3d.BufferType('idxNorm')
bk3dIdxBuf_N.dataType = DataType.UINT32
bk3dIdxBuf_N.components = 1
# attach it to bk3dSingleIdxBuffer
bk3dSingleIdxBuffer.SIB_AddBuffers(bk3dIdxBuf_N, bvtxnSrc, bvtxn)

# create the references to vertices/normals (and others if there were)
faceindices = [[0,0],[1,0],[2,0],[2,0],[3,0],[0,0]]
for i in faceindices:
	bk3dIdxBuf_Pos.addData(i[0])
	bk3dIdxBuf_N.addData(i[1])

#just another example on how to feed the buffer:
faceindices = [
	[5,4,7,7,6,5, 4,0,3,3,7,4, 1,5,6,6,2,1, 4,5,1,1,0,4, 2,6,7,7,3,2], # pos
	[1,1,1,1,1,1, 2,2,2,2,2,2, 3,3,3,3,3,3, 4,4,4,4,4,4, 5,5,5,5,5,5]  # normals
]
bk3dIdxBuf_Pos.addData(faceindices[0])
bk3dIdxBuf_N.addData(faceindices[1])

#Compile the single index buffer
bk3dSingleIdxBuffer.SIB_Compile()
bk3dSingleIdxBuffer.SIB_ClearBuffers()
#get back the indices newly computed as a list()
singleIdxList = bk3dSingleIdxBuffer.dataAsList()

# Create the bk3d Mesh
mesh = bk3d.MeshType(name = "MyMesh")
# attach the main transformation
mesh.addTransformReference(bk3dTr)

# add the buffers
mesh.addVtxBuffer(bvtx)
mesh.addVtxBuffer(bvtxn)

# Primitive Group creation:
# a primitive group doesn't have any Python object... just a reference Id
# maybe later we shall add one
primId = mesh.createPrimGroup("primGroupName", bk3dSingleIdxBuffer, Topology.TRIANGLES, bk3dmat)

# bounding volumes
mesh.computeBoundingVolumes(bvtx)
# add this mesh
head.addMesh(mesh)

print("cooking and saving")
head.save("c:/tmp/simple_test.bk3d")

# execute the simple viewer to look at the result
os.system("\"C:\\p4\\GitHub\\Bak3d\\cmake_built\\bin\\Debug\\simpleOpenGL.exe\"")

