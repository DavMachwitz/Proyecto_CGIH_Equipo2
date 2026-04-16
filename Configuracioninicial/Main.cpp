

#include <iostream>
#include <vector>
#define _USE_MATH_DEFINES   // FIX: activa M_PI en MSVC (debe ir antes de cmath)
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();

const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

Camera  camera(glm::vec3(0.0f, 4.0f, 5.5f));
GLfloat lastX = WIDTH / 2.0f;
GLfloat lastY = HEIGHT / 2.0f;
bool    keys[1024];
bool    firstMouse = true;
bool mouseCaptured = false; //para capturar el mouse (Con M) y permitir interactuar con la PC sin que se mueva la camara
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// Conversion metros -> GL (escala 0.10)
inline float mX(float xm) { return (xm - 21.25f) * 0.10f; }
inline float mZ(float zm) { return -(zm - 13.50f) * 0.10f; }

// VAO con EBO
GLuint makeVAO(const GLfloat* v, GLsizeiptr vs, const GLuint* idx, GLsizeiptr is)
{
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

// VAO sin EBO (lineas y circulos)
GLuint makeVAO_raw(const GLfloat* v, GLsizeiptr vs)
{
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

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Planta Base - Atzin", nullptr, nullptr);
    if (!window) { glfwTerminate(); return EXIT_FAILURE; }
    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit()) return EXIT_FAILURE;
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    Shader shader("Shader/core.vs", "Shader/lamp.frag");
    Shader shader1("Shader/modelLoading.vs", "Shader/modelLoading.frag");

    Model sotano((char*)"Models/SotanoFI.obj");
    Model reja((char*)"Models/reja.obj");
    glm::mat4 projection = glm::perspective(camera.GetZoom(), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
    // =====================================================================
    //  CUADRICULA DE REFERENCIA
    //
    //  X (metros):  0 | 8.5 | 17 | 25.5 | 34 | 42.5
    //  X (GL):  -2.125 | -1.275 | -0.425 | 0.425 | 1.275 | 2.125
    //
    //  Z (metros):  0  |  9  |  18  |  27
    //  Z (GL):   1.35 | 0.45 | -0.45 | -1.35
    // =====================================================================

    // PISO BASE  42.5 x 27 m
    GLfloat pisoV[] = {
        mX(0.0f),  0.000f, mZ(0.0f),
        mX(42.5f), 0.000f, mZ(0.0f),
        mX(42.5f), 0.000f, mZ(27.0f),
        mX(0.0f),  0.000f, mZ(27.0f),
    };
    GLuint pisoI[] = { 0,1,2, 0,2,3 };

    // EDIFICIO NARANJA  x: 8.5->17m  |  z: 9->18m
    GLfloat narV[] = {
        mX(8.5f),  0.002f, mZ(9.0f),
        mX(17.0f), 0.002f, mZ(9.0f),
        mX(17.0f), 0.002f, mZ(18.0f),
        mX(8.5f),  0.002f, mZ(18.0f),
    };
    GLuint narI[] = { 0,1,2, 0,2,3 };

    // EDIFICIO AZUL  x: 34->42.5m  |  z: 9->18m
    GLfloat azulV[] = {
        mX(34.0f), 0.002f, mZ(9.0f),
        mX(42.5f), 0.002f, mZ(9.0f),
        mX(42.5f), 0.002f, mZ(18.0f),
        mX(34.0f), 0.002f, mZ(18.0f),
    };
    GLuint azulI[] = { 0,1,2, 0,2,3 };

    // PERIMETRO  GL_LINE_LOOP
    GLfloat perimV[] = {
        mX(0.0f),  0.005f, mZ(0.0f),
        mX(42.5f), 0.005f, mZ(0.0f),
        mX(42.5f), 0.005f, mZ(27.0f),
        mX(0.0f),  0.005f, mZ(27.0f),
    };

    // LINEA VERTICAL derecha: x=34m, z=0->9 (muro entre perim y azul)
    GLfloat lineaV[] = {
        mX(34.0f), 0.005f, mZ(0.0f),
        mX(34.0f), 0.005f, mZ(9.0f),
    };

    GLuint VAO_piso = makeVAO(pisoV, sizeof(pisoV), pisoI, sizeof(pisoI));
    GLuint VAO_nar = makeVAO(narV, sizeof(narV), narI, sizeof(narI));
    GLuint VAO_azul = makeVAO(azulV, sizeof(azulV), azulI, sizeof(azulI));
    GLuint VAO_perim = makeVAO_raw(perimV, sizeof(perimV));
    GLuint VAO_line = makeVAO_raw(lineaV, sizeof(lineaV));

    // =====================================================================
    //  CIRCULO UNITARIO reutilizado para todas las columnas
    //  FIX: usa 3.14159265f en lugar de M_PI para evitar warnings MSVC
    // =====================================================================
    const int SEG = 40;
    std::vector<GLfloat> circV;

    // vertice central
    circV.push_back(0.0f);
    circV.push_back(0.0f);
    circV.push_back(0.0f);

    // anillo de vertices
    for (int i = 0; i <= SEG; i++)
    {
        float a = 2.0f * 3.14159265f * (float)i / (float)SEG;  // FIX: sin M_PI, sin warnings
        circV.push_back(cosf(a));
        circV.push_back(0.0f);
        circV.push_back(sinf(a));
    }

    GLuint VAO_circ = makeVAO_raw(circV.data(), (GLsizeiptr)(circV.size() * sizeof(GLfloat)));

    // Radio columna: 0.55m -> 0.055 GL
    const float R = 0.055f;
    const float Y_COL = 0.006f;

    // =====================================================================
    //  POSICIONES DE COLUMNAS (en metros, exactas sobre la cuadricula)
    //
    //  Naranja (esquinas):   (8.5,18)(17,18)(8.5,9)(17,9)
    //  Azul (esquinas):      (34,18)(42.5,18)(34,9)(42.5,9)
    //  Standalone centro:    (25.5,18)(25.5,9)
    //  Perim. superior z=27: (25.5,27)(34,27)(42.5,27)
    //  Perim. inferior z=0:  (17,0)(25.5,0)(34,0)
    // =====================================================================
    struct Col { float xm, zm; };
    Col cols[] = {
        // Naranja
        {  8.5f, 18.0f }, { 17.0f, 18.0f },
        {  8.5f,  9.0f }, { 17.0f,  9.0f },
        // Azul
        { 34.0f, 18.0f }, { 42.5f, 18.0f },
        { 34.0f,  9.0f }, { 42.5f,  9.0f },
        // Standalone centro
        { 25.5f, 18.0f },
        { 25.5f,  9.0f },
        // Perimetro superior (z = 27)
        { 25.5f, 27.0f }, { 34.0f, 27.0f }, { 42.5f, 27.0f },
        // Perimetro inferior (z = 0)
        { 17.0f,  0.0f }, { 25.5f,  0.0f }, { 34.0f,  0.0f },
    };
    const int N_COL = sizeof(cols) / sizeof(Col);

    GLint colorLoc = glGetUniformLocation(shader.Program, "color");
    GLint modelLoc = glGetUniformLocation(shader.Program, "model");
    GLint viewLoc = glGetUniformLocation(shader.Program, "view");
    GLint projLoc = glGetUniformLocation(shader.Program, "projection");

    // Game Loop
    while (!glfwWindowShouldClose(window))
    {
        GLfloat cur = (GLfloat)glfwGetTime();   // FIX: cast explicito evita warning
        deltaTime = cur - lastFrame;
        lastFrame = cur;
        glfwPollEvents();
        DoMovement();

        glClearColor(0.18f, 0.18f, 0.18f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        

        shader.Use();

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 proj = glm::perspective(
            camera.GetZoom(),
            (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT,
            0.1f, 100.0f
        );

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

        glm::mat4 I(1);

        // Helper: setea color + model y dibuja
        auto setModel = [&](const glm::mat4& m) {
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            };

        // 1) Piso gris
        glUniform3f(colorLoc, 0.55f, 0.55f, 0.55f);
        setModel(I);
        glBindVertexArray(VAO_piso);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // 2) Edificio naranja/beige
        glUniform3f(colorLoc, 0.97f, 0.78f, 0.42f);
        setModel(I);
        glBindVertexArray(VAO_nar);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // 3) Edificio azul cielo
        glUniform3f(colorLoc, 0.40f, 0.65f, 0.90f);
        setModel(I);
        glBindVertexArray(VAO_azul);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // 4) Perimetro negro
        glLineWidth(3.5f);
        glUniform3f(colorLoc, 0.0f, 0.0f, 0.0f);
        setModel(I);
        glBindVertexArray(VAO_perim);
        glDrawArrays(GL_LINE_LOOP, 0, 4);

        // 5) Linea vertical derecha (muro externo)
        glLineWidth(3.5f);
        glUniform3f(colorLoc, 0.0f, 0.0f, 0.0f);
        setModel(I);
        glBindVertexArray(VAO_line);
        glDrawArrays(GL_LINES, 0, 2);

        // 6) Columnas circulares
        glBindVertexArray(VAO_circ);
        for (int i = 0; i < N_COL; i++)
        {
            float cx = mX(cols[i].xm);
            float cz = mZ(cols[i].zm);

            // Relleno blanco
            glm::mat4 mf = glm::translate(I, glm::vec3(cx, Y_COL, cz));
            mf = glm::scale(mf, glm::vec3(R, 1.0f, R));
            glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mf));
            glDrawArrays(GL_TRIANGLE_FAN, 0, SEG + 2);

            // Borde negro (radio ligeramente mayor)
            glm::mat4 mb = glm::translate(I, glm::vec3(cx, Y_COL + 0.001f, cz));
            mb = glm::scale(mb, glm::vec3(R + 0.01f, 1.0f, R + 0.01f));
            glUniform3f(colorLoc, 0.0f, 0.0f, 0.0f);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mb));
            glLineWidth(1.5f);
            glDrawArrays(GL_LINE_LOOP, 1, SEG);
        }

        shader1.Use();
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        glm::mat4 model(1);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        //model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        glUniformMatrix4fv(glGetUniformLocation(shader1.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        sotano.Draw(shader1);
        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

// Movimiento de camara
void DoMovement()
{
    if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])    camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])  camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])  camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) camera.ProcessKeyboard(RIGHT, deltaTime);
}

void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
    if (!mouseCaptured) return;

    if (firstMouse)
    {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }

    GLfloat xOffset = xPos - lastX;
    GLfloat yOffset = lastY - yPos;

    lastX = xPos;
    lastY = yPos;

    camera.ProcessMouseMovement(xOffset, yOffset);
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        // Al presionar ESC, liberamos el mouse y dejamos de mover la cámara
        mouseCaptured = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    if (key == GLFW_KEY_M && action == GLFW_PRESS)
    {
        // Al presionar M, capturamos el mouse para mover la cámara
        mouseCaptured = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        firstMouse = true; // Reiniciamos para evitar saltos bruscos al activar
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) keys[key] = true;
        else if (action == GLFW_RELEASE) keys[key] = false;
    }
}
//void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
//{
//    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
//        glfwSetWindowShouldClose(window, GL_TRUE);
//    if (key >= 0 && key < 1024)
//    {
//        if (action == GLFW_PRESS)        keys[key] = true;
//        else if (action == GLFW_RELEASE) keys[key] = false;
//    }
//}
//
//void MouseCallback(GLFWwindow* window, double xPos, double yPos)
//{
//    if (firstMouse) { lastX = xPos; lastY = yPos; firstMouse = false; }
//    GLfloat xOffset = (GLfloat)(xPos - lastX);   // FIX: cast explicito
//    GLfloat yOffset = (GLfloat)(lastY - yPos);     // FIX: cast explicito
//    lastX = (GLfloat)xPos;
//    lastY = (GLfloat)yPos;
//    camera.ProcessMouseMovement(xOffset, yOffset);
//}
