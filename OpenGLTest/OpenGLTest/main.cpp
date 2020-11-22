#include <iostream>
#include<fstream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include"HalfEdge.h"
using namespace std;


void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}


void checkCompile(GLuint shaderId, int shaderType) {
    int success;
    char log[512];
    if (shaderType == 0) {
        glGetProgramiv(shaderId, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderId, sizeof log, NULL, log);
            std::cout << "ERROR: LINK\n" << log << std::endl;
        }
    }
    else {
        const char* type;
        if (shaderType == 1)    type = "VERTEX";
        else                    type = "FRAGMENT";
        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shaderId, sizeof log, NULL, log);
            std::cout << "ERROR: " << type << " SHADER\n" << log << std::endl;
        }
    }
}

void drawLine(float* p, float* q) {
    // Vertex shader
    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    const char* vsource = "#version 330 core\n"
        "layout (location = 0) in vec3 in_pos;\n"
        "void main() {\n"
        "   gl_Position = vec4(in_pos, 1.0);\n"
        "}\n";
    glShaderSource(vshader, 1, &vsource, NULL);
    glCompileShader(vshader);
    checkCompile(vshader, 1);
    // Fragment shader
    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fsource = "#version 330 core\n"
        "out vec4 color;\n"
        "void main() {\n"
        "   color = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
        "}\n";
    glShaderSource(fshader, 1, &fsource, NULL);
    glCompileShader(fshader);
    checkCompile(fshader, 2);
    // Shader program
    GLuint program = glCreateProgram();
    glAttachShader(program, vshader);
    glAttachShader(program, fshader);
    glLinkProgram(program);
    checkCompile(program, 0);
    // VAO
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    // VBO
    GLuint vbuffer;
    glGenBuffers(1, &vbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vbuffer);
    GLfloat positions[] = { p[0], p[1], p[2], q[0], q[1], q[2] };
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
    // Fill vao
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Draw
    glBindVertexArray(vao);
    glUseProgram(program);
    glPointSize(5.f);
    glDrawArrays(GL_LINE_STRIP, 0, 2);
}

int main() {

    
    string path2 = "input.txt";
    ifstream ifs2(path2);
    Coord coord[13] ;
    for (int i = 0; i < 13; i++) {
        ifs2 >> coord[i].x >> coord[i].y >> coord[i].z;
    }

    auto ret0 = mvfs(coord[0]);
    auto loop = get<2>(ret0)->faces.front()->outerLoop;
    auto ret1 = mev(get<0>(ret0), loop, coord[1]);
    auto ret2 = mev(get<1>(ret1), loop, coord[2]);
    auto ret3 = mev(get<1>(ret2), loop, coord[3]);
    auto ret4 = mef(loop, get<1>(ret3), get<0>(ret0));

    loop = get<1>(ret4)->outerLoop;
    auto ret5 = mev(get<0>(ret0), loop, coord[4]);
    auto ret6 = kemr(get<0>(ret0), get<1>(ret5), loop);

    loop = get<0>(ret6);
    auto ret7 = mev(get<1>(ret5), loop, coord[5]);
    auto ret8 = mev(get<1>(ret7), loop, coord[6]);
    auto ret9 = mev(get<1>(ret8), loop, coord[7]);
    auto ret10 = mef(loop, get<1>(ret9), get<1>(ret5));

    loop = get<1>(ret4)->outerLoop;
    auto ret11 = mev(get<1>(ret1), loop, coord[8]);
    auto ret12 = kemr(get<1>(ret1), get<1>(ret11), loop);

    loop = get<0>(ret12);
    auto ret13 = mev(get<1>(ret11), loop, coord[9]);
    auto ret14 = mev(get<1>(ret13), loop, coord[10]);
    auto ret15 = mev(get<1>(ret14), loop, coord[11]);
    auto ret16 = mef(loop, std::get<1>(ret15), std::get<1>(ret11));

    sweep(get<1>(ret4), coord[12]);
    kfmrh(get<2>(ret0)->faces.front()->outerLoop, get<1>(ret10)->outerLoop);
    kfmrh(get<2>(ret0)->faces.front()->outerLoop, get<1>(ret16)->outerLoop);
    Print(get<2>(ret0), "out.brp");

    string path = "out.brp";
    ifstream ifs(path);
    float vertex[24][3] = {};
    string str;
    ifs >> str;

    for (int i = 0; i < 24; i++) {
        ifs >> vertex[i][0] >> vertex[i][1] >> vertex[i][2];
    }
    
    for (int i = 0; i < 24; i++) {
        for (int j = 0; j < 3 ; j++)
            vertex[i][j] *= 0.1;
    }

    ifs >> str;
    int lp[18][4] = {};
    for (int i = 0 ; i < 18; i++) {
        for (int j = 0; j < 4; j++) {
            ifs >> lp[i][j];
        }
    }


    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, 800, 600);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    while (!glfwWindowShouldClose(window)) {

        glClearColor(0.2, 0.2, 0.2, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        
        for (int i = 0; i < 18; i++) {
            for (int j = 0; j < 4; j += 1) {
                 
                if(j == 3)
                    drawLine(vertex[lp[i][j]], vertex[lp[i][0]]);
                else
                    drawLine(vertex[lp[i][j]], vertex[lp[i][j + 1]]);
            }
       }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
