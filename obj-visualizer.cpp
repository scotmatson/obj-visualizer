/*
  File:
    Matson_Scot_programming_project_3.cpp

  Author:
    Scot Matson

  Programming Project 3:
    Alias/Wavefront OBJ File Visualizer

  macOS Build:
    g++ -Wno-deprecated-declarations -framework openGL -framework GLUT [file.ext]
*/
#include <GLUT/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define X_RESOLUTION 1280
#define Y_RESOLUTION 800
#define GLUT_KEY_ESC 27
#define GLUT_KEY_COMMA 44
#define GLUT_KEY_PERIOD 46
#define GLUT_KEY_1 49
#define GLUT_KEY_2 50
#define GLUT_KEY_3 51
#define GLUT_KEY_4 52
#define GLUT_KEY_B 98
#define GLUT_KEY_H 104
#define GLUT_KEY_J 106
#define GLUT_KEY_K 107
#define GLUT_KEY_L 108
#define GLUT_KEY_P 112
#define GLUT_KEY_S 115
#define GLUT_KEY_T 116
#define GLUT_KEY_W 119

typedef struct node {
  GLfloat x;
  GLfloat y;
  GLfloat z;
  GLint   r;
  GLint   g;
  GLint   b;
  struct node *next;
} node_t;

void init(void);
void display(void);
void reshape(int, int);
void keyboard(unsigned char, int, int);
void arrow_keys(int, int, int);
void mouse(int, int);
void face_parser(char*);
void vertex_parser(char*);
void push(node_t*);
void parse_obj(void);
void draw_model(void);
void draw_bounds(void);
void find_bounds(node_t*);
void find_center(void);

////////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
////////////////////////////////////////////////////////////////////////////////
// Globals for obj file parsing
const int MAXLINE      = 80;
const int COMMENT      = '#';
const int VERTEX       = 'v';
const int FACE         = 'f';
const int VERTEX_COUNT = 2503;
const int FACE_COUNT   = 4968;
char vertices[VERTEX_COUNT][MAXLINE];
node_t *head;

// Object bounds
GLfloat min_x =  FLT_MAX;
GLfloat min_y =  FLT_MAX;
GLfloat min_z =  FLT_MAX;
GLfloat max_x = -FLT_MAX;
GLfloat max_y = -FLT_MAX;
GLfloat max_z = -FLT_MAX;
float center_x;
float center_y;
float center_z;

// Display background color (0/0/0 == Black)
GLfloat const BG_RED   = 0.0f;
GLfloat const BG_GREEN = 0.0f;
GLfloat const BG_BLUE  = 0.0f;
GLfloat const BG_ALPHA = 0.0f;

GLfloat const FIELD_OF_VIEW = 45.0f;
GLfloat const FAR_CLIPPING_PLANE = 100.0f;
GLfloat const NEAR_CLIPPING_PLANE = 0.0001f;

GLfloat const OBJ_SCALE_FACTOR = 4.0f;
GLfloat const OBJ_Y_OFFSET     = 0.5f;
const GLfloat CAMERA_MOVE_SPEED = 0.1f;
const GLfloat CAMERA_ROTATION_SPEED = 5.0f;

// Draw modes
GLenum polygon_mode = GL_LINE; // Default, alternatively use GL_FILL
GLenum draw_mode = GL_TRIANGLES; // Default, alternatively use GL_POINTS

// Color modes
GLboolean isBounded     = 0;
GLboolean isDisco       = 0;
GLboolean isGrayscale   = 0;
GLboolean isFlashing    = 0;
GLboolean isSpacey      = 0;
GLboolean isInteractive = 0;
GLfloat obj_r = 0.0f;
GLfloat obj_g = 0.0f;
GLfloat obj_b = 0.0f;

// Camera initial position
GLfloat cam_x;
GLfloat cam_y;
GLfloat cam_z = 1.4f;
GLfloat camera_y_angle_degrees = 0.0f;
GLfloat camera_x_angle_degrees = 0.0f;

// Position of the camera 'eye'
GLfloat camera_eye_x;
GLfloat camera_eye_y;
GLfloat camera_eye_z;

// Position of the reference point
GLfloat camera_lookAt_x;
GLfloat camera_lookAt_y;
GLfloat camera_lookAt_z;

// Direction of the 'up' vector
GLfloat camera_vector_x = 0.0f;
GLfloat camera_vector_y = 1.0f;
GLfloat camera_vector_z = 0.0f;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/*
 * Push nodes onto a linked list
 */
void push(node_t *new_node) {
  if (head == NULL) {
    head = (node_t*) malloc(sizeof(node_t));
    head->x = new_node->x;
    head->y = new_node->y;
    head->z = new_node->z;
    head->next = NULL;
  }
  else {
    node_t *current = head;

    while (current->next != NULL) {
      current = current->next;
    }

    current->next = (node_t*) malloc(sizeof(node_t));
    current->next->x = new_node->x;
    current->next->y = new_node->y;
    current->next->z = new_node->z;
    current->next->next = NULL;
  }
}

/*
 * Parse faces from an obj file and pass
 * the coordinate references to a vertex parser
 */
void face_parser(char *str) {
  char *token;
  char *vert1, *vert2, *vert3;

  token = strtok(str, " ");
  vert1 = (char*) malloc((strlen(vertices[atoi(token)-1])+1) * sizeof(char));
  strcpy(vert1, vertices[atoi(token)-1]);

  token = strtok(NULL, " ");
  vert2 = (char*) malloc((strlen(vertices[atoi(token)-1])+1) * sizeof(char));
  strcpy(vert2, vertices[atoi(token)-1]);

  token = strtok(NULL, " ");
  vert3 = (char*) malloc((strlen(vertices[atoi(token)-1])+1) * sizeof(char));
  strcpy(vert3, vertices[atoi(token)-1]);

  vertex_parser(vert1);
  free(vert1);
  vertex_parser(vert2);
  free(vert2);
  vertex_parser(vert3);
  free(vert3);
}

/*
 *  Parse vertices from an obj file
 */
void vertex_parser(char *str) {
  char *token;
  node_t triangles;

  token = strtok(str, " ");
  triangles.x = atof(token) * OBJ_SCALE_FACTOR;
  triangles.r = rand() % 255;

  token = strtok(NULL, " ");
  triangles.y = atof(token) * OBJ_SCALE_FACTOR - OBJ_Y_OFFSET;
  triangles.g = rand() % 255;

  token = strtok(NULL, " ");
  triangles.z = atof(token) * OBJ_SCALE_FACTOR;
  triangles.b = rand() % 255;

  triangles.next = NULL;
  find_bounds(&triangles);
  push(&triangles);
}

/*
 *  Calculates and stores the bounds
 *  for the object
 */
void find_bounds(node_t *node){
  /* Min/Max X */
  if (node->x < min_x) {min_x = node->x;}
  if (node->x > max_x) {max_x = node->x;}

  /* Min/Max X */
  if (node->y < min_y) {min_y = node->y;}
  if (node->y > max_y) {max_y = node->y;}

  /* Min/Max X */
  if (node->z < min_z) {min_z = node->z;}
  if (node->z > max_z) {max_z = node->z;}
}

/*
 *  Finds the intersection of 
 *  the x, y, and z planes.
 */
void find_center() {
  center_x = (max_x + min_x)/2;
  center_y = (max_y + min_y)/2;
  center_z = (max_z + min_z)/2;
}

/*
 *  Reads and parses an obj file
 */
void parse_obj() {
  FILE *fp;
  char line[MAXLINE];
  int c;

  fp = fopen("res/bunny.obj", "r");
  if (!fp) {
    perror("Error opening file");
    exit(1);
  }

  int i = 0;
  do {
    switch (c = fgetc(fp)) {
      case COMMENT:
        fgets(line, MAXLINE, fp); // Consume comments
        break;
      case VERTEX:
        fgets(vertices[i], MAXLINE, fp);
        i++;
        break;
      case FACE:
        fgets(line, MAXLINE, fp);
        face_parser(line);
        break;
      default:
        break;
    }
  } while (!feof(fp));
  fclose(fp);
}

/*
 * Draws a shape parsed from an obj file
 */
void draw_model() {
  node_t *current = head;
  glPushMatrix();
  glBegin(draw_mode);
    int i = 0;
    int j = rand() % 255;
    int k = rand() % 255;
    int l = rand() % 255;
    while (current != NULL) {
      if (i == 256) { i = 0; }

      if (isDisco) {
        glColor3ub(rand() % 256, rand() % 256, rand() % 256);
      }
      else
      if (isGrayscale) {
        glColor3ub(i, i, i);
      }
      else
      if (isFlashing) {
        glColor3ub(j, k, l);
      }
      else
      if (isSpacey) {
        glColor3ub(current->r, current->g, current->b);
      }
      else {
        glColor3ub(255, 255, 255);
      }
      glVertex3d(current->x, current->y, current->z);
      current = current->next;
      i++;
    }
  glEnd();
  glPopMatrix();
  glutPostRedisplay();
}

/*
 *  Initialize the scene
 */
void init() {
  srand((unsigned)time(NULL));
  // obj file parser
  parse_obj();
  find_center();

  // Initialize camera values
  cam_x = center_x;
  cam_y = center_y;
  cam_z = 1.4f;
  camera_lookAt_x = center_x;
  camera_lookAt_y = center_y-0.1f;;
  camera_lookAt_z = center_z;

  // bg-color
  glClearColor(BG_RED, BG_GREEN, BG_BLUE, BG_ALPHA);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_COLOR_MATERIAL);
}

/*
 *  Update the scene
 */
void display(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();

  gluLookAt(cam_x+camera_eye_x, cam_y+camera_eye_y, cam_z+camera_eye_z,
            camera_lookAt_x, camera_lookAt_y, camera_lookAt_z,
            camera_vector_x, camera_vector_y, camera_vector_z);
  glRotatef(camera_x_angle_degrees, 1.f, 0.f, 0.f);
  glRotatef(camera_y_angle_degrees, 0.f, 1.f, 0.f);

  glPolygonMode(GL_FRONT_AND_BACK, polygon_mode); 

  draw_model();
  if (isBounded) {
    draw_bounds();
  }

  glutSwapBuffers();
}

/*
 *  Surrounds the model with a bounded box
 */
void draw_bounds() {
  glPushMatrix();
  glColor3ub(255, 255, 255);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 
  glBegin(GL_QUADS);
    // +Z
    glVertex3d(min_x, max_y, max_z);
    glVertex3d(min_x, min_y, max_z);
    glVertex3d(max_x, min_y, max_z);
    glVertex3d(max_x, max_y, max_z);

    // -Z
    glVertex3d(min_x, max_y, min_z);
    glVertex3d(min_x, min_y, min_z);
    glVertex3d(max_x, min_y, min_z);
    glVertex3d(max_x, max_y, min_z);

    // +X
    glVertex3d(max_x, min_y, max_z);
    glVertex3d(max_x, min_y, min_z);
    glVertex3d(max_x, max_y, min_z);
    glVertex3d(max_x, max_y, max_z);

    // -X
    glVertex3d(min_x, min_y, max_z);
    glVertex3d(min_x, min_y, min_z);
    glVertex3d(min_x, max_y, min_z);
    glVertex3d(min_x, max_y, max_z);

    // +Y
    glVertex3d(min_x, max_y, max_z);
    glVertex3d(min_x, max_y, min_z);
    glVertex3d(max_x, max_y, min_z);
    glVertex3d(max_x, max_y, max_z);

    // +Y
    glVertex3d(min_x, min_y, max_z);
    glVertex3d(min_x, min_y, min_z);
    glVertex3d(max_x, min_y, min_z);
    glVertex3d(max_x, min_y, max_z);
  glEnd();
  glPopMatrix();
}

/*
 *  Maintains display dimensions when window size is modified
 */
void reshape(int w, int h) {
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(FIELD_OF_VIEW,
                 (float)w / (float)h, // Aspect ratio
                 NEAR_CLIPPING_PLANE,
                 FAR_CLIPPING_PLANE);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

/*
 *  Keyboard event handler
 */
void keyboard(unsigned char key, int x, int y) {
  switch(key) {
    case GLUT_KEY_ESC:
      exit(0);
      break;
    case GLUT_KEY_W: 
      // Wireframe mesh 
      polygon_mode = GL_LINE;
      draw_mode = GL_TRIANGLES;
      break;
    case GLUT_KEY_S:
      // Solid body model
      polygon_mode = GL_FILL;
      draw_mode = GL_TRIANGLES;
      break;
    case GLUT_KEY_H:
      // Richard
      camera_eye_x    += CAMERA_MOVE_SPEED;
      camera_lookAt_x += CAMERA_MOVE_SPEED;
      break;
    case GLUT_KEY_J:
      // Stallman
      camera_eye_y    += CAMERA_MOVE_SPEED;
      camera_lookAt_y += CAMERA_MOVE_SPEED;
      break;
    case GLUT_KEY_K:
      // Hearts
      camera_eye_y    -= CAMERA_MOVE_SPEED;
      camera_lookAt_y -= CAMERA_MOVE_SPEED;
      break;
    case GLUT_KEY_L:
      // Vim
      camera_eye_x    -= CAMERA_MOVE_SPEED;
      camera_lookAt_x -= CAMERA_MOVE_SPEED;
      break;
    case GLUT_KEY_PERIOD:
      camera_eye_z -= CAMERA_MOVE_SPEED;
      camera_lookAt_z -= CAMERA_MOVE_SPEED;
      break;
    case GLUT_KEY_COMMA:
      camera_eye_z += CAMERA_MOVE_SPEED;
      camera_lookAt_z += CAMERA_MOVE_SPEED;
      break;
    case GLUT_KEY_T:
      draw_mode = GL_TRIANGLES;
      break;
    case GLUT_KEY_P:
      draw_mode = GL_POINTS;
      break;
    case GLUT_KEY_B:
      isBounded = (isBounded) ? 0 : 1;
      break;
    case GLUT_KEY_1:
      isDisco = (isDisco) ? 0 : 1;
      isGrayscale=isFlashing=isSpacey=isInteractive =  0;
      break;
    case GLUT_KEY_2:
      isGrayscale = (isGrayscale) ? 0 : 1;
      isDisco=isFlashing=isSpacey=isInteractive = 0;
      break;
    case GLUT_KEY_3:
      isFlashing = (isFlashing) ? 0 : 1;
      isDisco=isGrayscale=isSpacey=isInteractive = 0;
      break;
    case GLUT_KEY_4:
      isSpacey = (isSpacey) ? 0: 1;
      isDisco=isGrayscale=isFlashing=isInteractive = 0;
      break;
    default:
      break;
  }
  glutPostRedisplay();
}

/*
 *  Arrow keys event handler
 */
void arrow_keys(int key, int x, int y) {
  GLfloat distance;
  float camera_angle_radians;
  switch(key) {
    case GLUT_KEY_UP:
      if (camera_x_angle_degrees >= 360.0f) {
        camera_x_angle_degrees = 0.0f;
      }
      else {
        camera_x_angle_degrees = camera_x_angle_degrees + CAMERA_ROTATION_SPEED;
      }
      break;
    case GLUT_KEY_RIGHT:
      if (camera_y_angle_degrees >= 360.0f) {
        camera_y_angle_degrees = 0.0f;
      }
      else {
        camera_y_angle_degrees = camera_y_angle_degrees + CAMERA_ROTATION_SPEED;
      }
      break;
    case GLUT_KEY_LEFT:
      // Counter-clockwise rotation
      if (camera_y_angle_degrees <= 0.0f) {
        camera_y_angle_degrees = 360.0f;
      }
      else {
        camera_y_angle_degrees = camera_y_angle_degrees - CAMERA_ROTATION_SPEED;
      }
      break;
    case GLUT_KEY_DOWN:
      if (camera_x_angle_degrees <= 0.0f) {
        camera_x_angle_degrees = 360.0f;
      }
      else {
        camera_x_angle_degrees = camera_x_angle_degrees - CAMERA_ROTATION_SPEED;
      }
      break;
    default: 
      break;
  }
  glutPostRedisplay();
}

/*
 *  The main method
 */
int main(int argc, char **argv) {
  // GLUT initialization
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);

  // Create the window
  glutInitWindowSize(X_RESOLUTION, Y_RESOLUTION);
  glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH)-X_RESOLUTION)/2,
                         (glutGet(GLUT_SCREEN_HEIGHT)-Y_RESOLUTION)/2);
  glutCreateWindow("Project 3");

  // Scene initialization
  init();

  // Register the callbacks
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(arrow_keys);

  // Start the main loop
  glutMainLoop();

  // Once we have entered the OpenGL execution cycle, the application will no
  // longer return from main unless an error has occurred.
  return 1;
}
