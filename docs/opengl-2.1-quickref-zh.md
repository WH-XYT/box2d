## OpenGL 2.1 中文速查表（Quick Reference）

> 适用：OpenGL 2.1 Core（含固定功能管线与 GLSL 1.20）

### 基本信息
- **头文件**: `GL/gl.h`, `GL/glext.h`
- **核心数据类型**:
  - `GLboolean`(1B), `GLbyte`(1B), `GLubyte`(1B)
  - `GLshort`(2B), `GLushort`(2B), `GLhalf`(2B)
  - `GLint`(4B), `GLuint`(4B), `GLsizei`(4B), `GLenum`(4B)
  - `GLfloat`(4B), `GLclampf`(4B), `GLdouble`(8B), `GLclampd`(8B)
  - `GLsizeiptr`(指针尺寸), `GLintptr`(指针尺寸)
- **错误码（`glGetError`）**: `GL_NO_ERROR`, `GL_INVALID_ENUM`, `GL_INVALID_VALUE`, `GL_INVALID_OPERATION`, `GL_STACK_OVERFLOW`, `GL_STACK_UNDERFLOW`, `GL_OUT_OF_MEMORY`

### 渲染管线总览
- **固定功能阶段**: 顶点变换 → 光照 → 裁剪 → 光栅化 → 片段操作
- **可编程阶段（GLSL 1.20）**: 顶点着色器（Vertex Shader）与片段着色器（Fragment Shader）

### 上下文与基础状态
- **清屏**: `glClearColor`, `glClearDepth`, `glClearStencil`, `glClear`
- **视口与投影**: `glViewport`, `glDepthRange`
- **启停状态**: `glEnable`, `glDisable`
  - 常用: `GL_DEPTH_TEST`, `GL_BLEND`, `GL_CULL_FACE`, `GL_SCISSOR_TEST`, `GL_ALPHA_TEST`, `GL_STENCIL_TEST`, `GL_TEXTURE_2D`
- **查询**: `glGetBooleanv`, `glGetIntegerv`, `glGetFloatv`, `glGetDoublev`, `glIsEnabled`

### 裁剪/光栅/片段
- **面剔除**: `glCullFace(GL_FRONT|GL_BACK|GL_FRONT_AND_BACK)`, `glFrontFace(GL_CCW|GL_CW)`
- **多边形模式**: `glPolygonMode(GL_FRONT_AND_BACK, GL_FILL|GL_LINE|GL_POINT)`
- **多边形偏移**: `glPolygonOffset`
- **剪裁**: `glScissor`
- **深度测试**: `glEnable(GL_DEPTH_TEST)`, `glDepthFunc(GL_LESS|GL_LEQUAL|...)`, `glDepthMask`
- **模板**: `glStencilFunc`, `glStencilOp`, `glStencilMask`
- **颜色写入遮罩**: `glColorMask`
- **混合**: `glEnable(GL_BLEND)`, `glBlendFunc(src, dst)`, `glBlendEquation`
  - 常用参数: `GL_SRC_ALPHA`, `GL_ONE_MINUS_SRC_ALPHA`, `GL_ONE`, `GL_ZERO`

### 顶点提交（固定功能）
- **立即模式（兼容）**: `glBegin(mode) ... glEnd()`
  - `mode`: `GL_POINTS`, `GL_LINES`, `GL_LINE_STRIP`, `GL_TRIANGLES`, `GL_TRIANGLE_STRIP`, `GL_TRIANGLE_FAN`, `GL_QUADS`, `GL_POLYGON`
  - 顶点属性: `glVertex*`, `glColor*`, `glNormal*`, `glTexCoord*`, `glMultiTexCoord*`, `glFogCoord*`
- **顶点数组**: `glEnableClientState`, `glDisableClientState`
  - 指针设置: `glVertexPointer`, `glColorPointer`, `glNormalPointer`, `glTexCoordPointer`
  - 绘制: `glDrawArrays`, `glDrawElements`

### 顶点缓冲对象（VBO）
- **绑定目标**: `GL_ARRAY_BUFFER`, `GL_ELEMENT_ARRAY_BUFFER`
- **创建与绑定**: `glGenBuffers`, `glBindBuffer`, `glDeleteBuffers`
- **数据上传**: `glBufferData(target, size, data, usage)`, `glBufferSubData`
  - `usage`: `GL_STATIC_DRAW`, `GL_DYNAMIC_DRAW`, `GL_STREAM_DRAW`（及对应 READ/COPY 变体）
- **映射**: `glMapBuffer(target, access)`, `glUnmapBuffer`

### 可编程管线（GLSL 1.20）
- **版本与关键字**: `#version 120`
  - 顶点输入: `attribute`
  - 常量: `uniform`
  - 逐顶点传递: `varying`（顶点 → 片段）
- **对象创建与链接**:
  - `glCreateShader(GL_VERTEX_SHADER|GL_FRAGMENT_SHADER)`
  - `glShaderSource`, `glCompileShader`, `glGetShaderiv`, `glGetShaderInfoLog`
  - `glCreateProgram`, `glAttachShader`, `glBindAttribLocation`, `glLinkProgram`, `glGetProgramiv`, `glGetProgramInfoLog`, `glUseProgram`
  - `glDeleteShader`, `glDeleteProgram`
- **属性与统一变量**:
  - 顶点属性: `glGetAttribLocation`, `glVertexAttribPointer`, `glEnableVertexAttribArray`, `glDisableVertexAttribArray`
  - Uniform: `glGetUniformLocation`, `glUniform1i/1f/2f/3f/4f`, `glUniformMatrix[234]fv`
- **内建变量（1.20）常见**:
  - VS 输出: `gl_Position`
  - VS/FS 插值: 自定义 `varying` 变量
  - FS 输入: `gl_FrontFacing`, `gl_FragCoord`
  - FS 输出: `gl_FragColor` 或 `gl_FragData[i]`（多渲染目标）

```glsl
// 顶点着色器 (GLSL 1.20)
#version 120
attribute vec3 aPosition;
attribute vec3 aNormal;
uniform mat4 uMVP;
varying vec3 vNormal;
void main() {
  vNormal = aNormal;
  gl_Position = uMVP * vec4(aPosition, 1.0);
}

// 片段着色器 (GLSL 1.20)
#version 120
varying vec3 vNormal;
void main() {
  vec3 n = normalize(vNormal);
  float lit = max(n.z, 0.0);
  gl_FragColor = vec4(vec3(lit), 1.0);
}
```

### 纹理
- **单元选择**: `glActiveTexture(GL_TEXTURE0 + i)`
- **对象生命周期**: `glGenTextures`, `glBindTexture`, `glDeleteTextures`
- **目标**: `GL_TEXTURE_1D`, `GL_TEXTURE_2D`, `GL_TEXTURE_3D`, `GL_TEXTURE_CUBE_MAP`
- **图像上传**: `glTexImage1D/2D/3D`, `glTexSubImage*`, `glCopyTexImage*`
- **参数**: `glTexParameteri/iv/fv`
  - 过滤: `GL_TEXTURE_MIN_FILTER`, `GL_TEXTURE_MAG_FILTER` → `GL_NEAREST`, `GL_LINEAR`, `GL_LINEAR_MIPMAP_LINEAR` 等
  - 环绕: `GL_TEXTURE_WRAP_S/T/R` → `GL_REPEAT`, `GL_CLAMP`, `GL_CLAMP_TO_EDGE`
  - 自动生成 mipmap（历史）: `GL_GENERATE_MIPMAP`（注意在较新版本中已弃用）
- **固定功能采样**: `glTexEnv*`（如 `GL_TEXTURE_ENV_MODE` 的 `GL_MODULATE`/`GL_REPLACE` 等）

### 矩阵与固定功能变换/光照（兼容路径）
- **矩阵模式**: `glMatrixMode(GL_MODELVIEW|GL_PROJECTION|GL_TEXTURE)`
- **栈操作**: `glPushMatrix`, `glPopMatrix`, `glLoadIdentity`, `glLoadMatrix*`, `glMultMatrix*`
- **常用变换**: `glTranslate*`, `glRotate*`, `glScale*`, `glFrustum`, `glOrtho`
- **光照**: `glEnable(GL_LIGHTING)`, `glEnable(GL_LIGHT0 + i)`, `glLight*`, `glMaterial*`, `glShadeModel(GL_SMOOTH|GL_FLAT)`

### 绘制调用
- **非索引**: `glDrawArrays(mode, first, count)`
- **索引**: `glDrawElements(mode, count, type, indices)`
- **模式同上**: 点/线/三角/条带/扇形等

### 常用限制与查询（部分）
- 纹理/属性/Uniform 等上限：`glGetIntegerv`
  - `GL_MAX_TEXTURE_SIZE`
  - `GL_MAX_TEXTURE_UNITS`（固定功能）
  - `GL_MAX_TEXTURE_IMAGE_UNITS`（片段着色器）
  - `GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS`（顶点着色器）
  - `GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS`
  - `GL_MAX_VERTEX_ATTRIBS`
  - `GL_MAX_VARYING_FLOATS`
  - `GL_MAX_VERTEX_UNIFORM_COMPONENTS`, `GL_MAX_FRAGMENT_UNIFORM_COMPONENTS`

### 像素传输与对齐
- **打包/解包对齐**: `glPixelStorei(GL_PACK_ALIGNMENT|GL_UNPACK_ALIGNMENT, n)`
- **读回**: `glReadPixels`

### 同步与性能
- **命令提交**: `glFlush`
- **强制完成**: `glFinish`
- **提示**: `glHint(target, mode)`（如 `GL_PERSPECTIVE_CORRECTION_HINT`）

### 调试与版本
- **厂商/渲染器/版本**: `glGetString(GL_VENDOR|GL_RENDERER|GL_VERSION|GL_SHADING_LANGUAGE_VERSION)`
- **扩展**: `glGetString(GL_EXTENSIONS)`（或使用扩展加载库查询）

### 最小工作流示例（VBO + GLSL）
```c
// 创建并填充 VBO
GLuint vbo; glGenBuffers(1, &vbo);
glBindBuffer(GL_ARRAY_BUFFER, vbo);
glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

// 顶点属性布局（位置位于 location 0）
glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const void*)0);

// 使用着色器程序
glUseProgram(program);
glUniformMatrix4fv(uMVP, 1, GL_FALSE, mvp);

// 绘制
glDrawArrays(GL_TRIANGLES, 0, vertexCount);
```

### 提示
- OpenGL 2.1 中 FBO/VAO 多以扩展形式存在（如 `GL_EXT_framebuffer_object`），实际支持取决于驱动。
- `GL_GENERATE_MIPMAP` 在更高版本中被废弃；建议使用工具链或手工生成。
- 2.1 的 GLSL 关键字为 `attribute/uniform/varying`，在 3.0+ 中分别演化为 `in/out` 等新语义。

---
文档定位：`docs/opengl-2.1-quickref-zh.md`

