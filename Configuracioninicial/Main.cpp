// =====================================================================
//  PROYECTO: Planta Base - Facultad
//  Implementación: Sistema Día/Noche + Reflector Interactivo (Tecla F)
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

// ---- Ventana --------------------------------------------------------
const GLuint WIDTH = 800, HEIGHT = 600;
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
// SISTEMA DE HORAS DEL DIA (INTENSIDADES REALISTAS)
// =====================================================================
struct TimeOfDayPreset {
    const char* name;
    glm::vec3   clearColor;
    glm::vec3   tint;         // Intensidad de la luz general
};

TimeOfDayPreset presets[3] = {
    // 1 (Mañana): Cielo amanecer, luz al ~65%
    { "Manana", glm::vec3(0.87f, 0.72f, 0.62f), glm::vec3(0.65f, 0.55f, 0.45f) },

    // 2 (Tarde): Cielo azul, luz al 100%
    { "Tarde",  glm::vec3(0.50f, 0.70f, 0.92f), glm::vec3(1.00f, 1.00f, 1.00f) },

    // 3 (Noche): Cielo oscuro, luz al ~20%
    { "Noche",  glm::vec3(0.04f, 0.05f, 0.12f), glm::vec3(0.20f, 0.25f, 0.40f) },
};

int currentPreset = 1; // Arranca en Tarde por defecto

// =====================================================================
// PROTOTIPOS Y UTILIDADES
// =====================================================================
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();

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

    // Carga de Shaders (Asegúrate de que tus archivos .vs y .frag estén actualizados)
    Shader shader("Shader/core.vs", "Shader/lamp.frag");
    Shader shader1("Shader/modelLoading.vs", "Shader/modelLoading.frag");

    // Modelos
    Model sotano((char*)"Models/SotanoFI.obj");
    Model reja((char*)"Models/reja.obj");
    Model luzTecho((char*)"Models/luz_techo.obj");

    glm::mat4 projection = glm::perspective(camera.GetZoom(), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

    // Geometría Procedural (Vértices de los edificios y piso)
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
    // PARAMETROS DE LA LUZ DE TECHO (Puedes editar esto para mover el foco)
    // =================================================================
    glm::vec3 luzTechoPos = glm::vec3(0.0f, 5.2f, 0.0f); // Posición en X, Y, Z
    glm::vec3 luzTechoScale = glm::vec3(0.5f);           // Tamaño del modelo
    float     luzTechoRotY = 0.0f;                       // Rotación

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

        const TimeOfDayPreset& P = presets[currentPreset];

        glClearColor(P.clearColor.r, P.clearColor.g, P.clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 proj = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

        // --- FUNCION AUXILIAR PARA ENVIAR DATOS DEL REFLECTOR A LOS SHADERS ---
        auto setSpotlightUniforms = [&](GLuint programID) {
            glUniform1i(glGetUniformLocation(programID, "spotLightOn"), spotLightOn);
            glUniform3f(glGetUniformLocation(programID, "spotLightPos"), luzTechoPos.x, luzTechoPos.y, luzTechoPos.z);
            glUniform3f(glGetUniformLocation(programID, "spotLightDir"), 0.0f, -1.0f, 0.0f); // Apunta hacia abajo (-Y)

            // 1. CONO MÁS GRANDE: Aumentamos los grados de apertura (antes 15 y 25)
            glUniform1f(glGetUniformLocation(programID, "spotCutOff"), glm::cos(glm::radians(30.0f))); // Círculo interno de luz pura
            glUniform1f(glGetUniformLocation(programID, "spotOuterCutOff"), glm::cos(glm::radians(45.0f))); // Borde donde se difumina la luz

            // 2. LUZ MÁS SUAVE: Reducimos los valores RGB casi a la mitad (antes 1.0, 1.0, 0.85)
            glUniform3f(glGetUniformLocation(programID, "spotLightColor"), 0.5f, 0.5f, 0.42f); // Luz cálida más tenue
            };

        // -------------------------------------------------------------
        // DIBUJO 1: GEOMETRIA DE LA FACULTAD (Piso, Paredes, Columnas)
        // -------------------------------------------------------------
        shader.Use();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

        // Enviamos la información del reflector al shader de geometría (lamp.frag)
        setSpotlightUniforms(shader.Program);

        glm::mat4 I(1);
        auto setModel = [&](const glm::mat4& m) { glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m)); };

        // Multiplica el color de la geometría por el tinte del día/noche actual
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
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        // Enviamos la intensidad general del día/noche (tintColor)
        GLint modelTintLoc = glGetUniformLocation(shader1.Program, "tintColor");
        glUniform3f(modelTintLoc, P.tint.r, P.tint.g, P.tint.b);

        // Enviamos la información del reflector al shader de los modelos (modelLoading.frag)
        setSpotlightUniforms(shader1.Program);

        // Sotano
        glm::mat4 model(1);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        sotano.Draw(shader1);

        // Reja
        model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(5.0f, 0.0f, 13.85f));
        model = glm::scale(model, glm::vec3(0.11f, 0.097f, 0.12f));
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        reja.Draw(shader1);

        // Luz de techo (Dibuja el modelo de la lámpara en la posición designada)
        model = glm::mat4(1);
        model = glm::translate(model, luzTechoPos);
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(luzTechoRotY), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, luzTechoScale);
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        luzTecho.Draw(shader1);

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

    // --- NUEVO CONTROL: Tecla F para apagar/encender el Reflector ---
    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        spotLightOn = !spotLightOn;
    }

    // Controles para cambiar el ciclo de dia
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) currentPreset = 0; // Mañana
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) currentPreset = 1; // Tarde
    if (key == GLFW_KEY_3 && action == GLFW_PRESS) currentPreset = 2; // Noche

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)        keys[key] = true;
        else if (action == GLFW_RELEASE) keys[key] = false;
    }
}