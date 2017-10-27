#include <iostream>
#include "ObjLoader.h"

const char* vertex_source[] = {
    "#version 430\n"
    "layout (location = 0) in vec3 vposition;\n"
    "uniform mat4 MVP_matrix;\n"
    "void main() {\n"
    "gl_Position = MVP_matrix * vec4(vposition,1.0);\n"
    "}\n" };

//texture(tex,TexCoord0)
const char* fragment_source[] = {
    "#version 430\n"
    "void main() {\n"
    " gl_FragColor = vec4(1.0,0.0,0.0,1.0);\n"
    "}\n" };

GLuint vs_shader_object;
GLuint fs_shader_object;
GLuint render_prog;

GLuint MVP_Location;

GLuint VAO;
GLuint VBO[1];

glm::mat4 projection;

GLuint render_vao;
GLuint render_vbo;

enum SIDE { FRONT,BACK,SPANNING,COPLANAR };

template <typename Comparable>
class BinarySearchTree
{
public:

    BinarySearchTree();
    ~BinarySearchTree();
    void BuildTree(std::vector<Comparable>& trinagleList);
    void DrawTree(glm::vec3 eye, std::vector<Triangles>& finalList);
private:

    struct Node
    {
        Comparable partition;
        Node* backnode;
        Node* frontnode;
        std::vector<Comparable> polygonList; //List of coplanar polygons

        Node()
        {
            backnode = nullptr;
            frontnode = nullptr;
        }

        Node(const Comparable& theElement, Node* lt, Node* rt):partition{theElement}, backnode{lt}, frontnode{rt}
        {}

        Node(Comparable&& theElement, Node* lt, Node* rt):partition{std::move(theElement)}, backnode{lt}, frontnode{rt}
        {}

        SIDE ClassifyPolygons(const Comparable& polygon);
        float ClassifyPoint(Comparable& polygon, glm::vec3 eye);
    };

    Node* root;
    
    void DrawTree(Node* root, glm::vec3 eye, std::vector<Triangles>& finalList);
    void BuildTree(Node* node ,std::vector<Comparable>& trinagleList);
};

BinarySearchTree<Triangles> tree;

template<typename Comparable>
BinarySearchTree<Comparable>::BinarySearchTree()
{
    root = new BinarySearchTree<Triangles>::Node();
}

template<typename Comparable>
BinarySearchTree<Comparable>::~BinarySearchTree()
{
}


template<typename Comparable>
void BinarySearchTree<Comparable>::BuildTree(std::vector<Comparable>& trinagleList)
{
    BuildTree(root, trinagleList);
}

template<typename Comparable>
void BinarySearchTree<Comparable>::DrawTree(glm::vec3 eye, std::vector<Triangles>& finalList)
{
    DrawTree(root, eye, finalList);
}

template<typename Comparable>
void BinarySearchTree<Comparable>::DrawTree(Node * node, glm::vec3 eye, std::vector<Triangles>& finalList)
{
    if (node != nullptr)
    {
        Triangles tri = node->partition;
        float s = node->ClassifyPoint(tri, eye);
        if (s > 0)
        {
            DrawTree(node->backnode, eye, finalList);
            for (std::vector<Comparable>::const_iterator it = node->polygonList.begin(); it < node->polygonList.end(); ++it)
            {
                finalList.push_back(*it);
            }
            DrawTree(node->frontnode, eye, finalList);
        }
        else
        {
            //DrawTree(node->frontnode, eye, finalList);
            for (std::vector<Comparable>::const_iterator it = node->polygonList.begin(); it < node->polygonList.end(); ++it)
            {
                finalList.push_back(*it);
            }
            //We want only front surface
           DrawTree(node->backnode, eye, finalList); 
        }
    }
}

template<typename Comparable>
void BinarySearchTree<Comparable>::BuildTree(Node* node ,std::vector<Comparable>& trinagleList)
{
     node->partition = *trinagleList.begin();
     std::vector<Comparable> front;
     std::vector<Comparable> back;

     for (std::vector<Comparable>::const_iterator it = trinagleList.begin(); it < trinagleList.end(); ++it)
     {
         SIDE result = node->ClassifyPolygons(*it);
         switch (result)
         {
         case FRONT:
             front.push_back(*it);
             break;
         case BACK:
             back.push_back(*it);
             break;
         case SPANNING:
             back.push_back(*it);
             break;
         case COPLANAR:
             node->polygonList.push_back(*it);
             break;
        
         }
     }

     if (front.size() > 0)
     {
         node->frontnode = new BinarySearchTree<Triangles>::Node();
         BuildTree(node->frontnode, front);
     }

     if (back.size() > 0)
     {
         node->backnode = new BinarySearchTree<Triangles>::Node();
         BuildTree(node->backnode, back);
     }
}

float roundm(float val)
{
    return roundf(val * 100)/100;
}

template<typename Comparable>
SIDE BinarySearchTree<Comparable>::Node::ClassifyPolygons(const Comparable & polygon)
{
    float A = partition.normal[0].x;
    float B = partition.normal[0].y;
    float C = partition.normal[0].z;
    float D = -(A * roundm(partition.vertices[0].x) + B * roundm(partition.vertices[0].y) + C * roundm(partition.vertices[0].z));
    float epsilon = 0.00001; float epsilon1 = -0.00001;

    float P[3]= {0.0,0.0,0.0};

    for (size_t i = 0; i < 3 ; i++)
    {
        float temp = A * roundm(polygon.vertices[i].x) + B * roundm(polygon.vertices[i].y) + C * roundm(polygon.vertices[i].z);
        P[i] = temp + D;
    }

    if (P[0] > 0.0f && P[1] > 0.0f && P[2] > 0.0f)
        return FRONT;
    else if (P[0] < 0.0f && P[1] < 0.0f && P[2] < 0.0f)
        return BACK;
    else if (P[0] == 0.0f && P[1] == 0.0f && P[2] == 0.0f)
        return COPLANAR;
    else
        return SPANNING;

}

template<typename Comparable>
float BinarySearchTree<Comparable>::Node::ClassifyPoint(Comparable & polygon, glm::vec3 eye)
{
    float A = polygon.normal[0].x;
    float B = polygon.normal[0].y;
    float C = polygon.normal[0].z;
    float D = -(A * polygon.normal[0].x + B * polygon.normal[0].y + C * polygon.normal[0].z);

    float p = (A * eye.x + B * eye.y + C * eye.z) + D;
    return p;
}

void PrepareVBO(std::vector<Triangles>& finalTriToDraw)
{
    std::vector<float> vertices;


    for (const auto& triagle : finalTriToDraw)
    {
        vertices.push_back(triagle.vertices[0].x); 
        vertices.push_back(triagle.vertices[0].y);
        vertices.push_back(triagle.vertices[0].z);

        vertices.push_back(triagle.vertices[1].x);
        vertices.push_back(triagle.vertices[1].y);
        vertices.push_back(triagle.vertices[1].z);

        vertices.push_back(triagle.vertices[2].x);
        vertices.push_back(triagle.vertices[2].y);
        vertices.push_back(triagle.vertices[2].z);
    }

    if (render_vao != 0)
        glDeleteVertexArrays(1, &render_vao);
    if (render_vbo != 0)
        glDeleteBuffers(1, &render_vbo);

    glGenVertexArrays(1, &render_vao);
    glBindVertexArray(render_vao);
    
    glGenBuffers(1, &render_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, render_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
 }

float transx = 0.0;

void Display()
{
    std::vector<Triangles> finalTriToDraw;
    glm::vec3 eye = glm::vec3(0.0, 0.0, 0.0);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    tree.DrawTree(eye, finalTriToDraw);
    PrepareVBO(finalTriToDraw);

    glUseProgram(render_prog);
    glBindVertexArray(render_vao);

    glm::mat4 view = glm::lookAt(eye, glm::vec3(0.0, 0.0, -5.0), glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 model = glm::mat4(1.0);
    glm::mat4 MVP = projection*view*model;
    
    glUniformMatrix4fv(MVP_Location, 1, GL_FALSE, &MVP[0][0]);

    glDrawArrays(GL_TRIANGLES, 0, finalTriToDraw.size());

    glBindVertexArray(0);
    glUseProgram(0);

    glutSwapBuffers();
    glutPostRedisplay();
}

void ChangeSize(int w, int h)
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glViewport(0, 0, w, h);
    projection = glm::perspective(45.0f, w / (float)h, 0.1f, 100.0f);
  
}

void Key(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'a':
        transx -= 1.1;
        break;
    case 'z':
        transx += 1.1;
        break;
    case 'r':
        transx += 0.1;
        break;
    case 'w':
        transx -= 0.1;
        break;
    }

}

bool PrepareShaders()
{
    GLint success;

    vs_shader_object = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs_shader_object, 1, vertex_source, NULL);
    glCompileShader(vs_shader_object);


    glGetShaderiv(vs_shader_object, GL_COMPILE_STATUS, &success);

    if (!success) {
        char InfoLog[1024];
        glGetShaderInfoLog(vs_shader_object, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling render vs: '%s'\n", InfoLog);
        return false;
    }

    fs_shader_object = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs_shader_object, 1, fragment_source, NULL);
    glCompileShader(fs_shader_object);


    glGetShaderiv(fs_shader_object, GL_COMPILE_STATUS, &success);

    if (!success) {
        char InfoLog[1024];
        glGetShaderInfoLog(fs_shader_object, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling : '%s'\n", InfoLog);
        return false;
    }

    render_prog = glCreateProgram();
    glAttachShader(render_prog, vs_shader_object);
    glAttachShader(render_prog, fs_shader_object);
    glLinkProgram(render_prog);
       
    glGetShaderiv(render_prog, GL_LINK_STATUS, &success);

    if (!success) {
        char InfoLog[1024];
        glGetShaderInfoLog(render_prog, 1024, NULL, InfoLog);
        fprintf(stderr, "Error linking : '%s'\n", InfoLog);
        return false;
    }

    MVP_Location = glGetUniformLocation(render_prog, "MVP_matrix");

    glDetachShader(render_prog, vs_shader_object);
    glDetachShader(render_prog, fs_shader_object);

    return true;
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);

    glutInitWindowPosition(300, 300);
    glutInitWindowSize(800, 600);
    glutCreateWindow("TeslaOverDraw");

    GLenum res = glewInit();
    if (res != GLEW_OK) {
        std::cout << "Error: '%s'\n" << glewGetErrorString(res);
        return false;
    }

    //Load model
    std::vector<Triangles> triangleData;
    if (LoadModel(triangleData) == -1)
    {
        std::cout << "Error in loading file" << std::endl;
        return -1;
    }

    
    tree.BuildTree(triangleData);
    glutKeyboardFunc(Key);
    glutDisplayFunc(Display);
    glutReshapeFunc(ChangeSize);
    PrepareShaders();
    glutMainLoop();

}
