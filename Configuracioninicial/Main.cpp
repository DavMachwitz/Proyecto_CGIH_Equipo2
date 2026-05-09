// =====================================================================
//  PROYECTO: Planta Base - Facultad
//  Implementación: Sistema Día/Noche + Reflectores Múltiples (Tecla F)
//
//
// =====================================================================

#include <iostream>
#include <vector>
#define _USE_MATH_DEFINES
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "ModelAnimation.h"
#include "Animator.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


// ---- Ventana --------------------------------------------------------
const GLuint WIDTH = 1300, HEIGHT = 800;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// ---- Tiempo ---------------------------------------------------------
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// ---- Camara ---------------------------------------------------------
Camera  camera(glm::vec3(0.0f, 4.0f, 5.5f));
GLfloat lastX = WIDTH / 2.0f;
GLfloat lastY = HEIGHT / 2.0f;
bool    keys[1024];
bool    firstMouse = true;
bool    mouseCaptured = true;

// --- ESTADO DEL REFLECTOR ---
bool    spotLightOn = false;

// =====================================================================
// SISTEMA DE HORAS DEL DIA
// =====================================================================
struct TimeOfDayPreset {
    const char* name;
    glm::vec3   clearColor;
    glm::vec3   tint;
};

TimeOfDayPreset presets[3] = {
    { "Manana", glm::vec3(0.87f, 0.72f, 0.62f), glm::vec3(0.65f, 0.55f, 0.45f) },
    { "Tarde",  glm::vec3(0.50f, 0.70f, 0.92f), glm::vec3(1.00f, 1.00f, 1.00f) },
    { "Noche",  glm::vec3(0.04f, 0.05f, 0.12f), glm::vec3(0.20f, 0.25f, 0.40f) },
};

int currentPreset = 1;

// =====================================================================
// PROTOTIPOS Y UTILIDADES
// =====================================================================
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();
void Animation();

inline float mX(float xm) { return (xm - 21.25f) * 0.10f; }
inline float mZ(float zm) { return -(zm - 13.50f) * 0.10f; }

GLuint makeVAO(const GLfloat* v, GLsizeiptr vs, const GLuint* idx, GLsizeiptr is) {
    GLuint vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vs, v, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, is, idx, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    return vao;
}

GLuint makeVAO_raw(const GLfloat* v, GLsizeiptr vs) {
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vs, v, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    return vao;
}
// =====================================================================
// Estructuras para animación por Key Frames
// =====================================================================

// Silla
float sillaAsientoRot = 66.0f;
float sillaPatasRot = -60.0f;
float sillaPosX = 6.1088;
float sillaPosY = 0.0f;
float sillaPosZ = 7.6101f;

// Mesa
float mesaPosX = 6.0003f;
float mesaPosY = -1.0f;
float mesaPosZ = 8.10612f;
float mesaTablonRotX = 0.0f;
float mesaPata1Rot = -90.0f;
float mesaPata2Rot = 90.0f;
float tuboPata1Rot = 45.0f;

// Stand Octanorm
float standHoriz = 1.4481f;
float standBase = -3.7086f;
float panel = -6.0f;

//Persona 
glm::vec3 personaPos = glm::vec3(0.0f, 0.2f, -65.0f);
glm::vec3 personaPos2 = glm::vec3(2.0f, 0.2f, 65.0f);
float     personaSpeed = 1.0f;   // unidades por segundo
float     personaDirZ = 1.0f;   // camina en dirección Z
float     personaRotY = 0.0f;   // rotación para que mire hacia donde camina

float     persona2DirZ = -1.0f; // ← dirección contraria
float     persona2RotY = 180.0f;

#define MAX_FRAMES 9
int i_max_steps = 600;
int i_curr_steps = 0;
typedef struct _frame {
    // Silla 
    float sillaPosX, sillaPosY, sillaPosZ;
    float sillaAsientoRot, sillaPatasRot;

    // Mesa
    float mesaPosX;   
    float mesaPosY;
    float mesaPosZ;
    float mesaPata1Rot;
    float mesaPata2Rot;
    float tuboPata1Rot;
    float mesaTablonRotX;

    // Stand Octanorm 
    float standHoriz, standBase;
    float panel;

    // Incrementos para la interpolación
    float sillaAsientoInc, sillaPatasInc;
    float sillaPosXInc, sillaPosYInc, sillaPosZInc;
    float mesaPosXInc, mesaPosYInc, mesaPosZInc;
    float mesaPata1RotInc, mesaPata2RotInc, tuboPata1RotInc, mesaTablonRotXInc;
    float standHorizInc, standBaseInc;
    float panelInc;
} FRAME;

FRAME KeyFrame[MAX_FRAMES];
int FrameIndex = 0;		
bool play = false;
int playIndex = 0;
int direccion = 1; 
bool mostrarStands = true;



void interpolation(void)
{
    KeyFrame[playIndex].sillaPosXInc = (KeyFrame[playIndex + 1].sillaPosX - KeyFrame[playIndex].sillaPosX) / i_max_steps;
    KeyFrame[playIndex].sillaPosYInc = (KeyFrame[playIndex + 1].sillaPosY - KeyFrame[playIndex].sillaPosY) / i_max_steps;
    KeyFrame[playIndex].sillaPosZInc = (KeyFrame[playIndex + 1].sillaPosZ - KeyFrame[playIndex].sillaPosZ) / i_max_steps;
    KeyFrame[playIndex].sillaAsientoInc = (KeyFrame[playIndex + 1].sillaAsientoRot - KeyFrame[playIndex].sillaAsientoRot) / i_max_steps;
    KeyFrame[playIndex].sillaPatasInc = (KeyFrame[playIndex + 1].sillaPatasRot - KeyFrame[playIndex].sillaPatasRot) / i_max_steps;

    // Mesa
    KeyFrame[playIndex].mesaPosXInc = (KeyFrame[playIndex + 1].mesaPosX - KeyFrame[playIndex].mesaPosX) / i_max_steps;
    KeyFrame[playIndex].mesaPosYInc = (KeyFrame[playIndex + 1].mesaPosY - KeyFrame[playIndex].mesaPosY) / i_max_steps;
    KeyFrame[playIndex].mesaPosZInc = (KeyFrame[playIndex + 1].mesaPosZ - KeyFrame[playIndex].mesaPosZ) / i_max_steps;
    KeyFrame[playIndex].mesaPata1RotInc = (KeyFrame[playIndex + 1].mesaPata1Rot - KeyFrame[playIndex].mesaPata1Rot) / i_max_steps;
    KeyFrame[playIndex].mesaTablonRotXInc = (KeyFrame[playIndex + 1].mesaTablonRotX - KeyFrame[playIndex].mesaTablonRotX) / i_max_steps;
    KeyFrame[playIndex].mesaPata2RotInc = (KeyFrame[playIndex + 1].mesaPata2Rot - KeyFrame[playIndex].mesaPata2Rot) / i_max_steps;
    KeyFrame[playIndex].tuboPata1RotInc = (KeyFrame[playIndex + 1].tuboPata1Rot - KeyFrame[playIndex].tuboPata1Rot) / i_max_steps;

    // Stand
    KeyFrame[playIndex].standHorizInc = (KeyFrame[playIndex + 1].standHoriz - KeyFrame[playIndex].standHoriz) / i_max_steps;

    KeyFrame[playIndex].standBaseInc = (KeyFrame[playIndex + 1].standBase - KeyFrame[playIndex].standBase) / i_max_steps;
    KeyFrame[playIndex].panelInc = (KeyFrame[playIndex + 1].panel - KeyFrame[playIndex].panel) / i_max_steps;
   
}

// =====================================================================
// MAIN
// =====================================================================
int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Planta Base Facultad", nullptr, nullptr);
    if (!window) { glfwTerminate(); return EXIT_FAILURE; }
    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);

    if (mouseCaptured) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit()) return EXIT_FAILURE;
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);                                    // ← agregar
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader shader("Shader/core.vs", "Shader/lamp.frag");
    Shader shader1("Shader/modelLoading.vs", "Shader/modelLoading.frag");
    Shader shader2("Shader/lamp.vs", "Shader/modelLoading.frag");

    Model sotano((char*)"Models/SotanoFI.obj");
    Model reja((char*)"Models/reja.obj");
    Model luzTecho((char*)"Models/luz_techo.obj");
    Model upstairs((char*)"Models/Upstairs.obj");
    Model estatua1((char*)"Models/EstatuaEscalera.obj");
    Model base1((char*)"Models/BaseEstatuaEscalera.obj");
    Model estatua2((char*)"Models/EstatuaExamenes.obj");
    Model base2((char*)"Models/BaseEstatuaExamenes.obj");
    //Silla
    Model sillaMarco((char*)"Models/Stands/Chair/Silla_Marco.obj");
    Model sillaAsiento((char*)"Models/Stands/Chair/Silla_Asiento.obj");
    Model sillaPatas((char*)"Models/Stands/Chair/Silla_patas.obj");
    //Mesa
    Model tablon((char*)"Models/Stands/Table/tablon.obj");
    Model p1((char*)"Models/Stands/Table/pata1.obj");
    Model p2((char*)"Models/Stands/Table/pata2.obj");
    //StandOctanorm
    Model base((char*)"Models/Stands/Octanorm/base_stand.obj");
    Model trave((char*)"Models/Stands/Octanorm/trave_stand.obj");
    Model panel1((char*)"Models/Stands/Octanorm/panel1.obj");
    Model panel2((char*)"Models/Stands/Octanorm/panel2.obj");
    Model panel3((char*)"Models/Stands/Octanorm/panel3.obj");
    Model panel4((char*)"Models/Stands/Octanorm/panel4.obj");
    //People
    Model persona1((char*)"Models/People/person1.dae");
	Model persona2((char*)"Models/People/person2.dae");

    // NUEVO: crear animación y animator para persona1
    ModelAnimation danceAnim("Models/People/person1.dae", persona1.GetBoneInfoMap(),persona1.GetBoneCount());
    Animator animator(&danceAnim);
	ModelAnimation danceAnim2("Models/People/person2.dae", persona2.GetBoneInfoMap(), persona2.GetBoneCount());
	Animator animator2(&danceAnim2);

    //KeyFrames
    // FRAME 0: ESTADO INICIAL
    KeyFrame[0].sillaPosX = 5.968f;
    KeyFrame[0].sillaPosY = 0.0f;
    KeyFrame[0].sillaPosZ = 7.0114f;
    KeyFrame[0].sillaAsientoRot = 66.0f;
    KeyFrame[0].sillaPatasRot = -60.0f;

    KeyFrame[0].mesaPosX = 6.0003f;
    KeyFrame[0].mesaPosY = -1.0f;
    KeyFrame[0].mesaPosZ = 8.10612f;
    KeyFrame[0].mesaPata1Rot = -90.0f;
    KeyFrame[0].mesaPata2Rot = 90.0f;
    KeyFrame[0].mesaTablonRotX = 0.0f;
    KeyFrame[0].tuboPata1Rot = 45.0f;

    KeyFrame[0].standHoriz = 1.4481f;
    KeyFrame[0].standBase = -3.7086f;
    KeyFrame[0].panel = -6.0f;
    //Frame 2
    KeyFrame[1].sillaPosX = 5.968f;
    KeyFrame[1].sillaPosY = 0.0f;
    KeyFrame[1].sillaPosZ = 7.0114f;
    KeyFrame[1].sillaAsientoRot = 0.0f;
    KeyFrame[1].sillaPatasRot = 0.0f;

    KeyFrame[1].mesaPosX = 6.0003f;
    KeyFrame[1].mesaPosY = 0.0f;
    KeyFrame[1].mesaPosZ = 8.10612f;
    KeyFrame[1].mesaPata1Rot = 0.0f;
    KeyFrame[1].mesaPata2Rot = 90.0f;
    KeyFrame[1].tuboPata1Rot = 45.0f;
    KeyFrame[1].mesaTablonRotX = 25.0f;

    KeyFrame[1].standHoriz = 1.4481f;
    KeyFrame[1].standBase = -3.7086f;
    KeyFrame[1].panel = -6.0f;

    //Frame 3
    KeyFrame[2].sillaPosX = 5.968f;
    KeyFrame[2].sillaPosY = 0.0f;
    KeyFrame[2].sillaPosZ = 7.0114f;
    KeyFrame[2].sillaAsientoRot = 0.0f;
    KeyFrame[2].sillaPatasRot = 0.0f;

    KeyFrame[2].mesaPosX = 6.0003f;
    KeyFrame[2].mesaPosY = 0.0f;
    KeyFrame[2].mesaPosZ = 8.10612f;
    KeyFrame[2].mesaPata1Rot = 0.0f;
    KeyFrame[2].mesaPata2Rot = 0.0f;
    KeyFrame[2].tuboPata1Rot = 0.0f;
    KeyFrame[2].mesaTablonRotX = 0.0f;

    KeyFrame[2].standHoriz = 1.4481f;
    KeyFrame[2].standBase = -3.7086f;
    KeyFrame[2].panel = -6.0f;
    
    // FRAME 1: ESTADO FINAL
    KeyFrame[3].sillaPosX = 0.0f;
    KeyFrame[3].sillaPosY = 0.0f;
    KeyFrame[3].sillaPosZ = 0.0f;
    KeyFrame[3].sillaAsientoRot = 0.0f;
    KeyFrame[3].sillaPatasRot = 0.0f;

    KeyFrame[3].mesaPosX = 0.0;
    KeyFrame[3].mesaPosY = 0.0f;
    KeyFrame[3].mesaPosZ = 0.0f;
    KeyFrame[3].mesaPata1Rot = 0.0f;
    KeyFrame[3].mesaPata2Rot = 0.0f;
    KeyFrame[3].tuboPata1Rot = 0.0f;

    KeyFrame[3].standHoriz = 0.0f;
    KeyFrame[3].standBase = 0.0f;
    KeyFrame[3].panel = -0.0f;


    FrameIndex = 4;



    glm::mat4 projection = glm::perspective(camera.GetZoom(), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

    // Geometría Procedural
    GLfloat pisoV[] = { mX(0.0f), 0.000f, mZ(0.0f), mX(42.5f), 0.000f, mZ(0.0f), mX(42.5f), 0.000f, mZ(27.0f), mX(0.0f), 0.000f, mZ(27.0f) };
    GLuint pisoI[] = { 0,1,2, 0,2,3 };
    GLfloat narV[] = { mX(8.5f), 0.002f, mZ(9.0f), mX(17.0f), 0.002f, mZ(9.0f), mX(17.0f), 0.002f, mZ(18.0f), mX(8.5f), 0.002f, mZ(18.0f) };
    GLuint narI[] = { 0,1,2, 0,2,3 };
    GLfloat azulV[] = { mX(34.0f), 0.002f, mZ(9.0f), mX(42.5f), 0.002f, mZ(9.0f), mX(42.5f), 0.002f, mZ(18.0f), mX(34.0f), 0.002f, mZ(18.0f) };
    GLuint azulI[] = { 0,1,2, 0,2,3 };
    GLfloat perimV[] = { mX(0.0f), 0.005f, mZ(0.0f), mX(42.5f), 0.005f, mZ(0.0f), mX(42.5f), 0.005f, mZ(27.0f), mX(0.0f), 0.005f, mZ(27.0f) };
    GLfloat lineaV[] = { mX(34.0f), 0.005f, mZ(0.0f), mX(34.0f), 0.005f, mZ(9.0f) };

    GLuint VAO_piso = makeVAO(pisoV, sizeof(pisoV), pisoI, sizeof(pisoI));
    GLuint VAO_nar = makeVAO(narV, sizeof(narV), narI, sizeof(narI));
    GLuint VAO_azul = makeVAO(azulV, sizeof(azulV), azulI, sizeof(azulI));
    GLuint VAO_perim = makeVAO_raw(perimV, sizeof(perimV));
    GLuint VAO_line = makeVAO_raw(lineaV, sizeof(lineaV));

    const int SEG = 40;
    std::vector<GLfloat> circV;
    circV.push_back(0.0f); circV.push_back(0.0f); circV.push_back(0.0f);
    for (int i = 0; i <= SEG; i++) {
        float a = 2.0f * 3.14159265f * (float)i / (float)SEG;
        circV.push_back(cosf(a)); circV.push_back(0.0f); circV.push_back(sinf(a));
    }
    GLuint VAO_circ = makeVAO_raw(circV.data(), (GLsizeiptr)(circV.size() * sizeof(GLfloat)));

    const float R = 0.055f;
    const float Y_COL = 0.006f;

    struct Col { float xm, zm; };
    Col cols[] = {
        {  8.5f, 18.0f }, { 17.0f, 18.0f }, {  8.5f,  9.0f }, { 17.0f,  9.0f },
        { 34.0f, 18.0f }, { 42.5f, 18.0f }, { 34.0f,  9.0f }, { 42.5f,  9.0f },
        { 25.5f, 18.0f }, { 25.5f,  9.0f }, { 25.5f, 27.0f }, { 34.0f, 27.0f }, { 42.5f, 27.0f },
        { 17.0f,  0.0f }, { 25.5f,  0.0f }, { 34.0f,  0.0f },
    };
    const int N_COL = sizeof(cols) / sizeof(Col);

    GLint colorLoc = glGetUniformLocation(shader.Program, "color");
    GLint modelLoc = glGetUniformLocation(shader.Program, "model");
    GLint viewLoc = glGetUniformLocation(shader.Program, "view");
    GLint projLoc = glGetUniformLocation(shader.Program, "projection");

    // =================================================================
    // PARAMETROS GENERALES DE LAS LUCES
    // =================================================================
    glm::vec3 luzTechoPos = glm::vec3(0.0f, 5.2f, 10.0f); // Y referencia para la cuadricula
    glm::vec3 luzTechoScale = glm::vec3(0.5f);
    float     luzTechoRotY = 0.0f;

    // =================================================================
    // BLOQUE 1: CUADRICULA DE 6 LUCES (3 X 2)
    // =================================================================
    float luzPosX[3] = { -10.0f, 0.0f, 10.0f };
    float luzPosZ[2] = { 10.0f, 0.0f };

    // =================================================================
    // BLOQUE 2: 9 LUCES EXTRA (posiciones individuales X, Y, Z)
    // =================================================================
    std::vector<glm::vec3> lucesExtra = {
        glm::vec3(18.0f, 4.5f,   0.0f),   // 1
        glm::vec3(18.0f, 4.5f,   5.0f),   // 2
        glm::vec3(-19.0f, 5.4f,   1.0f),   // 3
        glm::vec3(-19.0f, 5.2f,  10.0f),   // 4
        glm::vec3(-19.0f, 5.5f, -10.0f),   // 5
        glm::vec3(-10.0f, 5.5f, -10.0f),   // 6
        glm::vec3(0.0f, 5.5f, -10.0f),   // 7
        glm::vec3(10.0f, 5.5f, -10.0f),   // 8
        glm::vec3(17.0f, 5.5f, -10.0f),   // 9
    };

    // Total: 6 (cuadricula) + 9 (extras) = 15 spotlights
    const int NUM_SPOTLIGHTS = 6 + (int)lucesExtra.size();

    // =================================================================
    // RENDER LOOP
    // =================================================================
    while (!glfwWindowShouldClose(window))
    {
        GLfloat cur = (GLfloat)glfwGetTime();
        deltaTime = cur - lastFrame;
        lastFrame = cur;

        glfwPollEvents();
        DoMovement();
        Animation();

        const TimeOfDayPreset& P = presets[currentPreset];

        glClearColor(P.clearColor.r, P.clearColor.g, P.clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 proj = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

        // --- Construimos arreglo COMPLETO de posiciones de los 15 spotlights ---
        std::vector<glm::vec3> spotPositions;
        spotPositions.reserve(NUM_SPOTLIGHTS);

        // Primero las 6 de la cuadricula
        for (int row = 0; row < 2; row++) {
            for (int col = 0; col < 3; col++) {
                spotPositions.push_back(glm::vec3(luzPosX[col], luzTechoPos.y, luzPosZ[row]));
            }
        }
        // Despues las 9 extras (cada una con su Y propia)
        for (const auto& p : lucesExtra) {
            spotPositions.push_back(p);
        }

        // --- FUNCION AUXILIAR PARA ENVIAR LOS 15 REFLECTORES A LOS SHADERS ---
        auto setSpotlightUniforms = [&](GLuint programID) {
            glUniform1i(glGetUniformLocation(programID, "spotLightOn"), spotLightOn);
            glUniform1i(glGetUniformLocation(programID, "numSpotLights"), NUM_SPOTLIGHTS);

            glUniform3fv(
                glGetUniformLocation(programID, "spotLightPos"),
                NUM_SPOTLIGHTS,
                glm::value_ptr(spotPositions[0])
            );

            glUniform3f(glGetUniformLocation(programID, "spotLightDir"), 0.0f, -1.0f, 0.0f);
            glUniform1f(glGetUniformLocation(programID, "spotCutOff"), glm::cos(glm::radians(30.0f)));
            glUniform1f(glGetUniformLocation(programID, "spotOuterCutOff"), glm::cos(glm::radians(45.0f)));
            glUniform3f(glGetUniformLocation(programID, "spotLightColor"), 0.5f, 0.5f, 0.42f);
            };

        // -------------------------------------------------------------
        // DIBUJO 1: GEOMETRIA DE LA FACULTAD
        // -------------------------------------------------------------
        shader.Use();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

        setSpotlightUniforms(shader.Program);

        glm::mat4 I(1);
        auto setModel = [&](const glm::mat4& m) { glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m)); };
        auto setColorTinted = [&](float r, float g, float b) {
            glm::vec3 c = P.tint * glm::vec3(r, g, b);
            glUniform3f(colorLoc, c.r, c.g, c.b);
            };

        setColorTinted(0.55f, 0.55f, 0.55f); setModel(I); glBindVertexArray(VAO_piso); glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        setColorTinted(0.97f, 0.78f, 0.42f); setModel(I); glBindVertexArray(VAO_nar); glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        setColorTinted(0.40f, 0.65f, 0.90f); setModel(I); glBindVertexArray(VAO_azul); glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glLineWidth(3.5f); setColorTinted(0.0f, 0.0f, 0.0f); setModel(I); glBindVertexArray(VAO_perim); glDrawArrays(GL_LINE_LOOP, 0, 4);
        glLineWidth(3.5f); setColorTinted(0.0f, 0.0f, 0.0f); setModel(I); glBindVertexArray(VAO_line); glDrawArrays(GL_LINES, 0, 2);

        glBindVertexArray(VAO_circ);
        for (int i = 0; i < N_COL; i++) {
            float cx = mX(cols[i].xm); float cz = mZ(cols[i].zm);
            glm::mat4 mf = glm::translate(I, glm::vec3(cx, Y_COL, cz));
            mf = glm::scale(mf, glm::vec3(R, 1.0f, R));
            setColorTinted(1.0f, 1.0f, 1.0f); glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mf));
            glDrawArrays(GL_TRIANGLE_FAN, 0, SEG + 2);

            glm::mat4 mb = glm::translate(I, glm::vec3(cx, Y_COL + 0.001f, cz));
            mb = glm::scale(mb, glm::vec3(R + 0.01f, 1.0f, R + 0.01f));
            setColorTinted(0.0f, 0.0f, 0.0f); glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mb));
            glLineWidth(1.5f); glDrawArrays(GL_LINE_LOOP, 1, SEG);
        }

        // -------------------------------------------------------------
        // DIBUJO 2: MODELOS EXTERNOS .OBJ
        // -------------------------------------------------------------
        shader1.Use();
        glUniform1i(glGetUniformLocation(shader1.Program, "useSkinning"), GL_FALSE);
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        GLint modelTintLoc = glGetUniformLocation(shader1.Program, "tintColor");
        glUniform3f(modelTintLoc, P.tint.r, P.tint.g, P.tint.b);

        setSpotlightUniforms(shader1.Program);

        // Sotano
        glm::mat4 model(1);
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        sotano.Draw(shader1);

        // Reja
        model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(5.0f, 0.0f, 13.85f));
        model = glm::scale(model, glm::vec3(0.11f, 0.097f, 0.12f));
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        reja.Draw(shader1);

        //Estatua escalera
        model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(-3.0f, 0.0f, 2.488f));
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        base1.Draw(shader1);

        model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(-3.0f, 0.0f, 2.5f));
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        estatua1.Draw(shader1);

        //Estatua examenes
        model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -0.5f));
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        base2.Draw(shader1);

        model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -0.5f));
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        estatua2.Draw(shader1);

        //Personas
		//shader2.Use();
        // ── Mover la posición del personaje ──────────────────────────
        personaPos.z += personaSpeed * personaDirZ * deltaTime;
        personaPos2.z += personaSpeed * persona2DirZ * deltaTime;
        
        if (personaPos.z > 13.0f) {
            personaDirZ = -1.0f;
            personaRotY = 180.0f;  // girar cuando cambia de dirección
        }
        if (personaPos.z < -13.0f) {
            personaDirZ = 1.0f;
            personaRotY = 0.0f;
        }

        if (personaPos2.z < -13.0f) {
            persona2DirZ = 1.0f;
            persona2RotY = 0.0f;
        }
        if (personaPos2.z > 13.0f) {
            persona2DirZ = -1.0f;
            persona2RotY = 180.0f;
        }
        // ── Skinning y matrices ───────────────────────────────────────
        glUniform1i(glGetUniformLocation(shader1.Program, "useSkinning"), GL_TRUE);
        animator.UpdateAnimation(deltaTime);
        const auto& boneMatrices = animator.GetFinalBoneMatrices();
        for (int i = 0; i < 100; i++) {
            string uName = "finalBonesMatrices[" + to_string(i) + "]";
            glUniformMatrix4fv(glGetUniformLocation(shader1.Program, uName.c_str()),
                1, GL_FALSE, &boneMatrices[i][0][0]);
        }

        // ── Dibujar en la posición actualizada ───────────────────────
        glm::mat4 modelPersona = glm::mat4(1.0f);
        modelPersona = glm::translate(modelPersona, personaPos);
        modelPersona = glm::rotate(modelPersona,glm::radians(personaRotY),glm::vec3(0.0f, 1.0f, 0.0f));
        modelPersona = glm::scale(modelPersona, glm::vec3(0.5f, 0.5f, 0.5f));
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"),1, GL_FALSE, glm::value_ptr(modelPersona));
        persona1.Draw(shader1);

        animator2.UpdateAnimation(deltaTime);
        const auto& bones2 = animator2.GetFinalBoneMatrices();
        for (int i = 0; i < 100; i++) {
            string uName = "finalBonesMatrices[" + to_string(i) + "]";
            glUniformMatrix4fv(glGetUniformLocation(shader1.Program, uName.c_str()),
                1, GL_FALSE, &bones2[i][0][0]);
        }
        glm::mat4 modelPersona2 = glm::mat4(1.0f);
        modelPersona2 = glm::translate(modelPersona2, personaPos2);
        //modelPersona2 = glm::rotate(modelPersona2, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelPersona2 = glm::rotate(modelPersona2, glm::radians(persona2RotY), glm::vec3(0.0f, 1.0f, 0.0f));
        modelPersona2 = glm::scale(modelPersona2, glm::vec3(1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelPersona2));
        persona2.Draw(shader1);

        glUniform1i(glGetUniformLocation(shader1.Program, "useSkinning"), GL_FALSE);       
        //Silla
        if (mostrarStands) {
            model = glm::mat4(1);
            model = glm::translate(model, glm::vec3(sillaPosX, sillaPosY, sillaPosZ));
            glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            sillaMarco.Draw(shader1);

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(sillaPosX, sillaPosY, sillaPosZ));
            model = glm::translate(model, glm::vec3(9.9f, 1.2448f, -1.80f));
            model = glm::rotate(model, glm::radians(sillaAsientoRot), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::translate(model, glm::vec3(-9.9f, -1.2448f, 1.80f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            sillaAsiento.Draw(shader1);

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(sillaPosX, sillaPosY, sillaPosZ));
            model = glm::translate(model, glm::vec3(9.76, 0.96, -1.25));
            model = glm::rotate(model, glm::radians(sillaPatasRot), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::translate(model, glm::vec3(-9.76, -0.96, 1.25));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            sillaPatas.Draw(shader1);

            //Mesa
            glm::mat4 modelMesa = glm::mat4(1.0f);

            // 1. Traslación global (Posición de la mesa en el sótano)
            modelMesa = glm::translate(modelMesa, glm::vec3(mesaPosX, mesaPosY, mesaPosZ));

            // 2. ROTACIÓN DEL TABLÓN (Inclinación sobre X)
            // Usamos un sándwich para que rote sobre su borde (ajusta el 1.5 y -1.73 según tu modelo)
            modelMesa = glm::translate(modelMesa, glm::vec3(8.30f, 1.5f, -1.73f));
            modelMesa = glm::rotate(modelMesa, glm::radians(mesaTablonRotX), glm::vec3(1.0f, 0.0f, 0.0f));
            modelMesa = glm::translate(modelMesa, glm::vec3(-8.30f, -1.5f, 1.73f));

            // Dibujamos el tablón
            glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelMesa));
            tablon.Draw(shader1);

            // --- PATA 1 (Hija del Tablón) ---
            // Empezamos con la matriz del tablón para que herede su inclinación
            glm::mat4 modelPata1 = modelMesa;
            // Ahora aplicamos su propia rotación interna para que se abra
            modelPata1 = glm::translate(modelPata1, glm::vec3(8.30f, 1.5f, -1.73f));
            modelPata1 = glm::rotate(modelPata1, glm::radians(mesaPata1Rot), glm::vec3(1.0f, 0.0f, 0.0f));
            modelPata1 = glm::translate(modelPata1, glm::vec3(-8.30f, -1.5f, 1.73f));

            glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelPata1));
            p1.Draw(shader1);

            // --- PATA 2 (Hija del Tablón) ---
            glm::mat4 modelPata2 = modelMesa; // También hereda la inclinación del tablón
            modelPata2 = glm::translate(modelPata2, glm::vec3(8.49f, 1.45f, 0.102f));
            modelPata2 = glm::rotate(modelPata2, glm::radians(mesaPata2Rot), glm::vec3(1.0f, 0.0f, 0.0f));
            modelPata2 = glm::translate(modelPata2, glm::vec3(-8.49f, -1.45f, -0.102f));

            glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelPata2));
            p2.Draw(shader1);


            //stand
            model = glm::mat4(1);
            model = glm::translate(model, glm::vec3(0.0, standBase, 0.0));
            glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            base.Draw(shader1);

            model = glm::mat4(1);
            model = glm::translate(model, glm::vec3(0.0, standHoriz, 0.0));
            glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            trave.Draw(shader1);

            model = glm::mat4(1);
            model = glm::translate(model, glm::vec3(0.0, panel, 0.0));
            glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            panel1.Draw(shader1);

            model = glm::mat4(1);
            model = glm::translate(model, glm::vec3(0.0, panel, 0.0));
            glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            panel2.Draw(shader1);

            model = glm::mat4(1);
            model = glm::translate(model, glm::vec3(0.0, panel, 0.0));
            glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            panel3.Draw(shader1);

            model = glm::mat4(1);
            model = glm::translate(model, glm::vec3(0.0, panel, 0.0));
            glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            panel4.Draw(shader1);
        }else{
            //stands mas complejos, solo traslaciones desde el suelo o el techo
        }

        // ============(   LUCES DE TECHO - CUADRICULA 2 X 3 = 6   )=================
        for (int row = 0; row < 2; row++) {
            for (int col = 0; col < 3; col++) {
                model = glm::mat4(1);
                model = glm::translate(model, glm::vec3(luzPosX[col], luzTechoPos.y, luzPosZ[row]));
                model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                model = glm::rotate(model, glm::radians(luzTechoRotY), glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::scale(model, luzTechoScale);
                glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
                luzTecho.Draw(shader1);
            }
        }
        // ============(   FIN CUADRICULA   )========================================

        // ============(   LUCES EXTRA (9 luces individuales)   )====================
        // Cada luz se dibuja en su (X, Y, Z) propia y emite reflector tambien.
        // ==========================================================================
        for (const auto& pos : lucesExtra) {
            model = glm::mat4(1);
            model = glm::translate(model, pos);
            model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(luzTechoRotY), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, luzTechoScale);
            glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
            luzTecho.Draw(shader1);
        }
        // ============(   FIN LUCES EXTRA   )=======================================

        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

// =====================================================================
// IMPLEMENTACION DE CALLBACKS Y MOVIMIENTO
// =====================================================================
void DoMovement() {
    if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])    camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])  camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])  camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) camera.ProcessKeyboard(RIGHT, deltaTime);
}

void MouseCallback(GLFWwindow* window, double xPos, double yPos) {
    if (!mouseCaptured) return;
    if (firstMouse) { lastX = xPos; lastY = yPos; firstMouse = false; }
    GLfloat xOffset = xPos - lastX;
    GLfloat yOffset = lastY - yPos;
    lastX = xPos; lastY = yPos;
    camera.ProcessMouseMovement(xOffset, yOffset);
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        mouseCaptured = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        mouseCaptured = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        firstMouse = true;
    }

    if (key == GLFW_KEY_F && action == GLFW_PRESS) spotLightOn = !spotLightOn;

    if (key == GLFW_KEY_1 && action == GLFW_PRESS) currentPreset = 0;
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) currentPreset = 1;
    if (key == GLFW_KEY_3 && action == GLFW_PRESS) currentPreset = 2;

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)        keys[key] = true;
        else if (action == GLFW_RELEASE) keys[key] = false;
    }
    if (key == GLFW_KEY_P && action == GLFW_PRESS && !play) {
        play = true;
        i_curr_steps = 0;

        // Si estamos en el inicio (Doblada), armamos hacia adelante
        if (sillaAsientoRot >= 60.0f) {
            direccion = 1;
            playIndex = 0; // Empezamos en el primer intervalo (0->1)
        }
        // Si estamos en el final (Abierta), desarmamos hacia atrás
        else {
            direccion = -1;
            playIndex = FrameIndex - 2; // Empezamos desde el último intervalo (2->1)
        }

        interpolation();
    }
    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        mostrarStands = !mostrarStands;
    }
}

//void Animation() {
//    if (play) {
//        if (i_curr_steps >= i_max_steps) {
//            play = false;
//            i_curr_steps = 0;
//        }
//        else {
//            // Silla
//            sillaPosX += KeyFrame[playIndex].sillaPosXInc * direccion;
//            sillaPosY += KeyFrame[playIndex].sillaPosYInc * direccion;
//            sillaPosZ += KeyFrame[playIndex].sillaPosZInc * direccion;
//            sillaAsientoRot += KeyFrame[playIndex].sillaAsientoInc * direccion;
//            sillaPatasRot += KeyFrame[playIndex].sillaPatasInc * direccion;
//
//            // Mesa
//            mesaPosX += KeyFrame[playIndex].mesaPosXInc * direccion;
//            mesaPosY += KeyFrame[playIndex].mesaPosYInc * direccion;
//            mesaPosZ += KeyFrame[playIndex].mesaPosZInc * direccion;
//            mesaPata1Rot += KeyFrame[playIndex].mesaPata1RotInc * direccion;
//            mesaPata2Rot += KeyFrame[playIndex].mesaPata2RotInc * direccion;
//            tuboPata1Rot += KeyFrame[playIndex].tuboPata1RotInc * direccion;
//
//            // Stand Octanorm
//            standHoriz += KeyFrame[playIndex].standHorizInc * direccion;
//            standBase += KeyFrame[playIndex].standBaseInc * direccion;
//            panel += KeyFrame[playIndex].panelInc * direccion;
//
//            i_curr_steps++;
//        }
//    }
//}
void Animation() {
    if (play) {
        if (i_curr_steps >= i_max_steps) { // ¿Terminó el intervalo actual?

            // Si vamos hacia adelante (armar)
            if (direccion == 1) {
                if (playIndex < FrameIndex - 2) { // ¿Hay más cuadros adelante?
                    playIndex++;
                    i_curr_steps = 0;
                    interpolation();
                }
                else {
                    play = false; // Llegamos al final (Frame 2)
                }
            }
            // Si vamos hacia atrás (desarmar)
            else {
                if (playIndex > 0) { // ¿Hay más cuadros atrás?
                    playIndex--;
                    i_curr_steps = 0;
                    interpolation();
                }
                else {
                    play = false; // Llegamos al inicio (Frame 0)
                }
            }
        }
        else {
            // Silla
            sillaPosX += KeyFrame[playIndex].sillaPosXInc * direccion;
            sillaPosY += KeyFrame[playIndex].sillaPosYInc * direccion;
            sillaPosZ += KeyFrame[playIndex].sillaPosZInc * direccion;
            sillaAsientoRot += KeyFrame[playIndex].sillaAsientoInc * direccion;
            sillaPatasRot += KeyFrame[playIndex].sillaPatasInc * direccion;

            // Mesa
            mesaPosX += KeyFrame[playIndex].mesaPosXInc * direccion;
            mesaPosY += KeyFrame[playIndex].mesaPosYInc * direccion;
            mesaPosZ += KeyFrame[playIndex].mesaPosZInc * direccion;
            mesaPata1Rot += KeyFrame[playIndex].mesaPata1RotInc * direccion;
            mesaPata2Rot += KeyFrame[playIndex].mesaPata2RotInc * direccion;
            mesaTablonRotX += KeyFrame[playIndex].mesaTablonRotXInc * direccion;
            tuboPata1Rot += KeyFrame[playIndex].tuboPata1RotInc * direccion;

            // Stand Octanorm
            standHoriz += KeyFrame[playIndex].standHorizInc * direccion;
            standBase += KeyFrame[playIndex].standBaseInc * direccion;
            panel += KeyFrame[playIndex].panelInc * direccion;

            i_curr_steps++;
        }
    }
}