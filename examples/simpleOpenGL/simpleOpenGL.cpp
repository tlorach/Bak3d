#ifdef USEANTTWEAKBAR
#undef USEANTTWEAKBAR
#endif

#include "main.h"
#include "WindowInertiaCamera.h"

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <GL/glew.h>

#include <windows.h>
#include "resources.h"

#include "nv_math.h"
using namespace nv_math;

#include "GLSLProgram.h"

GLSLProgram g_progGrid;
GLSLProgram g_progMesh;
GLuint      g_vboGrid;
GLuint      g_vao;

mat4f g_ViewProj;
mat4f g_World;
mat4f g_WorldView;
mat4f g_WorldViewProj;


/////////////////////////////////////////////////////////////////////////
// grid Floor
static const char *g_glslv_grid = 
"#version 330\n"
"uniform mat4 mWVP;\n"
"layout(location=0) in  vec3 P;\n"
"out gl_PerVertex {\n"
"    vec4  gl_Position;\n"
"};\n"
"void main() {\n"
"   gl_Position = mWVP * vec4(P, 1.0);\n"
"}\n"
;
static const char *g_glslf_grid = 
"#version 330\n"
"#extension GL_ARB_separate_shader_objects : enable\n"
"uniform vec3 diffuse;"
"layout(location=0) out vec4 outColor;\n"
"void main() {\n"
"   outColor = vec4(diffuse,1);\n"
"}\n"
;

/////////////////////////////////////////////////////////////////////////
// Mesh
static const char *g_glslv_mesh = 
"#version 330\n"
"#extension GL_ARB_separate_shader_objects : enable\n"
"uniform mat4 mWVP;\n"
"layout(location=0) in  vec3 P;\n"
"layout(location=1) in  vec3 N;\n"
"layout(location=1) out vec3 outN;\n"
"out gl_PerVertex {\n"
"    vec4  gl_Position;\n"
"};\n"
"void main() {\n"
"   outN = N;\n"
"   gl_Position = mWVP * vec4(P, 1.0);\n"
"}\n"
;
static const char *g_glslf_mesh = 
"#version 330\n"
"#extension GL_ARB_separate_shader_objects : enable\n"
"uniform vec3 diffuse;"
"uniform vec3 lightDir;"
"layout(location=1) in  vec3 N;\n"
"layout(location=0) out vec4 outColor;\n"
"void main() {\n"
"\n"
"   float d1 = max(0.0, dot(N, lightDir) );\n"
"   float d2 = 0.6 * max(0.0, dot(N, -lightDir) );\n"
"   outColor = vec4(diffuse * (d2 + d1),1);\n"
"}\n"
;

vec4f scaleBias;

//---------------- bk3d Mesh
#ifndef NOGZLIB
#   include <zlib.h>
#endif
#include "bk3dEx.h"
bk3d::FileHeader * meshFile;

//---------------------- other params
vec3f g_posOffset = vec3f(0,0,0);
float g_scale = 1.0;
bool  g_bUseMaterial = true;

//-----------------------------------------------------------------------------
// Derive the Window for this sample
//-----------------------------------------------------------------------------
class MyWindow: public WindowInertiaCamera
{
public:
    MyWindow();
    virtual bool init();
    virtual void shutdown();
    virtual void reshape(int w, int h);
    //virtual void motion(int x, int y);
    //virtual void mousewheel(short delta);
    //virtual void mouse(NVPWindow::MouseButton button, ButtonAction action, int mods, int x, int y);
    //virtual void menu(int m);
    virtual void keyboard(MyWindow::KeyCode key, ButtonAction action, int mods, int x, int y);
    virtual void keyboardchar(unsigned char key, int mods, int x, int y);
    //virtual void idle();
    virtual void display();

    int argc;
    const char ** argv;
};

MyWindow::MyWindow()
{
}

//------------------------------------------------------------------------------
void printMessage(int level, const char * txt)
{
}
//------------------------------------------------------------------------------
void MyWindow::reshape(int w, int h)
{
    WindowInertiaCamera::reshape(w, h);
}

#if _MSC_VER
    #define snprintf _snprintf
#endif

#define drawString(x, y, ...)               \
    { char line[1024]; \
      snprintf(line, 1024, __VA_ARGS__); \
      char *p = line; \
      glWindowPos2i(x, y); \
      while(*p) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *p++); } }


//------------------------------------------------------------------------------
void MyWindow::shutdown()
{
    WindowInertiaCamera::shutdown();
}

//------------------------------------------------------------------------------
void MyWindow::keyboard(NVPWindow::KeyCode key, MyWindow::ButtonAction action, int mods, int x, int y)
{
    WindowInertiaCamera::keyboard(key, action, mods, x, y);

    if(action == MyWindow::BUTTON_RELEASE)
        return;
    switch(key)
    {
    case NVPWindow::KEY_F1:
        break;
    //...
    case NVPWindow::KEY_F12:
        break;
    }
#ifdef USESVCUI
    flushMFCUIToggle(key);
#endif
}

void MyWindow::keyboardchar( unsigned char key, int mods, int x, int y )
{
    switch( key )
    {
        case ' ':
            g_bUseMaterial = g_bUseMaterial ? false : true;
            break;
        case 'a':
            g_scale *= 0.5f;
            break;
        case 'q':
            g_scale *= 2.0f;
            break;
        case '2':
            nonStopRendering() = nonStopRendering() ? false : true;
            break;
    }
}

//------------------------------------------------------------------------------
bool MyWindow::init()
{
	if(!WindowInertiaCamera::init())
		return false;
    glewInit();
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    //
    // Shader compilation
    //
    if(!g_progGrid.compileProgram(g_glslv_grid, NULL, g_glslf_grid))
        return false;
    if(!g_progMesh.compileProgram(g_glslv_mesh, NULL, g_glslf_mesh))
        return false;

    //
    // Misc OGL setup
    //
    glClearColor(0.0f, 0.1f, 0.1f, 1.0f);
    glGenVertexArrays(1, &g_vao);
    glBindVertexArray(g_vao);
    //
    // Grid floor
    //
    glGenBuffers(1, &g_vboGrid);
    glBindBuffer(GL_ARRAY_BUFFER, g_vboGrid);
    #define GRIDDEF 20
    #define GRIDSZ 1.0f
    vec3f *data = new vec3f[GRIDDEF*4];
    vec3f *p = data;
    int j=0;
    for(int i=0; i<GRIDDEF; i++)
    {
        *(p++) = vec3f(-GRIDSZ, 0.0, GRIDSZ*(-1.0f+2.0f*(float)i/(float)GRIDDEF));
        *(p++) = vec3f( GRIDSZ*(1.0f-2.0f/(float)GRIDDEF), 0.0, GRIDSZ*(-1.0f+2.0f*(float)i/(float)GRIDDEF));
        *(p++) = vec3f(GRIDSZ*(-1.0f+2.0f*(float)i/(float)GRIDDEF), 0.0, -GRIDSZ);
        *(p++) = vec3f(GRIDSZ*(-1.0f+2.0f*(float)i/(float)GRIDDEF), 0.0, GRIDSZ*(1.0f-2.0f/(float)GRIDDEF));
    }
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3f)*GRIDDEF*4, data[0].vec_array, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //
    // Load a model
    //
    //---------------
    #define MODEL "/RCcar_v134.bk3d.gz"
    //---------------
    char tmpstr[300];
    if(argc > 1)
        sprintf(tmpstr, argv[1]);
    else
        //sprintf(tmpstr, "." MODEL);
        sprintf(tmpstr, RESOURCE_DIR MODEL);
    LOGI("Loading Mesh... %s\n", tmpstr);
    if(!(meshFile = bk3d::load(tmpstr)))
    {
        
        if(!(meshFile = bk3d::load(RESOURCE_DIR "/NVShaderBall_134.bk3d.gz")))
        {
            LOGE("error in loading mesh\n");
            return false;
        }
    }
    // To be Core OpenGL, we must create some buffer objects
    // Check the bounding boxes and adjust scaling
    float min[3] = {10000.0, 10000.0, 10000.0};
    float max[3] = {-10000.0, -10000.0, -10000.0};
    for(int i=0; i<meshFile->pMeshes->n; i++)
    {
        bk3d::Mesh *pMesh = meshFile->pMeshes->p[i];
        // walk through the slots (== buffers of attributes) and create a VBO for it
        // bk3d already has all needed for a quick setup
        for(int j=0; j<pMesh->pSlots->n; j++)
        {
            bk3d::Slot* pSlot = pMesh->pSlots->p[j];
            glGenBuffers(1, (unsigned int*)&pSlot->userData); // store directly in the structure... easier
            glBindBuffer(GL_ARRAY_BUFFER, pSlot->userData);
            glBufferData(GL_ARRAY_BUFFER, pSlot->vtxBufferSizeBytes, pSlot->pVtxBufferData, GL_STATIC_DRAW);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        for(int j=0; j<pMesh->pPrimGroups->n; j++)
        {
            bk3d::PrimGroup* pPG = pMesh->pPrimGroups->p[j];
            if(pPG->pIndexBufferData == NULL)
            {
                pPG->userPtr = NULL;
            } else {
                // the element buffer can be shared between many primitive groups
                // the main owner is specified by "pOwnerOfIB"
                // if no pOwnerOfIB or if pOwnerOfIB is itself, then we create the buffer
                // later, the indexArrayByteOffset will tell where to fetch indices
                if((pPG->pOwnerOfIB == pPG) || (pPG->pOwnerOfIB == NULL))
                {
                    glGenBuffers(1, (unsigned int*)&pPG->userPtr); // store directly in the structure... easier
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (unsigned int)pPG->userPtr);
                    glBufferData(GL_ELEMENT_ARRAY_BUFFER, pPG->indexArrayByteSize, pPG->pIndexBufferData, GL_STATIC_DRAW);
                } else {
                    // here the primitive group uses a subset of an existing buffer
                    // we always assume the owner was first in the list of primgroups
                    pPG->userPtr = pPG->pOwnerOfIB->userPtr;
                }
            }
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        // NOTE: to be accurate, we should go through primitive groups
        // see if this prim group used a transformation. If so, we should
        // take its bounding box into account...
        // For now, let's only take the Mesh's bbox
        mat4f matModel;
        vec3f meshMin(pMesh->aabbox.min);
        vec3f meshMax(pMesh->aabbox.max);
        if(pMesh->pTransforms && pMesh->pTransforms->n == 1)
            matModel = mat4f(pMesh->pTransforms->p[0]->Matrix());
        else
            matModel.identity();
        meshMin = matModel * meshMin;
        meshMax = matModel * meshMax;
        if(meshMin[0] < min[0]) min[0] = meshMin[0];
        if(meshMin[1] < min[1]) min[1] = meshMin[1];
        if(meshMin[2] < min[2]) min[2] = meshMin[2];
        if(meshMax[0] > max[0]) max[0] = meshMax[0];
        if(meshMax[1] > max[1]) max[1] = meshMax[1];
        if(meshMax[2] > max[2]) max[2] = meshMax[2];
    }
    g_posOffset.x = (max[0] + min[0])*0.5f;
    g_posOffset.y = (max[1] + min[1])*0.5f;
    g_posOffset.z = (max[2] + min[2])*0.5f;
    float bigger = 0;
    if((max[0]-min[0]) > bigger) bigger = (max[0]-min[0]);
    if((max[1]-min[1]) > bigger) bigger = (max[1]-min[1]);
    if((max[2]-min[2]) > bigger) bigger = (max[2]-min[2]);
    if((bigger) > 0.001)
    {
        g_scale = 1.0f / bigger;
        LOGI("Scaling the model by %f...\n", g_scale);
    }
    LOGI("Pg Up/Down : zoom\n");
    LOGI("Arrows: rotate the camera\n");
#ifdef NOGLUT
    LOGI("Ctrl + Arrows: pan the camera taget\n");
#endif
    LOGI("Mouse + left button: rotate the camera\n");
    LOGI("Mouse + middle button: Pan the camera target\n");
    LOGI("Mouse + right button: rotate Horizontally and change camera distance\n");
    LOGI("space: toggle wireframe\n");
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MyWindow::display()
{

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, getWidth(), getHeight());
    glEnable(GL_DEPTH_TEST);

    mat4f mWVP;
    mWVP = projMat() * viewMat();
    /////////////////////////////////////////////////
    //// Grid floor
    g_progGrid.enable();
    g_progGrid.setUniformMatrix4fv("mWVP", mWVP.mat_array, false);
    g_progGrid.setUniform3f("diffuse", 0.3f, 0.3f, 1.0f);
    glBindBuffer(GL_ARRAY_BUFFER, g_vboGrid);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3f), NULL);
    glDrawArrays(GL_LINES, 0, GRIDDEF*4);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(0);
    //g_progGrid.disable();

    //
    // setup the block1 with base matrices
    //
    vec3f up(0,1,0);
    //g_ViewIT = ...todo
    g_ViewProj = projMat() * viewMat();

    //
    // Mesh rendering
    //
    if(!meshFile)
    {
        swapBuffers();
        return;
    }
    //
    // default values for second block made of World...
    //
    g_progMesh.enable();
    vec3f lightDir(0.4f,0.8f,0.3f);
    lightDir.normalize();
    g_progMesh.setUniform3f("lightDir", lightDir[0], lightDir[1], lightDir[2]);

    mat4f m4_worldBase;
    m4_worldBase.identity();
    m4_worldBase.scale(g_scale);
    m4_worldBase.rotate(-nv_to_rad*90.0, vec3f(1,0,0));
    m4_worldBase.translate(g_posOffset);
    g_WorldView = viewMat() * m4_worldBase;
    g_WorldViewProj = projMat() * g_WorldView;

    g_progMesh.setUniformMatrix4fv("mWVP", g_WorldViewProj.mat_array, false);

    for(int i=0; i< meshFile->pMeshes->n; i++)
    {
        bk3d::Mesh *pMesh = meshFile->pMeshes->p[i];
        if(pMesh->visible == 0)
            continue;
        // case where the mesh references a transformation
        // the absolute value must be available by default
        // if more than one transf, skip it : this might be a list of skeleton transformations
        if(pMesh->pTransforms && pMesh->pTransforms->n == 1)
        {
            g_World = m4_worldBase * mat4f(pMesh->pTransforms->p[0]->Matrix());
            g_WorldView = viewMat() * g_World;
            g_WorldViewProj = projMat() * g_WorldView;
            //g_WorldIT = ... todo;
            mat4f matMV = viewMat() * g_World;

            g_progMesh.setUniformMatrix4fv("mWVP", g_WorldViewProj.mat_array, false);

        }
        //
        // let's make it very simple : each mesh attribute is *supposed* to match
        // the attribute Id of the shader. In real, meshes might not always match
        // but this is totally arbitrary...
        //
        int j = 0;
        for(int k=0; k<pMesh->pSlots->n;k++)
        {
            bk3d::Slot* pSlot = pMesh->pSlots->p[k];
            glBindBuffer(GL_ARRAY_BUFFER, pSlot->userData);
            // assign buffers
            for(int l=0; l<pSlot->pAttributes->n; l++)
            {
                glEnableVertexAttribArray(j);
                glVertexAttribPointer(j,
                    pSlot->pAttributes->p[l].p->numComp, 
                    pSlot->pAttributes->p[l].p->formatGL, GL_FALSE,
                    pSlot->pAttributes->p[l].p->strideBytes,
                    (void*)pSlot->pAttributes->p[l].p->dataOffsetBytes);
                j++;
            }
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        int MaxAttr = 16; //glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &MaxAttr);
        for(; j<MaxAttr;j++)
            glDisableVertexAttribArray(j);

        bk3d::PrimGroup* pPG;
        for(int pg=0; pg<pMesh->pPrimGroups->n; pg++)
        {
            pPG = pMesh->pPrimGroups->p[pg];
            if(pPG->visible == 0)
                continue;
            // case where the Primitive group references a transformation
            // the absolute value must be available by default
            if(pPG->pTransforms && pPG->pTransforms->n > 0)
            {
                g_World = m4_worldBase * mat4f(pPG->pTransforms->p[0]->Matrix());
                g_WorldView = viewMat() * g_World;
                g_WorldViewProj = projMat() * g_WorldView;
                g_progMesh.setUniformMatrix4fv("mWVP", g_WorldViewProj.mat_array, false);
            }

            bk3d::Material *pMat = pPG->pMaterial;
            if(pMat && g_bUseMaterial)
                g_progMesh.setUniform3f("diffuse", pMat->Diffuse()[0], pMat->Diffuse()[1], pMat->Diffuse()[2]);
            else
                g_progMesh.setUniform3f("diffuse", 0.8f, 0.8f, 0.8f);
            if(pPG->indexFormatGL)
            {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)pPG->userPtr);
                glDrawElements(
                    pPG->topologyGL,
                    pPG->indexCount,
                    pPG->indexFormatGL,
                    (void*)pPG->indexArrayByteOffset);
            } else {
                glDrawArrays(
                    pPG->topologyGL,
                    0, pPG->indexCount);
            }
        }

    }
	WindowInertiaCamera::display();
    swapBuffers();
}

/////////////////////////////////////////////////////////////////////////
// Main initialization point
//
int sample_main(int argc, const char** argv)
{
    // you can create more than only one
    static MyWindow myWindow;

    NVPWindow::ContextFlags context(
    4,      //major;
    4,      //minor;
    false,   //core;
    8,      //MSAA;
    true,   //debug;
    false,  //robust;
    false,  //forward;
    NULL   //share;
    );

    myWindow.argc = argc;
    myWindow.argv = argv;

    if(!myWindow.create("Empty", &context))
        return false;

    myWindow.makeContextCurrent();
    myWindow.swapInterval(0);

    while(MyWindow::sysPollEvents(false) )
    {
        myWindow.idle();
    }
    return true;
}
