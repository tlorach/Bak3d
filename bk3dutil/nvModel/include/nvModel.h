//
// nvModel.h - Model support class
//
// The nvModel class implements an interface for a multipurpose model
// object. This class is useful for loading and formatting meshes
// for use by OpenGL. It can compute face normals, tangents, and
// adjacency information. The class supports the obj file format.
//
// Author: Evan Hart
// Email: sdkfeedback@nvidia.com
//
// Copyright (c) NVIDIA Corporation. All rights reserved.
////////////////////////////////////////////////////////////////////////////////

#ifndef NV_MODEL_H
#define NV_MODEL_H

#ifdef WIN32
/*#ifdef NVMODEL_EXPORTS
#define NVSDKENTRY __declspec(dllexport)
#else
#define NVSDKENTRY __declspec(dllimport)
#endif*/
#define NVSDKENTRY
#if _MSC_VER >= 1400  
# pragma warning(disable:4996) 
#endif 
#else
#define NVSDKENTRY
#endif

#ifndef GLuint
    typedef unsigned int GLuint;
#endif

#include <vector>
#include <assert.h>

namespace nv {

    class Model {
    private:
        NVSDKENTRY Model();

    public:

        //
        // Enumeration of primitive types
        //
        //////////////////////////////////////////////////////////////
        enum PrimType {
            eptNone = 0x0,
            eptPoints = 0x1,
            eptEdges = 0x2,
            eptTriangles = 0x4,
            eptTrianglesWithAdjacency = 0x8,
            eptAll = 0xf
        };

        static const int NumPrimTypes = 4;

        // 
        // Create a model
        //
        //////////////////////////////////////////////////////////////
        NVSDKENTRY static Model* Create();

        NVSDKENTRY virtual ~Model();

        //
        // loadModelFromFile
        //
        //    This function attempts to determine the type of
        //  the filename passed as a parameter. If it understands
        //  that file type, it attempts to parse and load the file
        //  into its raw data structures. If the file type is
        //  recognized and successfully parsed, the function returns
        //  true, otherwise it returns false.
        //
        //////////////////////////////////////////////////////////////
        NVSDKENTRY bool loadModelFromFile( const char* file);

        //
        //  compileModel
        //
        //    This function takes the raw model data in the internal
        //  structures, and attempts to bring it to a format directly
        //  accepted for vertex array style rendering. This means that
        //  a unique compiled vertex will exist for each unique
        //  combination of position, normal, tex coords, etc that are
        //  used in the model. The prim parameter, tells the model
        //  what type of index list to compile. By default it compiles
        //  a simple triangle mesh with no connectivity. 
        //
        //////////////////////////////////////////////////////////////
        NVSDKENTRY void compileModel( PrimType prim = eptTriangles);
        NVSDKENTRY void compileModel2();

        //
        //  computeBoundingBox
        //
        //    This function returns the points defining the axis-
        //  aligned bounding box containing the model.
        //
        //////////////////////////////////////////////////////////////
        NVSDKENTRY void computeBoundingBox( float *minVal_, float *maxVal_);

        //
        //  rescale
        //
        //  rescales object based on bounding box
        //
        //////////////////////////////////////////////////////////////
        NVSDKENTRY void rescale( float radius);

        //
        //  buildTangents
        //
        //    This function computes tangents in the s direction on
        //  the model. It operates on the raw data, so it should only
        //  be used before compiling a model into a HW friendly form.
        //
        //////////////////////////////////////////////////////////////
        NVSDKENTRY void computeTangents();

        //
        //  computeNormals
        //
        //    This function computes vertex normals for a model
        //  which did not have them. It computes them on the raw
        //  data, so it should be done before compiling the model
        //  into a HW friendly format.
        //
        //////////////////////////////////////////////////////////////
        NVSDKENTRY void computeNormals();

        NVSDKENTRY void removeDegeneratePrims();

        //
        //general query functions
        //
        NVSDKENTRY bool hasNormals() const;
        NVSDKENTRY bool hasTexCoords() const;
        NVSDKENTRY bool hasTangents() const;
        NVSDKENTRY bool hasColors() const;

        NVSDKENTRY int getPositionSize() const;
        NVSDKENTRY int getNormalSize() const;
        NVSDKENTRY int getTexCoordSize() const;
        NVSDKENTRY int getTangentSize() const;
        NVSDKENTRY int getColorSize() const;

        //
        //  Functions for the management of raw data
        //
        NVSDKENTRY void clearNormals();
        NVSDKENTRY void clearTexCoords();
        NVSDKENTRY void clearTangents();
        NVSDKENTRY void clearColors();

        //
        //raw data access functions
        //  These are to be used to get the raw array data from the file, each array has its own index
        //
        NVSDKENTRY const float* getCompiledAttributes(int &stride
                        ,int &nOffset
                        ,int &tcOffset
                        ,int &sTanOffset
                        ,int &cOffset
                        ) const;
        NVSDKENTRY const float* getPositions() const;
        NVSDKENTRY const float* getNormals() const;
        NVSDKENTRY const float* getTexCoords() const;
        NVSDKENTRY const float* getTangents() const;
        NVSDKENTRY const float* getColors() const;

        NVSDKENTRY const GLuint* getPositionIndices() const;
        NVSDKENTRY const GLuint* getNormalIndices() const;
        NVSDKENTRY const GLuint* getTexCoordIndices() const;
        NVSDKENTRY const GLuint* getTangentIndices() const;
        NVSDKENTRY const GLuint* getColorIndices() const;

        NVSDKENTRY int getPositionCount() const;
        NVSDKENTRY int getNormalCount() const;
        NVSDKENTRY int getTexCoordCount() const;
        NVSDKENTRY int getTangentCount() const;
        NVSDKENTRY int getColorCount() const;

        NVSDKENTRY int getObjectCount() const;
        NVSDKENTRY int getGroupCount() const;
        NVSDKENTRY int getMaterialCount() const;
        NVSDKENTRY int getSmoothGroupCount() const;

        NVSDKENTRY bool getObject(int n, int *offset, const char** name) const;
        NVSDKENTRY bool getGroup(int n, int *offset, const char** name) const;
        NVSDKENTRY bool getMaterial(int n, int *offset, const char** name) const;
        NVSDKENTRY bool getSmoothGroup(int n, int *offset, bool* mode) const;

        NVSDKENTRY int getIndexCount() const;

        //
        //compiled data access functions
        //
        NVSDKENTRY const float* getCompiledVertices() const;
        NVSDKENTRY const GLuint* getCompiledIndices( PrimType prim = eptTriangles) const;

        NVSDKENTRY int getCompiledPositionOffset() const;
        NVSDKENTRY int getCompiledNormalOffset() const;
        NVSDKENTRY int getCompiledTexCoordOffset() const;
        NVSDKENTRY int getCompiledTangentOffset() const;
        NVSDKENTRY int getCompiledColorOffset() const;

        // returns the size of the merged vertex in # of floats
        NVSDKENTRY int getCompiledVertexSize() const;

        NVSDKENTRY int getCompiledVertexCount() const;
        NVSDKENTRY int getCompiledIndexCount( PrimType prim = eptTriangles) const;

        NVSDKENTRY int getOpenEdgeCount() const;

    protected:

        //Would all this be better done as a channel abstraction to handle more arbitrary data?

        //data structures for model data, not optimized for rendering
        std::vector<float> _positions;
        std::vector<float> _normals;
        std::vector<float> _texCoords;
        std::vector<float> _sTangents;
        std::vector<float> _colors;
        int _posSize;
        int _tcSize;
        int _cSize;

        std::vector<GLuint> _pIndex;
        std::vector<GLuint> _nIndex;
        std::vector<GLuint> _tIndex;
        std::vector<GLuint> _tanIndex;
        std::vector<GLuint> _cIndex;
        // ways to partition indices with material assignment
        struct OffsetAndName {
            OffsetAndName(OffsetAndName &s) : offset(s.offset), name(s.name) {}
            OffsetAndName(GLuint i, std::string s) : offset(i), name(s) {}
            GLuint  offset;
            std::string name;
        };
        std::vector<OffsetAndName> _objects;
        std::vector<OffsetAndName> _groups;
        std::vector<OffsetAndName> _materials;
        struct Smooth {
            Smooth(Smooth &s) : offset(s.offset), mode(s.mode) {}
            Smooth(GLuint i, bool m) : offset(i), mode(m) {}
            GLuint  offset;
            bool    mode;
        };
        std::vector<Smooth> _smoothGroups;

        //data structures optimized for rendering, compiled model
        std::vector<GLuint> _indices[NumPrimTypes];
        std::vector<float> _vertices;
        int _pOffset;
        int _nOffset;
        int _tcOffset;
        int _sTanOffset;
        int _cOffset;
        int _vtxSize;

        int _openEdges;

        //
        // Static elements used to dispatch to proper sub-readers
        //
        //////////////////////////////////////////////////////////////
        struct FormatInfo {
            const char* extension;
            bool (*reader)( const char* file, Model& i);
        };

        static FormatInfo formatTable[]; 

        NVSDKENTRY static bool loadObjFromFile( const char *file, Model &m);
    };
};


#endif