/*
  ==============================================================================

    SliderBarGL.cpp
    Created: 28 Jan 2021 6:42:26pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#include "SliderBarGL.h"

#include <JuceHeader.h>
#include <juce_opengl/opengl/juce_gl.h>

#include <fstream>
#include <string>

//==============================================================================
SliderBarGL::SliderBarGL(const std::string& filenameNoPath)
    : shader_(openGLContext_), filename_(filenameNoPath), value_(0.0f), vmValue_(0.0f)
{
  auto now  = std::chrono::high_resolution_clock::now();
  startTime = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
  //    time_ = juce::Time::getCurrentTime();
  // Sets OpenGL version to 3.2
  openGLContext_.setOpenGLVersionRequired(juce::OpenGLContext::OpenGLVersion::openGL3_2);
  // Attach the OpenGL context to SliderBarGL OpenGLRenderer
  openGLContext_.setRenderer(this);
  openGLContext_.attachTo(*this);
}

SliderBarGL::~SliderBarGL()
{
  // Stop & detach OpenGL
  openGLContext_.setContinuousRepainting(false);
  openGLContext_.detach();
}

void SliderBarGL::newOpenGLContextCreated()
{
  // Setup Shaders
  createShaders();

  // Load image for texture
  diffTexture_.loadImage(diffImage_);

  openGLContext_.setTextureMagnificationFilter(juce::OpenGLContext::TextureMagnificationFilter::linear);
  //  loadCubeMap(textureFaces_);

  // Setup Buffer Objects
  juce::OpenGLExtensionFunctions::glGenBuffers(1, &VBO_);  // Vertex Buffer Object
  juce::OpenGLExtensionFunctions::glGenBuffers(1, &EBO_);  // Element Buffer Object
}

void SliderBarGL::openGLContextClosing()
{
  diffTexture_.release();
  shader_.release();  // OpenGLProgram release! not unique_ptr release
}

void SliderBarGL::renderOpenGL()
{
  jassert(juce::OpenGLHelpers::isContextActive());

  // Setup Viewport
  const auto renderingScale = static_cast<float>(openGLContext_.getRenderingScale());
  juce::gl::glViewport(0, 0, juce::roundToInt(renderingScale * getWidth()), juce::roundToInt(renderingScale * getHeight()));

  // Set background Color
  juce::OpenGLHelpers::clear(juce::Colours::darkgrey);

  // Enable Alpha Blending
  juce::gl::glEnable(juce::gl::GL_BLEND);
  juce::gl::glBlendFunc(juce::gl::GL_SRC_ALPHA, juce::gl::GL_ONE_MINUS_SRC_ALPHA);

  // Enable 2D Diffuse Texture
  juce::OpenGLExtensionFunctions::glActiveTexture(juce::gl::GL_TEXTURE1);
  juce::gl::glEnable(juce::gl::GL_TEXTURE_2D);

  if (uniforms_ == nullptr)
  {
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

  juce::gl::glTexParameteri(juce::gl::GL_TEXTURE_2D, juce::gl::GL_TEXTURE_WRAP_S, juce::gl::GL_REPEAT);
  juce::gl::glTexParameteri(juce::gl::GL_TEXTURE_2D, juce::gl::GL_TEXTURE_WRAP_T, juce::gl::GL_REPEAT);

  // Use Shader Program that's been defined
  shader_.use();

  // Setup the Uniforms for use in the Shader

  if (uniforms_->resolution != nullptr)  // resolution
  {
    uniforms_->resolution->set((GLfloat)renderingScale * getWidth(), (GLfloat)renderingScale * getHeight());
  }

  if (uniforms_->sliderVal != nullptr)  // sliderVal
  {
    uniforms_->sliderVal->set((GLfloat)value_);
  }

  if (uniforms_->vmVal != nullptr)  // vmVal
  {
    uniforms_->vmVal->set((GLfloat)vmValue_);
  }

  if (uniforms_->diffTexture != nullptr)
  {
    uniforms_->diffTexture->set((GLint)diffTexture_.getTextureID());
  }

  if (uniforms_->specTexture != nullptr)
  {
    uniforms_->specTexture->set((GLint)specTexture_.getTextureID());
  }

  if (uniforms_->runTime != nullptr)
  {
    auto now     = std::chrono::high_resolution_clock::now();
    auto sysTime = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
    auto elapsed = static_cast<double>(sysTime - startTime);
    auto seconds = static_cast<float>(elapsed / 1000000000.0);
    uniforms_->runTime->set((GLfloat)seconds);
  }

  // Define Vertices for a Square (the view plane)
  GLfloat vertices[] = {
    1.0f,  1.0f,  0.0f, 0.0f,  // Top Right + Tex Coord.
    1.0f,  -1.0f, 1.0f, 0.0f,  // Bottom Right + Tex Coord.
    -1.0f, -1.0f, 1.0f, 1.0f,  // Bottom Left + Tex Coord.
    -1.0f, 1.0f,  0.0f, 1.0f,  // Top Left + Tex Coord.
  };
  // Define Which Vertex Indexes Make the Square
  GLuint indices[] = {
    // Note that we start from 0!
    0, 1, 3,  // First Triangle
    1, 2, 3   // Second Triangle
  };

  // Vertex Array Object stuff for later
  // openGLContext_.extensions.glGenVertexArrays(1, &VAO);
  // openGLContext_.extensions.glBindVertexArray(VAO);

  // VBO (Vertex Buffer Object) - Bind and Write to Buffer
  juce::OpenGLExtensionFunctions::glBindBuffer(juce::gl::GL_ARRAY_BUFFER, VBO_);
  juce::OpenGLExtensionFunctions::glBufferData(juce::gl::GL_ARRAY_BUFFER, sizeof(vertices), vertices,
                                               juce::gl::GL_STREAM_DRAW);

  // EBO (Element Buffer Object) - Bind and Write to Buffer
  juce::OpenGLExtensionFunctions::glBindBuffer(juce::gl::GL_ELEMENT_ARRAY_BUFFER, EBO_);
  juce::OpenGLExtensionFunctions::glBufferData(juce::gl::GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
                                               juce::gl::GL_STREAM_DRAW);

  // Setup Vertex Attributes
  juce::OpenGLExtensionFunctions::glVertexAttribPointer(0, 2, juce::gl::GL_FLOAT, juce::gl::GL_FALSE, 4 * sizeof(GLfloat),
                                                        (GLvoid*)nullptr);
  juce::OpenGLExtensionFunctions::glEnableVertexAttribArray(0);

  // Setup Texture Coordinate Attributes
  juce::OpenGLExtensionFunctions::glVertexAttribPointer(1, 2, juce::gl::GL_FLOAT, juce::gl::GL_FALSE, 4 * sizeof(GLfloat),
                                                        (GLvoid*)(2 * sizeof(GLfloat)));
  juce::OpenGLExtensionFunctions::glEnableVertexAttribArray(1);

  // Draw Vertices
  // glDrawArrays (GL_TRIANGLES, 0, 6); // For just VBO's (Vertex Buffer Objects)
  juce::gl::glDrawElements(juce::gl::GL_TRIANGLES, 6, juce::gl::GL_UNSIGNED_INT,
                           nullptr);  // For EBO's (Element Buffer Objects) (Indices)

  // Reset the element buffers so child Components draw correctly
  juce::OpenGLExtensionFunctions::glBindBuffer(juce::gl::GL_ARRAY_BUFFER, 0);
  juce::OpenGLExtensionFunctions::glBindBuffer(juce::gl::GL_ELEMENT_ARRAY_BUFFER, 0);
  // openGLContext_.extensions.glBindVertexArray(0);
}

void SliderBarGL::paint(juce::Graphics& g) { ignoreUnused(g); }

void SliderBarGL::resized() { }

void SliderBarGL::createShaders()
{
  if (filename_ == "liquidmetal.shader")
  {
    vertexShader_ =
        R"(
        #version 330 core
        layout (location = 0) in vec2 position;
        layout (location = 1) in vec2 textureCoordIn;
        out vec2 textureCoordOut;
        
        void main()
        {
            textureCoordOut = textureCoordIn;
            gl_Position = vec4(position, 0., 1.);
        }
        )";
    fragmentShader_ =
        R"(
        #version 330 core
        in vec2 textureCoordOut;
        out vec4 fragColor;
        uniform vec2 resolution;
        uniform float sliderValue;
        uniform float vomValue;
        uniform sampler2D diffTexture;
        uniform samplerCube specTexture;
        uniform float runTime;
        
        float signcos(in float v) {
            return cos(v)*.5+.5;
        }
        float height(in vec2 p) {
            vec2 uv = p;
            float res = 1.;
            for (int i = 0; i < 3; i++) {
                res += cos(uv.y*12.345 - runTime * 3. + cos(res*12.234)*.2 + cos(uv.x*32.2345 + cos(uv.y*17.234)) ) + 
        cos(uv.x*12.345);
                uv = uv.yx;
                uv.x += res*.1;
            }
            return res;
        }
        vec2 normal(in vec2 p) {
            const vec2 NE = vec2(.1,0.);
            return normalize(vec2( height(p+NE)-height(p-NE),
            height(p+NE.yx)-height(p-NE.yx) ));
        }
        vec3 diffuse(in vec2 p) {
        
            vec2 uv = p;
            float res = 1.;
            for (int i = 0; i < 3; i++) {
                res += cos(uv.y*12.345 - runTime *4. + cos(res*12.234)*.2 + cos(uv.x*32.2345 + cos(uv.y*17.234)) ) + 
        cos(uv.x*12.345);
                uv = uv.yx;
                uv.x += res*.1;
            }
        
            return texture(diffTexture, uv).xyz;
        }
        
        
        void main()
        {
            vec2 uv = gl_FragCoord.xy / resolution.xy;
        
            vec3 lightDir = normalize(vec3(sin(runTime),1.,cos(runTime)));
        
            vec3 norm3d = normalize(vec3(normal(uv),1.).xzy);
            vec3 diff = diffuse(uv);
            diff *= .25 + max(0., dot(norm3d, lightDir));
            vec3 view = normalize(vec3(uv,-1.).xzy);
            vec3 spec = vec3(.8, .8, .8);
        
            if (uv.y < sliderValue)
            {
                fragColor = vec4(255. / 255., 212. / 255., 121. / 255., 1.);
            }
            else if (uv.y < vomValue && uv.y > sliderValue)
            {
                fragColor = vec4(mix(diff,spec,.5), 1.);
            }
            else
            {
                fragColor = vec4(1., 1., 1., .0);
            }
        })";
  }
  else if (filename_ == "basic.shader")
  {
    vertexShader_ =
        R"(
        #version 330 core
        layout (location = 0) in vec4 position;
        
        void main()
        {
            gl_Position = vec4(position.xy, 0., 1.);
        })";
    fragmentShader_ =
        R"(
        #version 330 core
        out vec4 fragColor;
        uniform vec2 resolution;
        uniform float sliderValue;
        uniform float vomValue;
        uniform sampler2D diffTexture;
        uniform samplerCube specTexture;
        uniform float runTime;
        
        float signcos(in float v) {
            return cos(v)*.5+.5;
        }
        float height(in vec2 p) {
            vec2 uv = p;
            float res = 1.;
            for (int i = 0; i < 3; i++) {
                res += cos(uv.y*12.345 - runTime *4. + cos(res*12.234)*.2 + cos(uv.x*32.2345 + cos(uv.y*17.234)) ) + 
        cos(uv.x*12.345);
                uv = uv.yx;
                uv.x += res*.1;
            }
            return res;
        }
        vec2 normal(in vec2 p) {
            const vec2 NE = vec2(.1,0.);
            return normalize(vec2( height(p+NE)-height(p-NE),
            height(p+NE.yx)-height(p-NE.yx) ));
        }
        vec3 diffuse(in vec2 p) {
        
            vec2 uv = p;
            float res = 1.;
            for (int i = 0; i < 3; i++) {
                res += cos(uv.y*12.345 - runTime *3. + cos(res*12.234)*.2 + cos(uv.x*32.2345 + cos(uv.y*17.234)) ) + 
        cos(uv.x*12.345);
                uv = uv.yx;
                uv.x += res*.1;
            }
        
            return texture(diffTexture, uv).xyz;
        }
        
        void main()
        {
            vec2 uv = gl_FragCoord.xy / resolution.xy;
        
            vec3 lightDir = normalize(vec3(sin(runTime),1.,cos(runTime)));
        
            vec3 norm3d = normalize(vec3(normal(uv),1.).xzy);
            vec3 diff = diffuse(uv);
            diff *= .25 + max(0., dot(norm3d, lightDir));
            vec3 view = normalize(vec3(uv,-1.).xzy);
            vec3 spec = vec3(.8, .8, .8);
        
            if (uv.y > sliderValue)
            {
                fragColor = vec4(mix(diff,spec,.5), 1.);
            }
            else
            {
                fragColor = vec4(255. / 255., 212. / 255., 121. / 255., 1.);
            }
        }
        )";
  }

  // Sets up pipeline of shaders and compiles the program
  if (shader_.addShader(vertexShader_, juce::gl::GL_VERTEX_SHADER) &&
      shader_.addShader(fragmentShader_, juce::gl::GL_FRAGMENT_SHADER) && shader_.link())
  {
    uniforms_ = std::make_unique<Uniforms>(openGLContext_, shader_);

    statusText_ = "GLSL: v" + juce::String(juce::OpenGLShaderProgram::getLanguageVersion(), 2);
  }
  else
  {
    statusText_ = shader_.getLastError();
  }
}
