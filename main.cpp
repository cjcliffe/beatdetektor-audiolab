#include <stdio.h>
#include <stdlib.h>
#include <GLUT/glut.h>
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <iostream>
#include <math.h>
#include <vector>
#include <map>


#define FULLSCREEN 0

#if FULLSCREEN
	
	#define SCREEN_W 1440
	#define SCREEN_H 900

#else

	int SCREEN_W = 1024;
	int SCREEN_H = 768;

#endif


#include "Timer.h"
#include "FFT.h"
#include "bit_font.h"
#include "BeatDetektor.h"
#include "Font.h"


// keep detection range so that it does not contain two factors
// i.e. 75 x 2 = 150 so a range of 75-150 will confuse 75 /w 150 easily
// 100.0 - 199.0 works well for most dance / electronic music so far

std::vector<BeatDetektor *> dets;
BeatDetektor *det_vu;
BeatDetektorVU *vu;
BeatDetektorContest *contest;
Font *drawText;


#define MAP_BAR_DIVS 16
#define MAP_WIDTH 16

int trigger_map[4*MAP_BAR_DIVS*MAP_WIDTH];
int trigger_accum[MAP_WIDTH];
int trigger_ac_map[MAP_WIDTH] = { 1, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 6 };
int map_position = 0;
int map_last_position = 0;
float map_timer = 0;
float map_speed = 0;




#define SRATE 44100
#define BUF 1024

using namespace std;

ALCdevice *audio_device;
ALbyte audio_buffer[SRATE];
ALint samples;

vector<float> sample_data;
vector<float> fft_data;
vector<float> fft_collapse;

Timer visTimer;
bool bpm_latch = false;



// view scale functions
float _x(float n)
{
	return (n/(float)SCREEN_W);
}

float _y(float n)
{
	return (n/(float)SCREEN_H);
}




/* resize handler (GLUT) */
static void resize(int width, int height)
{
	const float ar = (float) width / (float) height;
	
#if !FULLSCREEN
	SCREEN_W = width;
	SCREEN_H = height;	
#else
	
#endif
	
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-ar, ar, -1.0, 1.0, 2.0, 100.0);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


// Capture audio sample (OpenAL) and compute FFT
static void capture(void)
{
	int x;
	
	int lval=32768;
	float bval = 0.0;

	alcGetIntegerv(audio_device, ALC_CAPTURE_SAMPLES, (ALCsizei)sizeof(ALint), &samples);
	alcCaptureSamples(audio_device, (ALCvoid *)audio_buffer, samples);
	
	for (x=0; x<BUF; x++)
	{
		bval = (((float)((ALint *)audio_buffer)[x])/32767.0) / (float)lval;
		sample_data[x]=bval;
	}
	
	fft_data = sample_data;
	
	DanielsonLanczos <(BUF/2), float> dlfft;
	
	unsigned long n, m, j, i;
	
	// reverse-binary reindexing
    n = (BUF/2)<<1;
    j=1;
    for (i=1; i<n; i+=2) {
        if (j>i) {
            swap(fft_data[j-1], fft_data[i-1]);
            swap(fft_data[j], fft_data[i]);
        }
        m = (BUF/2);
        while (m>=2 && j>m) {
            j -= m;
            m >>= 1;
        }
        j += m;
    };

	
	dlfft.apply(&fft_data[0]);
		
//	fvals = dft(cvals);
	
}	


// simple box functions
static void drawBox(float x1,float y1,float x2,float y2)
{
	glBegin(GL_LINE_STRIP);
		glVertex3f(x1, y1, -3.0f);
		glVertex3f(x1, y2, -3.0f);
		glVertex3f(x2, y2, -3.0f);
		glVertex3f(x2, y1, -3.0f);
		glVertex3f(x1, y1, -3.0f);
	glEnd();
}

static void colorBox(float x1,float y1,float x2,float y2, float r1, float g1, float b1, float r2, float g2, float b2)
{
	glBegin(GL_QUADS);
	glColor3f(r1,g1,b1);
		glVertex3f(x1, y1, -3.0f);
	glColor3f(r2,g2,b2);
		glVertex3f(x1, y2, -3.0f);
		glVertex3f(x2, y2, -3.0f);
	glColor3f(r1,g1,b1);
		glVertex3f(x2, y1, -3.0f);
//		glVertex3f(x1, y1, -3.0f);
	glEnd();
}




static void textStart()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glViewport(0,0,SCREEN_W,SCREEN_H);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,SCREEN_W, 0, SCREEN_H, -10.0, 10.0);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDepthMask(0);
	glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);		
}

static void textEnd()
{
	glPopAttrib();
	resize(SCREEN_W,SCREEN_H);
}
	

static void display(void)
{
	int x;
	float fx,fy;
	float x1,y1,x2,y2;
	int range = 0;
	float view_scale = ((float)SCREEN_W/((float)BUF/4.0));
	unsigned int range_step = (sample_data.size()/BD_DETECTION_RANGES);
	
	visTimer.update();

	capture();	
	
//	int fft_size = fft_data.size();
	fft_collapse.clear();	
	for (x = 0; x < 256; x++)
	{
		fft_collapse.push_back(fft_data[x]);
	}
	for (x = BUF-256; x < BUF; x++)
	{
		fft_collapse.push_back(fft_data[x]);
	}
	
	float timer_seconds = visTimer.getSeconds();

	std::vector<BeatDetektor *>::iterator det_i;
	
	for (det_i = dets.begin(); det_i != dets.end(); det_i++)
	{
		(*det_i)->process(timer_seconds,fft_collapse);
		contest->process(timer_seconds,(*det_i));
	}
	
	det_vu->process(timer_seconds,fft_collapse);
	if (contest->win_bpm_int) vu->process(det_vu,((float)contest->win_bpm_int/10.0));

	contest->run();
		
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glClearColor(0,0,0,1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glColor3f(1.0,1.0,1.0);
		
	string s;

	if (contest->winning_bpm && contest->win_bpm_int && ((contest->bpm_contest[contest->win_bpm_int] > (BD_FINISH_LINE/2) && !bpm_latch) || (contest->bpm_contest[contest->win_bpm_int] > (BD_FINISH_LINE/3) && bpm_latch)))
	{
		char bpmstr[255];
		
		sprintf(bpmstr,"%0.1f",(float)contest->win_bpm_int/10.0);
		
		s.append("BPM:");
		s.append(bpmstr);
		bpm_latch = true;
	}
	else
	{
		s.append("Searching..");
		bpm_latch = false;
	}
	
	textStart();
	
	glColor3f(1.0,1.0,1.0);
	drawText->stringShadow(20,SCREEN_H-20,s.c_str());

	textEnd();
	

	
	// beat counter
	colorBox(-0.125,1.3 ,-0.025,1.4,	0,0,0.5,0,0,(((contest->beat_counter+3)%4)?0.2:1.0));
	colorBox( 0.025,1.3 , 0.125,1.4,	0,0,0.5,0,0,(((contest->beat_counter+2)%4)?0.2:1.0));
	colorBox( 0.025,1.15, 0.125,1.25,	0,0,0.5,0,0,(((contest->beat_counter+1)%4)?0.2:1.0));
	colorBox(-0.125,1.15,-0.025,1.25,	0,0,0.5,0,0,(((contest->beat_counter)%4)?0.2:1.0));
	
	
	glPushMatrix();
	glColor3f(1.0,1.0,1.0);
//	glRotatef(visTimer.getSeconds()*30.0,0,0,1);


/*	float sample_max = 0;
	for (x=0; x<sample_data.size(); x++)
	{
		float v = fabs(sample_data.at(x));
		if (v>sample_max) sample_max =v;
	}
	
	// waveform
	glBegin(GL_LINE_STRIP);
	for (x=0; x<sample_data.size(); x++)
	{
		float v = sample_data.at(x);
		float a = 0.1+fabs(v*2.0);
		glColor3f(a*1.0,a*1.0,a*1.0);
		fx = _x(x*((float)SCREEN_W/((float)BUF/4.0))) - 2.0f;
		fy = (v/sample_max)*0.25 - 0.1f;
		
		
		//float cp = (float)x/(float)sample_data.size();
	
		//glVertex3f(fx, fy, -3.0f);
		glVertex3f(fx, fy, -3.0f);
	}
	glEnd();
*/	
	
	for (x=0; x<sample_data.size(); x+=range_step)
	{
		(det_vu->detection[range])?glLineWidth(2.0):glLineWidth(0.2);
		
		// moving average display
		x1 = _x((float)x*view_scale) - 2.0f;
		y1 = 0.5;
		x2 = _x((float)(x+range_step)*view_scale) - 2.0f;
		y2 = 0.5+0.3*(det_vu->ma_freq_range[range]/det_vu->maa_freq_range[range]);
		
//		y2 = 0.5+0.3*(vu->vu_levels[range]);

		colorBox(x1,y1,x2,y2,0,0,0,0,det_vu->detection[range]?1:0,det_vu->detection[range]?0:1);

		// draw additional box underneath range
		if (det_vu->detection[range])
		{
			x1 = _x((float)x*view_scale) - 2.0f;
			y1 = 0.5;
			x2 = _x((float)(x+range_step)*view_scale) - 2.0f;
			y2 = 0.45;			
			colorBox(x1,y1,x2,y2,0,0,0,0,1,0);
		}
		
				
		range++;
	}
		
	if (det_vu->detection[0] && (det_vu->ma_freq_range[0]/det_vu->maa_freq_range[0])>1.1) 
	{
		x1 = _x(0) - 1.0f;
		y1 = 0.4;
		x2 = _x(SCREEN_W*2.0) - 1.0f;
		y2 = 0.35;			
		colorBox(x1,y1,x2,y2,1,0,0,0.5,0,0);
	}
	
	
	glLineWidth(1.0);
	
	float syncPercent = contest->bpm_contest[contest->win_bpm_int]/((float)BD_FINISH_LINE-5.0);
	if (syncPercent > 1.0) syncPercent = 1.0;
	if (syncPercent < 0.0) syncPercent = 0;

	
	x1 = -2.0;
	y1 = 0.25;
	x2 = 2.0;
	y2 = 0.3;			
	colorBox(x1,y1,x1+syncPercent*(x2-x1),y2,0,0,1.0,0.0,0,0.7);
	
	
	textStart();
	
	if (contest->win_bpm_int)
	{
		map_timer += visTimer.lastUpdateSeconds();
	}
	
	float timer_max = ((60.0/((float)contest->win_bpm_int/10.0))/(float)MAP_BAR_DIVS);
	
	if (map_timer > timer_max)
	{
		map_timer -= timer_max;
		map_position += 1;
	}
	
	
	int iptr = 0;
	int jptr = 0;
	do
	{
		int acm = iptr+trigger_ac_map[jptr];
		
		trigger_accum[jptr] = 0;
		
		for (int i = iptr; i < acm; i++)
		{
//			if (det_vu->detection[i]) trigger_accum[jptr]++; else trigger_accum[jptr]--;
//			if (vu->vu_levels[i]>0.60) trigger_accum[jptr]+=1;
//			if (vu->vu_levels[i]>0.50) trigger_accum[jptr]+=1;
			if (det_vu->detection[i] && (det_vu->ma_freq_range[i]/det_vu->maa_freq_range[i]>1.1)) trigger_accum[jptr]++; 
			else trigger_accum[jptr]--;
		}
		
		int map_index = jptr+MAP_WIDTH*(map_position%(MAP_BAR_DIVS*4));
		
		if (trigger_accum[jptr]>0)
		{
			trigger_map[map_index] = 1;
		}
		else if (map_last_position != map_position)
		{
			trigger_map[map_index] = 0;	
		}
		
		iptr=acm;
		jptr++;
	}
	while (jptr<MAP_WIDTH);
	
	
	map_last_position = map_position;
	
	
	float map_w = (float)SCREEN_W / (float)(MAP_BAR_DIVS*4);
	float map_h = 128.0/(float)MAP_WIDTH;
	float map_s = 0;
	float map_top = (SCREEN_H/3)+30;
	float map_t = map_top;

	for (int i = 0; i < (4*MAP_BAR_DIVS*MAP_WIDTH); i+=MAP_WIDTH)
	{
		bool current_map = (i==((map_position%(MAP_BAR_DIVS*4))*MAP_WIDTH));
		
		for (int j = 0; j < MAP_WIDTH; j++)
		{
			x1 = map_s;
			y1 = map_t;
			x2 = map_s+map_w;
			y2 = map_t+map_h;			
			
			int map_index = i+j;
			
			if (trigger_map[map_index]) 
			{
				if (current_map)
				{
					colorBox(x2,y1,x1,y2,0.0,1.0,0.0,0.0,0.5,0.0);
				}
				else
				{
					colorBox(x2,y1,x1,y2,1.0,1.0,1.0,0.0,0,0.5);					
				}
			}
			
			map_t+=map_h;
		}

		if (current_map)
		{
			y1 += map_h;
			y2 += map_h;
			colorBox(x2,y1,x1,y2,0.0,1.0,0.0,0.0,0.5,0.0);
		}
		
		map_s+=map_w;
		map_t=map_top;
	}
	

	textEnd();
	
	
	// debug display of bpm detection for each range, bright = higher quality, y height=current detection 
	glBegin(GL_LINE_STRIP);
	for (x=0; x<BD_DETECTION_RANGES; x++)
	{
		glColor3f((det_vu->detection_quality[x])/(det_vu->quality_avg),0,0);

		fx = _x(x*((float)SCREEN_W/((float)BD_DETECTION_RANGES/4.0))) - 2.0f;
		fy = fabs(det_vu->maa_bpm_range[x]) + 0.5f;
		glVertex3f(fx, fy, -3.0f);
	}
	glEnd();
	
	
	std::map<int,float>::iterator contest_i;
	std::map<int,float>::iterator contest_center;
	std::map<int,float>::iterator contest_left;
	std::map<int,float>::iterator contest_right;
	
	float contest_max = 0;
	char contest_val[255];					
	

	
	if (contest->bpm_contest.size())
	{
		for (contest_i = contest->bpm_contest.begin(); contest_i != contest->bpm_contest.end(); contest_i++)
		{
			if ((*contest_i).second > contest_max)
			{
				contest_max = (*contest_i).second;
				contest_center = contest_i;
				contest_left = contest_i;
				contest_right = contest_i;
			}
		}

		if (contest_max)
		{
			if (contest_max < BD_FINISH_LINE) contest_max = BD_FINISH_LINE;
			
			textStart();
			
			float bar_w = 40;
			float bar_s = 2;

			x1 = (SCREEN_W/2)-bar_w/2;
			y1 = 0;
			x2 = (SCREEN_W/2)+bar_w/2;;
			y2 = ((*contest_center).second/contest_max)*(SCREEN_H/3);			

			while (contest_right != contest->bpm_contest.end() && x2 < SCREEN_W)
			{
				y2 = ((*contest_right).second/contest_max)*(SCREEN_H/3);

				if (contest_right == contest_center)
				{
					colorBox(x2,y1,x1,y2,1.0,1.0,1.0,0.0,0,0.5);
				}
				else
				{				
					colorBox(x2,y1,x1,y2,0,0,1.0,0.0,0,0.5);
				}
				
				sprintf(contest_val,"%0.1f",(float)((*contest_right).first/10.0));
				drawText->string(x1,y2+10,contest_val);
				contest_right++;
				x1 += bar_w+bar_s;
				x2 += bar_w+bar_s;
			}
			
			
			if (contest_left != contest->bpm_contest.begin())
			{
				x1 = (SCREEN_W/2)-bar_w/2;
				y1 = 0;
				x2 = (SCREEN_W/2)+bar_w/2;;
				y2 = ((*contest_center).second/contest_max)*(SCREEN_H/3);			

				contest_left--;
				x1 -= bar_w+bar_s;
				x2 -= bar_w+bar_s;

				while (contest_left != contest->bpm_contest.begin() && x1 > 0)
				{
					y2 = ((*contest_left).second/contest_max)*(SCREEN_H/3);
					colorBox(x2,y1,x1,y2,0,0,1.0,0.0,0,0.5);
					sprintf(contest_val,"%0.1f",(float)((*contest_left).first/10.0));
					drawText->string(x1,y2+10,contest_val);
					contest_left--;
					x1 -= bar_w+bar_s;
					x2 -= bar_w+bar_s;
				}
			}
		
			textEnd();
		}
		
	}
	
	
	
	
	glPopMatrix();	
	glutSwapBuffers();
	
}

static void key(unsigned char key, int x, int y)
{
	switch (key)
	{
		case 'q':
			exit(0);
			break;
		case 'r':
			std::vector<BeatDetektor *>::iterator det_i;
			
			for (det_i = dets.begin(); det_i != dets.end(); det_i++)
			{
				(*det_i)->reset();
			}
			contest->reset();
			break;
	}
	glutPostRedisplay();
}

static void idle(void)
{
	glutPostRedisplay();
}

/* Program entry point */
int main(int argc, char *argv[])
{
 	alGetError();
	const ALchar *pDeviceList = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
	if (pDeviceList) {
		printf("Available Capture Devices are:\n");
		while (*pDeviceList) {
			printf("**%s**\n", pDeviceList);
			pDeviceList += strlen(pDeviceList) + 1;
		}
	}
	
	audio_device = alcCaptureOpenDevice(NULL, SRATE*2, AL_FORMAT_STEREO16, BUF);
	if (alGetError() != AL_NO_ERROR)
	{
		return 0;
	}
	alcCaptureStart(audio_device);
	
	
	glutInit(&argc, argv);
#if FULLSCREEN
	glutGameModeString( "1440x900:32@60" );
	glutEnterGameMode();
#else
	glutInitWindowSize(SCREEN_W,SCREEN_H);
	glutCreateWindow("CubicFX [BeatDetektor-CORE-OSX] - (r)eset or (q)uit, { 85-169bpm }");
#endif
	
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE );

	sample_data.resize(BUF);
	fft_data.resize(BUF);

	for (float i = 8.0; i <= 12.0; i += 1.0)
	{
		BeatDetektor *det = new BeatDetektor(65,110);
		det->detection_rate = i;
		det->quality_reward = 15.0;
		dets.push_back(det);

		det = new BeatDetektor(80,150);
		det->detection_rate = i;
		det->quality_reward = 20.0;
		dets.push_back(det);

		det = new BeatDetektor(110,170);
		det->detection_rate = i;
		det->quality_reward = 10.0;
		dets.push_back(det);
	}
	
	det_vu = new BeatDetektor(100,120);
	det_vu->detection_factor = 0.95;
	det_vu->detection_rate = 10.0;
	vu = new BeatDetektorVU();
	
	contest = new BeatDetektorContest();
	contest->no_contest_decay = true;
	contest->finish_line = 1000;
	
	drawText = new Font(8,8,(unsigned char *)bit_font);
	
	glutReshapeFunc(resize);
	glutDisplayFunc(display);
	glutKeyboardFunc(key);
	glutIdleFunc(idle);
	
	glClearColor(1,1,1,1);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	
	visTimer.start();
	
	glutMainLoop();
	
	alcCaptureStop(audio_device);
	alcCaptureCloseDevice(audio_device);
	
	return EXIT_SUCCESS;
}
