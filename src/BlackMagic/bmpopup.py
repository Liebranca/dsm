import bpy

from bpy.types import Operator
from bpy.props import BoolProperty
from bpy.utils import register_class, unregister_class

from .bmtypes import DA_MaterialFlags

#   ---     ---     ---     ---     ---

class DA_materialFlagSetter(Operator):

    bl_idname      = "blackmagic.setmateflags";
    bl_label       = "Set DarkAge material flags";

    bl_description = "Set flags for this material. Doesn't affect selected SIN shader";

    specular       = BoolProperty(name = "Specular",   default = 1, description = "Enables specular light"              );
    opaque         = BoolProperty(name = "Opaque",     default = 1, description = "Disables transparency"               );
    reflective     = BoolProperty(name = "Reflective", default = 0, description = "Enables spheremap faux-reflections"  );
    metallic       = BoolProperty(name = "Metallic",   default = 0, description = "Enables metalness shading"           );
    radiance       = BoolProperty(name = "Radiance",   default = 0, description = "Enables glow effect"                 );
    animated       = BoolProperty(name = "Animated",   default = 0, description = "Texture is read as a spritesheet"    );
    sprite         = BoolProperty(name = "Sprite",     default = 0, description = "Enables cam-alignment for faces"     );
    nonmat         = BoolProperty(name = "NonMat",     default = 0, description = "Disables material creation"          );

    mate           = None;
    valid          = 1;

    def __init__(self):

        self.mate = bpy.context.scene.BlackMagic.curmat;

        if not self.mate:
            self.report({'ERROR'}, "No material selected in BlackMagic panel!");
            self.valid = 0; return;

        self.specular   = (self.mate.BlackMagic.flags & DA_MaterialFlags["Specular"  ]) != 0;
        self.opaque     = (self.mate.BlackMagic.flags & DA_MaterialFlags["Opaque"    ]) != 0;
        self.reflective = (self.mate.BlackMagic.flags & DA_MaterialFlags["Reflective"]) != 0;
        self.metallic   = (self.mate.BlackMagic.flags & DA_MaterialFlags["Metallic"  ]) != 0;
        self.radiance   = (self.mate.BlackMagic.flags & DA_MaterialFlags["Radiance"  ]) != 0;
        self.animated   = (self.mate.BlackMagic.flags & DA_MaterialFlags["Animated"  ]) != 0;
        self.sprite     = (self.mate.BlackMagic.flags & DA_MaterialFlags["Sprite"    ]) != 0;
        self.nonmat     = (self.mate.BlackMagic.flags & DA_MaterialFlags["NonMat"    ]) != 0;

        super().__init__();

#   ---     ---     ---     ---     ---

    def execute(self, context):

        self.mate.BlackMagic.flags = ( (self.specular   * DA_MaterialFlags["Specular"  ])
                                   |   (self.opaque     * DA_MaterialFlags["Opaque"    ])
                                   |   (self.reflective * DA_MaterialFlags["Reflective"])
                                   |   (self.metallic   * DA_MaterialFlags["Metallic"  ])
                                   |   (self.radiance   * DA_MaterialFlags["Radiance"  ])
                                   |   (self.animated   * DA_MaterialFlags["Animated"  ])
                                   |   (self.sprite     * DA_MaterialFlags["Sprite"    ])
                                   |   (self.nonmat     * DA_MaterialFlags["NonMat"    ]) );

        self.report({'INFO'}, "Flags set for material %s"%self.mate.name);

        return {'FINISHED'}

    def invoke(self, context, event):

        if not self.valid: return {'CANCELLED'};

        wm = context.window_manager;
        return wm.invoke_props_dialog(self);

#   ---     ---     ---     ---     ---

def register():

    register_class(DA_materialFlagSetter);

def unregister():

    unregister_class(DA_materialFlagSetter);

#   ---     ---     ---     ---     ---
