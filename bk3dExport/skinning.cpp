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

#  pragma warning(disable:4786)
#include <algorithm>
#include <list>

#include <maya/MPxCommand.h>

#include "bk3dExport.h"
#include "MayaHelpers.h"
#include "MiscHelpers.h"

bool sortPair(std::pair<int, float> a, std::pair<int, float> b)
{
	return a.second > b.second;
}
/*----------------------------------------------------------------------------------*/ /**


**/ //----------------------------------------------------------------------------------
MStatus bk3dTranslator::ManageSkinning(MDagPath &dagPath, bk3dlib::PMesh bk3dMesh)
{
    MStatus status;
    AttributeInfo * attrOffsets = findAttrInfo(BonesOffsets, false);
    AttributeInfo * attrWeights = findAttrInfo(BonesWeights, false);
    if((attrOffsets==NULL)||(attrWeights==NULL))
    {
        PF(("No Skinning available\n"));
        return status;
    }
	//PF2(("ManageSkinning\n"));
    MFnDagNode curDagNode(dagPath.node());
    MFnMesh mesh(dagPath.node());

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
    MObject skinclusterobj = getConnectedObjectType(dagPath.node(), "inMesh", MFn::kSkinClusterFilter, true, false, 4, &status);
    MFnSkinCluster skinCluster(skinclusterobj, &status);
    if(!status)
        return status;
    DPF(("====> Found SkinCluster...\n"));
    MDagPathArray influencePaths;
    int numInfluencePaths;
    numInfluencePaths = skinCluster.influenceObjects( influencePaths, &status );
    for(int i=0; i<numInfluencePaths; i++)
    {
        MSTRING name = influencePaths[i].partialPathName().asChar();
        DPF(("Influence %d : %s\n", i, name.asChar()));
        // take the root as a starting point.
        // WHY ? Because for now I need the parent "bk3dlib::PTransform bk3dTransformParent" as argument of readDagPath
        // so that the graph is properly created... TODO: change this ?
        MDagPath dp(influencePaths[i]);
        int l = dp.length() - 1;
        if(l > 0)
            dp.pop(l);
        // Now let's run the recursive walk-through of DAGPath :
        // Note that in the same call, other paths for this skinning could be updated : readDagPath walks through children
        // Note that the paths already processed will lead readDagPath() to return without any work
        status = readDagPath(dp, 0, /*bIntermediate*/true, NULL);
        // obsolete : m_usedTransforms.push_back(influencePaths[i].partialPathName());
    }
    //
    // Buffers for Weights and offsets
    //
    bk3dBufferSkinOffsets = bk3dlib::Buffer::CreateVtxBuffer(MESH_BONESOFFSETS, 4, attrOffsets->slot, bk3dlib::FLOAT32);//UINT32/*TODO: more choice*/);
    bk3dMesh->AttachVtxBuffer(bk3dBufferSkinOffsets);
    bk3dBufferSkinWeights = bk3dlib::Buffer::CreateVtxBuffer(MESH_BONESWEIGHTS, 4, attrWeights->slot, bk3dlib::FLOAT32/*TODO: more choice*/);
    bk3dMesh->AttachVtxBuffer(bk3dBufferSkinWeights);
    // create the same for later SIB (Single Index Buffer) computation
    bk3dBufferSkinOffsets_SIB = bk3dlib::Buffer::CreateVtxBuffer(MESH_BONESOFFSETS, 4, attrOffsets->slot, bk3dlib::FLOAT32);//UINT32/*TODO: more choice*/);
    bk3dMesh->AttachVtxBuffer(bk3dBufferSkinOffsets_SIB);
    bk3dBufferSkinWeights_SIB = bk3dlib::Buffer::CreateVtxBuffer(MESH_BONESWEIGHTS, 4, attrWeights->slot, bk3dlib::FLOAT32/*TODO: more choice*/);
    bk3dMesh->AttachVtxBuffer(bk3dBufferSkinWeights_SIB);
    //
    // array of groups of weights : for each component
    //
    std::vector<std::list<std::pair<int, float> > > weightArrayMap;
    //
    //Iterate through the components to gather weights and offsets
    //
    MItGeometry gIter(dagPath.node(), &status);
    m_numbones = 0;
    DPF(("\n"));
    for (; !gIter.isDone(); gIter.next())
    {
        uint weightCount = 0;
        MDoubleArray weights;
        MObject component = gIter.component(&status);
        status = skinCluster.getWeights(dagPath, component, weights, weightCount);
        // could it be possible that it failed... maybe
        if (status)
        {
            //
            //gather in a map of influence object's offsets the weights, *if relevant*
            //
            std::list<std::pair<int, float> > weightmap;
            float sum=0;
            unsigned wl = weights.length();
            for(unsigned int i=0; i<wl; i++)
            {
                if(weights[i] > 0.051)  // relevant only for more than this value
                {
                    sum += (float)weights[i];
                    std::pair<int,float> v(i,(float)weights[i]);
                    weightmap.push_back(v);
                    if((int)weightmap.size() > m_numbones) 
                        m_numbones = weightmap.size(); //m_numbones is the max # of bones involved for vertices
                }
                /*else
                {
                    DPF(("Skipping weight %d (%f)\n", i, weights[i]))
                }*/
            }
            // Sort the weights according to the offsets so all components have the same order referenced in their weightmap
            weightmap.sort(sortPair);
            weightArrayMap.push_back(weightmap);
            //DPF(("%f | ", sum));
        }
        else // case where nothing relevant for this component : just put an empty group to be consistent
        {
            weightArrayMap.push_back(std::list<std::pair<int, float> >());
        }
    }
    //DPF(("\nSkinning : %d bones for %d vertices\n",m_numbones, weightArrayMap.size()));
    //
    // Now create the serie of offsets/weights made of N x m_numbones in in_attr
    //
    // Set the # of components for this mesh
    // Indeed : for now the mesh will have a fixed amount of influences. Most of the time : 4

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//TODO: make this work for <4 bone !!!!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    bk3dMesh->SetMaxNumBones(4/*m_numbones*/);

    //DPF(("Weight sums\n"));
    unsigned int sz = weightArrayMap.size();
    for(unsigned int i=0; i < sz; i++)
    {
        std::list<std::pair<int, float> >::iterator it = weightArrayMap[i].begin();
        //DPF((">>>> Weights : "))
        float sum = 0;
        float w[4] = {0,0,0,0};
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//TODO: make this work for <4 bone !!!!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
DPF(("Vtx \t%d : ", i));
        for(int j=0; j< 4/*m_numbones*/; j++)
        {
            if(j >= 4) 
            {
                //DPF(("WARNING: %d exceeding 4 weights for the model\n", i));
                break;
            }
            if(it == weightArrayMap[i].end())
            {
                bk3dBufferSkinWeights->AddData(0.0f);
                bk3dBufferSkinOffsets->AddData(0.0f);
            } else {
                if(j >=m_numbones)
                {
                    DPF(("WARNING : bone ref %d is useless... TODO... Fix this and allow <4 bones influence!\n",j)); 
                    bk3dBufferSkinOffsets->AddData(0.0f);
                    bk3dBufferSkinWeights->AddData(0.0f);
                    w[j] = 0.0;
                } else {
                    // Now we need to get the index # from within bk3d library
                    MSTRING name = influencePaths[it->first].partialPathName();
                    int transformID;
                    // IMPORTANT: skinnign offset are computed against the TOTAL list of the whole bk3d file
                    // and not against a Mesh-local list.
                    // TODO?: maybe later, we might want to generate 
                    // - a local list of the skeleton for the mesh
                    // - reference offsets against this local list
                    // this could be usedful if we had a complex transform tree structure
                    // and wanted to avoid pushing all in a big buffer... (but this is questionalble)
                    m_bk3dHeader->GetTransform(name.asChar(), &transformID);
                    assert(transformID >= 0);
                    bk3dBufferSkinOffsets->AddData((float)transformID); // TODO: make it int
                    bk3dBufferSkinWeights->AddData((float)it->second);
DPF(("(%f %f %s) ",(float)it->second, (float)transformID, name.asChar())); 
                    sum += it->second;
                    w[j] = it->second;
                }
                ++it;
            }
        }
DPF(("\n"));
        if(sum < 0.001) { DPF(("sum= %f (%f %f %f %f)\n", sum, w[0], w[1], w[2], w[3])); }
		while(it != weightArrayMap[i].end())
		{
			DPF(("WARNING : skipping weight %d : %f\n",it->first, it->second)); 
			++it;
		}
        //DPF((" - "));
    } // for(unsigned int i=0; i<weightArrayMap.size(); i++)
    // FORCE bones to 4...
    // TODO: we should really find a more flexible solution, here...
    m_numbones = 4;
    DPF(("\n"));
    return status;
}
/*
//
// renormalize after all of the possible bone optomizations
//
void NVMayaMeshConverter::NormalizeWeights()
{
    uint numBones  = m_skinInfo.m_boneWeights.size();
    uint numCoords = m_skinInfo.m_boneWeights[0].size();
    for (uint ci = 0; ci < numCoords; ci++)
    {
        float accum = 0.0f;
        for (uint bi = 0; bi < numBones; bi++)
        {
            accum += m_skinInfo.m_boneWeights[bi][ci];
        }

        float accumInv = 1.0f / accum;
        for (uint bi = 0; bi < numBones; bi++)
        {
            m_skinInfo.m_boneWeights[bi][ci] *= accumInv;
        }
    }
}
*/

