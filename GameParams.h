#ifndef GAME_PARAMS
#define GAME_PARAMS

#include <GL/glew.h>
#include <GL/glut.h>
#include <al.h>
#include <alc.h>
//#include <GL/glu.h>
//#include <GL/GL.h>

//General parameters
const float PI = 3.14159265;
const float FRAMES_PER_SECOND = 60.0;

//Window parameters
const int INITIAL_WINDOW_X = 0;
const int INITIAL_WINDOW_Y = 0;
const int INITIAL_WINDOW_HEIGHT = 500;
const int INITIAL_WINDOW_WIDTH = 500;

//Camera parameters
const float CAMERA_FOVY = 50.0;
const float Z_NEAR = 1.0;
const float Z_FAR = 30000.0;
const float	INITIAL_WALKING_X = 50.5877228;//70.5877228; //0 //120
const float INITIAL_WALKING_Y = 80.0; //80
const float INITIAL_WALKING_Z = -175.260544; //0 //-90
const float INITIAL_VERT_ANGLE = .73999;//.73999;  //pi/2
const float INITIAL_HORIZ_ANGLE = -.40999;//-.40999;
const float WALK_SPEED = 10.0; //20.0;
const float WALK_ACCELERATION = 0.93;//1.133;
const float JUMP_SPEED = 10.0;
const float JUMP_ACCELERATION = -1.0;
const float VERT_UPPER_BOUND = 3.11;
const float VERT_LOWER_BOUND = 0.2;
const float LOOK_VELOCITY = 0.002;
const float MIN_DISTANCE_FROM_WALL = 50.0; //15

//Room parameters
const float WALL_DISTANCE = 512.0; //Distance from center to wall 400 512
const float WALL_HEIGHT = 281.0; //Total height of wall 200 256
const float WALL_TEXTURE_S = 32.0; //65 32
const float WALL_TEXTURE_T = 10.0; //20 8
const float FLOOR_TEXTURE_S = 32.0; //32
const float FLOOR_TEXTURE_T = 32.0; //32
const int WALL_QUADS_HORIZ = 10;
const int WALL_QUADS_VERT = 10;
const int FLOOR_QUADS_HORIZ = 10;
const int FLOOR_QUADS_VERT = 10;
const float LOOK_RANGE = 50.0;
const int NUM_LOOKS = 2;

//Elementalist-Denial font parameters
const int DENIAL_CHAR_WIDTH = 32;//32;
const int DENIAL_CHAR_HEIGHT = 65;//65;
const int DENIAL_WIDTH_OFFSET = 7;//4

//Desktop parameters
const float ALPHA_INCREMENT = 0.05;

//Power button parameters
const int POWER_BUTTON_MS_TO_FRAME_CHANGE = 40;
const int POWER_BUTTON_MAX_FRAMES = 6;

//Text field parameters
const int TEXT_FIELD_MS_TO_FRAME_CHANGE = 500;

#endif