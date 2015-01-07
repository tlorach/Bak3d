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
from . import export_bk3d


bl_info = {
    "name": "bk3d mesh format (.bk3d)",
    "author": "Tristan Lorach",
    "version": (0, 1),
    "blender": (2, 5, 7),
    "location": "File > Import-Export > bk3d Faces (.bk3d) ",
    "description": "Import-Export bk3d Faces",
    "warning": "",
    "wiki_url": "",
    "tracker_url": "",
    "category": "Import-Export"}

if "bpy" in locals():
    import imp
    if "import_bk3d" in locals():
        imp.reload(import_bk3d)
    if "export_bk3d" in locals():
        imp.reload(export_bk3d)
else:
    import bpy

from bpy.props import StringProperty, BoolProperty
from bpy_extras.io_utils import ExportHelper


class bk3dImporter(bpy.types.Operator):
    """Load bk3d data"""
    bl_idname = "import_mesh.bk3d"
    bl_label = "Import bk3d"
    bl_options = {'UNDO'}

    filepath = StringProperty(
            subtype='FILE_PATH',
            )
    filter_glob = StringProperty(default="*.bk3d", options={'HIDDEN'})

    def execute(self, context):
        from . import import_bk3d
        import_bk3d.read(self.filepath)
        return {'FINISHED'}

    def invoke(self, context, event):
        wm = context.window_manager
        wm.fileselect_add(self)
        return {'RUNNING_MODAL'}


class bk3dExporter(bpy.types.Operator, ExportHelper):
    """Save bk3d data"""
    bl_idname = "export_mesh.bk3d"
    bl_label = "Export bk3d"

    filename_ext = ".bk3d"
    filter_glob = StringProperty(default="*.bk3d", options={'HIDDEN'})

    apply_modifiers = BoolProperty(
            name="Apply Modifiers",
            description="Use transformed mesh data from each object",
            default=True,
            )
    triangulate = BoolProperty(
            name="Triangulate",
            description="Triangulate quads",
            default=True,
            )

    def execute(self, context):
        export_bk3d.process_selected(self.filepath)
#                         ,self.apply_modifiers
#                         ,self.triangulate
#                         )

        return {'FINISHED'}


def menu_import(self, context):
    self.layout.operator(bk3dImporter.bl_idname, text="bk3d Faces (.bk3d)")


def menu_export(self, context):
    self.layout.operator(bk3dExporter.bl_idname, text="bk3d Faces (.bk3d)")


def register():
    bpy.utils.register_module(__name__)

    bpy.types.INFO_MT_file_import.append(menu_import)
    bpy.types.INFO_MT_file_export.append(menu_export)


def unregister():
    bpy.utils.unregister_module(__name__)

    bpy.types.INFO_MT_file_import.remove(menu_import)
    bpy.types.INFO_MT_file_export.remove(menu_export)

if __name__ == "__main__":
    register()
