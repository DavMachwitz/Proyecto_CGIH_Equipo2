// =====================================================================
// =====================================================================
// Planta Base - Facultad
// 
// Galindo Granados Abner Alejandro
// 320001567
// 
// Nava Benitez David Emilio
// 320291599
// 
// Ugalde Santos Atzin
// 319057399
// 
//  Implementacion: Sistema Dia/Noche + Reflectores Multiples (Tecla F)
//                  + Animacion KeyFrames (Tecla P) - 5 frames
//                  + Personas con esqueleto animado
//                  + MODO FIESTA (Tecla G): luces de colores + musica
//                  + Stand Complejo (FBX con animacion por nodos)
//                  + Doble set de stands (silla 2, mesa 2, octanorm 2)
//                  + Expositor que saluda por proximidad
//                  + SKYBOX (cubemap sensible al dia/tarde/noche)
//
//  CONTROLES:
//      W/A/S/D     : moverse (horizontal, requiere fix en Camera.h)
//      M / ESC     : capturar / liberar mouse
//      1 / 2 / 3   : cambiar hora del dia
//      F           : encender / apagar reflectores
//      P           : reproducir animacion (armar / desarmar mobiliario)
//      L           : mostrar / ocultar stands
//      G           : MODO FIESTA (luces de colores + musica)
// =====================================================================
// =====================================================================


// =====================================================================
// 1) INCLUDES
// =====================================================================
#include <iostream>
#include <vector>
#define _USE_MATH_DEFINES
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "ModelAnimation.h"
#include "Animator.h"
#include "NodeAnimation.h"
#include "NodeAnimator.h"
#include "Texture.h"            // <-- agregado: usa TextureLoading::LoadCubemap

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// ---- Audio (Windows nativo, no requiere instalar nada) --------------
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")


// =====================================================================
// 2) VARIABLES GLOBALES
// =====================================================================

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

// ---- Estado del reflector -------------------------------------------
bool    spotLightOn = false;

// ---- Expositor Stand (Saludo por proximidad) ------------------------
glm::vec3 posExpositorStand = glm::vec3(9.15f, 0.38f, 2.9f);
float     radioDeteccion = 8.0f;


// =====================================================================
// 2.b) MODO FIESTA (luces de colores + musica)
// =====================================================================
bool  fiestaMode = false;
float fiestaTimer = 0.0f;
int   fiestaOffset = 0;
const float FIESTA_INTERVAL = 0.4f;

// Paleta de 15 colores UNICOS (uno por cada spotlight)
glm::vec3 fiestaColors[15] = {
    glm::vec3(1.00f, 0.00f, 0.00f),  // Rojo
    glm::vec3(0.00f, 1.00f, 0.00f),  // Verde
    glm::vec3(0.00f, 0.40f, 1.00f),  // Azul
    glm::vec3(1.00f, 0.00f, 1.00f),  // Magenta
    glm::vec3(0.00f, 1.00f, 1.00f),  // Cian
    glm::vec3(1.00f, 1.00f, 0.00f),  // Amarillo
    glm::vec3(1.00f, 0.50f, 0.00f),  // Naranja
    glm::vec3(0.55f, 0.00f, 1.00f),  // Violeta
    glm::vec3(0.60f, 1.00f, 0.00f),  // Lima
    glm::vec3(1.00f, 0.00f, 0.50f),  // Fucsia
    glm::vec3(0.00f, 0.85f, 0.65f),  // Turquesa
    glm::vec3(1.00f, 0.85f, 0.00f),  // Dorado
    glm::vec3(1.00f, 0.40f, 0.40f),  // Coral
    glm::vec3(0.00f, 1.00f, 0.70f),  // Aqua-verde
    glm::vec3(0.70f, 0.00f, 0.90f),  // Purpura
};

const glm::vec3 colorLuzNormal = glm::vec3(0.5f, 0.5f, 0.42f);


// =====================================================================
// 2.c) SKYBOX (cubemap envolvente)
// =====================================================================
//  36 vertices del cubo (sin texturas ni normales: solo posiciones)
//  El cubemap se mapea automaticamente por la direccion del vector.
// =====================================================================
GLfloat skyboxVertices[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

GLuint skyboxIndices[] = {
    0,  1,  2,  3,
    4,  5,  6,  7,
    8,  9, 10, 11,
   12, 13, 14, 15,
   16, 17, 18, 19,
   20, 21, 22, 23,
   24, 25, 26, 27,
   28, 29, 30, 31,
   32, 33, 34, 35
};


// =====================================================================
// 3) SISTEMA DE HORAS DEL DIA
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
// 4) ANIMACION POR KEYFRAMES (variables globales y struct)
// =====================================================================

// ---- Silla ----------------------------------------------------------
float sillaAsientoRot = 66.0f;
float sillaPatasRot = -60.0f;
float sillaPosX = 6.1088f;
float sillaPosY = 0.0f;
float sillaPosZ = 7.6101f;

// ---- Mesa -----------------------------------------------------------
float mesaPosX = 6.0003f;
float mesaPosY = -1.0f;
float mesaPosZ = 8.10612f;
float mesaTablonRotX = 0.0f;
float mesaPata1Rot = -90.0f;
float mesaPata2Rot = 90.0f;
float tuboPata1Rot = 45.0f;

// ---- Stand Octanorm -------------------------------------------------
float standHoriz = 1.4481f;
float standBase = -3.7086f;
float panel = -6.0f;

// ---- Stand Complejo (animacion FBX por nodos) -----------------------
float          standAnimTime = 0.0f;
NodeAnimation* globalStandAnimPtr = nullptr;
float          rotY = -90.0f;

// ---- Personas (movimiento manual, no por keyframes) -----------------
glm::vec3 personaPos = glm::vec3(0.0f, 0.2f, -65.0f);
glm::vec3 personaPos2 = glm::vec3(2.0f, 0.2f, 65.0f);
float     personaSpeed = 1.0f;
float     personaDirZ = 1.0f;
float     personaRotY = 0.0f;
float     persona2DirZ = -1.0f;
float     persona2RotY = 180.0f;

// ---- KeyFrames ------------------------------------------------------
#define MAX_FRAMES 9
int i_max_steps = 200;
int i_curr_steps = 0;

typedef struct _frame {
    float sillaPosX, sillaPosY, sillaPosZ;
    float sillaAsientoRot, sillaPatasRot;
    float mesaPosX, mesaPosY, mesaPosZ;
    float mesaPata1Rot, mesaPata2Rot, tuboPata1Rot, mesaTablonRotX;
    float standHoriz, standBase, panel;
    float sillaAsientoInc, sillaPatasInc;
    float sillaPosXInc, sillaPosYInc, sillaPosZInc;
    float mesaPosXInc, mesaPosYInc, mesaPosZInc;
    float mesaPata1RotInc, mesaPata2RotInc, tuboPata1RotInc, mesaTablonRotXInc;
    float standHorizInc, standBaseInc, panelInc;
} FRAME;

FRAME KeyFrame[MAX_FRAMES];
int   FrameIndex = 0;
bool  play = false;
int   playIndex = 0;
int   direccion = 1;
bool  mostrarStands = true;
bool  standsArmados = false;


// =====================================================================
// 5) PROTOTIPOS Y UTILIDADES
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

void interpolation(void)
{
    KeyFrame[playIndex].sillaPosXInc = (KeyFrame[playIndex + 1].sillaPosX - KeyFrame[playIndex].sillaPosX) / i_max_steps;
    KeyFrame[playIndex].sillaPosYInc = (KeyFrame[playIndex + 1].sillaPosY - KeyFrame[playIndex].sillaPosY) / i_max_steps;
    KeyFrame[playIndex].sillaPosZInc = (KeyFrame[playIndex + 1].sillaPosZ - KeyFrame[playIndex].sillaPosZ) / i_max_steps;
    KeyFrame[playIndex].sillaAsientoInc = (KeyFrame[playIndex + 1].sillaAsientoRot - KeyFrame[playIndex].sillaAsientoRot) / i_max_steps;
    KeyFrame[playIndex].sillaPatasInc = (KeyFrame[playIndex + 1].sillaPatasRot - KeyFrame[playIndex].sillaPatasRot) / i_max_steps;
    KeyFrame[playIndex].mesaPosXInc = (KeyFrame[playIndex + 1].mesaPosX - KeyFrame[playIndex].mesaPosX) / i_max_steps;
    KeyFrame[playIndex].mesaPosYInc = (KeyFrame[playIndex + 1].mesaPosY - KeyFrame[playIndex].mesaPosY) / i_max_steps;
    KeyFrame[playIndex].mesaPosZInc = (KeyFrame[playIndex + 1].mesaPosZ - KeyFrame[playIndex].mesaPosZ) / i_max_steps;
    KeyFrame[playIndex].mesaPata1RotInc = (KeyFrame[playIndex + 1].mesaPata1Rot - KeyFrame[playIndex].mesaPata1Rot) / i_max_steps;
    KeyFrame[playIndex].mesaTablonRotXInc = (KeyFrame[playIndex + 1].mesaTablonRotX - KeyFrame[playIndex].mesaTablonRotX) / i_max_steps;
    KeyFrame[playIndex].mesaPata2RotInc = (KeyFrame[playIndex + 1].mesaPata2Rot - KeyFrame[playIndex].mesaPata2Rot) / i_max_steps;
    KeyFrame[playIndex].tuboPata1RotInc = (KeyFrame[playIndex + 1].tuboPata1Rot - KeyFrame[playIndex].tuboPata1Rot) / i_max_steps;
    KeyFrame[playIndex].standHorizInc = (KeyFrame[playIndex + 1].standHoriz - KeyFrame[playIndex].standHoriz) / i_max_steps;
    KeyFrame[playIndex].standBaseInc = (KeyFrame[playIndex + 1].standBase - KeyFrame[playIndex].standBase) / i_max_steps;
    KeyFrame[playIndex].panelInc = (KeyFrame[playIndex + 1].panel - KeyFrame[playIndex].panel) / i_max_steps;
}


// =====================================================================
// =====================================================================
//  MAIN
// =====================================================================
// =====================================================================
int main()
{
    // -----------------------------------------------------------------
    // 6) INICIALIZACION GLFW / GLEW
    // -----------------------------------------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Equipo2_ProyectoFinal", nullptr, nullptr);
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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    // -----------------------------------------------------------------
    // 7) SHADERS
    // -----------------------------------------------------------------
    Shader shader("Shader/core.vs", "Shader/lamp.frag");
    Shader shader1("Shader/modelLoading.vs", "Shader/modelLoading.frag");
    Shader shader2("Shader/lamp.vs", "Shader/modelLoading.frag");
    Shader skyboxShader("Shader/SkyBox.vs", "Shader/SkyBox.frag");


    // -----------------------------------------------------------------
    // 8) CARGA DE MODELOS
    // -----------------------------------------------------------------

    // ---- Edificio base ----------------------------------------------
    Model sotano((char*)"Models/SotanoFI.obj");
    Model reja((char*)"Models/reja.obj");
    Model luzTecho((char*)"Models/luz_techo.obj");
    Model upstairs((char*)"Models/Upstairs.obj");

    // ---- Fondos e Instalaciones Exteriores --------------------------
    Model fondo((char*)"Models/Fondo_Final.obj");
    Model fondoPuente((char*)"Models/fondo_puente.obj");
    Model fondoCorredor((char*)"Models/fondo_corredor.obj");
    Model pisoFuera((char*)"Models/piso_fuera.obj");

    // ---- Estatuas ---------------------------------------------------
    Model estatua1((char*)"Models/EstatuaEscalera.obj");
    Model base1((char*)"Models/BaseEstatuaEscalera.obj");
    Model estatua2((char*)"Models/EstatuaExamenes.obj");
    Model base2((char*)"Models/BaseEstatuaExamenes.obj");

    // ---- Cristal (transparencia) ------------------------------------
    Model cristal((char*)"Models/Cristal/Cristal.obj");

    // ---- Stands: Silla ----------------------------------------------
    Model sillaMarco((char*)"Models/Stands/Chair/Silla_Marco.obj");
    Model sillaAsiento((char*)"Models/Stands/Chair/Silla_Asiento.obj");
    Model sillaPatas((char*)"Models/Stands/Chair/Silla_patas.obj");

    // ---- Stands: Mesa -----------------------------------------------
    Model tablon((char*)"Models/Stands/Table/tablon.obj");
    Model p1((char*)"Models/Stands/Table/pata1.obj");
    Model p2((char*)"Models/Stands/Table/pata2.obj");

    // ---- Stands: Octanorm -------------------------------------------
    Model base((char*)"Models/Stands/Octanorm/base_stand.obj");
    Model trave((char*)"Models/Stands/Octanorm/trave_stand.obj");
    Model panel1((char*)"Models/Stands/Octanorm/panel1.obj");
    Model panel2((char*)"Models/Stands/Octanorm/panel2.obj");
    Model panel3((char*)"Models/Stands/Octanorm/panel3.obj");
    Model panel4((char*)"Models/Stands/Octanorm/panel4.obj");

    // ---- Stand Complejo (FBX con animacion por nodos) ---------------
    Model         standComplejo((char*)"Models/Stands/Complex1/StandComplex1.fbx");
    NodeAnimation standAnimNode("Models/Stands/Complex1/StandComplex1.fbx");
    globalStandAnimPtr = &standAnimNode;
    NodeAnimator  standAnimatorNode(&standAnimNode);

    // ---- Personas (con esqueleto) -----------------------------------
    Model persona1((char*)"Models/People/person1.dae");
    Model persona2((char*)"Models/People/person2.dae");
    Model personaSaludo((char*)"Models/People/personWaving.fbx");

    ModelAnimation danceAnim("Models/People/person1.dae", persona1.GetBoneInfoMap(), persona1.GetBoneCount());
    Animator       animator(&danceAnim);
    ModelAnimation danceAnim2("Models/People/person2.dae", persona2.GetBoneInfoMap(), persona2.GetBoneCount());
    Animator       animator2(&danceAnim2);
    ModelAnimation wavingAnim("Models/People/personWaving.fbx", personaSaludo.GetBoneInfoMap(), personaSaludo.GetBoneCount());
    Animator       animatorSaludo(&wavingAnim);
    animatorSaludo.UpdateAnimation(0.0f);


    // -----------------------------------------------------------------
    // 8.b) SETUP DEL SKYBOX (VAO/VBO/EBO + carga del cubemap)
    //  Usa TextureLoading::LoadCubemap de Texture.h (con stb_image)
    // -----------------------------------------------------------------
    GLuint skyboxVAO, skyboxVBO, skyboxEBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glGenBuffers(1, &skyboxEBO);

    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glBindVertexArray(0);

    // Carga de las 6 caras (orden CRITICO: right, left, top, bottom, back, front)
    vector<const GLchar*> faces;
    faces.push_back("SkyBox/right.jpg");
    faces.push_back("SkyBox/left.jpg");
    faces.push_back("SkyBox/top.jpg");
    faces.push_back("SkyBox/bottom.jpg");
    faces.push_back("SkyBox/back.jpg");
    faces.push_back("SkyBox/front.jpg");

    // Llama a la función LoadCubemap
    GLuint cubemapTexture = TextureLoading::LoadCubemap(faces);

    // Vincular la textura del cubemap al sampler "skybox" en el shader
    skyboxShader.Use();
    glUniform1i(glGetUniformLocation(skyboxShader.Program, "skybox"), 0);





    // -----------------------------------------------------------------
    // 9) DEFINICION DE KEYFRAMES (5 frames - estados de la animacion)
    // -----------------------------------------------------------------
    // FRAME 0: ESTADO INICIAL (todo armado)
    KeyFrame[0].sillaPosX = 5.968f;   KeyFrame[0].sillaPosY = 0.0f;     KeyFrame[0].sillaPosZ = 7.0114f;
    KeyFrame[0].sillaAsientoRot = 66.0f; KeyFrame[0].sillaPatasRot = -60.0f;
    KeyFrame[0].mesaPosX = 6.0003f;   KeyFrame[0].mesaPosY = -1.0f;     KeyFrame[0].mesaPosZ = 8.10612f;
    KeyFrame[0].mesaPata1Rot = -90.0f; KeyFrame[0].mesaPata2Rot = 90.0f;
    KeyFrame[0].mesaTablonRotX = 0.0f; KeyFrame[0].tuboPata1Rot = 45.0f;
    KeyFrame[0].standHoriz = 1.4481f; KeyFrame[0].standBase = -3.7086f; KeyFrame[0].panel = -6.0f;

    // FRAME 1: silla y mesa abriendose (mesa a la mitad)
    KeyFrame[1].sillaPosX = 5.968f;   KeyFrame[1].sillaPosY = 0.0f;     KeyFrame[1].sillaPosZ = 7.0114f;
    KeyFrame[1].sillaAsientoRot = 0.0f; KeyFrame[1].sillaPatasRot = 0.0f;
    KeyFrame[1].mesaPosX = 6.0003f;   KeyFrame[1].mesaPosY = 0.0f;      KeyFrame[1].mesaPosZ = 8.10612f;
    KeyFrame[1].mesaPata1Rot = -45.0f; KeyFrame[1].mesaPata2Rot = 45.0f;
    KeyFrame[1].tuboPata1Rot = 45.0f; KeyFrame[1].mesaTablonRotX = 25.0f;
    KeyFrame[1].standHoriz = 1.4481f; KeyFrame[1].standBase = -3.7086f; KeyFrame[1].panel = -6.0f;

    // FRAME 2: mesa completamente plegada
    KeyFrame[2].sillaPosX = 5.968f;   KeyFrame[2].sillaPosY = 0.0f;     KeyFrame[2].sillaPosZ = 7.0114f;
    KeyFrame[2].sillaAsientoRot = 0.0f; KeyFrame[2].sillaPatasRot = 0.0f;
    KeyFrame[2].mesaPosX = 6.0003f;   KeyFrame[2].mesaPosY = 0.0f;      KeyFrame[2].mesaPosZ = 8.10612f;
    KeyFrame[2].mesaPata1Rot = 0.0f;  KeyFrame[2].mesaPata2Rot = 0.0f;
    KeyFrame[2].tuboPata1Rot = 0.0f;  KeyFrame[2].mesaTablonRotX = 0.0f;
    KeyFrame[2].standHoriz = 1.4481f; KeyFrame[2].standBase = -3.7086f; KeyFrame[2].panel = -6.0f;

    // FRAME 3: silla y mesa al origen, stand SIGUE abierto
    KeyFrame[3].sillaPosX = 0.0f;     KeyFrame[3].sillaPosY = 0.0f;     KeyFrame[3].sillaPosZ = 0.0f;
    KeyFrame[3].sillaAsientoRot = 0.0f; KeyFrame[3].sillaPatasRot = 0.0f;
    KeyFrame[3].mesaPosX = 0.0f;      KeyFrame[3].mesaPosY = 0.0f;      KeyFrame[3].mesaPosZ = 0.0f;
    KeyFrame[3].mesaPata1Rot = 0.0f;  KeyFrame[3].mesaPata2Rot = 0.0f;
    KeyFrame[3].tuboPata1Rot = 0.0f;  KeyFrame[3].mesaTablonRotX = 0.0f;
    KeyFrame[3].standHoriz = 1.4481f; KeyFrame[3].standBase = -3.7086f; KeyFrame[3].panel = -6.0f;

    // FRAME 4: ESTADO FINAL (stand cerrado, animacion FBX se completa aqui)
    KeyFrame[4].sillaPosX = 0.0f;     KeyFrame[4].sillaPosY = 0.0f;     KeyFrame[4].sillaPosZ = 0.0f;
    KeyFrame[4].sillaAsientoRot = 0.0f; KeyFrame[4].sillaPatasRot = 0.0f;
    KeyFrame[4].mesaPosX = 0.0f;      KeyFrame[4].mesaPosY = 0.0f;      KeyFrame[4].mesaPosZ = 0.0f;
    KeyFrame[4].mesaPata1Rot = 0.0f;  KeyFrame[4].mesaPata2Rot = 0.0f;
    KeyFrame[4].tuboPata1Rot = 0.0f;  KeyFrame[4].mesaTablonRotX = 0.0f;
    KeyFrame[4].standHoriz = 0.0f;    KeyFrame[4].standBase = 0.0f;     KeyFrame[4].panel = 0.0f;

    FrameIndex = 5;


    // -----------------------------------------------------------------
    // 10) GEOMETRIA PROCEDURAL DE LA PLANTA
    // -----------------------------------------------------------------
    glm::mat4 projection = glm::perspective(camera.GetZoom(), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

    GLfloat pisoV[] = { mX(0.0f), 0.000f, mZ(0.0f), mX(42.5f), 0.000f, mZ(0.0f), mX(42.5f), 0.000f, mZ(27.0f), mX(0.0f), 0.000f, mZ(27.0f) };
    GLuint  pisoI[] = { 0,1,2, 0,2,3 };
    GLfloat narV[] = { mX(8.5f), 0.002f, mZ(9.0f), mX(17.0f), 0.002f, mZ(9.0f), mX(17.0f), 0.002f, mZ(18.0f), mX(8.5f), 0.002f, mZ(18.0f) };
    GLuint  narI[] = { 0,1,2, 0,2,3 };
    GLfloat azulV[] = { mX(34.0f), 0.002f, mZ(9.0f), mX(42.5f), 0.002f, mZ(9.0f), mX(42.5f), 0.002f, mZ(18.0f), mX(34.0f), 0.002f, mZ(18.0f) };
    GLuint  azulI[] = { 0,1,2, 0,2,3 };
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


    // -----------------------------------------------------------------
    // 11) PARAMETROS DE LAS LUCES DE TECHO
    // -----------------------------------------------------------------
    glm::vec3 luzTechoPos = glm::vec3(0.0f, 5.2f, 10.0f);
    glm::vec3 luzTechoScale = glm::vec3(0.5f);
    float     luzTechoRotY = 0.0f;

    float luzPosX[3] = { -10.0f, 0.0f, 10.0f };
    float luzPosZ[2] = { 10.0f, 0.0f };

    std::vector<glm::vec3> lucesExtra = {
        glm::vec3(18.0f, 4.5f,   0.0f),
        glm::vec3(18.0f, 4.5f,   5.0f),
        glm::vec3(-19.0f, 5.4f,   1.0f),
        glm::vec3(-19.0f, 5.2f,  10.0f),
        glm::vec3(-19.0f, 5.5f, -10.0f),
        glm::vec3(-10.0f, 5.5f, -10.0f),
        glm::vec3(0.0f, 5.5f, -10.0f),
        glm::vec3(10.0f, 5.5f, -10.0f),
        glm::vec3(17.0f, 5.5f, -10.0f),
    };

    const int NUM_SPOTLIGHTS = 6 + (int)lucesExtra.size();


    // -----------------------------------------------------------------
    // 12) PARAMETROS DE LOS MODELOS DE FONDO
    // -----------------------------------------------------------------
    glm::vec3 fondoPos = glm::vec3(-3.0f, 0.0f, -13.4f);
    glm::vec3 fondoScale = glm::vec3(0.15f, 0.14f, 0.14f);
    float     fondoRotY = 0.0f;

    glm::vec3 fondoPuentePos = glm::vec3(14.0f, 0.0f, -13.4f);
    glm::vec3 fondoPuenteScale = glm::vec3(0.22f, 0.19f, 0.19f);
    float     fondoPuenteRotY = 0.0f;

    glm::vec3 fondoCorredorPos = glm::vec3(21.0f, 0.0f, -8.9f);
    glm::vec3 fondoCorredorScale = glm::vec3(0.61f, 0.24f, 0.24f);
    float     fondoCorredorRotY = 0.0f;

    // --- NUEVO PISO EXTERIOR ---
    glm::vec3 pisoFueraPos = glm::vec3(-50.0f, 0.7f, 20.0f);
    glm::vec3 pisoFueraScale = glm::vec3(1.5f, 1.5f, 1.5f);
    float     pisoFueraRotY = 0.0f;


    // =================================================================
    // 13) RENDER LOOP
    // =================================================================
    while (!glfwWindowShouldClose(window))
    {
        // ---- a) Tiempo, input, animacion ----------------------------
        GLfloat cur = (GLfloat)glfwGetTime();
        deltaTime = cur - lastFrame;
        lastFrame = cur;

        glfwPollEvents();
        DoMovement();
        Animation();

        // ---- a.1) ACTUALIZAR MODO FIESTA (rotacion de colores) ------
        if (fiestaMode) {
            fiestaTimer += deltaTime;
            if (fiestaTimer >= FIESTA_INTERVAL) {
                fiestaTimer = 0.0f;
                fiestaOffset = (fiestaOffset + 1) % 15;
            }
        }

        // ---- b) Preset de hora del dia ------------------------------
        const TimeOfDayPreset& P = presets[currentPreset];
        glClearColor(P.clearColor.r, P.clearColor.g, P.clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ---- c) Matrices de vista y proyeccion ----------------------
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 proj = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

        // ---- d) Posiciones de los 15 spotlights ---------------------
        std::vector<glm::vec3> spotPositions;
        spotPositions.reserve(NUM_SPOTLIGHTS);
        for (int row = 0; row < 2; row++)
            for (int col = 0; col < 3; col++)
                spotPositions.push_back(glm::vec3(luzPosX[col], luzTechoPos.y, luzPosZ[row]));
        for (const auto& p : lucesExtra) spotPositions.push_back(p);

        // ---- d.1) COLORES DE CADA SPOTLIGHT (modo fiesta o normal) --
        std::vector<glm::vec3> spotColors;
        spotColors.reserve(NUM_SPOTLIGHTS);
        for (int i = 0; i < NUM_SPOTLIGHTS; i++) {
            if (fiestaMode) {
                spotColors.push_back(fiestaColors[(i + fiestaOffset) % 15]);
            }
            else {
                spotColors.push_back(colorLuzNormal);
            }
        }

        auto setSpotlightUniforms = [&](GLuint programID) {
            glUniform1i(glGetUniformLocation(programID, "spotLightOn"), spotLightOn);
            glUniform1i(glGetUniformLocation(programID, "numSpotLights"), NUM_SPOTLIGHTS);
            glUniform3fv(glGetUniformLocation(programID, "spotLightPos"), NUM_SPOTLIGHTS, glm::value_ptr(spotPositions[0]));
            glUniform3fv(glGetUniformLocation(programID, "spotLightColor"), NUM_SPOTLIGHTS, glm::value_ptr(spotColors[0]));
            glUniform3f(glGetUniformLocation(programID, "spotLightDir"), 0.0f, -1.0f, 0.0f);
            glUniform1f(glGetUniformLocation(programID, "spotCutOff"), glm::cos(glm::radians(30.0f)));
            glUniform1f(glGetUniformLocation(programID, "spotOuterCutOff"), glm::cos(glm::radians(45.0f)));
            };

        // =============================================================
        //   DIBUJO 1: GEOMETRIA PROCEDURAL
        // =============================================================
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
        setColorTinted(0.97f, 0.78f, 0.42f); setModel(I); glBindVertexArray(VAO_nar);  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        setColorTinted(0.40f, 0.65f, 0.90f); setModel(I); glBindVertexArray(VAO_azul); glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glLineWidth(3.5f); setColorTinted(0.0f, 0.0f, 0.0f); setModel(I); glBindVertexArray(VAO_perim); glDrawArrays(GL_LINE_LOOP, 0, 4);
        glLineWidth(3.5f); setColorTinted(0.0f, 0.0f, 0.0f); setModel(I); glBindVertexArray(VAO_line);  glDrawArrays(GL_LINES, 0, 2);

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

        // =============================================================
        //   DIBUJO 2: MODELOS .OBJ
        // =============================================================
        shader1.Use();
        glUniform1i(glGetUniformLocation(shader1.Program, "useSkinning"), GL_FALSE);
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniform3f(glGetUniformLocation(shader1.Program, "tintColor"), P.tint.r, P.tint.g, P.tint.b);
        setSpotlightUniforms(shader1.Program);

        // ---- Sotano -------------------------------------------------
        glm::mat4 model(1);
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        sotano.Draw(shader1);

        // ---- Reja ---------------------------------------------------
        model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(5.0f, 0.0f, 13.85f));
        model = glm::scale(model, glm::vec3(0.11f, 0.097f, 0.12f));
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        reja.Draw(shader1);

        // ============(   FONDO_FINAL (escala y posicion editables)   )=============
        model = glm::mat4(1);
        model = glm::translate(model, fondoPos);
        model = glm::rotate(model, glm::radians(fondoRotY), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, fondoScale);
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        fondo.Draw(shader1);
        // ============(   FIN FONDO_FINAL   )=======================================

        // ============(   FONDO_PUENTE (escala y posicion editables)   )============
        model = glm::mat4(1);
        model = glm::translate(model, fondoPuentePos);
        model = glm::rotate(model, glm::radians(fondoPuenteRotY), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, fondoPuenteScale);
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        fondoPuente.Draw(shader1);
        // ============(   FIN FONDO_PUENTE   )======================================

        // ============(   FONDO_CORREDOR (escala y posicion editables)   )==========
        model = glm::mat4(1);
        model = glm::translate(model, fondoCorredorPos);
        model = glm::rotate(model, glm::radians(fondoCorredorRotY), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, fondoCorredorScale);
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        fondoCorredor.Draw(shader1);
        // ============(   FIN FONDO_CORREDOR   )====================================

        // ============(   PISO EXTERIOR (escala y posicion editables)   )===========
        model = glm::mat4(1);
        model = glm::translate(model, pisoFueraPos);
        model = glm::rotate(model, glm::radians(pisoFueraRotY), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, pisoFueraScale);
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        pisoFuera.Draw(shader1);
        // ============(   FIN PISO EXTERIOR   )=====================================

        // ---- Estatua escalera ---------------------------------------
        model = glm::mat4(1); model = glm::translate(model, glm::vec3(-3.0f, 0.0f, 2.488f));
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        base1.Draw(shader1);

        model = glm::mat4(1); model = glm::translate(model, glm::vec3(-3.0f, 0.0f, 2.5f));
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        estatua1.Draw(shader1);

        // ---- Estatua examenes ---------------------------------------
        model = glm::mat4(1); model = glm::translate(model, glm::vec3(0.0f, 0.0f, -0.5f));
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        base2.Draw(shader1);

        model = glm::mat4(1); model = glm::translate(model, glm::vec3(0.0f, 0.0f, -0.5f));
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        estatua2.Draw(shader1);

        // =============================================================
        //   DIBUJO 3: PERSONAS CON ESQUELETO ANIMADO
        // =============================================================
        personaPos.z += personaSpeed * personaDirZ * deltaTime;
        personaPos2.z += personaSpeed * persona2DirZ * deltaTime;

        if (personaPos.z > 13.0f) { personaDirZ = -1.0f; personaRotY = 180.0f; }
        if (personaPos.z < -13.0f) { personaDirZ = 1.0f; personaRotY = 0.0f; }
        if (personaPos2.z < -13.0f) { persona2DirZ = 1.0f; persona2RotY = 0.0f; }
        if (personaPos2.z > 13.0f) { persona2DirZ = -1.0f; persona2RotY = 180.0f; }

        glUniform1i(glGetUniformLocation(shader1.Program, "useSkinning"), GL_TRUE);

        animator.UpdateAnimation(deltaTime);
        const auto& boneMatrices = animator.GetFinalBoneMatrices();
        for (int i = 0; i < 100; i++) {
            string uName = "finalBonesMatrices[" + to_string(i) + "]";
            glUniformMatrix4fv(glGetUniformLocation(shader1.Program, uName.c_str()), 1, GL_FALSE, &boneMatrices[i][0][0]);
        }
        glm::mat4 modelPersona = glm::mat4(1.0f);
        modelPersona = glm::translate(modelPersona, personaPos);
        modelPersona = glm::rotate(modelPersona, glm::radians(personaRotY), glm::vec3(0.0f, 1.0f, 0.0f));
        modelPersona = glm::scale(modelPersona, glm::vec3(0.5f));
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelPersona));
        persona1.Draw(shader1);

        animator2.UpdateAnimation(deltaTime);
        const auto& bones2 = animator2.GetFinalBoneMatrices();
        for (int i = 0; i < 100; i++) {
            string uName = "finalBonesMatrices[" + to_string(i) + "]";
            glUniformMatrix4fv(glGetUniformLocation(shader1.Program, uName.c_str()), 1, GL_FALSE, &bones2[i][0][0]);
        }
        glm::mat4 modelPersona2 = glm::mat4(1.0f);
        modelPersona2 = glm::translate(modelPersona2, personaPos2);
        modelPersona2 = glm::rotate(modelPersona2, glm::radians(persona2RotY), glm::vec3(0.0f, 1.0f, 0.0f));
        modelPersona2 = glm::scale(modelPersona2, glm::vec3(1.0f));
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelPersona2));
        persona2.Draw(shader1);

        glUniform1i(glGetUniformLocation(shader1.Program, "useSkinning"), GL_FALSE);

        // =============================================================
        //   DIBUJO 4: STANDS (tecla L oculta)
        // =============================================================
        if (mostrarStands) {

            // ============(   STAND 1: Silla + Mesa + Octanorm   )=================
            glm::mat4 modelSilla1 = glm::mat4(1.0f);
            modelSilla1 = glm::translate(modelSilla1, glm::vec3(sillaPosX, sillaPosY, sillaPosZ));
            glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelSilla1));
            sillaMarco.Draw(shader1);

            glm::mat4 modelAsiento = modelSilla1;
            modelAsiento = glm::translate(modelAsiento, glm::vec3(9.9f, 1.2448f, -1.80f));
            modelAsiento = glm::rotate(modelAsiento, glm::radians(sillaAsientoRot), glm::vec3(0.0f, 0.0f, 1.0f));
            modelAsiento = glm::translate(modelAsiento, glm::vec3(-9.9f, -1.2448f, 1.80f));
            glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelAsiento));
            sillaAsiento.Draw(shader1);

            glm::mat4 modelPatas = modelSilla1;
            modelPatas = glm::translate(modelPatas, glm::vec3(9.76f, 0.96f, -1.25f));
            modelPatas = glm::rotate(modelPatas, glm::radians(sillaPatasRot), glm::vec3(0.0f, 0.0f, 1.0f));
            modelPatas = glm::translate(modelPatas, glm::vec3(-9.76f, -0.96f, 1.25f));
            glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelPatas));
            sillaPatas.Draw(shader1);

            glm::mat4 modelMesa = glm::mat4(1.0f);
            modelMesa = glm::translate(modelMesa, glm::vec3(mesaPosX, mesaPosY, mesaPosZ));
            modelMesa = glm::translate(modelMesa, glm::vec3(8.30f, 1.5f, -1.73f));
            modelMesa = glm::rotate(modelMesa, glm::radians(mesaTablonRotX), glm::vec3(1.0f, 0.0f, 0.0f));
            modelMesa = glm::translate(modelMesa, glm::vec3(-8.30f, -1.5f, 1.73f));
            glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelMesa));
            tablon.Draw(shader1);

            glm::mat4 modelPata1 = modelMesa;
            modelPata1 = glm::translate(modelPata1, glm::vec3(8.30f, 1.5f, -1.73f));
            modelPata1 = glm::rotate(modelPata1, glm::radians(mesaPata1Rot), glm::vec3(1.0f, 0.0f, 0.0f));
            modelPata1 = glm::translate(modelPata1, glm::vec3(-8.30f, -1.5f, 1.73f));
            glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelPata1));
            p1.Draw(shader1);

            glm::mat4 modelPata2 = modelMesa;
            modelPata2 = glm::translate(modelPata2, glm::vec3(8.49f, 1.45f, 0.102f));
            modelPata2 = glm::rotate(modelPata2, glm::radians(mesaPata2Rot), glm::vec3(1.0f, 0.0f, 0.0f));
            modelPata2 = glm::translate(modelPata2, glm::vec3(-8.49f, -1.45f, -0.102f));
            glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelPata2));
            p2.Draw(shader1);

            if (standsArmados) {
                model = glm::mat4(1); model = glm::translate(model, glm::vec3(0.0f, standBase, 0.0f));
                glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
                base.Draw(shader1);

                model = glm::mat4(1); model = glm::translate(model, glm::vec3(0.0f, standHoriz, 0.0f));
                glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
                trave.Draw(shader1);

                model = glm::mat4(1); model = glm::translate(model, glm::vec3(0.0f, panel, 0.0f));
                glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
                panel1.Draw(shader1);

                model = glm::mat4(1); model = glm::translate(model, glm::vec3(0.0f, panel, 0.0f));
                glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
                panel2.Draw(shader1);

                model = glm::mat4(1); model = glm::translate(model, glm::vec3(0.0f, panel, 0.0f));
                glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
                panel3.Draw(shader1);

                model = glm::mat4(1); model = glm::translate(model, glm::vec3(0.0f, panel, 0.0f));
                glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
                panel4.Draw(shader1);
            }

            // ============(   STAND 2: Silla + Mesa + Octanorm (offset Z)   )======
            glm::mat4 modelSilla2 = glm::mat4(1.0f);
            modelSilla2 = glm::translate(modelSilla2, glm::vec3(sillaPosX, sillaPosY, sillaPosZ + 4.7f));
            glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelSilla2));
            sillaMarco.Draw(shader1);

            glm::mat4 modelAsiento2 = modelSilla2;
            modelAsiento2 = glm::translate(modelAsiento2, glm::vec3(9.9f, 1.2448f, -1.80f));
            modelAsiento2 = glm::rotate(modelAsiento2, glm::radians(sillaAsientoRot), glm::vec3(0.0f, 0.0f, 1.0f));
            modelAsiento2 = glm::translate(modelAsiento2, glm::vec3(-9.9f, -1.2448f, 1.80f));
            glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelAsiento2));
            sillaAsiento.Draw(shader1);

            glm::mat4 modelPatas2 = modelSilla2;
            modelPatas2 = glm::translate(modelPatas2, glm::vec3(9.76f, 0.96f, -1.25f));
            modelPatas2 = glm::rotate(modelPatas2, glm::radians(sillaPatasRot), glm::vec3(0.0f, 0.0f, 1.0f));
            modelPatas2 = glm::translate(modelPatas2, glm::vec3(-9.76f, -0.96f, 1.25f));
            glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelPatas2));
            sillaPatas.Draw(shader1);

            glm::mat4 modelMesa2 = glm::mat4(1.0f);
            modelMesa2 = glm::translate(modelMesa2, glm::vec3(mesaPosX, mesaPosY, mesaPosZ + 3.5f));
            modelMesa2 = glm::translate(modelMesa2, glm::vec3(8.30f, 1.5f, -1.73f));
            modelMesa2 = glm::rotate(modelMesa2, glm::radians(mesaTablonRotX), glm::vec3(1.0f, 0.0f, 0.0f));
            modelMesa2 = glm::translate(modelMesa2, glm::vec3(-8.30f, -1.5f, 1.73f));
            glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelMesa2));
            tablon.Draw(shader1);

            glm::mat4 modelPata11 = modelMesa2;
            modelPata11 = glm::translate(modelPata11, glm::vec3(8.30f, 1.5f, -1.73f));
            modelPata11 = glm::rotate(modelPata11, glm::radians(mesaPata1Rot), glm::vec3(1.0f, 0.0f, 0.0f));
            modelPata11 = glm::translate(modelPata11, glm::vec3(-8.30f, -1.5f, 1.73f));
            glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelPata11));
            p1.Draw(shader1);

            glm::mat4 modelPata22 = modelMesa2;
            modelPata22 = glm::translate(modelPata22, glm::vec3(8.49f, 1.45f, 0.102f));
            modelPata22 = glm::rotate(modelPata22, glm::radians(mesaPata2Rot), glm::vec3(1.0f, 0.0f, 0.0f));
            modelPata22 = glm::translate(modelPata22, glm::vec3(-8.49f, -1.45f, -0.102f));
            glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelPata22));
            p2.Draw(shader1);

            if (standsArmados) {
                model = glm::mat4(1); model = glm::translate(model, glm::vec3(0.0f, standBase, 3.5f));
                glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
                base.Draw(shader1);

                model = glm::mat4(1); model = glm::translate(model, glm::vec3(0.0f, standHoriz, 3.5f));
                glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
                trave.Draw(shader1);

                model = glm::mat4(1); model = glm::translate(model, glm::vec3(0.0f, panel, 3.5f));
                glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
                panel1.Draw(shader1);

                model = glm::mat4(1); model = glm::translate(model, glm::vec3(0.0f, panel, 3.5f));
                glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
                panel2.Draw(shader1);

                model = glm::mat4(1); model = glm::translate(model, glm::vec3(0.0f, panel, 3.5f));
                glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
                panel3.Draw(shader1);

                model = glm::mat4(1); model = glm::translate(model, glm::vec3(0.0f, panel, 3.5f));
                glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
                panel4.Draw(shader1);

                // ============(   STAND COMPLEJO (FBX con animacion por nodos)   )=====
                standAnimatorNode.SetTime(standAnimTime);
                auto transforms = standAnimatorNode.GetFinalTransforms();

                shader1.Use();
                glUniform1i(glGetUniformLocation(shader1.Program, "useSkinning"), GL_FALSE);

                glm::mat4 modelStandGeneral = glm::mat4(1.0f);
                standComplejo.DrawNodeAnimation(shader1, transforms, modelStandGeneral);
                // ============(   FIN STAND COMPLEJO   )===============================
            }
        }

        // =============================================================
        //  DIBUJO 5: EXPOSITOR (Saluda si esta armado y cerca)
        // =============================================================
        if (standsArmados && !play && playIndex == 4) {

            float distancia = glm::distance(camera.GetPosition(), posExpositorStand);

            if (distancia < radioDeteccion) {
                animatorSaludo.UpdateAnimation(deltaTime);
            }
            else {
                animatorSaludo.PlayAnimation(&wavingAnim);
                animatorSaludo.UpdateAnimation(0.0f);
            }

            shader1.Use();
            glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
            glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniform1i(glGetUniformLocation(shader1.Program, "useSkinning"), GL_TRUE);

            glm::mat4 modelS = glm::mat4(1.0f);
            modelS = glm::translate(modelS, posExpositorStand);
            modelS = glm::rotate(modelS, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            modelS = glm::scale(modelS, glm::vec3(0.013f));

            glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelS));

            auto transformsS = animatorSaludo.GetFinalBoneMatrices();
            for (int i = 0; i < transformsS.size(); i++) {
                string uName = "finalBonesMatrices[" + to_string(i) + "]";
                glUniformMatrix4fv(glGetUniformLocation(shader1.Program, uName.c_str()), 1, GL_FALSE, &transformsS[i][0][0]);
            }

            personaSaludo.Draw(shader1);
            glUniform1i(glGetUniformLocation(shader1.Program, "useSkinning"), GL_FALSE);
        }

        // ============(   LUCES DE TECHO - CUADRICULA 6   )=========================
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

        // ============(   LUCES EXTRA (9)   )=======================================
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

        // ============(  CRISTAL  )=================================================
        shader1.Use();
        glUniform1i(glGetUniformLocation(shader1.Program, "transparency"), 1);

        model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(16.253f, 2.7417f, -0.14173f));
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        cristal.Draw(shader1);

        model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(16.253f, 2.7417f, 2.015f));
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        cristal.Draw(shader1);

        model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(16.253f, 2.7417f, 3.9932f));
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        cristal.Draw(shader1);

        model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(16.253f, 2.7417f, -2.5471f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.287f));
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        cristal.Draw(shader1);

        model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(17.784f, 2.7417f, -3.9583f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.397f));
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        cristal.Draw(shader1);

        model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(20.172f, 2.7417f, -3.9583f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        cristal.Draw(shader1);

        glUniform1i(glGetUniformLocation(shader1.Program, "transparency"), 0);
        // ============(   FIN CRISTAL   )==========================================

        // =============================================================
        //   DIBUJO 6: SKYBOX (cubemap envolvente, sensible al dia/noche)
        //   IMPORTANTE: se dibuja AL FINAL para que actue como fondo.
        //   - glDepthFunc(GL_LEQUAL): permite que el skybox se vea detras de todo
        //   - mat3(view): quita la traslacion, deja solo la rotacion
        //   - skyTint e intensity: oscurece el skybox de noche,
        //     calido en la manana, neutro en la tarde
        // =============================================================
        glDepthFunc(GL_LEQUAL);
        skyboxShader.Use();

        // Vista sin traslacion (solo rotacion) para que el skybox parezca infinito
        glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(skyboxView));
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(proj));

        // ---- Sensibilidad al dia/tarde/noche: tint del preset actual y la intensidad ----
        // Usamos "skyTint" para coincidir con nuestro skybox.frag
        glUniform3f(glGetUniformLocation(skyboxShader.Program, "skyTint"), P.tint.r, P.tint.g, P.tint.b);

        // Agregamos la "intensity" (Asumiendo que preset == 2 es la noche)
        // Si tienes otra variable que controle la noche, cambiala aquí.
        float currentIntensity = (currentPreset == 2) ? 0.3f : 1.0f;
        glUniform1f(glGetUniformLocation(skyboxShader.Program, "intensity"), currentIntensity);

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glDepthFunc(GL_LESS);   // restaurar para el siguiente frame
        // ============(   FIN SKYBOX   )===========================================

        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }

    // ---- Detener musica al cerrar ----------------------------------
    PlaySoundA(NULL, NULL, 0);

    glfwTerminate();
    return 0;
}


// =====================================================================
// =====================================================================
//  CALLBACKS Y MOVIMIENTO
// =====================================================================
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

    // ---- TECLA G: MODO FIESTA --------------------------------------
    if (key == GLFW_KEY_G && action == GLFW_PRESS) {
        fiestaMode = !fiestaMode;

        if (fiestaMode) {
            spotLightOn = true;
            fiestaTimer = 0.0f;
            fiestaOffset = 0;
            PlaySoundA("Audio/party_1.wav", NULL,
                SND_FILENAME | SND_LOOP | SND_ASYNC | SND_NODEFAULT);
        }
        else {
            PlaySoundA(NULL, NULL, 0);
        }
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)        keys[key] = true;
        else if (action == GLFW_RELEASE) keys[key] = false;
    }

    if (key == GLFW_KEY_P && action == GLFW_PRESS && !play) {
        play = true;
        i_curr_steps = 0;

        if (sillaAsientoRot >= 60.0f) {
            direccion = 1;
            playIndex = 0;
            standsArmados = true;
        }
        else {
            direccion = -1;
            playIndex = FrameIndex - 2;
        }

        interpolation();
    }

    if (key == GLFW_KEY_L && action == GLFW_PRESS) mostrarStands = !mostrarStands;
}

void Animation() {
    if (!play) return;

    if (i_curr_steps >= i_max_steps) {
        if (direccion == 1) {
            if (playIndex < FrameIndex - 1) {
                playIndex++;
                i_curr_steps = 0;
                interpolation();
            }
            else {
                play = false;
            }
        }
        else {
            if (playIndex > 0) {
                playIndex--;
                i_curr_steps = 0;
                interpolation();
            }
            else {
                play = false;
                standsArmados = false;
            }
        }
    }
    else {
        sillaPosX += KeyFrame[playIndex].sillaPosXInc * direccion;
        sillaPosY += KeyFrame[playIndex].sillaPosYInc * direccion;
        sillaPosZ += KeyFrame[playIndex].sillaPosZInc * direccion;
        sillaAsientoRot += KeyFrame[playIndex].sillaAsientoInc * direccion;
        sillaPatasRot += KeyFrame[playIndex].sillaPatasInc * direccion;

        mesaPosX += KeyFrame[playIndex].mesaPosXInc * direccion;
        mesaPosY += KeyFrame[playIndex].mesaPosYInc * direccion;
        mesaPosZ += KeyFrame[playIndex].mesaPosZInc * direccion;
        mesaPata1Rot += KeyFrame[playIndex].mesaPata1RotInc * direccion;
        mesaPata2Rot += KeyFrame[playIndex].mesaPata2RotInc * direccion;
        mesaTablonRotX += KeyFrame[playIndex].mesaTablonRotXInc * direccion;
        tuboPata1Rot += KeyFrame[playIndex].tuboPata1RotInc * direccion;

        standHoriz += KeyFrame[playIndex].standHorizInc * direccion;
        standBase += KeyFrame[playIndex].standBaseInc * direccion;
        panel += KeyFrame[playIndex].panelInc * direccion;

        if (globalStandAnimPtr != nullptr) {
            float duration = globalStandAnimPtr->GetDuration();

            if (playIndex >= 3) {
                float pasosTotalesTramoFinal = (float)i_max_steps * 2.0f;
                float pasosAcumulados = 0.0f;

                float actualSteps = (float)i_curr_steps;
                if (direccion == -1) {
                    actualSteps = (float)i_max_steps - (float)i_curr_steps;
                }

                if (playIndex == 3) {
                    pasosAcumulados = actualSteps;
                }
                else if (playIndex == 4) {
                    pasosAcumulados = (float)i_max_steps + actualSteps;
                }

                standAnimTime = (pasosAcumulados / pasosTotalesTramoFinal) * duration;
            }
            else {
                standAnimTime = 0.0f;
            }

            if (standAnimTime > duration) standAnimTime = duration;
            if (standAnimTime < 0.0f)     standAnimTime = 0.0f;
        }

        i_curr_steps++;
    }
}