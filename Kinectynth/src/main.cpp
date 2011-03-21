/****************************************************************************
*                                                                           *
*   Nite 1.3 - Point Viewer Sample                                          *
*                                                                           *
*   Author:     Oz Magal                                                    *
*                                                                  s        *
****************************************************************************/

/****************************************************************************
*                                                                           *
*   Nite 1.3	                                                            *
*   Copyright (C) 2006 PrimeSense Ltd. All Rights Reserved.                 *
*                                                                           *
*   This file has been provided pursuant to a License Agreement containing  *
*   restrictions on its use. This data contains valuable trade secrets      *
*   and proprietary information of PrimeSense Ltd. and is protected by law. *
*                                                                           *
****************************************************************************/

// Headers for OpenNI
#include <XnOpenNI.h>
#include <XnCppWrapper.h>
#include <XnHash.h>
#include <XnLog.h>

// Header for NITE
#include "XnVNite.h"
// local header
#include "PointDrawer.h"
#include <iostream>

#include "camera.h"

#include<stdlib.h>
#include<windows.h>
#include <shellapi.h>
#define ShowUpvector	

CCamera Camera;

#define CHECK_RC(rc, what)											\
	if (rc != XN_STATUS_OK)											\
{																\
	printf("%s failed: %s\n", what, xnGetStatusString(rc));		\
	return rc;													\
}

#define USE_GLUT
#ifdef USE_GLUT
#include <GL/glut.h>
#else
#endif

// OpenNI objects
xn::Context g_Context;
xn::DepthGenerator g_DepthGenerator;
xn::HandsGenerator g_HandsGenerator;

// NITE objects
XnVSessionManager* g_pSessionManager;
XnVFlowRouter* g_pFlowRouter;

// the drawer
XnVPointDrawer* g_pDrawer;

#define GL_WIN_SIZE_X 720
#define GL_WIN_SIZE_Y 480

// Draw the depth map?
XnBool g_bDrawDepthMap = false;    // Setting it to false from first ... :Rahul 
XnBool g_bPrintFrameID = false;
// Use smoothing?
XnFloat g_fSmoothing = 0.0f;
XnBool g_bPause = false;
XnBool g_bQuit = false;

SessionState g_SessionState = NOT_IN_SESSION;

/** Rahul Variables */

int number_of_hands=0;
float x,y,z;

float last_x=0,last_y=0,last_z=0;

float both_hands_x[4]={0,0,0,0},both_hands_y[4]={0,0,0,0},both_hands_z[4]={0,0,0,0};

#define MAXPOINTS 400000


FILE *bundle_out_fp,*csv_fp;	
char *dummy;
char input_line[800];
int count_points,num_cameras;
double x3d[MAXPOINTS], y3d[MAXPOINTS], z3d[MAXPOINTS];
double r[MAXPOINTS],g[MAXPOINTS],b[MAXPOINTS];
double camerax[MAXPOINTS],cameray[MAXPOINTS],cameraz[MAXPOINTS],camerarotx[MAXPOINTS],cameraroty[MAXPOINTS],camerarotz[MAXPOINTS];
double useless;
double rotx=0,roty=0,rotz=0;
static int cameraid=0;
int buttonstate=923;

GLdouble projection_matrix[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
GLdouble modelview_matrix[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
int viewport_matrix[4] = {0,0,640,480};

float stepx=0.0,stepy=0.0,stepz=0.0;


GLdouble xx,yy,zz;
GLuint texture;
bool drawcameras=0,drawclouds=1;
int point_number=-1,stored_points=0,cloud_point_size=2;
float stored_points_position[4][3],frame_rate=33;


/***** End of my Definitions **/
using namespace std;

string bundler_file;

void DrawNet(GLfloat size, GLint LinesX, GLint LinesZ)
{
	glBegin(GL_LINES);
	for (int xc = 0; xc < LinesX; xc++)
	{
		glVertex3f(	-size / 2.0 + xc / (GLfloat)(LinesX-1)*size,
			0.0,
			size / 2.0);
		glVertex3f(	-size / 2.0 + xc / (GLfloat)(LinesX-1)*size,
			0.0,
			size / -2.0);
	}
	for (int zc = 0; zc < LinesX; zc++)
	{
		glVertex3f(	size / 2.0,
			0.0,
			-size / 2.0 + zc / (GLfloat)(LinesZ-1)*size);
		glVertex3f(	size / -2.0,
			0.0,
			-size / 2.0 + zc / (GLfloat)(LinesZ-1)*size);
	}
	glEnd();
}

void load_ply()
{

	bundle_out_fp=fopen(bundler_file.c_str(),"r");
	/* Skip over the Ply File Information till the end_header string is not reached , this is because different ply_files have different end_header locations */
	int i;		
	for(i=0;i<13;i++)
		dummy = fgets(input_line, 800, bundle_out_fp);
 
	cout<<"\n\n"<<dummy;
	count_points=0;

	while(!feof(bundle_out_fp))

	{
		dummy = fgets(input_line, 800, bundle_out_fp);

		sscanf(input_line, "%lf %lf %lf %lf %lf %lf %lf %lf %lf ", & x3d[count_points],&y3d[count_points],&z3d[count_points],&r[count_points], &g[count_points], &b[count_points]);
		// cout<<"\n\n "<<x3d[count_points]<<"   "<<y3d[count_points]<<"    "<<z3d[count_points];
		//cout<<input_line;
		count_points++;

	}

	cout<<count_points;

	cout<<"\n\n The number of points in the scene are"<<count_points<<endl;
}

void draw_camera()
{

	glEnable(GL_POINT_SMOOTH);
	glPointSize(cloud_point_size+8);

	glBegin(GL_POINTS);

	glColor3f(1,1,1);

	for (int i = 0; i < num_cameras; i++) 
	{
		if(i==cameraid)
			glColor3f(0,0,1);
		else glColor3f(1,1,1);
		//glColor3f(r[i]/255.0,g[i]/255.0,b[i]/255.0); // Randomizing the color of the points ..
		glVertex3f(camerax[i],cameray[i],cameraz[i]);
		//glVertex2f(x3d[i],y3d[i]);

	}

	glEnd();


	glDisable(GL_POINT_SMOOTH);

}

void load_cameras()
{

	csv_fp=fopen("indian_temple.csv","r");

	dummy = fgets(input_line, 800, csv_fp); // Useless line
	num_cameras=0;

	while(!feof(csv_fp))

	{
		dummy = fgets(input_line, 800, csv_fp);

		sscanf(input_line, "%lf,%lf,%lf,%lf,%lf,%lf,%lf ", &useless,&camerax[num_cameras],&cameray[num_cameras],&cameraz[num_cameras],&camerarotx[num_cameras], &cameraroty[num_cameras], &camerarotz[num_cameras]);

		//cout<<input_line;
		num_cameras++;

	}
	// cout<<"\n\n "<<camerax[10]<<"   "<<cameray[10]<<"    "<<cameraz[10];
	cout<<"\n\n The number of cameras in the scene are"<<num_cameras<<endl;

}	

void draw_ply(){

	glEnable(GL_POINT_SMOOTH);
	glPointSize(cloud_point_size);

	glBegin(GL_POINTS);
	glDisable(GL_DITHER);

	for (int i = 0; i < count_points; i++) 
	{
		glPushMatrix();
		glColor3f(r[i]/255.0,g[i]/255.0,b[i]/255.0); //  the color of the points ..

		glVertex3f(x3d[i],y3d[i],z3d[i]);
		//glVertex2f(x3d[i],y3d[i]);
		glPopMatrix();
	}
	//glDisable(GL_DEPTH_TEST);
	//glPointSize(10);
	glEnable(GL_DITHER);

	glEnd();
	glDisable(GL_POINT_SMOOTH);

}



void CleanupExit()
{
	g_Context.Shutdown();

	exit (1);
}

// Callback for when the focus is in progress
void XN_CALLBACK_TYPE FocusProgress(const XnChar* strFocus, const XnPoint3D& ptPosition, XnFloat fProgress, void* UserCxt)
{
	//	printf("Focus progress: %s @(%f,%f,%f): %f\n", strFocus, ptPosition.X, ptPosition.Y, ptPosition.Z, fProgress);
}
// callback for session start
void XN_CALLBACK_TYPE SessionStarting(const XnPoint3D& ptPosition, void* UserCxt)
{
	printf("Session start: (%f,%f,%f)\n", ptPosition.X, ptPosition.Y, ptPosition.Z);
	g_SessionState = IN_SESSION;
}
// Callback for session end
void XN_CALLBACK_TYPE SessionEnding(void* UserCxt)
{
	printf("Session end\n");
	g_SessionState = NOT_IN_SESSION;
}
void XN_CALLBACK_TYPE NoHands(void* UserCxt)
{
	printf("Quick refocus\n");
	g_SessionState = QUICK_REFOCUS;
}

// this function is called each frame
void glutDisplay (void)
{

	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	Camera.Render();

	//Draw the "world":
	if(drawclouds)
		draw_ply();
	if(drawcameras)
		draw_camera();


	// Setup the OpenGL viewpoint
	/*glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	XnMapOutputMode mode;
	g_DepthGenerator.GetMapOutputMode(mode);
	#ifdef USE_GLUT
	glOrtho(0, mode.nXRes, mode.nYRes, 0, -1.0, 1.0);
	#else
	glOrthof(0, mode.nXRes, mode.nYRes, 0, -1.0, 1.0);
	#endif*/

	//glDisable(GL_TEXTURE_2D);

	if (!g_bPause)
	{
		// Read next available data
		g_Context.WaitAndUpdateAll();
		// Update NITE tree
		g_pSessionManager->Update(&g_Context);
		//PrintSessionState(g_SessionState);
	}



	/*******************************************************************************************************//**
																											 *									Rahuls Modifications 
																											 *
																											 */
	//Are u tracking hands ..?
	if(g_SessionState==0)
	{
		//cout<<"I am Tracking Now"<<endl;

		//How many Hands r u tracking ?

		std::map<XnUInt32, std::list<XnPoint3D> >::const_iterator iter;
		std::list<XnPoint3D>::const_iterator iter2;

		// The size Parameter will give me the number of hands that are currently being Tracked ..
		cout<<"\n  "<<g_pDrawer->m_History.size(); 

		number_of_hands=g_pDrawer->m_History.size();
		int i=0;
		//g_pDrawer->m_History[0];

		//XnPoint3D test;std::list<XnPoint3D>

		// Go over all previous positions of current hand
		if(number_of_hands==1)
		{
			for (iter = g_pDrawer->m_History.begin(); iter != g_pDrawer->m_History.end(); ++iter)
			{
				iter2=iter->second.begin();
				//iter2=iter->second.front();

				cout<<iter->second.size()<<endl;

				float x2=0,y2=0,z2=0;

				x=iter2->X;y=iter2->Y;z=iter2->Z;

				int k=0;

				for (iter2 = iter->second.begin();iter2!= iter->second.end();++iter2,++k)
				{
					if(k<10)
					{
						XnPoint3D pt(*iter2);
						x2=pt.X;
						y2=pt.Y;
						z2=pt.Z;
						cout<<"One Hand"<<endl;
					}
				}

				cout<<"Difference is"<<x2-x<<endl;

				if(abs(x2-x)>100)
				{
					cout<<"Translation X";    
					Camera.RotateY((x2-x)/75.0);
				}

				else if (abs(y2-y)>100)
				{
					cout<<"Translation Y";
					Camera.RotateX((y2-y)/75.0);
				}

				else if(abs(z2-z)>100)
				{
					cout<<"Translation Z";
					Camera.MoveForward((z-z2)/150.0);
				}

				last_x=x;last_y=y;last_z=z;

			}

		}


		else if (number_of_hands==2)
		{

			///both_hands_x

			int iter_index=0;

			// Check the positions buffer of both hands ..
			for (iter = g_pDrawer->m_History.begin(); iter != g_pDrawer->m_History.end(); ++iter)
			{
				iter2=iter->second.begin();
				//iter2=iter->second.front();

				cout<<iter->second.size()<<endl;

				float x2=0,y2=0,z2=0;

				both_hands_x[iter_index]=iter2->X;both_hands_y[iter_index]=iter2->Y;both_hands_z[iter_index++]=iter2->Z;

				int k=0;

				for (iter2 = iter->second.begin();iter2!= iter->second.end();++iter2,++k)
				{
					if(k<10)
					{
						XnPoint3D pt(*iter2);
						both_hands_x[iter_index]=iter2->X;both_hands_y[iter_index]=iter2->Y;both_hands_z[iter_index]=iter2->Z;
						cout<<"Both hands \n"<<endl;
					}
				}
				iter_index++;

			}
		
		/// Examine the positions ..
		cout<<both_hands_x[0]<<"\t"<<both_hands_x[1]<<"\t"<<both_hands_x[2]<<"\t"<<both_hands_x[3]<<"\n";

			if(both_hands_x[1]-both_hands_x[0]>50 && both_hands_x[3]-both_hands_x[2]<-50 )
				{
					cout<<"Translation Z +";    
					Camera.MoveForward((both_hands_x[1]-both_hands_x[0])/100.0);
				}

			if(both_hands_x[1]-both_hands_x[0]<-50 && both_hands_x[3]-both_hands_x[2]>50 )
				{
					cout<<"Translation Z -";    
					Camera.MoveForward((both_hands_x[1]-both_hands_x[0])/100.0);
				}
			// if both hands move like in vertical direction then do a z-rotate ..
			if( abs(both_hands_x[1]-both_hands_x[0])<20 && abs(both_hands_x[3]-both_hands_x[2])<20 && abs((both_hands_y[1]+both_hands_y[0])/2  - (both_hands_y[2]+both_hands_y[3])/2) >50  )
			{
					Camera.RotateZ(   (  (both_hands_y[1]+both_hands_y[0])/2  - (both_hands_y[2]+both_hands_y[3])/2 )/75);

			}
		}



	}


	//for (iter = g_pDrawer->m_History.begin(); iter != g_pDrawer->m_History.end(); ++iter)
	//	{
	//	}




	/********************************** End of Modifications ***************************************************/







#ifdef USE_GLUT
	glutSwapBuffers();
#endif
}

#ifdef USE_GLUT
void glutIdle (void)
{
	if (g_bQuit) {
		CleanupExit();
	}

	// Display the frame
	glutPostRedisplay();
}

void glutKeyboard (unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		// Exit
		CleanupExit();
	case'p':
		// Toggle pause
		g_bPause = !g_bPause;
		break;
	case 'd':
		// Toggle drawing of the depth map
		g_bDrawDepthMap = !g_bDrawDepthMap;
		g_pDrawer->SetDepthMap(g_bDrawDepthMap);
		break;
	case 'f':
		g_bPrintFrameID = !g_bPrintFrameID;
		g_pDrawer->SetFrameID(g_bPrintFrameID);
		break;
	case 's':
		// Toggle smoothing
		if (g_fSmoothing == 0)
			g_fSmoothing = 0.1;
		else 
			g_fSmoothing = 0;
		g_HandsGenerator.SetSmoothing(g_fSmoothing);
		break;
	case 'e':
		// end current session
		g_pSessionManager->EndSession();
		break;
	}
}
void reshape(int x, int y)
{
	if (y == 0 || x == 0) return;  //Nothing is visible then, so return

	//Set a new projection matrix
	glMatrixMode(GL_PROJECTION);  
	glLoadIdentity();
	//Angle of view:40 degrees
	//Near clipping plane distance: 0.5
	//Far clipping plane distance: 20.0

	gluPerspective(40.0,(GLdouble)x/(GLdouble)y,0.5,2000.0);

	glMatrixMode(GL_MODELVIEW);
	glViewport(0,0,x,y);  //Use the whole window for rendering

}

void glInit (int * pargc, char ** argv)
{
	glutInit(pargc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(GL_WIN_SIZE_X, GL_WIN_SIZE_Y);

	glutCreateWindow ("Kinectynth");
	//glutFullScreen();

	//ShellExecute(NULL, L"open", L"http://mail.google.com",    NULL, NULL, SW_SHOWNORMAL);
	Camera.Move( F3dVector(camerax[cameraid],cameray[cameraid], cameraz[cameraid]));
	Camera.RotateY(90);
	Camera.RotateZ(90);
	Camera.Render();

	glutSetCursor(GLUT_CURSOR_NONE);

	glutKeyboardFunc(glutKeyboard);
	glutDisplayFunc(glutDisplay);
	glutReshapeFunc(reshape);
	glutIdleFunc(glutIdle);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}
#endif

// xml to initialize OpenNI
#define SAMPLE_XML_PATH "../Data/Sample-Tracking.xml"

int main(int argc, char ** argv)
{
	if(argc<2)
	{
		cout<<"Too Few Arguements!";
		exit(0);
	}
	bundler_file+=argv[1];

	if(argc==3)
	load_cameras();
	else cout<<"No camera File Found";

	load_ply();

	XnStatus rc = XN_STATUS_OK;

	// Initialize OpenNI
	rc = g_Context.InitFromXmlFile(SAMPLE_XML_PATH);
	CHECK_RC(rc, "InitFromXmlFile");

	rc = g_Context.FindExistingNode(XN_NODE_TYPE_DEPTH, g_DepthGenerator);
	CHECK_RC(rc, "Find depth generator");
	rc = g_Context.FindExistingNode(XN_NODE_TYPE_HANDS, g_HandsGenerator);
	CHECK_RC(rc, "Find hands generator");

	// Create NITE objects
	g_pSessionManager = new XnVSessionManager;
	rc = g_pSessionManager->Initialize(&g_Context, "Click,Wave", "RaiseHand");
	CHECK_RC(rc, "SessionManager::Initialize");

	g_pSessionManager->RegisterSession(NULL, SessionStarting, SessionEnding, FocusProgress);

	g_pDrawer = new XnVPointDrawer(30, g_DepthGenerator); 
	g_pFlowRouter = new XnVFlowRouter;
	g_pFlowRouter->SetActive(g_pDrawer);

	g_pSessionManager->AddListener(g_pFlowRouter);

	g_pDrawer->RegisterNoPoints(NULL, NoHands);
	g_pDrawer->SetDepthMap(g_bDrawDepthMap);

	// Initialization done. Start generating
	rc = g_Context.StartGeneratingAll();
	CHECK_RC(rc, "StartGenerating");

	// Mainloop
#ifdef USE_GLUT

	glInit(&argc, argv);
	//glutFullScreen();
	glutMainLoop();

#else
#endif
}
