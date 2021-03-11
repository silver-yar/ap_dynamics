/*
  ==============================================================================

    SliderBarGL.cpp
    Created: 28 Jan 2021 6:42:26pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#include <JuceHeader.h>
#include <fstream>
#include <string>
#include "SliderBarGL.h"

//==============================================================================
SliderBarGL::SliderBarGL(const std::string& filenameNoPath) : shader_(openGLContext_),
                                                              filename_ (filenameNoPath),
                                                              value_ (0.0f),
                                                              vmValue_ (0.0f)
{
    auto now = std::chrono::high_resolution_clock::now();
    startTime = std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch()).count();
//    time_ = juce::Time::getCurrentTime();
    // Sets OpenGL version to 3.2
    openGLContext_.setOpenGLVersionRequired (
            juce::OpenGLContext::OpenGLVersion::openGL3_2);
    // Attach the OpenGL context to SliderBarGL OpenGLRenderer
    openGLContext_.setRenderer (this);
    openGLContext_.attachTo (*this);
}

SliderBarGL::~SliderBarGL()
{
    // Stop & detach OpenGL
    openGLContext_.setContinuousRepainting (false);
    openGLContext_.detach();
}

void SliderBarGL::newOpenGLContextCreated()
{
    // Setup Shaders
    createShaders();

    // Load image for texture
    diffTexture_.loadImage (diffImage_);

    // Load images for cubemap texture
//    rightTex_.loadImage(textureFaces_[0]);
//    leftTex_.loadImage(textureFaces_[1]);
//    topTex_.loadImage(textureFaces_[2]);
//    bottomTex_.loadImage(textureFaces_[3]);
//    backTex_.loadImage(textureFaces_[4]);
//    frontTex_.loadImage(textureFaces_[5]);

//    specTexture_.loadImage (diffImage_);
    openGLContext_.setTextureMagnificationFilter(juce::OpenGLContext::TextureMagnificationFilter::linear);

    // Setup Buffer Objects
    openGLContext_.extensions.glGenBuffers (1, &VBO_); // Vertex Buffer Object
    openGLContext_.extensions.glGenBuffers (1, &EBO_); // Element Buffer Object
}

void SliderBarGL::openGLContextClosing()
{
    // uniforms_.release();
    diffTexture_.release();
//    rightTex_.release();
//    leftTex_.release();
//    topTex_.release();
//    bottomTex_.release();
//    backTex_.release();
//    frontTex_.release();
//    for (auto &texture : cubeMapTextures_)
//    {
//        texture.release();
//    }
//    specTexture_.release();
    shader_.release(); // OpenGLProgram release! not unique_ptr release
}

void SliderBarGL::renderOpenGL()
{
    jassert (juce::OpenGLHelpers::isContextActive());
    
    // Setup Viewport
    const float renderingScale = (float) openGLContext_.getRenderingScale();
    glViewport (0, 0, juce::roundToInt (renderingScale * getWidth()), juce::roundToInt (renderingScale * getHeight()));
    
    // Set background Color
    juce::OpenGLHelpers::clear (juce::Colours::darkgrey);
    
    // Enable Alpha Blending
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable 2D Diffuse Texture
    openGLContext_.extensions.glActiveTexture (GL_TEXTURE1);
    glEnable (GL_TEXTURE_2D);

    if (uniforms_ == nullptr) {
        return;
    }

    if (uniforms_->diffTexture != nullptr)
    {
        diffTexture_.bind();
    }


//    if (uniforms_->specTexture != nullptr)
//    {
//        specTexture_.bind();
//    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    // Use Shader Program that's been defined
    shader_.use();
    
    // Setup the Uniforms for use in the Shader

    if (uniforms_->resolution != nullptr) // resolution
    {
        uniforms_->resolution->set((GLfloat) renderingScale * getWidth(),
                                   (GLfloat) renderingScale * getHeight());
    }

    if (uniforms_->sliderVal != nullptr) // sliderVal
    {
        uniforms_->sliderVal->set((GLfloat) value_);
    }

    if (uniforms_->vmVal != nullptr) // vmVal
    {
        uniforms_->vmVal->set((GLfloat) vmValue_);
    }

    if (uniforms_->diffTexture != nullptr)
    {
        uniforms_->diffTexture->set((GLint) diffTexture_.getTextureID());
    }

    if (uniforms_->specTexture != nullptr)
    {
        uniforms_->specTexture->set((GLint) loadCubeMap(textureFaces_));
    }

    if (uniforms_->runTime != nullptr) {
        auto now = std::chrono::high_resolution_clock::now();
        auto sysTime = std::chrono::duration_cast<std::chrono::nanoseconds>(
                now.time_since_epoch()).count();
        auto elapsed = sysTime - startTime;
        float seconds = elapsed / 1000000000.0;
        uniforms_->runTime->set((GLfloat) seconds );
    }
    
    // Define Vertices for a Square (the view plane)
    GLfloat vertices[] = {
            1.0f,   1.0f, 0.0f, 0.0f,  // Top Right + Tex Coord.
            1.0f,  -1.0f, 1.0f, 0.0f, // Bottom Right + Tex Coord.
            -1.0f, -1.0f, 1.0f, 1.0f, // Bottom Left + Tex Coord.
            -1.0f,  1.0f, 0.0f, 1.0f,  // Top Left + Tex Coord.
    };
    // Define Which Vertex Indexes Make the Square
    GLuint indices[] = {  // Note that we start from 0!
            0, 1, 3,   // First Triangle
            1, 2, 3    // Second Triangle
    };
    
    // Vertex Array Object stuff for later
    //openGLContext_.extensions.glGenVertexArrays(1, &VAO);
    //openGLContext_.extensions.glBindVertexArray(VAO);
    
    // VBO (Vertex Buffer Object) - Bind and Write to Buffer
    openGLContext_.extensions.glBindBuffer (GL_ARRAY_BUFFER, VBO_);
    openGLContext_.extensions.glBufferData (GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
    
    // EBO (Element Buffer Object) - Bind and Write to Buffer
    openGLContext_.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, EBO_);
    openGLContext_.extensions.glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STREAM_DRAW);
    
    // Setup Vertex Attributes
    openGLContext_.extensions.glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    openGLContext_.extensions.glEnableVertexAttribArray (0);

    // Setup Texture Coordinate Attributes
    openGLContext_.extensions.glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
                                                     (GLvoid*) (2 * sizeof(GLfloat)));
    openGLContext_.extensions.glEnableVertexAttribArray (1);
    
    // Draw Vertices
    //glDrawArrays (GL_TRIANGLES, 0, 6); // For just VBO's (Vertex Buffer Objects)
    glDrawElements (GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // For EBO's (Element Buffer Objects) (Indices)
    
    
    
    // Reset the element buffers so child Components draw correctly
    openGLContext_.extensions.glBindBuffer (GL_ARRAY_BUFFER, 0);
    openGLContext_.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
    //openGLContext_.extensions.glBindVertexArray(0);
}

GLint SliderBarGL::loadCubeMap(std::vector<juce::Image> texture_images)
{
    auto texID = specTexture_.getTextureID();
    glActiveTexture (GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texID);

    for (auto i = 0; i < textureFaces_.size(); i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0,
                     GL_RGBA,
                     textureFaces_[i].getWidth(),
                     textureFaces_[i].getHeight(),
                     0,
                     GL_RGBA,
                     GL_UNSIGNED_BYTE,
                     (GLvoid*) textureFaces_[i].getPixelData()
        );
    }

    glEnable (GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return specTexture_.getTextureID();
}

void SliderBarGL::paint (juce::Graphics& g)
{
}

void SliderBarGL::resized()
{
}

void SliderBarGL::createShaders()
{
    if (filename_ == "liquidmetal.shader")
    {
        vertexShader_ =
                "#version 330 core\n"
                "layout (location = 0) in vec2 position;\n"
                "layout (location = 1) in vec2 textureCoordIn;\n"
                "out vec2 textureCoordOut;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    textureCoordOut = textureCoordIn;\n"
                "    gl_Position = vec4(position, 0., 1.);\n"
                "}\n"
                "\n";
        fragmentShader_ =
                "#version 330 core\n"
                "in vec2 textureCoordOut;\n"
                "out vec4 fragColor;\n"
                "uniform vec2 resolution;\n"
                "uniform float sliderValue;\n"
                "uniform float vomValue;\n"
                "uniform sampler2D diffTexture;\n"
                "uniform samplerCube specTexture;\n"
                "uniform float runTime;\n"
                "\n"
                "float signcos(in float v) {\n"
                "    return cos(v)*.5+.5;\n"
                "}\n"
                "float height(in vec2 p) {\n"
                "    vec2 uv = p;\n"
                "    float res = 1.;\n"
                "    for (int i = 0; i < 3; i++) {\n"
                "        res += cos(uv.y*12.345 - runTime *4. + cos(res*12.234)*.2 + cos(uv.x*32.2345 + cos(uv.y*17.234)) ) + cos(uv.x*12.345);\n"
                "        uv = uv.yx;\n"
                "        uv.x += res*.1;\n"
                "    }\n"
                "    return res;\n"
                "}\n"
                "vec2 normal(in vec2 p) {\n"
                "    const vec2 NE = vec2(.1,0.);\n"
                "    return normalize(vec2( height(p+NE)-height(p-NE),\n"
                "    height(p+NE.yx)-height(p-NE.yx) ));\n"
                "}\n"
                "vec3 diffuse(in vec2 p) {\n"
                "\n"
                "    vec2 uv = p;\n"
                "    float res = 1.;\n"
                "    for (int i = 0; i < 3; i++) {\n"
                "        res += cos(uv.y*12.345 - runTime *4. + cos(res*12.234)*.2 + cos(uv.x*32.2345 + cos(uv.y*17.234)) ) + cos(uv.x*12.345);\n"
                "        uv = uv.yx;\n"
                "        uv.x += res*.1;\n"
                "    }\n"
                "\n"
                "    return texture(diffTexture, uv).xyz;\n"
                "}\n"
                "\n"
                "\n"
                "void main()\n"
                "{\n"
                "    vec2 uv = gl_FragCoord.xy / resolution.xy;\n"
                "\n"
                "    vec3 lightDir = normalize(vec3(sin(runTime),1.,cos(runTime)));\n"
                "\n"
                "    vec3 norm3d = normalize(vec3(normal(uv),1.).xzy);\n"
                "    vec3 diff = diffuse(uv);\n"
                "    diff *= .25 + max(0., dot(norm3d, lightDir));\n"
                "    vec3 view = normalize(vec3(uv,-1.).xzy);\n"
                "    vec3 spec = texture(specTexture, reflect(view, norm3d)).xyz *\n"
                "                                            max(0.,dot(-norm3d,view));\n"
                "\n"
                "    if (uv.y < sliderValue)\n"
                "    {\n"
                "        fragColor = vec4(255. / 255., 212. / 255., 121. / 255., 1.);\n"
                "    }\n"
                "    else if (uv.y < vomValue && uv.y > sliderValue)\n"
                "    {\n"
                "        fragColor = vec4(mix(diff,spec,.5), 1.);\n"
                "    }\n"
                "    else\n"
                "    {\n"
                "        fragColor = vec4(1., 1., 1., .0);\n"
                "    }\n"
                "}\n";
    }
    else if (filename_ == "basic.shader")
    {
        vertexShader_ =
                "#version 330 core\n"
                "layout (location = 0) in vec4 position;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    gl_Position = vec4(position.xy, 0., 1.);\n"
                "}\n";
        fragmentShader_ =
                "#version 330 core\n"
                "out vec4 fragColor;\n"
                "uniform vec2 resolution;\n"
                "uniform float sliderValue;\n"
                "uniform float vomValue;\n"
                "uniform sampler2D diffTexture;\n"
                "uniform samplerCube specTexture;\n"
                "uniform float runTime;\n"
                "\n"
                "float signcos(in float v) {\n"
                "    return cos(v)*.5+.5;\n"
                "}\n"
                "float height(in vec2 p) {\n"
                "    vec2 uv = p;\n"
                "    float res = 1.;\n"
                "    for (int i = 0; i < 3; i++) {\n"
                "        res += cos(uv.y*12.345 - runTime *4. + cos(res*12.234)*.2 + cos(uv.x*32.2345 + cos(uv.y*17.234)) ) + cos(uv.x*12.345);\n"
                "        uv = uv.yx;\n"
                "        uv.x += res*.1;\n"
                "    }\n"
                "    return res;\n"
                "}\n"
                "vec2 normal(in vec2 p) {\n"
                "    const vec2 NE = vec2(.1,0.);\n"
                "    return normalize(vec2( height(p+NE)-height(p-NE),\n"
                "    height(p+NE.yx)-height(p-NE.yx) ));\n"
                "}\n"
                "vec3 diffuse(in vec2 p) {\n"
                "\n"
                "    vec2 uv = p;\n"
                "    float res = 1.;\n"
                "    for (int i = 0; i < 3; i++) {\n"
                "        res += cos(uv.y*12.345 - runTime *4. + cos(res*12.234)*.2 + cos(uv.x*32.2345 + cos(uv.y*17.234)) ) + cos(uv.x*12.345);\n"
                "        uv = uv.yx;\n"
                "        uv.x += res*.1;\n"
                "    }\n"
                "\n"
                "    return texture(diffTexture, uv).xyz;\n"
                "}\n"
                "\n"
                "void main()\n"
                "{\n"
                "    vec2 uv = gl_FragCoord.xy / resolution.xy;\n"
                "\n"
                "    vec3 lightDir = normalize(vec3(sin(runTime),1.,cos(runTime)));\n"
                "\n"
                "    vec3 norm3d = normalize(vec3(normal(uv),1.).xzy);\n"
                "    vec3 diff = diffuse(uv);\n"
                "    diff *= .25 + max(0., dot(norm3d, lightDir));\n"
                "    vec3 view = normalize(vec3(uv,-1.).xzy);\n"
                "    vec3 spec = texture(specTexture, reflect(view, norm3d)).xyz *\n"
                "                                            max(0.,dot(-norm3d,view));\n"
                "\n"
                "    if (uv.y > sliderValue)\n"
                "    {\n"
                "        fragColor = vec4(mix(diff,spec,.5), 1.);\n"
                "    }\n"
                "    else\n"
                "    {\n"
                "        fragColor = vec4(255. / 255., 212. / 255., 121. / 255., 1.);\n"
                "    }\n"
                "}\n";
    }
//    std::unique_ptr<juce::OpenGLShaderProgram> shaderProgramAttempt = std::make_unique<juce::OpenGLShaderProgram> (openGLContext_);

    // Retrieve shader from file
//    ShaderProgramSource source = ParseShader("../../../../../../Resources/shaders/" + filename_);
//    DBG("Shader Selected: " + filename_ + '\n');
//    DBG("Vertex Source: \n" << source.VertexSource << '\n');
//    DBG("Fragment Source: \n" << source.FragmentSource << '\n');

    // Sets up pipeline of shaders and compiles the program
    if (shader_.addShader (vertexShader_, GL_VERTEX_SHADER)
        && shader_.addShader (fragmentShader_, GL_FRAGMENT_SHADER)
        && shader_.link())
    {
//        shader_ = std::move (shader_);
        uniforms_ = std::make_unique<Uniforms> (openGLContext_, shader_);

        statusText_ = "GLSL: v" + juce::String (juce::OpenGLShaderProgram::getLanguageVersion(), 2);
    }
    else
    {
        statusText_ = shader_.getLastError();
    }

    //triggerAsyncUpdate();
}
