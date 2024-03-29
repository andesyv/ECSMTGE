#ifndef SHADER_H
#define SHADER_H

#include <QOpenGLFunctions_4_1_Core>
#include "matrix4x4.h"

//#include "GL/glew.h" //We use QOpenGLFunctions instead, so no need for Glew (or GLAD)!

//This class is pretty much a copy of the shader class at
//https://github.com/SonarSystems/Modern-OpenGL-Tutorials/blob/master/%5BLIGHTING%5D/%5B8%5D%20Basic%20Lighting/Shader.h
//which is based on stuff from http://learnopengl.com/ and http://open.gl/.

//must inherit from QOpenGLFunctions_4_1_Core, since we use that instead of glfw/glew/glad

enum class ShaderType
{
    Forward,
    Deferred,
    PostProcessing,
    WeirdStuff,
    Light
};

typedef std::variant<bool, int, float, gsl::vec2, gsl::vec3, gsl::vec4> ShaderParamType;

/** OpenGL shaderprogram. Describes the renderpipeline of a drawcall.
 * A OpenGL shader is a vertex + (geometry +) fragment shader linked into a shaderprogram.
 * @brief OpenGL shaderprogram. Describes the renderpipeline of a drawcall.
 * @see https://www.khronos.org/opengl/wiki/GLSL_Object#Program_objects
 */
class Shader : protected QOpenGLFunctions_4_1_Core
{
public:
    // Constructor generates the shader on the fly
    Shader(const std::string shaderName, ShaderType type);
    Shader(const std::string vertexPath, const std::string fragmentPath, ShaderType type);
    Shader(const std::string vertexPath, const std::string fragmentPath, const std::string geometryPath, ShaderType type);

    virtual ~Shader();

    // Use the current shader
    void use( );

    //Get program number for this shader
    GLuint getProgram() const;

   // virtual void transmitUniformData(gsl::Matrix4x4 *modelMatrix, MaterialClass *material = nullptr);


    std::string mName{};
    ShaderType mRenderingType{};

    std::map<std::string, ShaderParamType> params;

protected:
    GLuint program{0};
    GLint mMatrixUniform{-1};
    GLint vMatrixUniform{-1};
    GLint pMatrixUniform{-1};

private:
    void updateParams(const std::string& path);
    void addParam(const std::string& name, const std::string& type, const std::string& value = "");
};

#endif // SHADER_H

