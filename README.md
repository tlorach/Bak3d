This binary file contains some baked 3D data (read bak3d with an inverted 'E' ) very much ready to be sent down to graphic APIs.
The idea is to provide in the most direct way the data that we need to render some geometry.

It should not be considered as a scene, nor as a *scene-graph*. The only scene-graph-looking feature is the hierarchy of transformations. But I didn't want to venture to scene graph work for two reasons :
* the complexity of this file would seriously increase;
* scene-graphs aren't always the best solution for some projects.

You should considere it as a piece of data that one would want to use in a more complex scene definition. In other words, a scene-graph definition could be maintaned outside, while a bk3d file would be used to add data to this graph. In other words, you could consider one bk3d file as one Mesh object.

bk3d file can be created from various ways (to be published here asap) :
* Maya : exporter available. But new version under construction
* Blender : I createa a Python module and some Python scripting to allow Blender to export in this format
* bk3dutil : multi-purpose util tool to tweak the file
* Bak3dView : simple viewer
* bk3dlib : a library that can be used to generate the binary files. It can also be used to load these files but it is not a necessary component since the whole point of bk3d file is to be self sufficient and to not require complex parsing from a library. So bk3dlib is only necessary to edit or create a bk3d binary file. 

# Features

* **API** agnostic : can work either for OpenGL or DX9/10/11. Just need to use a different helper
* contains scene information : transformation hierarchy, mesh, bones refs, blendshapes
* provides "baked" information, meaning that you only need to bind pointers, values to OpenGL
The exporter did already all the Job. This means that we don't have anything more
to do at runtime but bind these data to OpenGL API. The Exporter did :
 * multi-index solved
 * choice of specific attributes done at export time
 * interleaved attribs vs attrib separately : choose in exporter
 * Stripifier done at export time
 * Meshmender (tan binorm computation) done at export time
* helper available to do the job for you. This demo don't use it...
* everything in one .h files (without animation and helpers), so nothing hidden in a lib
* can read gzipped files
* can concatenate many files together : we can append binary files together and use 
the resulting big one as a whole scene.
* flexible attribute assignment : at export time you can choose many solutions for attribs
 * interleave some of the or keep some others in separate buffers
 * ask for maya's Blind attributes
 * texcoords, tangent, binormal, normal, bones weights/offsets...
 * provide blendshapes as a set of interleaved buffers


# How to use it

The use of this file is fairly easy :
* load the binary file bk3d ( bk3d::Load() can help )
* resolve the pointers of this binary area
* use it : just walk through the structures to find back data
 * either use directly the buffers
 * or setup the API resources from the bk3d file : DX buffers, OpenGL VBOs...

Note that this binary file obviously doesn't provide the tools to compute anything : it only contains data for these computations. This work must be done by the applications. For example, the curve computation, the transformations must be calculated by the application.

Here are the main objects available in this file :

* bk3d::FileHeader : the main entry point. The main Node, too. This structure gathers all the objects that are available in the file. You can consider it as the root of a scene, although it doesn't claim to be as complex as a scene is in a scene-graph. This header gives access to the following :
 * bk3d::Meshes ( bk3d::Mesh ) gathered in a bk3d::MeshPool
 * bk3d::Transform ( in bk3d::TransformPool ) : all the transformations
 * bk3d::MayaCurve ( in bk3d::MayaCurvePool ) : all the Curves (from Maya)
 * bk3d::Material ( in bk3d::MaterialPool ) : all the materials
 * bk3d::IKHandle ( in bk3d::IKHandlePool ) : Inverse Kinematic handles
 * bk3d::RelocationTable : this table is used to compute the offsets into pointers after load-time

* bk3d::Mesh : a mesh is really a single vertex-buffer geometry made of 1 or more rendering-groups  (Primitive groups) bk3d::PrimGroup. 
Note that a mesh is not considered as a node of a scene-graph. But a mesh will reference 0 or many transformations : this is the way we know how a mesh would need to be transformed.
 * bk3d::Slot ( in bk3d::SlotPool ) : 
 * bk3d::PrimGroup ( in bk3d::PrimGroupPool ) : a primitive group is directly what is needed to perform a Drawcall (triangles, strips etc.)
  * bk3d::Material
  * bk3d::Transform ( in bk3d::TransformRefs ) : transformation(s) for this Primitive group (drawcall)
 * bk3d::Attribute ( in bk3d::AttributePool ) : 
 * bk3d::Attribute ( in bk3d::AttributePool ) : 
 * bk3d::Slot ( in bk3d::SlotPool ) : 
 * bk3d::Transform ( in bk3d::TransformRefs ) : 
 * bk3d::FloatArray ( in bk3d::FloatArrayPool ) :
 * bk3d::BSphere : Bounding Sphere for this Mesh
 * bk3d::AABBox : Axis-aligned bounding box
 
* bk3d::Transform : this is a Maya-style transformation. It contains a bunch of transformation components, such as position, rotation, pivot, absolute matrix (resulting from components...). These transformations also contain some hierarchy : matrices can have a parent and have some children. This hierarchy can be considered as the only scene-graph oriented data. However a transformation doesn't reference to any children nodes such as a Mesh or other things.

* bk3d::MayaCurve : the maya curve is a generic curve spline definition for any purpose. The idea is to expose curves and connect them to targets for animation. See bk3d::FloatArray for more about connection.

* bk3d::Material : contains material properties that are typically exposed in DCC applications such as Maya or Blender. This is also where we store texture names for various uses and even Shader effect names.

* bk3d::IKHandle : Inverse Kinematic Handle is the object that should be used to resolve IK : this is the End-point of a skeleton that tries to converge to a target destination.

Here is an example on how to use a Mesh in the simplest way ever : by using the immediate mode of OpenGL

````
glEnableClientState(GL_VERTEX_ARRAY);
glEnableClientState(GL_NORMAL_ARRAY);
for(int i=0; i< meshFile->pMeshes->n; i++)
{
    bk3d::Mesh *pMesh = meshFile->pMeshes->p[i];
    //glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexPointer(
        pMesh->pAttributes->p[0].p->numComp, 
        pMesh->pAttributes->p[0].p->formatGL,
        pMesh->pAttributes->p[0].p->strideBytes,
        pMesh->pAttributes->p[0].p->pAttributeBufferData);
    glNormalPointer(
        pMesh->pAttributes->p[1].p->formatGL,
        pMesh->pAttributes->p[1].p->strideBytes,
        pMesh->pAttributes->p[1].p->pAttributeBufferData);
    for(int pg=0; pg<pMesh->pPrimGroups->n; pg++)
    {
        glDrawElements(
            pMesh->pPrimGroups->p[pg]->topologyGL,
            pMesh->pPrimGroups->p[pg]->indexCount,
            pMesh->pPrimGroups->p[pg]->indexFormatGL,
            pMesh->pPrimGroups->p[pg]->pIndexBufferData);
    }

}
````

Note that arguments for drawcalls are available from the bk3d data. The idea behind this approach : baked data are supposed to be usable as fast as possible. They grouping of primitives was done at export time and is available for rendering without further work.

The same is true for Direct 9/10/11 : arguments are also available directly from the bk3d data. However DX10/11 requires you to create an Input layout from the attributes to use. Such a structure is not available from here. However the data exposed here are enough to generate the Input Layout.

Every modern renderer will use buffers : VBO/IBO for OpenGL and vertex buffers for DX. In this case additional setup must be done prior to use bk3d meshes.

This essentially boils down to walk through Meshes and create a DX/OpenGL buffer object for every single buffer of vertices and every buffer of Elements.

A convenient way is to attach to any bk3d::Mesh data that will maintain DX or OpenGL resources : you can store these additional data via the **"userPtr"** ( bk3d::Mesh::userPtr, bk3d::Slot::userPtr and bk3d::PrimGroup::userPtr ) . Then everytime you need to access the Mesh's resources, you can use a wrapper to extend the access of extra data :

````
class OGLMeshWrapper
{
...
    Mesh *    m_pMesh;
    OGLMeshWrapper(Mesh *pMesh, COGLSceneHelper* pOwner=NULL) : m_pMesh(pMesh) {}
    inline  AttrMapping*        getAttrMapping() { 
        return (AttrMapping*) m_pMesh->userPtr;
    }
    inline  AttrMapping*        createAttrMapping(AttrMapping* p=NULL) { 
        m_pMesh->userPtr = (p ? p : malloc(sizeof(AttrMapping)));
        if(m_pMesh->userPtr) ZeroMemory(m_pMesh->userPtr, sizeof(AttrMapping));
        return (AttrMapping*) m_pMesh->userPtr;
    }
    // we can even redefine '->' operator to directly access m_pMesh...
...
{;
````

Then you can do a cheap wrapping of bk3d::Mesh with OGLMeshWrapper anywhere, as a local variable :
````
for(int i=0; i< header->pMeshes->n; i++) {
    OGLMeshWrapper oglMesh(header->pMeshes->p[i])
    ...
````

More details are available from COGLSceneHelper & OGLMeshWrapper classes, as an example.

# TODO

\todo camera infos, deformers (Lattice, sculpt...), lights...

----------------------------------------------------------------------------
````
    Copyright (c) 2013, Tristan Lorach. All rights reserved.

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
````
