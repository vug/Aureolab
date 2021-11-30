:: Given a shader name, SHADER, compiles SHADER.vert into SHADER-vert.spv and SHADER.frag into SHADER-frag.spv
if exist %1.vert (
	glslc.exe %1.vert -o %1-vert.spv
)
if exist %1.frag (
	glslc.exe %1.frag -o %1-frag.spv
)