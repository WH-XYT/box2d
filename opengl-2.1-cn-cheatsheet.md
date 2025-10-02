## OpenGL 2.1 速查规范表（中文）

> 说明：本表面向 OpenGL 2.1 核心功能点；着色器为 GLSL 1.20。兼顾常用函数、固定管线要点与限制。若与实现不一致，请以驱动与官方规范为准。

### 上下文与版本
- **版本**: OpenGL 2.1（GLSL 1.20）
- **固定功能管线**: 仍可用（矩阵堆栈、光照、纹理坐标生成等）
- **可编程管线**: 顶点/片段着色器（GLSL 1.20），无几何着色器，无统一缓冲

### 常用能力与限制（典型实现，查询请用 glGet*）
- **最大纹理尺寸**: `GL_MAX_TEXTURE_SIZE`
- **最大纹理单元数**: `GL_MAX_TEXTURE_IMAGE_UNITS`（片段）；`GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS`（顶点，常见为 0）
- **最大顶点属性数**: `GL_MAX_VERTEX_ATTRIBS`
- **Varying 向量数**: `GL_MAX_VARYING_FLOATS / 4`
- **Uniform 数量**: `GL_MAX_VERTEX_UNIFORM_COMPONENTS`，`GL_MAX_FRAGMENT_UNIFORM_COMPONENTS`

### 缓冲与数组
- **顶点数组对象（VAO）**: 不在 2.1 核心（可通过扩展，如 GL_APPLE_vertex_array_object）
- **缓冲对象**: `glGenBuffers/glBindBuffer/glBufferData/glBufferSubData`（`GL_ARRAY_BUFFER`/`GL_ELEMENT_ARRAY_BUFFER`）
- **指针设定（固定管线）**: `glVertexPointer/glNormalPointer/glColorPointer/glTexCoordPointer` + `glEnableClientState`
- **指针设定（可编程）**: `glVertexAttribPointer/glEnableVertexAttribArray`

### 着色器（GLSL 1.20）
- **对象创建**: `glCreateShader/glShaderSource/glCompileShader/glCreateProgram/glAttachShader/glLinkProgram/glUseProgram`
- **变量限定符**: `attribute`（顶点输入），`varying`（顶点->片段），`uniform`（常量）
- **精度限定**: 桌面 GLSL 1.20 无需 `precision` 修饰
- **内置变量（片段）**: `gl_FragColor`、`gl_FragData[i]`（MRT 取决于扩展）、`gl_FragCoord`、`gl_FrontFacing`
- **内置变量（顶点）**: `gl_Position`、可写 `gl_PointSize`
- **纹理函数**: `texture2D/textureCube/texture2DProj/...`（采样器类型与函数需匹配）

### 矩阵与固定功能（2.1 仍保留）
- **矩阵堆栈**: `glMatrixMode(GL_MODELVIEW/GL_PROJECTION/TEXTURE)`，`glLoadIdentity/glLoadMatrix/glMultMatrix`，`glPushMatrix/glPopMatrix`
- **变换**: `glTranslatef/glRotatef/glScalef`，`gluPerspective/glOrtho`（需要 GLU）
- **光照**: `glEnable(GL_LIGHTING)`，`glLightfv`，材质 `glMaterial*`
- **纹理坐标生成**: `glTexGen*`
- 建议：新项目优先使用 GLSL 自管矩阵与光照

### 纹理
- **目标**: `GL_TEXTURE_1D/2D/3D/CUBE_MAP/RECTANGLE`（后者依实现和扩展）
- **创建与数据**: `glTexImage2D/glTexSubImage2D`，`glTexParameteri`（过滤与包裹）
- **Mipmap**: `gluBuild2DMipmaps` 或扩展 `glGenerateMipmapEXT`（核心的 `glGenerateMipmap` 在 3.0+）
- **压缩纹理**: 依扩展（如 S3TC/DXT，需检测）

### 帧缓冲与渲染缓冲
- **FBO**: 非 2.1 核心，使用 `GL_EXT_framebuffer_object`（函数名带 EXT）
- **PBO**: `GL_PIXEL_PACK_BUFFER/GL_PIXEL_UNPACK_BUFFER`（可加速纹理传输与读回）

### 多重采样与混合
- **混合**: `glEnable(GL_BLEND)`；`glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)` 常用
- **Alpha 测试**: `glEnable(GL_ALPHA_TEST)`；`glAlphaFunc`
- **深度测试**: `glEnable(GL_DEPTH_TEST)`；`glDepthFunc`
- **多重采样**: 通过像素格式/窗口系统与扩展（如 `GL_ARB_multisample`）

### 绘制与基本体
- **固定管线立即模式**: `glBegin(GL_TRIANGLES)...glEnd()`（调试/教学用）
- **索引绘制**: `glDrawElements`；**非索引**: `glDrawArrays`
- **点/线宽**: `glPointSize/glLineWidth`（受限于实现）

### 状态查询与错误
- **状态查询**: `glGetBooleanv/glGetIntegerv/glGetFloatv`，字符串 `glGetString`
- **错误检查**: `glGetError`（循环读取直到返回 `GL_NO_ERROR`）

### 常见扩展（2.1 时代常见，需运行时检测）
- `GL_EXT_framebuffer_object`：FBO
- `GL_ARB_pixel_buffer_object`：PBO
- `GL_EXT_blend_equation_separate / GL_EXT_blend_func_separate`
- `GL_ARB_vertex_program / GL_ARB_fragment_program`（固定功能时代可编程前身）
- `GL_EXT_texture_filter_anisotropic`（各向异性过滤）

### 查询上限示例（C/C++）
```c
GLint maxTextureSize = 0;
glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);

GLint maxTexUnitsFrag = 0;
glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTexUnitsFrag);

GLint maxVaryingFloats = 0;
glGetIntegerv(GL_MAX_VARYING_FLOATS, &maxVaryingFloats);
```

### 简单 GLSL 1.20 例子
```glsl
// 顶点着色器
attribute vec3 aPos;
attribute vec3 aNormal;
uniform mat4 uMVP;
varying vec3 vNormal;
void main() {
  vNormal = aNormal;
  gl_Position = uMVP * vec4(aPos, 1.0);
}

// 片段着色器
varying vec3 vNormal;
uniform vec3 uColor;
void main() {
  vec3 n = normalize(vNormal);
  float l = clamp(dot(n, normalize(vec3(0.5, 0.7, 1.0))), 0.0, 1.0);
  gl_FragColor = vec4(uColor * (0.2 + 0.8 * l), 1.0);
}
```

### 术语速览
- **Attribute**: 顶点输入属性
- **Varying**: 顶点传至片段的插值变量
- **Uniform**: 常量参数（跨顶点/片段保持不变）
- **Sampler**: 纹理采样器类型（如 `sampler2D`）

### 参考
- 官方规范（英文，2.1）：`https://www.khronos.org/registry/OpenGL/specs/gl/glspec21.pdf`
- 官方快速参考卡（英文，历史版本页）：`https://www.khronos.org/opengl/wiki/Reference_Cards`
- OpenGL Registry（全部扩展与规范）：`https://www.khronos.org/registry/OpenGL/`

（注：本表为便携速查，非官方翻译；建议结合驱动与实现查询上限与扩展。）

