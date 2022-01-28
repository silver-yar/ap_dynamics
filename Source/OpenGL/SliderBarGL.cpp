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

#include <string>
#include <utility>

#include "../../Helpers/APDefines.h"

//==============================================================================
SliderBarGL::SliderBarGL(std::string filenameNoPath)
    : shader_(openGLContext_), filename_(std::move(filenameNoPath)), value_(0.0f), vmValue_(0.0f)
{
  const auto now = std::chrono::high_resolution_clock::now();
  startTime_     = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
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
  juce::gl::glViewport(0, 0, juce::roundToInt(renderingScale * static_cast<float>(getWidth())),
                       juce::roundToInt(renderingScale * static_cast<float>(getHeight())));

  // Set background Color
  juce::OpenGLHelpers::clear(APConstants::Colors::DARK_GREY);

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

  juce::gl::glTexParameteri(juce::gl::GL_TEXTURE_2D, juce::gl::GL_TEXTURE_WRAP_S, juce::gl::GL_REPEAT);
  juce::gl::glTexParameteri(juce::gl::GL_TEXTURE_2D, juce::gl::GL_TEXTURE_WRAP_T, juce::gl::GL_REPEAT);

  // Use Shader Program that's been defined
  shader_.use();

  // Set up the Uniforms for use in the Shader

  if (uniforms_->resolution != nullptr)  // resolution
  {
    uniforms_->resolution->set(static_cast<float>(renderingScale) * static_cast<float>(getWidth()),
                               static_cast<float>(renderingScale) * static_cast<float>(getHeight()));
  }

  if (uniforms_->sliderVal != nullptr)  // sliderVal
  {
    uniforms_->sliderVal->set(static_cast<float>(value_));
  }

  if (uniforms_->vmVal != nullptr)  // vmVal
  {
    uniforms_->vmVal->set(static_cast<float>(vmValue_));
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
    const auto now     = std::chrono::high_resolution_clock::now();
    const auto sysTime = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
    const auto elapsed = static_cast<double>(sysTime - startTime_);
    const auto seconds = static_cast<float>(elapsed / 1000000000.0);
    uniforms_->runTime->set(static_cast<float>(seconds));
  }

  // Define Vertices for a Square (the view plane)
  constexpr GLfloat vertices[] = {
    1.0f,  1.0f,  0.0f, 0.0f,  // Top Right + Tex Coord.
    1.0f,  -1.0f, 1.0f, 0.0f,  // Bottom Right + Tex Coord.
    -1.0f, -1.0f, 1.0f, 1.0f,  // Bottom Left + Tex Coord.
    -1.0f, 1.0f,  0.0f, 1.0f,  // Top Left + Tex Coord.
  };
  // Define Which Vertex Indexes Make the Square
  constexpr GLuint indices[] = {
    // Note that we start from 0!
    0, 1, 3,  // First Triangle
    1, 2, 3   // Second Triangle
  };

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
  juce::gl::glDrawElements(juce::gl::GL_TRIANGLES, 6, juce::gl::GL_UNSIGNED_INT,
                           nullptr);  // For EBO's (Element Buffer Objects) (Indices)

  // Reset the element buffers so child Components draw correctly
  juce::OpenGLExtensionFunctions::glBindBuffer(juce::gl::GL_ARRAY_BUFFER, 0);
  juce::OpenGLExtensionFunctions::glBindBuffer(juce::gl::GL_ELEMENT_ARRAY_BUFFER, 0);
}

void SliderBarGL::paint(juce::Graphics& g) { ignoreUnused(g); }

void SliderBarGL::resized() { }

void SliderBarGL::createShaders()
{
  if (filename_ == "liquidmetal.shader")
  // Licence CC0: Liquid Metal
  // https://www.shadertoy.com/view/7tyXDw
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
        #define PI  3.141592654
        #define TAU (2.0*PI)

        uniform float runTime;
        uniform vec2 resolution;
        uniform float sliderValue;
        uniform float vomValue;
        out vec4 fragColor;

        void rot(inout vec2 p, float a) {
          float c = cos(a);
          float s = sin(a);
          p = vec2(c*p.x + s*p.y, -s*p.x + c*p.y);
        }

        float hash(in vec2 co) {
          return fract(sin(dot(co.xy ,vec2(12.9898,58.233))) * 13758.5453);
        }

        vec2 hash2(vec2 p) {
          p = vec2(dot(p,vec2(127.1,311.7)), dot(p,vec2(269.5,183.3)));
          return fract(sin(p)*18.5453);
        }

        float psin(float a) {
          return 0.5 + 0.5*sin(a);
        }

        float tanh_approx(float x) {
          float x2 = x*x;
          return clamp(x*(27.0 + x2)/(27.0+9.0*x2), -1.0, 1.0);
        }

        float onoise(vec2 x) {
          x *= 0.5;
          float a = sin(x.x);
          float b = sin(x.y);
          float c = mix(a, b, psin(TAU*tanh_approx(a*b+a+b)));

          return c;
        }

        float vnoise(vec2 x) {
          vec2 i = floor(x);
          vec2 w = fract(x);

          #if 1
          // quintic interpolation
          vec2 u = w*w*w*(w*(w*6.0-15.0)+10.0);
          #else
          // cubic interpolation
          vec2 u = w*w*(3.0-2.0*w);
          #endif

          float a = hash(i+vec2(0.0,0.0));
          float b = hash(i+vec2(1.0,0.0));
          float c = hash(i+vec2(0.0,1.0));
          float d = hash(i+vec2(1.0,1.0));

          float k0 =   a;
          float k1 =   b - a;
          float k2 =   c - a;
          float k3 =   d - c + a - b;

          float aa = mix(a, b, u.x);
          float bb = mix(c, d, u.x);
          float cc = mix(aa, bb, u.y);

          return k0 + k1*u.x + k2*u.y + k3*u.x*u.y;
        }

        float fbm1(vec2 p) {
          vec2 op = p;
          const float aa = 0.45;
          const float pp = 2.03;
          const vec2 oo = -vec2(1.23, 1.5);
          const float rr = 1.2;

          float h = 0.0;
          float d = 0.0;
          float a = 1.0;

          for (int i = 0; i < 5; ++i) {
          h += a*onoise(p);
          d += (a);
          a *= aa;
          p += oo;
          p *= pp;
          rot(p, rr);
        }

        return mix((h/d), -0.5*(h/d), pow(vnoise(0.9*op), 0.25));
        }

        float fbm2(vec2 p) {
          vec2 op = p;
          const float aa = 0.45;
          const float pp = 2.03;
          const vec2 oo = -vec2(1.23, 1.5);
          const float rr = 1.2;

          float h = 0.0;
          float d = 0.0;
          float a = 1.0;

          for (int i = 0; i < 7; ++i) {
          h += a*onoise(p);
          d += (a);
          a *= aa;
          p += oo;
          p *= pp;
          rot(p, rr);
        }

        return mix((h/d), -0.5*(h/d), pow(vnoise(0.9*op), 0.25));
        }

        float fbm3(vec2 p) {
          vec2 op = p;
          const float aa = 0.45;
          const float pp = 2.03;
          const vec2 oo = -vec2(1.23, 1.5);
          const float rr = 1.2;

          float h = 0.0;
          float d = 0.0;
          float a = 1.0;

          for (int i = 0; i < 3; ++i) {
          h += a*onoise(p);
          d += (a);
          a *= aa;
          p += oo;
          p *= pp;
          rot(p, rr);
        }

        return mix((h/d), -0.5*(h/d), pow(vnoise(0.9*op), 0.25));
        }

        // controls speed
        float warp(vec2 p) {
          vec2 v = vec2(fbm1(p), fbm1(p+0.7*vec2(1.0, 1.0)));

          rot(v, 1.0+runTime*0.2);

          vec2 vv = vec2(fbm2(p + 3.7*v), fbm2(p + -2.7*v.yx+0.7*vec2(1.0, 1.0)));

          rot(vv, -1.0+runTime*1.8);

          return fbm3(p + 9.0*vv);
        }

        float height(vec2 p) {
          float a = 0.045*runTime;
          p += 9.0*vec2(cos(a), sin(a));
          p *= 2.0;
          p += 13.0;
          float h = warp(p);
          float rs = 3.0;
          return 0.35*tanh_approx(rs*h)/rs;
        }

        vec3 normal(vec2 p) {
          // As suggested by IQ, thanks!
          vec2 eps = -vec2(2.0/resolution.y, 0.0);

          vec3 n;

          n.x = height(p + eps.xy) - height(p - eps.xy);
          n.y = 2.0*eps.x;
          n.z = height(p + eps.yx) - height(p - eps.yx);


          return normalize(n);
        }

        vec3 postProcess(vec3 col, vec2 q)  {
          col=pow(clamp(col,0.0,1.0),vec3(0.75));
          col=col*0.6+0.4*col*col*(3.0-2.0*col);  // contrast
          col=mix(col, vec3(dot(col, vec3(0.33))), -0.4);  // satuation
          col*=0.5+0.5*pow(19.0*q.x*q.y*(1.0-q.x)*(1.0-q.y),0.7);  // vigneting
          return col;
        }

        void main() {
          vec2 q = gl_FragCoord.xy/resolution.xy;
          vec2 p = -1. + 2. * q;
          p.x*=resolution.x/resolution.y;
          //lights positions
          const vec3 lp1 = vec3(2.1, -0.5, -0.1);
          const vec3 lp2 = vec3(-2.1, -0.5, -0.1);

          float h = height(p);
          vec3 pp = vec3(p.x, h, p.y);
          float ll1 = length(lp1.xz - pp.xz);
          vec3 ld1 = normalize(lp1 - pp);
          vec3 ld2 = normalize(lp2 - pp);

          vec3 n = normal(p);
          float diff1 = max(dot(ld1, n), 0.0);
          float diff2 = max(dot(ld2, n), 0.0);
          //lights colors
          // color read b,g,r for some reason?
          vec3 baseCol1 = vec3(181. / 255., 181. / 255., 181. / 255.);
          vec3 baseCol2 = vec3(121. / 255., 212. / 255., 255. / 255.);

          float oh = height(p + ll1*0.05*normalize(ld1.xz));
          const float level0 = 0.0;
          const float level1 = 0.125;
          // VERY VERY fake shadows + hilight
          vec3 scol1 = baseCol1*(smoothstep(level0, level1, h) - smoothstep(level0, level1, oh));
          vec3 scol2 = baseCol2*(smoothstep(level0, level1, h) - smoothstep(level0, level1, oh));
          // specular and diffuse strenght
          vec3 col = vec3(0.0);
          col += 0.55*baseCol1.zyx*pow(diff1, 1.0);
          col += 0.55*baseCol1.zyx*pow(diff1, 1.0);
          col += 0.55*baseCol2.zyx*pow(diff2, 1.0);
          col += 0.55*baseCol2.zyx*pow(diff2, 1.0);
          col += scol1*0.5;
          col += scol2*0.5;

          // fragColor = vec4(col, 1.0);
          if (q.y < sliderValue)
            {
                fragColor = vec4(255. / 255., 212. / 255., 121. / 255., 1.);
            }
            else if (q.y < vomValue && q.y > sliderValue)
            {
                fragColor = vec4(col, 1.0);
            }
            else
            {
                fragColor = vec4(1., 1., 1., .0);
            }
        }
        )";
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
        #define PI  3.141592654
        #define TAU (2.0*PI)

        uniform float runTime;
        uniform vec2 resolution;
        uniform float sliderValue;
        out vec4 fragColor;

        void rot(inout vec2 p, float a) {
          float c = cos(a);
          float s = sin(a);
          p = vec2(c*p.x + s*p.y, -s*p.x + c*p.y);
        }

        float hash(in vec2 co) {
          return fract(sin(dot(co.xy ,vec2(12.9898,58.233))) * 13758.5453);
        }

        vec2 hash2(vec2 p) {
          p = vec2(dot(p,vec2(127.1,311.7)), dot(p,vec2(269.5,183.3)));
          return fract(sin(p)*18.5453);
        }

        float psin(float a) {
          return 0.5 + 0.5*sin(a);
        }

        float tanh_approx(float x) {
          float x2 = x*x;
          return clamp(x*(27.0 + x2)/(27.0+9.0*x2), -1.0, 1.0);
        }

        float onoise(vec2 x) {
          x *= 0.5;
          float a = sin(x.x);
          float b = sin(x.y);
          float c = mix(a, b, psin(TAU*tanh_approx(a*b+a+b)));

          return c;
        }

        float vnoise(vec2 x) {
          vec2 i = floor(x);
          vec2 w = fract(x);

          #if 1
          // quintic interpolation
          vec2 u = w*w*w*(w*(w*6.0-15.0)+10.0);
          #else
          // cubic interpolation
          vec2 u = w*w*(3.0-2.0*w);
          #endif

          float a = hash(i+vec2(0.0,0.0));
          float b = hash(i+vec2(1.0,0.0));
          float c = hash(i+vec2(0.0,1.0));
          float d = hash(i+vec2(1.0,1.0));

          float k0 =   a;
          float k1 =   b - a;
          float k2 =   c - a;
          float k3 =   d - c + a - b;

          float aa = mix(a, b, u.x);
          float bb = mix(c, d, u.x);
          float cc = mix(aa, bb, u.y);

          return k0 + k1*u.x + k2*u.y + k3*u.x*u.y;
        }

        float fbm1(vec2 p) {
          vec2 op = p;
          const float aa = 0.45;
          const float pp = 2.03;
          const vec2 oo = -vec2(1.23, 1.5);
          const float rr = 1.2;

          float h = 0.0;
          float d = 0.0;
          float a = 1.0;

          for (int i = 0; i < 5; ++i) {
          h += a*onoise(p);
          d += (a);
          a *= aa;
          p += oo;
          p *= pp;
          rot(p, rr);
        }

        return mix((h/d), -0.5*(h/d), pow(vnoise(0.9*op), 0.25));
        }

        float fbm2(vec2 p) {
          vec2 op = p;
          const float aa = 0.45;
          const float pp = 2.03;
          const vec2 oo = -vec2(1.23, 1.5);
          const float rr = 1.2;

          float h = 0.0;
          float d = 0.0;
          float a = 1.0;

          for (int i = 0; i < 7; ++i) {
          h += a*onoise(p);
          d += (a);
          a *= aa;
          p += oo;
          p *= pp;
          rot(p, rr);
        }

        return mix((h/d), -0.5*(h/d), pow(vnoise(0.9*op), 0.25));
        }

        float fbm3(vec2 p) {
          vec2 op = p;
          const float aa = 0.45;
          const float pp = 2.03;
          const vec2 oo = -vec2(1.23, 1.5);
          const float rr = 1.2;

          float h = 0.0;
          float d = 0.0;
          float a = 1.0;

          for (int i = 0; i < 3; ++i) {
          h += a*onoise(p);
          d += (a);
          a *= aa;
          p += oo;
          p *= pp;
          rot(p, rr);
        }

        return mix((h/d), -0.5*(h/d), pow(vnoise(0.9*op), 0.25));
        }


        float warp(vec2 p) {
          vec2 v = vec2(fbm1(p), fbm1(p+0.7*vec2(1.0, 1.0)));
          float stressFactor = sliderValue + 0.01f;

          rot(v, 1.0+runTime*stressFactor);

          vec2 vv = vec2(fbm2(p + 3.7*v), fbm2(p + -2.7*v.yx+0.7*vec2(1.0, 1.0)));

          rot(vv, -1.0+runTime*1.8);

          return fbm3(p + 9.0*vv);
        }

        float height(vec2 p) {
          float a = 0.045*runTime*0.1;
          p += 9.0*vec2(cos(a), sin(a));
          p *= 2.0;
          p += 13.0;
          float h = warp(p);
          float rs = 3.0;
          return 0.35*tanh_approx(rs*h)/rs;
        }

        vec3 normal(vec2 p) {
          // As suggested by IQ, thanks!
          vec2 eps = -vec2(2.0/resolution.y, 0.0);

          vec3 n;

          n.x = height(p + eps.xy) - height(p - eps.xy);
          n.y = 2.0*eps.x;
          n.z = height(p + eps.yx) - height(p - eps.yx);


          return normalize(n);
        }

        void main() {
          vec2 q = gl_FragCoord.xy/resolution.xy;
          vec2 p = -1. + 2. * q;
          p.x*=resolution.x/resolution.y;
          //lights positions
          const vec3 lp1 = vec3(2.1, -0.5, -0.1);
          const vec3 lp2 = vec3(-2.1, -0.5, -0.1);

          float h = height(p);
          vec3 pp = vec3(p.x, h, p.y);
          float ll1 = length(lp1.xz - pp.xz);
          vec3 ld1 = normalize(lp1 - pp);
          vec3 ld2 = normalize(lp2 - pp);

          vec3 n = normal(p);
          float diff1 = max(dot(ld1, n), 0.0);
          float diff2 = max(dot(ld2, n), 0.0);
          //lights colors
          // color read b,g,r for some reason?
          vec3 baseCol1 = vec3(181. / 255., 181. / 255., 181. / 255.);
          vec3 baseCol2 = vec3(121. / 255., 212. / 255., 255. / 255.);

          float oh = height(p + ll1*0.05*normalize(ld1.xz));
          const float level0 = 0.0;
          const float level1 = 0.125;
          // VERY VERY fake shadows + hilight
          vec3 scol1 = baseCol1*(smoothstep(level0, level1, h) - smoothstep(level0, level1, oh));
          vec3 scol2 = baseCol2*(smoothstep(level0, level1, h) - smoothstep(level0, level1, oh));
          // specular and diffuse strenght
          vec3 col = vec3(0.0);
          col += 0.55*baseCol1.zyx*pow(diff1, 1.0);
          col += 0.55*baseCol1.zyx*pow(diff1, 1.0);
          col += 0.55*baseCol2.zyx*pow(diff2, 1.0);
          col += 0.55*baseCol2.zyx*pow(diff2, 1.0);
          col += scol1*0.5;
          col += scol2*0.5;

          // fragColor = vec4(col, 1.0);
          if (q.y > sliderValue)
            {
                fragColor = vec4(col, 1.0);
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
    uniforms_ = std::make_unique<Uniforms>(shader_);

    statusText_ = "GLSL: v" + juce::String(juce::OpenGLShaderProgram::getLanguageVersion(), 2);
  }
  else
  {
    statusText_ = shader_.getLastError();
  }
}
