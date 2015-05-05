#Maya Exporter for bk3d format

This code is available "as-is", without any warranty that it will work correctly for any case.

I just made it available to help whoever wants to export something from Maya or look at the code as an example for another exporter. Better than keeping it for myself.

This exporter if far from being *waterproof*... I do not claim to know Maya plugin SDK so I did what I could as a first draft. In the meantime, I created the bk3dlib, a library that helps to create bk3d files (not needed to read them, though) and then modified the plugin again so that it goes through this library.

##Known Issues

###mel script to me at the right location
It is very much possible that Maya complains due to the fact that it didn't find the script entry point (bk3dExporterOptions)... or didn't even find *bk3dExportOptions.mel*...
Please refer to Maya documentation on how to make sure that it can locate the file containing the mel script for the plugin

A temporary hack: copy the whole text from *bk3dExportOptions.mel* and paste it to the command-line script window of Maya, then "ctrl-enter"...

###failure to load the mll plugin
Maya might also complain that it didn't find bk3dExporter.mll, although you specified the right path...
One of the main reason for why it failed is that *zlib.dll* should be accessible for the bk3dExporter.mll to load. You could simply drop zlib.dll down to the exe folder where Maya.exe is located... or add it to the path env. variable

2 other solutions against this:
* build the project without linking with zlib
* change the project to link with a static zlib version, rather than using the dll

###exporting a Maya scene
For now, this plugin can **only** export selected meshes. If you didn't select any and try to export the whole scene, I don't think it is going to work...

###debug build vs. release build
I recall that I encountered some issues when using a release build. If something went wrong, you could have more chances with a debug build.

###exporting many times
I am not 100% sure that I do clean-up everything once exported. There could still be some garbage preventing a second export to happen correctly.

##exporting

When exporting 1 or more selected meshes, the associated objects that are needed for the mesh to be consistent will be fetched:

 * transformations
 * Skinning info
 * Blendshape info
 * ...

Thus it is not necessary to select the skeleton of a mesh to export the transforms needed for this mesh to work.

When exporting a selected mesh, all the children objects will be exported, too.

###Options for exporting
You will see that the options for exporting require you to explicitly choose how attributes for the vertices will be organized in memory. Please have a look at the code for this: 
 * Slots: are grouping attributes together in an interleaved way
 * Attribute: typically the kind of data for a vertex we want to export: position, normals, blendshape weights, tangent, binormal, color...


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
