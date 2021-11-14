:: Given a shader name, SHADER, compiles SHADER.vert into SHADER-vert.spv and SHADER.frag into SHADER-frag.spv
glslc.exe %1.vert -o %1-vert.spv
glslc.exe %1.frag -o %1-frag.spv