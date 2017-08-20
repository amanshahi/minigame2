#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <bits/stdc++.h>
#include <ao/ao.h>
#include <mpg123.h>
#define BITS 8
using namespace std;
struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;
    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;
typedef struct COLOR{
    float r;
    float g;
    float b; 
}Color;
struct GLMatrices {
    glm::mat4 projection;
    glm::mat4 model;
    glm::mat4 view;
    GLuint MatrixID;
} Matrices;
typedef struct mirrors {
  GLfloat vertex[18];
  GLfloat color[18];
} MIRRORS;
typedef struct objects {
  GLfloat vertex[18];
  GLfloat color[18];
} OBJECTS;
typedef struct AllObjectsInfo {
    COLOR color;
    float x,y,x_original, y_original,anglePresent,height,width;VAO* object;int falling;    
}AllObjectsInfo;
COLOR grey = {168.0/255.0,168.0/255.0,168.0/255.0};
COLOR shade = {0.3,0.3,0};
COLOR green = {0,1,0};
COLOR gold = {218.0/255.0,165.0/255.0,32.0/255.0};
COLOR red = {1,0,0};
COLOR black = {30/255.0,30/255.0,21/255.0};
COLOR blue = {0,0,1};
COLOR white = {255/255.0,255/255.0,255/255.0};
AllObjectsInfo tempObject;
vector <AllObjectsInfo> objectBorder,objectMirror,objectLaser,objectScores,objectBricks,objectBucket,objectCannon;
double last_update_time = glfwGetTime(), current_time;
int previousXcoor = 0;
int previousYcoor = 0;
float angle = 0;
int gameScore=0;
double presentXcoor,presentYcoor;
int checkMousePressed=0;
int vis[11002], timeDiff = 0,laser_pos=0;
int zoom_camera=1;
GLuint programID;
glm::vec3 rect_pos, boxPosition,basePosition;
float rectangle_rotation = 0;
vector <OBJECTS > finalObjects;
vector <OBJECTS > fallingBlocks;
vector <MIRRORS > mirrors;
OBJECTS bucket1,bucket2,shooter1,shooter2, tempBlock,Laser;
MIRRORS tempMirror;
VAO  *rectangle;
VAO *triangle,*sh1,*sh2,*laser;
int mouseFlag = 0;
double presentMouseX,YcoorMouse,previousXMouse,previousYMouse;
float x_change=0,y_change=0;
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open()){
        std::string Line = "";
        while(getline(VertexShaderStream, Line))
        VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
        std::string Line = "";
        while(getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }
    GLint Result = GL_FALSE;
    int InfoLogLength;
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> VertexShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);
    return ProgramID;
}
static void error_callback(int error, const char* description){
    fprintf(stderr, "Error: %s\n", description);
}
void quit(GLFWwindow *window){
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
void initGLEW(void){
    glewExperimental = GL_TRUE;
    if(glewInit()!=GLEW_OK){
        fprintf(stderr,"Glew failed to initialize : %s\n", glewGetErrorString(glewInit()));
    }
    if(!GLEW_VERSION_3_3){
       fprintf(stderr, "3.3 version not available\n");
    }
}
int xa=0;
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertexCoor, const GLfloat* colorCoor, GLenum fill_mode=GL_FILL){
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors
    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertexCoor, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), colorCoor, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertexCoor, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL){
    GLfloat* colorCoor = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i+=1) {
        colorCoor [3*i] = red;
        colorCoor [3*i + 1] = green;
        colorCoor [3*i + 2] = blue;
    }
    return create3DObject(primitive_mode, numVertices, vertexCoor, colorCoor, fill_mode);
}
void draw3DObject (struct VAO* vao){
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);
    glBindVertexArray (vao->VertexArrayID);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}
float rectangle_rot_dir = 1;
bool rectangle_rot_status = true;
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods){
    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_C:
                rectangle_rot_status = !rectangle_rot_status;
                break;
            case GLFW_KEY_P:
                break;
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            default:
                break;
        }
    }
}
mpg123_handle *mh;
unsigned char *buffer;
size_t buffer_size;
size_t done;
int err;
int driver;
ao_device *dev;
ao_sample_format format;
int channels, encoding;
long rate;
void audio_init() {
    ao_initialize();
    driver = ao_default_driver_id();
    mpg123_init();
    mh = mpg123_new(NULL, &err);
    buffer_size = 2500;
    buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));

    mpg123_open(mh, "themeSong.mp3");
    mpg123_getformat(mh, &rate, &channels, &encoding);

    format.bits = mpg123_encsize(encoding) * BITS;
    format.rate = rate;
    format.channels = channels;
    format.byte_format = AO_FMT_NATIVE;
    format.matrix = 0;
    dev = ao_open_live(driver, &format, NULL);
}
void audio_play() {
    if (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK)
        ao_play(dev, (char*) buffer, done);
    else mpg123_seek(mh, 0, SEEK_SET);
}
void audio_close() {
    free(buffer);
    ao_close(dev);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    ao_shutdown();
}



glm::vec3 rot,temp_1,camera_eye;
int flagToggle = 1;
int leftDir = 0,rightDir = 0,upDir = 0,downDir = 0,cameraAngle=90,firstPersonView = 0,backPersonView = 0;
int lastPressed = 0;;
int topView,towerview,topRotation,towerRotation;
void keyboardChar (GLFWwindow* window, unsigned int key){
    switch (key) {
    case 'Q':
    case 'q':
    quit(window);
    break;
    case 'a': 
        leftDir = 1;
        gameScore+=1;
        lastPressed = 2;
        break;
    case 'd': 
        rightDir = 1;
        gameScore+=1;
        lastPressed = 1;
        break;
    case 'w': 
        upDir = 1;
        gameScore+=1;
        lastPressed = 3;
        break;
    case 's': 
        downDir = 1;
        gameScore+=1; 
        lastPressed = 4;
        break;  
    case 'e': 
    	topView = 1;
    	topRotation = 0;
    	towerRotation = 0;
    	towerview = 0;
    	backPersonView = 0;
        firstPersonView = 0;
        camera_eye = glm::vec3(0,0,10);
        break;
    case 'r': //towerView
    	firstPersonView = 0;
    	topView = 0;
    	topRotation = 0;
    	towerRotation = 0;
    	towerview = 1;
        backPersonView = 0;
        cameraAngle = 90;
        camera_eye = glm::vec3(-5*cos(cameraAngle*M_PI/180.0f),-5*sin(cameraAngle*M_PI/180.0f),8);
        break;
    case 't': 
    	topView = 0;
    	topRotation = 1;
    	towerRotation = 0;
    	towerview = 0;
        firstPersonView = 0;
        backPersonView = 0;
        cameraAngle += 7;
        camera_eye = glm::vec3(-5*cos(cameraAngle*M_PI/180.0f),-5*sin(cameraAngle*M_PI/180.0f),8);
        break;
    case 'y' :
    	topView = 0;
    	topRotation = 0;
    	towerRotation = 1;
    	towerview = 0 ;
    	firstPersonView = 0;
    	backPersonView = 0;
        cameraAngle -= 7;
        camera_eye = glm::vec3(-5*cos(cameraAngle*M_PI/180.0f),-5*sin(cameraAngle*M_PI/180.0f),8);
        break;
    case 'u':
        backPersonView = 0;firstPersonView = 1;
        break;
    case 'i':
        backPersonView = 1;firstPersonView = 0;
        break;
    default:
    break;
    }
}
void mouseButton (GLFWwindow* window, int button, int action, int mods){
    switch (button) {
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) {
                rectangle_rot_dir *= -1;
            }
        break;
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE) {
                mouseFlag = 0;
            }
            else if (action == GLFW_PRESS) {
                mouseFlag = 1;
            }
        break;
        default:
        break;
    }
}
GLfloat colorCoor_1 [1008];
GLfloat colorCoor_2 [1008];
GLfloat colorCoor_3 [1008];
GLfloat colorCoor_4 [1008];
void reshapeWindow (GLFWwindow* window, int width, int height){
    int fbwidth=width, fbheight=height;
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);
    GLfloat fov = M_PI/2;
    glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);
    Matrices.projection = glm::perspective(fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.05f, 25.05f);
}
int boxAllignment;
VAO *cam, *floor_vao, *box;
vector<VAO *> obj;
void createRectangle (){
    GLfloat vertexCoor [] = {
    -0.5, 0.5, 0.5, 
    -0.5, -0.5, 0.5, 
    0.5, -0.5, 0.5,
    -0.5, 0.5, 0.5, 
    0.5, -0.5, 0.5,
    0.5, 0.5, 0.5,
    0.5, 0.5, 0.5,
    0.5, -0.5, 0.5,
    0.5, -0.5, -0.5,
    0.5, 0.5, 0.5,
    0.5, -0.5, -0.5,
    0.5, 0.5, -0.5,
    0.5, 0.5, -0.5,
    0.5, -0.5, -0.5,
    -0.5, -0.5, -0.5,
    0.5, 0.5, -0.5,
    -0.5, -0.5, -0.5,
    -0.5, 0.5, -0.5,
    -0.5, 0.5, -0.5,
    -0.5, -0.5, -0.5,
    -0.5, -0.5, 0.5, 
    -0.5, 0.5, -0.5,
    -0.5, -0.5, 0.5, 
    -0.5, 0.5, 0.5, 
    -0.5, 0.5, -0.5,
    -0.5, 0.5, 0.5, 
    0.5, 0.5, 0.5,
    -0.5, 0.5, -0.5,
    0.5, 0.5, 0.5,
    0.5, 0.5, -0.5,
    -0.5, -0.5, 0.5, 
    -0.5, -0.5, -0.5,
    0.5, -0.5, -0.5,
    -0.5, -0.5, 0.5, 
    0.5, -0.5, -0.5,
    0.5, -0.5, 0.5,
    };

    static const GLfloat colorCoor [] = {
    0.5f, 0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,
    0.2f, 0.8f, 0.0f,
    0.2f, 0.8f, 0.0f,
    0.2f, 0.8f, 0.0f,
    0.2f, 0.8f, 0.0f,
    0.2f, 0.8f, 0.0f,
    0.2f, 0.8f, 0.0f,
    0.0f, 0.8f, 0.0f,
    0.0f, 0.8f, 0.0f,
    0.0f, 0.8f, 0.0f,
    0.0f, 0.8f, 0.0f,
    0.0f, 0.8f, 0.0f,
    0.0f, 0.8f, 0.0f,
    0.4f, 0.0f, 0.0f,
    0.4f, 0.0f, 0.0f,
    0.4f, 0.0f, 0.0f,
    0.4f, 0.0f, 0.0f,
    0.4f, 0.0f, 0.0f,
    0.4f, 0.0f, 0.0f,
    0.0f, 0.4f, 0.0f,
    0.0f, 0.4f, 0.0f,
    0.0f, 0.4f, 0.0f,
    0.0f, 0.4f, 0.0f,
    0.0f, 0.4f, 0.0f,
    0.0f, 0.4f, 0.0f,
    0.0f, 0.0f, 0.4f,
    0.0f, 0.0f, 0.4f,
    0.0f, 0.0f, 0.4f,
    0.0f, 0.0f, 0.4f,
    0.0f, 0.0f, 0.4f,
    0.0f, 0.0f, 0.4f,
    };
    for (int i=0; i<36; i+=1){
        vertexCoor[i*3] += 0.5;
        vertexCoor[i*3+1] += 0.5; 
        vertexCoor[i*3+2] /= 2;
		colorCoor_2[i*3]=colorCoor_3[i*3+1]=colorCoor_4[i*3+2]=1.0f;
    }
    obj.push_back(create3DObject(GL_TRIANGLES, 36, vertexCoor, colorCoor, GL_FILL));
    obj.push_back(create3DObject(GL_TRIANGLES, 36, vertexCoor, colorCoor_2, GL_FILL));
    obj.push_back(create3DObject(GL_TRIANGLES, 36, vertexCoor, colorCoor_3, GL_FILL));
    obj.push_back(create3DObject(GL_TRIANGLES, 36, vertexCoor, colorCoor_4, GL_FILL));
    obj.push_back(create3DObject(GL_TRIANGLES, 36, vertexCoor, colorCoor_1, GL_LINE));
    for (int i=0; i<36; i+=1){
        vertexCoor[i*3+2] = 4*vertexCoor[i*3+2];
    }
    obj.push_back(create3DObject(GL_TRIANGLES, 36, vertexCoor, colorCoor, GL_FILL));
}
int baseCoor[10][10]={
    {0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,10,1,1},
    {0,0,0,0,0,0,0,1,5,1},
    {0,0,0,0,0,0,0,1,1,1},
    {0,0,0,0,0,0,1,1,2,2},
    {0,0,0,0,0,0,2,2,2,2},
    {0,0,0,0,0,0,1,1,1,1},
    {0,0,0,0,0,0,0,0,1,1},
    {0,0,0,0,0,0,0,0,1,1},
    {0,0,0,0,0,0,0,1,1,1},
};
int base_Level[10][10]={
    {0,0,0,0,0,0,1,1,1,1},
    {0,0,0,0,0,0,0,0,0,1},
    {0,0,0,0,0,0,0,0,1,1},
    {0,0,0,0,0,0,0,0,1,67},
    {0,0,0,0,0,0,0,0,0,3},
    {0,0,0,0,0,0,0,0,1,1},
    {1,1,1,0,1,1,1,1,1,1},
    {0,1,1,1,1,0,0,0,1,2},
    {0,0,1,5,1,0,0,0,1,2},
    {0,0,1,1,1,1,0,1,1,1},
};
int var1, var2, var3, var4, flag=0;
int blockInitAngle=0;
glm::vec3 axis_rotation;
int boxXcoor1,boxXcoor2,boxYcoor1,boxYcoor2;
int mousescroll(GLFWwindow* window, double xoffset, double yoffset){
    if (yoffset==-1) { 
        zoom_camera /= 1.1;
    }
    else if(yoffset==1){
        zoom_camera *= 1.1;
    }
    if (zoom_camera<=1) {
        zoom_camera = 1;
    }
    if (zoom_camera>=4) {
        zoom_camera=4;
    }
    if(x_change-400.0f/zoom_camera<-400)
        x_change=-400+400.0f/zoom_camera;
    else if(x_change+400.0f/zoom_camera>400)
        x_change=400-400.0f/zoom_camera;
    if(y_change-400.0f/zoom_camera<-400)
        y_change=-300+300.0f/zoom_camera;
    else if(y_change+400.0f/zoom_camera>400)
        y_change=400-400.0f/zoom_camera;
    Matrices.projection = glm::ortho((float)(-400.0f/zoom_camera + x_change), (float)(400.0f/zoom_camera + x_change), (float)(-400.0f/zoom_camera + y_change), (float)(400.0f/zoom_camera + y_change), 0.1f, 500.0f);
    return 0;
}
void movementBox(int conditionEntered){
    if(boxAllignment==2){
        if(downDir == 1){
            conditionEntered = 1;
            downDir = 0;
            boxPosition.y -= 1;
            blockInitAngle = 90;
            boxAllignment = 2;
            axis_rotation = glm::vec3(0,1,0);
            boxPosition.z = 1.25;
            boxXcoor1 += 1;
            boxXcoor2 += 1;
        }
        else if(rightDir == 1){
            conditionEntered = 1;
            rightDir = 0;
            boxPosition.x += 1;
            blockInitAngle = 0;
            boxAllignment = 1;
            axis_rotation = glm::vec3(0,1,0);
            boxYcoor1 = boxYcoor2 = max(boxYcoor1,boxYcoor2) + 1;
        }
        else if(upDir == 1){
            conditionEntered = 1;
            upDir = 0;
            boxPosition.y += 1;
            blockInitAngle = 90;
            boxAllignment = 2;
            axis_rotation = glm::vec3(0,1,0);
            boxPosition.z = 1.25;
            boxXcoor1 -= 1;
            boxXcoor2 -= 1;
        }
        else if(leftDir == 1){
            conditionEntered = 1;
            leftDir = 0;
            boxPosition.x -= 2;
            blockInitAngle = 0;
            boxAllignment = 1;
            axis_rotation = glm::vec3(0,1,0);
            boxYcoor1 = boxYcoor2 = min(boxYcoor1, boxYcoor2) - 1;
        } 
    }
    else if(boxAllignment==3){
        if(downDir == 1){
            conditionEntered = 1;
            downDir = 0;
            boxPosition.y -= 2;
            blockInitAngle = 0;
            boxAllignment = 1;
            axis_rotation = glm::vec3(1,0,0);
            boxPosition.z = 1.25;
            boxXcoor1 = boxXcoor2 = max(boxXcoor1, boxXcoor2) + 1;
        }
        else if(upDir == 1){
            conditionEntered = 1;
            upDir = 0;
            boxPosition.y += 1;
            blockInitAngle = 0;
            boxAllignment = 1;
            axis_rotation = glm::vec3(1,0,0);
            boxPosition.z = 1.25;
            boxXcoor1 = boxXcoor2 = min(boxXcoor1, boxXcoor2) - 1;
        }
        else if(leftDir == 1){
            conditionEntered = 1;
            leftDir = 0;
            boxPosition.x -= 1;
            blockInitAngle = 90;
            boxAllignment = 3;
            boxPosition.z = 0.25;
            axis_rotation = glm::vec3(1,0,0);
            boxYcoor1 -= 1;
            boxYcoor2 -= 1;
        }
        else if(rightDir == 1){
            conditionEntered = 1;
            rightDir = 0;
            boxPosition.x += 1;
            blockInitAngle = 90;
            boxAllignment = 3;
            boxPosition.z = 0.25;
            axis_rotation = glm::vec3(1,0,0);
            boxYcoor1 += 1;
            boxYcoor2 += 1;
        }
         
    }
    else if(boxAllignment==1){
        if(leftDir == 1){
            conditionEntered = 1;
            leftDir = 0;
            boxPosition.x -= 1;
            blockInitAngle = 90;
            boxAllignment = 2;
            axis_rotation = glm::vec3(0,1,0);
            boxYcoor1 = boxYcoor1-1;
            boxYcoor2 = boxYcoor2-2;
        }
        else if(downDir == 1){
            downDir = 0;
            conditionEntered = 1;
            boxPosition.y -= 1;
            blockInitAngle = 90;
            boxAllignment = 3;
            axis_rotation = glm::vec3(1,0,0);
            boxPosition.z = 0.25;
            boxXcoor1 += 1;
            boxXcoor2 = boxXcoor1+1;
        }
        else if(upDir == 1){
            upDir = 0;
            conditionEntered = 1;
            boxPosition.y += 2;
            blockInitAngle = 90;
            boxAllignment = 3;
            axis_rotation = glm::vec3(1,0,0);
            boxPosition.z = 0.25;
            boxXcoor1 -= 1;
            boxXcoor2 = boxXcoor1-1;
        }
        else if(rightDir == 1){
            rightDir = 0;
            conditionEntered = 1;
            boxPosition.x += 2;
            blockInitAngle = 90;
            boxAllignment = 2;
            axis_rotation = glm::vec3(0,1,0);
            boxYcoor1 = boxYcoor1+1;
            boxYcoor2 = boxYcoor2+2;
        } 
    }
    if(flagToggle == 0 && (baseCoor[boxXcoor1][9-boxYcoor1] == 4 || baseCoor[boxXcoor2][9-boxYcoor2] == 4) && conditionEntered == 1){
        flagToggle = 1;
    }
    if((baseCoor[boxXcoor1][9-boxYcoor1] == 10 || baseCoor[boxXcoor2][9-boxYcoor2] == 10) && conditionEntered == 1){
    	cout << "ok please" << endl;
    	baseCoor[5][8] = baseCoor[5][9] = !baseCoor[5][9];
    }
}
int checkToggle(int flag){
    if((baseCoor[boxXcoor1][9-boxYcoor1] == 4 || baseCoor[boxXcoor2][9-boxYcoor2] == 4)){
        if(flagToggle == 1){
            for(int i=0;i<9;i+=1){
                baseCoor[i][5] = !baseCoor[i][5];
            }
            return 1; 
        }
    }
    return 0;
}
int fallingBlock(int var1, int var2, int var3, int var4){
    if((boxAllignment==1 && baseCoor[boxXcoor1][9-boxYcoor1]==2) || (baseCoor[boxXcoor1][9-boxYcoor1] == 0 || baseCoor[boxXcoor2][9-boxYcoor2] == 0) || boxXcoor1 < 0 || boxYcoor1 < 0 || boxXcoor2 < 0 || boxYcoor2 < 0 || boxXcoor1 > 9 || boxYcoor1 > 9 || boxXcoor2 > 9 || boxYcoor2 > 9) return 1;
    if(baseCoor[boxXcoor1][9-boxYcoor1]==3 || baseCoor[boxXcoor2][9-boxYcoor2]==3){
        for(int i=0;i<9;i+=1){
            baseCoor[i][5] = 1;
        }
        baseCoor[3][9] = 4;
        baseCoor[1][8] = 1;
    }
    if(checkToggle(flag) == 1){
        flagToggle = 0;
    }
    return 0;
}
int check_pan(){
    if(x_change-400.0f/zoom_camera<-400)
        x_change=-400+400.0f/zoom_camera;
    else if(x_change+400.0f/zoom_camera>400)
        x_change=400-400.0f/zoom_camera;
    if(y_change-300.0f/zoom_camera<-300)
        y_change=-300+300.0f/zoom_camera;
    else if(y_change+300.0f/zoom_camera>300)
        y_change=300-300.0f/zoom_camera;
    return 0;
}
void createTriangle (){
  static const GLfloat vertexCoor [] = {
    0, 1,0, // vertex 0
    -1,-1,0, // vertex 1
    1,-1,0, // vertex 2
  };
  static const GLfloat colorCoor [] = {
    1,0,0, // color 0
    0,1,0, // color 1
    0,0,1, // color 2
  };
  triangle = create3DObject(GL_TRIANGLES, 3, vertexCoor, colorCoor, GL_LINE);
}
void initObjects(){
  GLfloat vertexCoor_bucket1 [] = {
    125,-400,0, // vertex 1
    175,-400,0, // vertex 2
    200, -350,0, // vertex 3

    200, -350,0, // vertex 3
    100, -350,0, // vertex 4
    125,-400,0  // vertex 1
  };
  GLfloat colorCoor_bucket1 [] = {
    0,1,0, // color 1
    0,1,0, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0,1,0, // color 4
    0,1,0  // color 1
  };
  for(int i=0;i<18;i+=1){bucket1.vertex[i]=vertexCoor_bucket1[i];vis[(int)(vertexCoor_bucket1[0])+400]=1;}
  for(int i=0;i<18;i+=1){bucket1.color[i]=colorCoor_bucket1[i];}
    GLfloat vertexCoor_bucket2 [] = {
    -175,-400,0, // vertex 1
    -125,-400,0, // vertex 2
    -100, -350,0, // vertex 3

    -100, -350,0, // vertex 3
    -200, -350,0, // vertex 4
    -175,-400,0  // vertex 1
  };
  GLfloat colorCoor_bucket2 [] = {
    1,0,0, // color 1
    1,0,0, // color 2
    1,0,0, // color 3

    1,0,0, // color 3
    1,0,0, // color 4
    1,0,0  // color 1
  };
  for(int i=0;i<18;i+=1){bucket2.vertex[i]=vertexCoor_bucket2[i];vis[(int)(vertexCoor_bucket1[0])+400]=1;}
  for(int i=0;i<18;i+=1){bucket2.color[i]=colorCoor_bucket2[i];}
   GLfloat vertexCoor_shooter1 [] = {
    -350,15,0, // vertex 1
    -400,15,0, // vertex 2
    -400, -15,0, // vertex 3

    -400, -15,0, // vertex 3
    -350, -15,0, // vertex 4
    -350,15,0  // vertex 1
  };
  GLfloat colorCoor_shooter1 [] = {
    0,0,0, // color 1
    0.3,0.3,0.3, // color 2
    0.3,0.3,0.3, // color 3

    0.3,0.3,0.3, // color 3
    0,0,0, // color 4
    0,0,0  // color 1
  };
  for(int i=0;i<18;i+=1){shooter1.vertex[i]=vertexCoor_shooter1[i];}
  for(int i=0;i<18;i+=1){shooter1.color[i]=colorCoor_shooter1[i];}
  sh1=create3DObject(GL_TRIANGLES,6,shooter1.vertex,shooter1.color,GL_FILL);
  GLfloat vertexCoor_shooter2 [] = {
    -325,10,0, // vertex 1
    -350,10,0, // vertex 2
    -350, -10,0, // vertex 3

    -350, -10,0, // vertex 3
    -325, -10,0, // vertex 4
    -325,10,0  // vertex 1
  };
  GLfloat colorCoor_shooter2 [] = {
    0,0,0, // color 1
    0.5,0.5,0.5, // color 2
    0.5,0.5,0.5, // color 3

    0.5,0.5,0.5, // color 3
    0,0,0, // color 4
    0,0,0  // color 1
  };
  for(int i=0;i<18;i+=1){shooter2.vertex[i]=vertexCoor_shooter2[i];}
  for(int i=0;i<18;i+=1){shooter2.color[i]=colorCoor_shooter2[i];}
  sh2=create3DObject(GL_TRIANGLES,6,shooter2.vertex,shooter2.color,GL_FILL);
  GLfloat vertexCoor_laser [] = {
    400,10,0, // vertex 1
    -325,10,0, // vertex 2
    -325, -10,0, // vertex 3

    -325, -10,0, // vertex 3
    400, -10,0, // vertex 4
    400,10,0  // vertex 1
  };

  GLfloat colorCoor_L_color [] = {
    1,0,0, // color 1
    1,0.5,0.5, // color 2
    1,0.5,0.5, // color 3

    1,0.5,0.5, // color 3
    1,0,0, // color 4
    1,0,0  // color 1
  };
  laser=create3DObject(GL_TRIANGLES,6,vertexCoor_laser,colorCoor_L_color,GL_FILL);
  finalObjects.push_back(bucket1);
  finalObjects.push_back(bucket2);
  finalObjects.push_back(shooter1);
  finalObjects.push_back(shooter2);
}
void createLine(){
  return ;
}
int drawAllObjects(int i, int j){
    if(baseCoor[i][j]==1)
        draw3DObject(obj[0]);
    else if(baseCoor[i][j]==2)
        draw3DObject(obj[1]);
    else if(baseCoor[i][j]==3)
        draw3DObject(obj[2]);
    else if(baseCoor[i][j]==4)
        draw3DObject(obj[3]);
    draw3DObject(obj[4]);
}
int level = 1;
int checkLevelChange(){
    if(level == 3){
    	gameScore=10;
        return 1;
    }
    if((baseCoor[boxXcoor1][9-boxYcoor1] == 5 || baseCoor[boxXcoor2][9-boxYcoor2] == 5) && boxAllignment == 1){
    	if(boxPosition.z >= -5.0){
    		boxPosition.z -= 0.001;
    	}
    	else{
	        level+=1;
	        gameScore=0;
	        int i=0,j=0;
	        int waitTime = 100000000;
	    	while(waitTime--){}
	        while(i<10){
	        	j=0;
	            while(j<10){
	                baseCoor[i][j] = base_Level[i][j]; j+=1;
	            }
	            i+=1;
	        }
	        boxPosition = glm::vec3(-4,-4,1.25);
	        boxXcoor1 = 9;boxXcoor2 = 9; 
	        boxYcoor1 = 0;boxYcoor2 = 0;
	        boxAllignment = 1;
	        camera_eye = glm::vec3(0,-5,8);
	        blockInitAngle = 0;
	    }
    }
    return 0;
}
int tempX,tempY;
int ShowScoreInSegments[9];
void showScore(int a){
  for(int i=0;i<10;i+=1) ShowScoreInSegments[i] = 0;
  if(a == 0){for(int i=0;i<7;i+=1){ShowScoreInSegments[i] = 1;}ShowScoreInSegments[6] = 0;}
  if(a == 1){ShowScoreInSegments[1] = 1;ShowScoreInSegments[2] = 1;}
  if(a == 2){ShowScoreInSegments[0] = 1;ShowScoreInSegments[1] = 1;ShowScoreInSegments[3] = 1;ShowScoreInSegments[4] = 1;ShowScoreInSegments[6] = 1;}
  if(a == 3){for(int i=0;i<7;i+=1){ShowScoreInSegments[i] = 1;}ShowScoreInSegments[4] = 0;ShowScoreInSegments[5] = 0;}
  if(a == 4){ShowScoreInSegments[1] = ShowScoreInSegments[2] = ShowScoreInSegments[5] = ShowScoreInSegments[6] = 1;}
  if(a == 5){for(int i=0;i<7;i+=1){ShowScoreInSegments[i] = 1;}ShowScoreInSegments[1] = 0;ShowScoreInSegments[4] = 0;}
  if(a == 6){for(int i=0;i<7;i+=1){ShowScoreInSegments[i] = 1;}ShowScoreInSegments[1] = 0;}
  if(a == 7){ShowScoreInSegments[0] = ShowScoreInSegments[1] = ShowScoreInSegments[2] = 1;}
  if(a == 8){for(int i=0;i<7;i+=1){ShowScoreInSegments[i] = 1;}}
  if(a == 9){for(int i=0;i<7;i+=1){ShowScoreInSegments[i] = 1;}ShowScoreInSegments[4] = 0;}
  return ;
}
void createScores (string assignedShape, COLOR colorA, COLOR colorB, COLOR colorC, COLOR colorD, float x, float y, float height, float width){
    float w=width/2,h=height/2;
    GLfloat vertex_buffer_data [] = {
        -w,-h,0, // vertex 1
        -w,h,0, // vertex 2
        w,h,0, // vertex 3

        w,h,0, // vertex 3
        w,-h,0, // vertex 4
        -w,-h,0  // vertex 1
    };
    COLOR first=colorA,second=colorB,third=colorC,fourth=colorD;
    GLfloat color_buffer_data [] = {
        first.r,first.g,first.b, // color 1
        second.r,second.g,second.b, // color 2
        third.r,third.g,third.b, // color 3

        third.r,third.g,third.b, // color 4
        fourth.r,fourth.g,fourth.b, // color 5
        first.r,first.g,first.b // color 6
    };
    rectangle=create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
    AllObjectsInfo newObject = {};
    newObject.color = colorA;
    newObject.object = rectangle;
    newObject.x=newObject.x_original=x;
    newObject.y=newObject.y_original=y;
    newObject.height=height;
    newObject.width=width;
    newObject.falling=0;
    newObject.anglePresent = 0;
    if(assignedShape=="cannonBig" || assignedShape == "cannonSmall") objectCannon.push_back(newObject);
    if(assignedShape=="bucketBlue" || assignedShape == "bucketRed") objectBucket.push_back(newObject);
    if(assignedShape=="brick") objectBricks.push_back(newObject);
    if(assignedShape=="scores") objectScores.push_back(newObject);
    if(assignedShape=="0") objectLaser[0]=newObject;
    if(assignedShape=="1") objectLaser[1]=newObject;
    if(assignedShape=="2") objectLaser[2]=newObject;
    if(assignedShape=="3") objectLaser[3]=newObject;
    if(assignedShape=="4") objectLaser[4]=newObject;
    if(assignedShape=="5") objectLaser[5]=newObject;
    if(assignedShape=="6") objectLaser[6]=newObject;
    if(assignedShape=="7") objectLaser[7]=newObject;
    if(assignedShape=="8") objectLaser[8]=newObject;
    if(assignedShape=="9") objectLaser[9]=newObject;
    if(assignedShape=="mirror"){  newObject.anglePresent=45;objectMirror.push_back(newObject);}
    if(assignedShape=="borderLine") objectBorder.push_back(newObject);
}
void draw (GLFWwindow* window, float x, float y, float w, float h){
    int fbwidth, fbheight;
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);
    glViewport((int)(x*fbwidth), (int)(y*fbheight), (int)(w*fbwidth), (int)(h*fbheight));
    glUseProgram(programID);
    if (mouseFlag != 0){
			glfwGetCursorPos(window, &presentMouseX, &YcoorMouse);
			cameraAngle += ((presentMouseX - previousXMouse)/4)*previousYMouse>300?-1:1;previousXMouse = presentMouseX;previousYMouse = YcoorMouse;
			camera_eye = glm::vec3(-5*sin(cameraAngle*M_PI/180.0f), -5*cos(cameraAngle*M_PI/180.0f), 8);
			glm::vec3 target = glm::vec3(0, 0, 0);
			glm::vec3 up = glm::vec3(0, 1, 0);
    
	}
	if(firstPersonView){
		tempX = tempY = 0;
    	if(lastPressed == 1){tempX=-0;}
    	else if(lastPressed == 2){tempX=0;}
    	else if(lastPressed == 3){tempY=-0;}
    	else if(lastPressed == 4){tempY=0;}
        camera_eye = glm::vec3(boxPosition.x + tempX, boxPosition.y + tempY, 3);
        glm::vec3 up (0, 1, 0);
        if(lastPressed == 1){
            glm::vec3 target (5,boxPosition.y,1.25);
            Matrices.view = glm::lookAt(camera_eye, target, up); // Fixed camera for 2D (ortho) in XY plane
        }
        else if(lastPressed == 2){
            glm::vec3 target (-5,boxPosition.y,1.25);
            Matrices.view = glm::lookAt(camera_eye, target, up); // Fixed camera for 2D (ortho) in XY plane
        }
        else if(lastPressed == 3){
            glm::vec3 target (boxPosition.x,5,1.25);
            Matrices.view = glm::lookAt(camera_eye, target, up); // Fixed camera for 2D (ortho) in XY plane
        }
        else if(lastPressed == 4){
            glm::vec3 target (boxPosition.x,5,1.25);
            Matrices.view = glm::lookAt(camera_eye, target, up); // Fixed camera for 2D (ortho) in XY plane
        }
        else{
        	cout << "Chitia" << endl;
            glm::vec3 target (5,boxPosition.y,1.25);
            Matrices.view = glm::lookAt(camera_eye, target, up); // Fixed camera for 2D (ortho) in XY plane   
        }
        
    }
    else if(backPersonView){
    	tempX = tempY = 0;
    	if(lastPressed == 1){tempX=-4;}
    	else if(lastPressed == 2){tempX=4;}
    	else if(lastPressed == 3){tempY=-4;}
    	else if(lastPressed == 4){tempY=4;}
        camera_eye = glm::vec3(boxPosition.x+tempX, boxPosition.y+tempY, 3);
        glm::vec3 up (0, 1, 0);
        if(lastPressed == 1){
            glm::vec3 target (5,boxPosition.y,1.25);
            Matrices.view = glm::lookAt(camera_eye, target, up); // Fixed camera for 2D (ortho) in XY plane
        }
        else if(lastPressed == 2){
            glm::vec3 target (-5,boxPosition.y,1.25);
            Matrices.view = glm::lookAt(camera_eye, target, up); // Fixed camera for 2D (ortho) in XY plane
        }
        else if(lastPressed == 3){
            glm::vec3 target (boxPosition.x,5,1.25);
            Matrices.view = glm::lookAt(camera_eye, target, up); // Fixed camera for 2D (ortho) in XY plane
        }
        else if(lastPressed == 4){
            glm::vec3 target (boxPosition.x,5,1.25);
            Matrices.view = glm::lookAt(camera_eye, target, up); // Fixed camera for 2D (ortho) in XY plane
        }
        else{
            glm::vec3 target (5,boxPosition.y,1.25);
            Matrices.view = glm::lookAt(camera_eye, target, up); // Fixed camera for 2D (ortho) in XY plane   
        }
        
    }else {
        glm::vec3 target (0,0,0);
        glm::vec3 up (0, 1, 0);
        Matrices.view = glm::lookAt(camera_eye, target, up); // Fixed camera for 2D (ortho) in XY plane
    }
    glm::vec3 temp;
    int i=0,j=0;
    while(i<10){
    	j=0;
        while(j<10){
            if(checkLevelChange() == 1) quit(window);
            if(baseCoor[i][j]!=0){
                glm::mat4 VP;
                VP = Matrices.projection * Matrices.view;
                glm::mat4 MVP;  // MVP = Projection * View * Model
                glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
                Matrices.model = glm::mat4(1.0f);
                temp = rect_pos;
                temp.x = rect_pos.x + -j;
                temp.y = rect_pos.y + (-i);
                temp.x+=5;
                temp.y+=5;
                glm::mat4 rotateRectangle = glm::rotate((float)(0), glm::vec3(0,1,0));
                glm::mat4 translateRectangle = glm::translate (temp)* glm::rotate((float)(0), glm::vec3(0,1,0));;        // glTranslatef
                Matrices.model *= (translateRectangle);
                MVP = VP * Matrices.model;
                glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
                drawAllObjects(i,j);
            }
            j+=1;
        }
        i+=1;
    }
    //Matrices.view = glm::lookAt(camera_eye, target, up); 
    glm::mat4 VP;
    VP = Matrices.projection * Matrices.view;    
    glm::mat4 MVP; 
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    Matrices.model = glm::mat4(1.0f);    
    movementBox(0);
    glm::mat4 rotateRectangle = glm::rotate((float)(blockInitAngle*M_PI/180.0f), axis_rotation);
    glm::mat4 translateRectangle = glm::translate (boxPosition) * glm::rotate((float)(blockInitAngle*M_PI/180.0f), axis_rotation);;  
    Matrices.model *= (translateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(obj[5]);
    if(fallingBlock(var1, var2, var3, var4)) {
    	quit(window);
    }
    //Scoring of the player.
    showScore(gameScore%10);
    for(int i=0;i<7;i+=1){
    	if(ShowScoreInSegments[i] == 1){
    		cout << i << " " ;
			glm::mat4 MVP;  // MVP = Projection * View * Model
	        Matrices.model = glm::mat4(1.0f);
	        glm::mat4 ObjectTransform;
	        glm::mat4 translateObject = glm::translate (glm::vec3(objectScores[i].x , objectScores[i].y, 0.0f)); // glTranslatef
	        ObjectTransform=translateObject;
	        Matrices.model *= ObjectTransform;
	        MVP = VP * Matrices.model; // MVP = p * V * M        
	        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	        draw3DObject(objectScores[i].object);
    	}
    	cout << endl;
    }
    if(gameScore/10 != 0){
    	showScore(gameScore/10);
    	cout << "Yes" << endl;
    	for(int i=7;i<14;i+=1){
    		if(ShowScoreInSegments[i-7] == 1){
	    		cout << i << " " ;
				glm::mat4 MVP;  // MVP = Projection * View * Model
		        Matrices.model = glm::mat4(1.0f);
		        glm::mat4 ObjectTransform;
		        glm::mat4 translateObject = glm::translate (glm::vec3(objectScores[i].x , objectScores[i].y, 0.0f)); // glTranslatef
		        ObjectTransform=translateObject;
		        Matrices.model *= ObjectTransform;
		        MVP = VP * Matrices.model; // MVP = p * V * M        
		        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		        draw3DObject(objectScores[i].object);
	    	}
	    	cout << endl;		
    	}
    }
}
GLFWwindow* initGLFW (int width, int height){
    GLFWwindow* window; // window desciptor/handle
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);
    if (!window) {
    exit(EXIT_FAILURE);
        glfwTerminate();
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval( 1 );
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);
    glfwSetWindowCloseCallback(window, quit);
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
    return window;
}
void bringDownBricks(){
  for(int i=0;i<objectBricks.size();i+=1){
      int xa=rand()%objectBricks.size();
      if(objectBricks[xa].falling==0){
        objectBricks[xa].falling = 1;
        break;
      }
  }
}
void initGL (GLFWwindow* window, int width, int height){
    createRectangle ();
    
    createScores("scores",black,black,black,black,6.5,8.5,0.25,3);
    createScores("scores",black,black,black,black,8,7,3,0.25);
    createScores("scores",black,black,black,black,8,4,3,0.25);
    createScores("scores",black,black,black,black,6.5,2.5,0.25,3);
    createScores("scores",black,black,black,black,5,4,3,0.25);
    createScores("scores",black,black,black,black,5,7,3,0.25);
    createScores("scores",black,black,black,black,6.5,5.5,0.25,3);

    createScores("scores",black,black,black,black,2.5,8.5,0.25,3);
    createScores("scores",black,black,black,black,4,7,3,0.25);
    createScores("scores",black,black,black,black,4,4,3,0.25);
    createScores("scores",black,black,black,black,2.5,2.5,0.25,3);
    createScores("scores",black,black,black,black,1,4,3,0.25);
    createScores("scores",black,black,black,black,1,7,3,0.25);
    createScores("scores",black,black,black,black,2.5,5.5,0.25,3);


    
    


    programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
    Matrices.MatrixID = glGetUniformLocation(programID, "MVP");
    reshapeWindow (window, width, height);
    glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
    glClearDepth (1.0f);
    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);
}
int GoalBlockPosition(AllObjectsInfo * tempLaser){
  for(int i=0;i<objectMirror.size();i+=1){
    float xCoor1 = tempLaser->x + tempLaser->width*0.5*cos((tempLaser->anglePresent/180.0f)*M_PI);
    float yCoor1 = tempLaser->y + tempLaser->width*0.5*sin((tempLaser->anglePresent/180.0f)*M_PI);
    float distance_1 = (xCoor1 - objectMirror[i].x)/cos((objectMirror[i].anglePresent/180.0f)*M_PI);
    float distance_2 = (yCoor1 - objectMirror[i].y)/sin((objectMirror[i].anglePresent/180.0f)*M_PI);
    //cout << (xCoor1 - objectMirror[i].x) << " " << (yCoor1 - objectMirror[i].y) << endl;
    if(distance_1 >= -objectMirror[i].width*0.5f - 2 && distance_1 <= objectMirror[i].width*0.5f + 2 && distance_2 >= -objectMirror[i].width*0.5f - 2 && distance_2 <= objectMirror[i].width*0.5 + 2 && abs(distance_1-distance_2) <= 5.0f){
      tempLaser->anglePresent = 2*objectMirror[i].anglePresent - tempLaser->anglePresent;
      cout << "Hello"<< endl;
      return 1;
    }
  }
  return 0;
}
void drawMirror(){
  int x=rand()%3 + 1,r=rand()%400;
  for(int i=0;i<x;i++){
      int x1=rand()%50,y1=rand()%50,a1=rand()%6 - 3;
      GLint vect [] = {
        x1,y1,0,
        x1 + r*cos(a1),y1 + r*sin(a1),0
      };
      GLint colr [] = {
        0.3,0.3,1, // vertex 1
        0.3,0.3,1 // vertex 2
      };
      for(int j=0;j<6;j++){tempMirror.vertex[j]=vect[j];tempMirror.color[j]=colr[j];}
      mirrors.push_back(tempMirror);
  }
}

void fallBlocks(){
  int x = rand()%4 + 1,fl=0,x1,x2,colorBlock = rand()%3;
  vector <int > d;
  for(int i=0;i<fallingBlocks.size();i+=1){
    fl=0;
    for(int j=1;j<18;j+=3){
      fallingBlocks[i].vertex[j]-=25;
      if(fallingBlocks[i].vertex[j] <= -380) fl=1;
    }
    if(fl == 1){
      d.push_back(i);
    }
    /*else{
      glm::translate(glm::vec3(0.0f, -25, 0.0f));
    }*/
  }
  for(int i=0;i<d.size();i+=1){fallingBlocks.erase(fallingBlocks.begin() + d[i]);}
  timeDiff+=1;
  if(timeDiff%1 == 0){
    timeDiff = 0;
    if(colorBlock == 0){x=1;}
    for(int i=0;i<x;i+=1){
      GLint colr[18];
      x1=rand()%800 - 400;
      cout << "X1 is : " << x1 << "Loop size is : " << x << endl; 
      GLint vect [] = {
        x1,400,0, // vertex 1
        x1-10,400,0, // vertex 2
        x1-10, 375,0, // vertex 3

        x1-10, 375,0, // vertex 3
        x1, 375,0, // vertex 4
        x1,400,0  // vertex 1
      };
      if(colorBlock == 0){GLint colr [] = {
          0,0,0, // vertex 1
          0,0,0, // vertex 2
          0,0,0, // vertex 3

          0,0,0, // vertex 3
          0,0,0, // vertex 4
          0,0,0  // vertex 1
        };}
      else if(colorBlock == 1){GLint colr [] = {
          0,1,0, // vertex 1
          0,1,0, // vertex 2
          0,1,0, // vertex 3

          0,1,0, // vertex 3
          0,1,0, // vertex 4
          0,1,0  // vertex 1
        };}
      else if(colorBlock == 2){GLint colr [] = {
          1,0,0, // vertex 1
          1,0,0, // vertex 2
          1,0,0, // vertex 3

          1,0,0, // vertex 3
          1,0,0, // vertex 4
          1,0,0  // vertex 1
        };}
      for(int j=0;j<18;j+=1){tempBlock.vertex[j]=vect[j];tempBlock.color[j]=colr[j];}
      fallingBlocks.push_back(tempBlock);
    }
  }
}
int main (int argc, char** argv){
    int width = 600;
    int height = 600;
    rect_pos = glm::vec3(0, 0, 0);
    boxPosition = glm::vec3(-4,-4,1.25);
    boxXcoor1 = 9;boxXcoor2 = 9; 
    boxYcoor1 = 0;boxYcoor2 = 0;
    audio_init();
    boxAllignment = 1;
    axis_rotation = glm::vec3(0,1,0);
    camera_eye = glm::vec3(0,-5,8);
    GLFWwindow* window = initGLFW(width, height);
    initGLEW();
    initGL (window, width, height);
    last_update_time = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        audio_play();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        current_time = glfwGetTime();
        last_update_time = current_time;
        draw(window, 0, 0, 1, 1);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
}
