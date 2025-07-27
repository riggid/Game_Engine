#include "Setup.h"

void shaderViewSetup(Shader shader);
// settings:
unsigned int SCR_WIDTH = 1600;
unsigned int SCR_HEIGHT = 900;

// camera
Camera camera(glm::vec3(0.0f, 2.0f, 2.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
double lastTime = glfwGetTime();
int nbFrames = 0;
// lighting
glm::vec3 lightPos(0.0f, 3.0f, 0.0f);

glm::vec3 girlpos = glm::vec3(0.0f);
int currentEyeballs = 5;
bool showMenu = false;
bool shoot = false;

// Laser properties
glm::vec3 laserStart;
glm::vec3 laserColor = glm::vec3(1.0f, 0.0f, 0.0f);
float laserDuration = 0.2f;
float laserTimer = 0.0f;

bool isSpacePressed = false;

float sliceCheckInterval = 0.1f;
float lastSliceCheck = 0.0f;

// Bloom settings
const unsigned int BLOOM_MIPS = 5;
float bloomStrength = 2.0f;
float bloomThreshold = 0.8f;
float exposure = 0.8f;

bool vsyncEnabled = true;

int main() {
  // Initialize GLFW and create window
  GLFWwindow* window = Setup::initializeWindow(SCR_WIDTH, SCR_HEIGHT, camera);
  if (!window) {
    return -1;
  }

  // Set initial VSync state
  glfwSwapInterval(1);

  camera.setHeight(2.0f); // Set camera height to 2 units

  // Initialize GLAD
  Setup::initializeGLAD();

  // Initialize ImGui
  Setup::initializeImGui(window);

  // Configure OpenGL state
  Setup::setupOpenGLState();

  // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
  stbi_set_flip_vertically_on_load(true);

  // build and compile shaders
  // -------------------------

  // For error checking
  GLenum err;

  Shader lightingShader("res/shaders/lighting.vert","res/shaders/lighting.frag");
  Shader lightCubeShader("res/shaders/lightCube.vert","res/shaders/lightCube.frag");
  Shader laserShader("res/shaders/lazer.vert", "res/shaders/lazer.frag");
  Shader lineShader("res/shaders/line.vert", "res/shaders/line.frag");

  // Check for OpenGL errors after shader compilation
  while ((err = glGetError()) != GL_NO_ERROR) {
    std::cout << "OpenGL error after shader compilation: " << err << std::endl;
  }

  //  load models``
  //  -----------
  Model girlModel("res/Objects/girl.obj");
  Model ourModel("res/Objects/scean.obj");
  Model eyeballModel("res/Objects/eyeball.obj");
  Model lightCubeModel("res/Objects/untitled.obj");
  Model laserModel("res/Objects/cylender.obj");

  // draw in wireframe
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // shader configuration
  // --------------------
  lightingShader.use();
  lightingShader.setInt("material.diffuse", 0);
  lightingShader.setInt("material.specular", 1);
  lightingShader.setFloat("material.shininess", 1.0f);

  lightingShader.setVec3("light.position", lightPos);
  lightingShader.setVec3("viewPos", camera.Position);

  // light properties
  lightingShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
  lightingShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
  lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
  lightingShader.setFloat("light.constant", 1.0f);
  lightingShader.setFloat("light.linear", 0.09f);
  lightingShader.setFloat("light.quadratic", 0.032f);

  // Set up laser shader
  laserShader.use();
  laserShader.setVec3("laserColor", laserColor);

  Drawer scene(ourModel,lightingShader);
  scene.setScale(glm::vec3(1.0f));
  
  Drawer girl(girlModel,lightingShader);
  girl.setRotationMode(RotationMode::Y_ONLY);
  
  Drawer eyeball(eyeballModel,lightingShader);
  eyeball.setScale(glm::vec3(0.05f));

  Drawer laser(laserModel,laserShader);
  laser.setScale(glm::vec3(0.5f, 0.5f, 10.0f));
  laser.setRotationMode(RotationMode::ALL);

  Drawer lightCube(lightCubeModel,lightCubeShader);
  lightCube.setPosition(lightPos);
  lightCube.setScale(glm::vec3(0.4f));


  // Check for OpenGL errors
  while ((err = glGetError()) != GL_NO_ERROR) {
    std::cout << "OpenGL error after buffer setup: " << err << std::endl;
  }

  // render loop
  // -----------
  while (!glfwWindowShouldClose(window)) {
    // Input processing
    glfwPollEvents();
    InputManager inputManager(camera, deltaTime, laserTimer, laserDuration, vsyncEnabled);
    inputManager.processInput(window);

    if (showMenu) {
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();
      ImGui::ShowDemoWindow();
    }

    // Frame time calculation
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // FPS counter
    nbFrames++;
    if (currentFrame - lastTime >= 1.0) { // If last print was more than 1 sec ago
      std::cout << 1000.0/double(nbFrames) << " ms/frame (" << nbFrames << " FPS)" << std::endl;
      nbFrames = 0;
      lastTime = currentFrame;
    }

    // Scene rendering
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shaderViewSetup(lightingShader);
    shaderViewSetup(laserShader);
    shaderViewSetup(lineShader);
    scene.draw();

    // Get bounding box info
    glm::vec3 minBounds = girlModel.getBoundingBoxMin();
    glm::vec3 maxBounds = girlModel.getBoundingBoxMax();
    glm::vec3 intersectionPoint;

    // Draw girl and bounding box
    girl.setupBoundingBox(lineShader);
    girl.setTarget(camera.Position);
    girl.draw();

    if (laserTimer > 0) {
      // Laser setup and drawing
      laserStart = camera.Position + camera.Front * 2.0f - glm::vec3(0.0f, 0.5f, 0.0f);
      glm::vec3 laserTarget = camera.Position + camera.Front * 50.0f - glm::vec3(0.0f, 0.5f, 0.0f);
      glm::vec3 laserDirection = glm::normalize(laserTarget - laserStart);
      laserTimer -= deltaTime;

      laser.setPosition(laserStart);
      laser.setTarget(laserTarget);
      
      laserShader.use();
      laserShader.setVec3("laserColor", glm::vec3(1.0f, 0.0f, 0.0f));
      laser.draw();

      // Collision and slicing
      float currentTime = static_cast<float>(glfwGetTime());
      if (currentTime - lastSliceCheck >= sliceCheckInterval) {
        lastSliceCheck = currentTime;

        glm::mat4 modelMatrix = girl.calculateModelMatrix();
        glm::mat4 inverseModel = glm::inverse(modelMatrix);
        
        glm::vec4 modelSpaceStart = inverseModel * glm::vec4(laserStart, 1.0f);
        glm::vec4 modelSpaceDir = inverseModel * glm::vec4(laserDirection, 0.0f);
        glm::vec3 modelStart = glm::vec3(modelSpaceStart);
        glm::vec3 modelDirection = glm::normalize(glm::vec3(modelSpaceDir));

        if (girlModel.HitBoundingBox(minBounds, maxBounds, modelStart, modelDirection, intersectionPoint)) {
          girlModel.sliceModelCylinder(modelStart, modelStart + modelDirection * 50.0f, 0.2f);
          girl.setModel(girlModel);
        }
      }
    }

    // Render eyeballs
    for (int i = 0; i < currentEyeballs; i++) {
      float angle = (2.0f * glm::pi<float>() * i) / (currentEyeballs);
      float radius = 1.0f;
      
      glm::vec3 Up = camera.Up;
      glm::vec3 Right = camera.Right;

      glm::vec3 eyeballPos = glm::vec3(
        radius * (cos(angle)*Up.x + sin(angle)*Right.x),
        radius * (cos(angle)*Up.y + sin(angle)*Right.y),
        radius * (cos(angle)*Up.z + sin(angle)*Right.z)
      ) + camera.Position + camera.Front * 2.0f;
              
      eyeball.setPosition(eyeballPos);
      eyeball.setTarget(girlpos);
      eyeball.draw();
    }

    // Final render
    if (showMenu) {
      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    glfwSwapBuffers(window);
  }

  // Cleanup
  glfwDestroyWindow(window);
  glfwTerminate();
  Setup::cleanup();
  return 0;
}

void shaderViewSetup(Shader shader) {
    shader.use();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),(float)SCR_WIDTH / (float)SCR_HEIGHT,0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
}

