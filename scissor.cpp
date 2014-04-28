// Ranjith Nair
//Copy and paste this codes in a new VC++ empty console project

/* This program is made using
>>OpenGL for Graphics, 
>>GLUT for window creation display image after rendering
>>SDL for Haptic feedback and joystick controls
>>WinAPI for virtual COM port creation to communicate with arduino through serial protocol.
>>>> ***Arduino Reads the angle of scissor arm movement through a potentiometer attached to its end gear. 

---The main objective of this program is to create a game with a scissor which which open/close on receipt of signals from external input through serial port(in this case COM4).
---The scissor can move forward and backward and sideways through joystick controls
---move the ball with the scissor when the tip of the scissor touches the ball
---sends signal to external device through serial communication for force feedback when the ball touches the scissor and angle between scissor arms is less than 30 degrees th

*/
/* Setting up SDL is crucial and all files to setup SDL and the setup process can be found in the link given below.
http://lazyfoo.net/SDL_tutorials/lesson01/windows/msvsnet2010e/index.php */

/* The haptic feedback will not work unless the joystick driver is not installed in the computer. So the driver setup file can be found in the logitech website */

#include <iostream>
#include <glut.h>  // GLUT library for OpenGL
#include <SDL_haptic.h> // libraries for force feedback using SDL 
#include <SDL.h> // standard library for SDL
#include <windows.h>




///////// Defining initial scissor co-ordinates//////////////////////////
static GLfloat scissor_angle = 10.0;  //angle between scissor arms
static GLfloat scissor_tilt = 0.0;    //tilt of scissor with horizontal-----for the time being not used
static GLfloat scissor_endpos = 0.0;  //z co-ordinate of the end of scissor-----not used

///////// Defining initial scissor position///////////////////////
static GLfloat xpos = 0.0;  //x- position of scissor
static GLfloat ypos = 3.3;  //y- position of scissor
static GLfloat zpos = 0.0;  // z position of scissor

///////// Defining initial position of the ball //////////////////////////////
static GLfloat ball_xpos = 0.0;
static GLfloat ball_ypos = 1.5;
static GLfloat ball_zpos = 0.0;


SDL_Haptic *haptic;      //an event declared for SDL_Haptic
SDL_HapticEffect effect; //an event declared for SDL_HapticEffect
int effect_id;           //Required for defining a unique id for the effect 

bool held = false;          //To check if the scissor tip touches the ball
HANDLE hCom;                //Object created for the Class HANDLE used for serial communication  
char receiveBuff[8] = {0};  //variable to store values read through serial port
char sendBuff[8] = {0};     // variable array to store data to be sent through serial port
DWORD dwBytesRead = 0;      


/* Latest computers doe not have a serial port. So devices 
like joysticks and mouse communicates through a virtual 
serial port created using Windows API.*/

void serial_com(void)  //  Function defined to open serial port and define communication parameters 
{

hCom = CreateFile("\\\\.\\COM4",              // open com4:
                    GENERIC_READ | GENERIC_WRITE, // Flags required for reading and writing operations
                    0,                            
                    NULL,                         
                    OPEN_EXISTING,              
                    FILE_ATTRIBUTE_NORMAL,
                    NULL) ;

if (hCom == INVALID_HANDLE_VALUE) //Gives error if serial port cannot be opened 
{
printf("ERROR - Unable to open serial port.\n");
}

DCB dcbSerialParams = {0};                           //Object created for the class DCB where communication parameters are set
dcbSerialParams.DCBlength=sizeof(dcbSerialParams);  //Defining the size of the DCB object
//
//
//
if (!GetCommState(hCom, &dcbSerialParams))   // Gives error if Unable to get the current state 
{
printf("ERROR getting state.\n");
}

/////////////////Defining COM port state parameters/////////////////////////////////////////////////////////
dcbSerialParams.BaudRate=CBR_9600;
dcbSerialParams.ByteSize=8;
dcbSerialParams.StopBits=ONESTOPBIT;
dcbSerialParams.Parity=NOPARITY;
if(!SetCommState(hCom, &dcbSerialParams))
{

printf("ERROR - setting serail port state.\n");//Gives error if unable to set port state
}

/* If there is no data coming from the port, then the program will hang 
in an attempt to read from the serial port, To avoid this we 
have to set timeouts */

COMMTIMEOUTS timeouts={0};
timeouts.ReadIntervalTimeout=50;
timeouts.ReadTotalTimeoutConstant=50;
timeouts.ReadTotalTimeoutMultiplier=10;
timeouts.WriteTotalTimeoutConstant=50;
timeouts.WriteTotalTimeoutMultiplier=10;
if(!SetCommTimeouts(hCom, &timeouts))
{
         printf("ERROR - timeouts.\n");// Gives error if unable to set timeout
}

}   




////////////////////////////reading data from the serial port///////////////////////////////////////////////////////////////
void read_pot(void)
{

if(!ReadFile(hCom, receiveBuff, 1, &dwBytesRead, NULL)) // opens file to read through object hCom and stores value in the variable 'receivebuff'
                                                      
    
        {
printf("ERROR - cant read.\n"); // if cannot read  
    }

else                                    // if data is read

    {
    scissor_angle = receiveBuff[0];     // changes the value of scissor angle according to the value received 

glutPostRedisplay();	            // redisplay the frame after rendering
         }

}	


////////////////////////////////////////////Function to create a Haptic feedback for joystick only using SDL//////////////////////

int hapticfeedback(void)                               
{

SDL_Init(SDL_INIT_EVERYTHING);  //Initialize all the SDL parameters

    haptic = SDL_HapticOpen(0);     //Opens first Joystick 
  if (haptic==NULL)
return -1;

memset(&effect,0,sizeof(SDL_HapticEffect));
effect.type = SDL_HAPTIC_CONSTANT;                     // sets type of effect as Constant. other types are Sine, Sawtooth, etc
effect.constant.direction.type = SDL_HAPTIC_POLAR;     // set direction type as Polar, other types are Cartesian and Sperical
effect.constant.direction.dir[0] = 0;                  // 0 = north, 9000 = west, 18000 = south, 27000 = east 
effect.constant.length = INFINITE;                     // Will continue to give force feedback until it receives input and stops when it does not
    effect.constant.level = 32767;                         // 0 = minimum, 32767 = maximum

    effect_id = SDL_HapticNewEffect( haptic, &effect );	   // give an identification to this effect, like this we can create many 
                                                       // other types of haptic effect and give it a different id and use it whenever 
                                                       // the particular type of feedback is required

SDL_HapticRunEffect( haptic, effect_id, 1 );           // upload the particular effect, can be put anywhere required,
                                                       //in this case we have only one effect, hene put in the function 
SDL_HapticClose(haptic);	                           // Close the haptic feedback immediately after it is not required
return 0;
}


///////////////////////////////write data///////////////////////////////////////////
void scissor_close(int status)          
{   

sendBuff[0] = status; // set to number 11
WriteFile(hCom, sendBuff, 1, &dwBytesRead, NULL); ///////////// write data through object hCom 
//glutPostRedisplay();           // not required
}


/////////////////////////////Initialize window///////////////////////////////////
void init(void)                       
{
glClearColor(0.0, 0.0, 0.0, 0.0); // clear background color to black
glEnable(GL_DEPTH_TEST);	   // performs hidden surface removal
glDepthFunc(GL_LEQUAL);	   // Passes if the incoming depth value is equal to the stored depth value.
glShadeModel(GL_SMOOTH);	     // smooth shading
}
/////////////////////////////Start drawing objects on the window////////////////////////////////////

void display(void)
{
  read_pot();                          //Function call to read value from serial port which decides the angle between scissor arms

glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the buffers

glPushMatrix();                                     // to move the co-ordinate for drawing the cube
    glTranslatef(0.0,-5.0,-20.0);                       // this will be the leftmost end of the base being drawn which is 20 units away from the camera
                                                    // This point will be act as a reference co-ordinate during the drawing operation 
    
     glPushMatrix();                                // Draw base
  glColor3f(1.0,0.5,0.8);                       
          glScalef(30.0,1.5,30.0);
          glutSolidCube(1.0);                           
         glPopMatrix();                                 // Finish base
     
        
glPushMatrix();                                // Draw ball
          glColor3f(1.0,0.0,0.0);                       
          glTranslatef(ball_xpos,ball_ypos,ball_zpos);  // position the ball 
          glScalef(0.5,10.0,0.5);                       
          glutSolidSphere(1.0,30,30);                  
          glColor3f(1.0,1.0,1.0);                     
          glutWireSphere(1.0,10.0,10.0);                
         glPopMatrix();                                 // Finish ball

         
glPushMatrix();                                // Start scissor arms
          glColor3f(0.0,1.0,0.0);
          glTranslatef(xpos,ypos,zpos);                 // translate the center of scissor to the center of the table
          glRotatef(-15.0,1.0,0.0,0.0);                 // Optional - can be midified to change the tilt of scissor along x-axis

          glPushMatrix();                               // first scissor arm start
             glRotatef(scissor_angle/2,0.0,1.0,0.0);    // position the first arm 
               
   glPushMatrix();                          //Draw
     glScalef(0.3,0.3,10.0);                //First 
             glutSolidSphere(1.0,30,30);            //Scissor 
             glColor3f(1.0,1.0,1.0);                //
glutWireSphere(1.0,10.0,10.0);         //arm
           glPopMatrix();

               glPushMatrix();                          //Draw the end ring
            glTranslatef(0.0,0.0,10.0);             
            glScalef(0.4,0.1,0.4);
            glRotatef(90,1.0,0.0,0.0);
            glColor3f(0.0,1.0,0.0);
            glutSolidTorus(0.5,0.6,5,20);
           glPopMatrix();

          glPopMatrix();                                // First arm finish

          glPushMatrix();                               // second arm
            glRotatef(-scissor_angle/2,0.0,1.0,0.0);    // position second arm conjugatively opposite to first 
             
  glPushMatrix();                           // Draw second arm
                glScalef(0.3,0.3,10.0);                 
                glutSolidSphere(1.0,30,30);
                glColor3f(1.0,1.0,1.0);
                glutWireSphere(1.0,10.0,10.0);
              glPopMatrix();                            

              glPushMatrix();                            // Draw end ring of second arm 
                glTranslatef(0.0,0.0,10.0);
                glScalef(0.4,0.1,0.4);
                glRotatef(90,1.0,0.0,0.0);
                glColor3f(0.0,1.0,0.0);
                glutSolidTorus(0.5,0.6,5,20);
              glPopMatrix();                             // Second arm finish

          glPopMatrix();                         

  
      glPopMatrix();

  glPopMatrix();

glutPostRedisplay();


///////////End of Drawing////////////////


glutSwapBuffers();         // Important when double buffering is used

}                                       //End of Display function


void reshape(int w, int h)
{
glViewport(0, 0, (GLsizei) w, (GLsizei) h); // keeps the ratio of the window
glMatrixMode(GL_PROJECTION);                // projection matrix 
glLoadIdentity(); // clears the matrixes
gluPerspective(45.0f,(GLfloat)w/(GLfloat)h,1.0f,100000.0f); //  specifies a viewing frustum into the world coordinate system
glMatrixMode(GL_MODELVIEW);
glLoadIdentity();// good practise to always switch back to modelview
gluLookAt(-3.0,25.0,0.0,0.0,0.0,-15.0,0.0,0.0,-1.0);  // position the camera from the origin at a certain distance for a proper view
//gluLookAt(xpos,ypos-3.0,zpos-20.0,xpos+0.1,(ypos-3.0),(zpos-15.0),0.0,1.0,0.0);  // Can be used to fix the camera on the moving scissor
  

}



////////////////Joystick control//////////////////////////////////////

void joystick(unsigned int buttonMask, int x, int y, int z)   
{
    
held = false;
      xpos = x/100.0;    // x movement control of scissor 
           zpos = y/100.0;    // y-movement control of scissor
   //scissor_angle = 20-(z/50); // OPTIONAL - can be used to control the scissor angle for testing purpose
                                // 'z' corresponds to the throttle lever of the joystick 

   
   scissor_close(2);   // writes value '2' to the serial port when the scissor doesn't hold the cube 
   
    
   if((zpos-3.0) <= (ball_zpos) && ypos > ball_ypos && scissor_angle<30.0)  /* This function moves the ball along with the scissor whenever 
    the scissor end touched the ball and scissor angle is
less than 30 */
   {
   
   if(xpos>(ball_xpos-2.0) && xpos<(ball_xpos+2.0))              // checks if the ball is held or not
     {
   held = true;	
   scissor_close(1);                                     /* writes value '2' to the serial port when the scissor holds the cube, 
                                                    this later gives a force feedback to the scissor 
                                                    [Note : This force feedback is not the same as the force feedback given to joystick] */
   ball_zpos = zpos-3.0;                                 // moves the ball along with the scissor   
   hapticfeedback();                                                                          
          
         }

     
      glutPostRedisplay();
   }     
   


   

   
       

  
}

int main(int argc, char** argv) //Main function
{

/* Standard glut functions to set up everything such as display mode, window parameters */
glutInit(&argc, argv);
glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
glutInitWindowSize(800, 600);
glutInitWindowPosition(200, 200);
glutCreateWindow(argv[0]);

init(); // initialize OpenGL

serial_com();

/* Calls these functions at 60 frames per second*/

glutReshapeFunc(reshape);
glutDisplayFunc(display);
glutJoystickFunc(joystick, 10);  // calls every 10ms
glutMainLoop(); // enter main loop and process events
CloseHandle(hCom); ///Close serial port

return 0;
}
