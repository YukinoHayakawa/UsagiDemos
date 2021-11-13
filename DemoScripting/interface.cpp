// Used for producing an .obj file with the same name so MSBuild doesn't
// complain about failing to link. (interface.hpp is used to generate PCH
// source and won't emit any .obj file but it's treated as a translation unit
// and supposed to be involved in the linking process.)
