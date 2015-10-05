//cs335 Spring 2015 Lab-1
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include <unistd.h>
extern "C" {
  #include "fonts.h"
}

#define WINDOW_WIDTH  500
#define WINDOW_HEIGHT 360
#define BOX_HEIGHT 7
#define BOX_WIDTH 60
#define START_Y 320
#define START_X 80
#define CIRCLE_RADIUS 150

#define MAX_PARTICLES 100000
#define GRAVITY 0.1
#define rnd() (double)rand()/(double)RAND_MAX


//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//Structures

struct Vec {
    float x, y, z;
};

struct Shape {
    float width, height;
    float radius;
    Vec center;
};


struct Particle {
    Shape s;
    Vec velocity;
};

struct Game {
    Shape box[5];
    Shape circle;
    Particle particle[MAX_PARTICLES];
    int lastMousex, lastMousey;
    int n;
};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void doMakeParticles(Game *game);
bool check_b(XEvent *e);
bool check_escape(XEvent *e);
void movement(Game *game);
void render(Game *game);

int main(void)
{
    bool b_done=false;
    bool done=false;
    srand(time(NULL));
    initXWindows();
    init_opengl();
    //declare game object
    Game game;
    game.n=0;

    //declare a box shape
    game.box[0].width = BOX_WIDTH;
    game.box[0].height = BOX_HEIGHT;
    game.box[0].center.x = WINDOW_WIDTH/7;
    game.box[0].center.y = WINDOW_HEIGHT - WINDOW_HEIGHT/7;

    game.box[1].width = BOX_WIDTH;
    game.box[1].height = BOX_HEIGHT;
    game.box[1].center.x = 2*WINDOW_WIDTH/7;
    game.box[1].center.y = WINDOW_HEIGHT - 2*(WINDOW_HEIGHT/7);

    game.box[2].width = BOX_WIDTH;
    game.box[2].height = BOX_HEIGHT;
    game.box[2].center.x = 3*WINDOW_WIDTH/7;
    game.box[2].center.y = WINDOW_HEIGHT - 3*(WINDOW_HEIGHT/7);

    game.box[3].width = BOX_WIDTH;
    game.box[3].height = BOX_HEIGHT;
    game.box[3].center.x = 4*WINDOW_WIDTH/7;
    game.box[3].center.y = WINDOW_HEIGHT - 4*(WINDOW_HEIGHT/7);

    game.box[4].width = BOX_WIDTH;
    game.box[4].height = BOX_HEIGHT;
    game.box[4].center.x = 5*WINDOW_WIDTH/7;
    game.box[4].center.y = WINDOW_HEIGHT - 5*(WINDOW_HEIGHT/7);

    game.circle.radius = CIRCLE_RADIUS;
    game.circle.center.x = WINDOW_WIDTH;
    game.circle.center.y = -80;

    while(XPending(dpy)) {
       XEvent e;
       XNextEvent(dpy, &e);
       b_done = check_b(&e);
          if (b_done){
              break;
          }
    }
    //start animation
    while(!done) {
        while(XPending(dpy)) {
           XEvent e;
           XNextEvent(dpy, &e);
           done = check_escape(&e);
        }
        doMakeParticles(&game);
        movement(&game);
        render(&game);
        glXSwapBuffers(dpy, win);    
    }
    cleanupXWindows();
    return 0;
}

void set_title(void)
{
    //Set the window title bar.
    XMapWindow(dpy, win);
    XStoreName(dpy, win, "335 Lab1   LMB for particle");
}

void cleanupXWindows(void) {
    //do not change
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
}

void initXWindows(void) {
    //do not change
    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
    dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
        std::cout << "\n\tcannot connect to X server\n" << std::endl;
        exit(EXIT_FAILURE);
    }
    Window root = DefaultRootWindow(dpy);
    XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
    if(vi == NULL) {
        std::cout << "\n\tno appropriate visual found\n" << std::endl;
        exit(EXIT_FAILURE);
    } 
    Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
        ButtonPress | ButtonReleaseMask |
        PointerMotionMask |
        StructureNotifyMask | SubstructureNotifyMask;
    win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
            InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
    set_title();
    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void)
{
    //OpenGL initialization
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    //Initialize matrices
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    //Set 2D mode (no perspective)
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    //Set the screen background color
    glClearColor(0.1, 0.1, 0.1, 1.0);
	glEnable(GL_TEXTURE_2D);
    initialize_fonts();
}


void makeParticle(Game *game, int x, int y) {
    if (game->n >= MAX_PARTICLES)
        return;
    std::cout << "makeParticle() " << x << " " << y << std::endl;
    //position of particle
    Particle *p = &game->particle[game->n];
    p->s.center.x = x;
    p->s.center.y = y;
    p->velocity.y = rnd()*1.0 + 0.7;
    p->velocity.x = 0.1;
    game->n++;
}

void doMakeParticles(Game *game){
   int y = START_Y;
   int x = START_X;
      for (int i=0;i<10; i++){
         makeParticle(game, x, y);
     }
}

bool check_b(XEvent *e)
{
    //Was 'B' pressed?
    if (e->type == KeyPress) {
        int key = XLookupKeysym(&e->xkey, 0);
        if (key == XK_b) {
            return true;
        }
    }
    return false;
}

bool check_escape(XEvent *e)
{
    //Was 'ESCAPE' pressed?
    if (e->type == KeyPress) {
        int key = XLookupKeysym(&e->xkey, 0);
        if (key == XK_Escape) {
            return true;
        }
    }
    return false;
}

void movement(Game *game)
{
    Particle *p;

    if (game->n <= 0)
        return;

    for (int i=0;i<10;i++)
        makeParticle(game, game->lastMousex, game->lastMousey);

    for (int i=0; i<game->n; i++){
        p = &game->particle[i];
        p->s.center.x += p->velocity.x;
        p->s.center.y += p->velocity.y;
        p->velocity.y -= GRAVITY;

        //check for collision with shapes...
        for (int j=0;j<5;j++){
            Shape *s=&game->box[j];

            if (p->s.center.y < s->center.y + s->height && (p->s.center.x >= s->center.x - s->width && p->s.center.x <= s->center.x + s->width) && (p->s.center.y > s->center.y - s->height)){
                p->velocity.y *= -.25;
                p->s.center.y = s->center.y + s->height + 0.01;
                p->velocity.x = 0.5;
            }

        }
        //CALCULATE PARTICLES DISTANCE TO CIRCLE CENTER:
        Shape *s=&game->circle;
        double distance = sqrt(pow(p->s.center.y - s->center.y,2) + pow(p->s.center.x - s->center.x,2));
        if (distance < s->radius){
                p->velocity.y *= -.25;
                p->velocity.x += rnd() -0.65;
                p->s.center.y = s->center.y + sqrt(pow(s->radius,2.0) - (pow(p->s.center.x - s->center.x,2.0))) + 0.01;
        }

        //check for off-screen
        if (p->s.center.y < 0.0 || p->s.center.y > WINDOW_HEIGHT) {
            std::cout << "off screen" << std::endl;
            memcpy(&game->particle[i], &game->particle[game->n-1], sizeof(Particle));
            game->n--;
        }
    }
}

void render(Game *game)
{
    float w, h;
    glClear(GL_COLOR_BUFFER_BIT);
    //Draw shapes...

    //draw box
    for (int j=0;j<5;j++){
        Shape *s;
        glColor3ub(0,255,0);
        s = &game->box[j];
        glPushMatrix();
        glTranslatef(s->center.x, s->center.y, s->center.z);
        w = s->width;
        h = s->height;
        glBegin(GL_QUADS);
        glVertex2i(-w,-h);
        glVertex2i(-w, h);
        glVertex2i( w, h);
        glVertex2i( w,-h);
        glEnd();
        glPopMatrix();
    }
    //-----------------------------------------------------
    

    //DRAW CIRCLE:
    //static int first_time=1;
    //if (first_time){
    glPushMatrix();
    glColor3ub(0,255,0);
    glBegin(GL_TRIANGLE_FAN);
    int num_segments=100;
    Shape *s;
    s = &game->circle;
    for(int seg = 0; seg < num_segments; seg++)
    {
        float theta = 2.0f * 3.1415926f * float(seg) / float(num_segments);//get the current angle

        float x = s->radius * cosf(theta);//calculate the x component
        float y = s->radius * sinf(theta);//calculate the y component

        glVertex2f(s->center.x + x + 0.5, s->center.y + y + 0.5);//output vertex

    }
    glEnd();
    glPopMatrix();
    //first_time=0;
    //}
    //********************************************************
    for (int i=0; i<game->n; i++){
        //draw all particles here
        glPushMatrix();
        glColor3ub(0,0,((int)100 + 155*rnd()));
        Vec *c = &game->particle[i].s.center;
        w = 2;
        h = 2;
        glBegin(GL_QUADS);
        glVertex2i(c->x-w, c->y-h);
        glVertex2i(c->x-w, c->y+h);
        glVertex2i(c->x+w, c->y+h);
        glVertex2i(c->x+w, c->y-h);
        glEnd();
        glPopMatrix();
    }
    
    //DRAW TEXTS:    
	//glBindTexture(GL_TEXTURE_2D, 0);  
	unsigned int cref = 0x00ff00ff;
    
    Rect r0;
    r0.bot = WINDOW_HEIGHT-20;
	r0.left = 10;
    r0.center = 0;
	ggprint12(&r0, 70, cref, "Waterfall Model, BY: Keith Harryman");    
  
    Rect r1;
    r1.bot = WINDOW_HEIGHT - WINDOW_HEIGHT/7 - BOX_HEIGHT;
	r1.left = WINDOW_WIDTH/7 + 0.3 - BOX_WIDTH/2;
    r1.center = 0;
	ggprint12(&r1, 30, cref, "Requirements");    
	
	Rect r2;
    r2.bot = WINDOW_HEIGHT - 2*WINDOW_HEIGHT/7 - BOX_HEIGHT;
	r2.left = 2*WINDOW_WIDTH/7 + 0.3 - BOX_WIDTH/2;
    r2.center = 0;
	ggprint12(&r2, 30, cref, "Design"); 
	
	Rect r3;
    r3.bot = WINDOW_HEIGHT - 3*WINDOW_HEIGHT/7 - BOX_HEIGHT;
	r3.left = 3*WINDOW_WIDTH/7 + 0.3 - BOX_WIDTH/2;
    r3.center = 0;
	ggprint12(&r3, 30, cref, "Coding");
	
	Rect r4;
    r4.bot = WINDOW_HEIGHT - 4*WINDOW_HEIGHT/7 - BOX_HEIGHT;
	r4.left = 4*WINDOW_WIDTH/7 + 0.3 - BOX_WIDTH/2;
    r4.center = 0;
	ggprint12(&r4, 30, cref, "Testing");
	
	Rect r5;
    r5.bot = WINDOW_HEIGHT - 5*WINDOW_HEIGHT/7 - BOX_HEIGHT;
	r5.left = 5*WINDOW_WIDTH/7 + 0.3 - BOX_WIDTH/2;
    r5.center = 0;
	ggprint12(&r5, 30, cref, "Maintenance");
    
}



