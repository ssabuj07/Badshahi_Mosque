#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<windows.h>
#include<GL/glut.h>

double cameraAngle;
double move_X, move_Y, move_Z;
int canDrawGrid, canDrawAxis;
double speed=30,ang_speed=.1;
double cameraRadius, cameraHeight, cameraAngleDelta;
int num_texture  = -1;


/***************************** VECTOR structure **********************************/

struct V;

V operator+(V a,V b);
V operator*(V a,V b);
V operator*(V b,double a);
V operator*(double a,V b);

struct V{
	double x,y,z;

	V(){}
	V(double _x,double _y){x=_x;y=_y;z=0;}
	V(double _x,double _y,double _z){x=_x;y=_y;z=_z;}


	double	mag(){	return sqrt(x*x+y*y+z*z);	}

	void 	norm(){	double d = mag();x/=d;y/=d;	z/=d;}
	V 		unit(){	V ret = *this;	ret.norm(); return ret;}

	double	dot(V b){		return x*b.x + y*b.y + z*b.z;}
	V		cross(V b){		return V( y*b.z - z*b.y , z*b.x - x*b.z , x*b.y - y*b.x );}


	double	projL(V on){	on.norm();	return this->dot(on);}
	V		projV(V on){	on.norm();	return on * projL(on);}

	V rot(V axis, double angle){
		return this->rot(axis, cos(angle), sin(angle));
	}

	V rot(V axis, double ca, double sa){
		V rotatee = *this;
		axis.norm();
		V normal = (axis * rotatee).unit();
		V mid = (normal * axis).unit();
		double r = rotatee.projL(mid);
		V ret=r*mid*ca + r*normal*sa + rotatee.projV(axis);
		return ret.unit();
	}
};

V operator+(V a,V b){		return V(a.x+b.x, a.y+b.y, a.z+b.z);	}
V operator-(V a){			return V (-a.x, -a.y, -a.z);			}
V operator-(V a,V b){		return V(a.x-b.x, a.y-b.y, a.z-b.z);	}
V operator*(V a,V b){		return a.cross(b);						}
V operator*(double a,V b){	return V(a*b.x, a*b.y, a*b.z);			}
V operator*(V b,double a){	return V(a*b.x, a*b.y, a*b.z);			}
V operator/(V b,double a){	return V(b.x/a, b.y/a, b.z/a);			}




V loc,dir,perp;


V  _L(0,-150,20);
V  _D(0,1,0);
V  _P(0,0,1);

/***************************** Texture Functions *******************************/



GLuint tmp,lrt,ft,backWall,sideWall,carpet,mBM1,grassimg,skyImg,floorimg,mBM,largeMinar,bwu,bw2,wallLight,minar,minarSide,cylimg,walimg,aimg,w6img,roof3img,mwallimg,topMost,w1;

int LoadBitmapImage(char *filename)
{
	int i, j=0;
	FILE *l_file;
	unsigned char *l_texture;

	BITMAPFILEHEADER fileheader;
	BITMAPINFOHEADER infoheader;
	RGBTRIPLE rgb;

	num_texture++;

	if( (l_file = fopen(filename, "rb"))==NULL) return (-1);

	fread(&fileheader, sizeof(fileheader), 1, l_file);

	fseek(l_file, sizeof(fileheader), SEEK_SET);
	fread(&infoheader, sizeof(infoheader), 1, l_file);

	l_texture = (byte *) malloc(infoheader.biWidth * infoheader.biHeight * 4);
	memset(l_texture, 0, infoheader.biWidth * infoheader.biHeight * 4);
	for (i=0; i < infoheader.biWidth*infoheader.biHeight; i++)
	{
		fread(&rgb, sizeof(rgb), 1, l_file);

		l_texture[j+0] = rgb.rgbtRed;
		l_texture[j+1] = rgb.rgbtGreen;
		l_texture[j+2] = rgb.rgbtBlue;
		l_texture[j+3] = 255;
		j += 4;
	}
	fclose(l_file);

	glBindTexture(GL_TEXTURE_2D, num_texture);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, infoheader.biWidth, infoheader.biHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, l_texture);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, infoheader.biWidth, infoheader.biHeight, GL_RGBA, GL_UNSIGNED_BYTE, l_texture);

	free(l_texture);

	return (num_texture);

}

void drawGambuj();
void piller();
void drawMiddlebodyMinar();
void drawbuildingMinar();
void drawbuildingLeft();
void drawbuildingRight();
void drawminar();
void drawtop();


void loadImage()
{

	minar = LoadBitmapImage("images/w2.bmp");
	minarSide = LoadBitmapImage("images/minarside.bmp");
	floorimg = LoadBitmapImage("images/wallroof.bmp");
	skyImg = LoadBitmapImage("images/sky.bmp");
	w6img = LoadBitmapImage("images/w6.bmp");
	roof3img = LoadBitmapImage("images/roof3.bmp");
	bwu = LoadBitmapImage("images/bwu1.bmp");
	largeMinar = LoadBitmapImage("images/brickWall2.bmp");
	bw2 = LoadBitmapImage("images/bw1.bmp");
	carpet = LoadBitmapImage("images/carpet.bmp");
	sideWall = LoadBitmapImage("images/sideWall.bmp");
	backWall = LoadBitmapImage("images/backWall1.bmp");


	topMost = LoadBitmapImage("images/top.bmp"); ///// khaj khaj most top er jonne ...
	ft = LoadBitmapImage("images/aaa1.bmp");  /// middle samner top pic
	lrt = LoadBitmapImage("images/LR1.bmp");
	tmp = LoadBitmapImage("images/tmp.bmp");



	///////////////// uporer gula used image ////////////

	//wallLight = LoadBitmapImage("images/wall.bmp");
	//cylimg = LoadBitmapImage("image/cyl.bmp");


	//mwallimg = LoadBitmapImage("image/mwall.bmp");


	//w1 = LoadBitmapImage("images/w2.bmp");





	//grassimg = LoadBitmapImage("images/minarside.bmp");


	//mBM = LoadBitmapImage("images/fbm.bmp");


	////////////// testing part ////////////////////


	mBM = LoadBitmapImage("images/111.bmp");
	mBM1 = LoadBitmapImage("images/sonet.bmp");
	///mBM1 = LoadBitmapImage("images/fbm.bmp");

	//////// end test part /////////////////////


	if(w6img != -1)
		printf("Load successful!!\n");
	else printf("Image loading Failed!!\n");


	if(roof3img != -1)
		printf("Load successful!!\n");
	else printf("Image loading Failed!!\n");


	if(floorimg != -1)
		printf("Load successful!!\n");
	else printf("Image loading Failed!!\n");
}


void solidCube1Part(double size) {
	glBegin(GL_QUADS);
	glTexCoord2f(0, 1);
	glVertex3f(-size, 0, size);
	glTexCoord2f(0, 0);
	glVertex3f(size, 0, size);
	glTexCoord2f(1, 0);
	glVertex3f(size, 0, -size);
	glTexCoord2f(1, 1);
	glVertex3f(-size, 0, -size);
	glEnd();
}
// up and down part
void solidCube2Part(double size) {
	solidCube1Part(size);
	glRotated(90, 1, 0, 0);
	glTranslated(0, -size, -size);
	solidCube1Part(size);
}

void customSolidCube(double size) {
	glPushMatrix(); {
		glTranslated(0, -size/2, 0);
		solidCube2Part(size/2);
		glRotated(90, 1, 0, 0);
		glTranslated(0, -size/2, -size/2);
		solidCube2Part(size/2);
	}glPopMatrix();

	glPushMatrix(); {
		glRotated(90, 0, 0, 1);
		glTranslated(0, size/2, 0);
		solidCube1Part(size/2);
	}glPopMatrix();

	glPushMatrix(); {
		glRotated(90, 0, 0, 1);
		glTranslated(0, -size/2, 0);
		solidCube1Part(size/2);
	}glPopMatrix();
}

void drawtop(){
	glPushMatrix();{
		glTranslatef(0,99.5,248);
		glScalef(.5,1,1);
		glPushMatrix();{

			glTranslatef(0,0,25);
			glColor3f(1,1,1);
			glRotatef(90,0,1,0);
			glScalef(50,2,400);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, topMost);
			customSolidCube(1);
			glDisable(GL_TEXTURE_2D);
		}glPopMatrix();
	}glPopMatrix();
}

void carpetInside(){
	glPushMatrix();{
		glColor3f(1,1,1);
		//glRotatef(90,0,1,0);
		glTranslatef(0,0,0);
		glScalef(790,190,2);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, carpet);
		//glBindTexture(GL_TEXTURE_2D,topMost);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();
}

void drawGambuj()
{

	//gombuj lower part
	glPushMatrix();
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,minarSide);
		glColor3f(1,1,1);
		//glColor3f(0.6, 0.6, 0.6);
		GLUquadricObj *object = gluNewQuadric();
		gluCylinder(object, 70, 70, 22, 20, 20);
		glDisable(GL_TEXTURE_2D);
	}
	glPopMatrix();


	// gombuj
	glPushMatrix();{
		glScalef(1.05,1.05,1);
		double equation[4];
		equation[0] = 0;
		equation[1] = 0;
		equation[2] = 1;
		equation[3] = -20;

		glPushMatrix();
		{
			glClipPlane(GL_CLIP_PLANE0, equation);

			glEnable(GL_CLIP_PLANE0);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D,minar);
			glColor3f(1,1,1);
			GLUquadric *qobj  = gluNewQuadric();
			gluQuadricTexture(qobj, GL_TRUE);
			gluSphere(qobj,70, 20, 20);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_CLIP_PLANE0);
		}
		glPopMatrix();
	}
	glPopMatrix();

	//minar er uporer cone part
	glPushMatrix();
	{
		glColor3f(0.85, 0.6, 0);
		glTranslatef(0, 0, 68);
		glutSolidCone(20, 20, 30, 10);
	}
	glPopMatrix();

	//cone er moddher gol lower part
	glPushMatrix();{
		glTranslatef(0,0,85);
		glColor3f(.7,.7,.7);
		glutSolidSphere(8.5,20,20);
	}glPopMatrix();

	//cone er moddher gol higher part
	glPushMatrix();{
		glTranslatef(0,0,100);
		glColor3f(.7,.7,.7);
		glutSolidSphere(3.5,20,20);
	}glPopMatrix();

	//cone er uporer lomba part
	glPushMatrix();
	{
		glColor3f(0.85, 0.6, 0);

		glTranslatef(0, 0, 80);

		glutSolidCone(5, 40, 20, 20);
	}
	glPopMatrix();

}

void drawFloor(){
	glPushMatrix();{
		glColor3f(1,1,1);
		//glRotatef(90,0,1,0);
		glTranslatef(0,-450,-3);
		glScalef(1600,1100,6);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, floorimg);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();
}

void drawGrass(){
	glPushMatrix();{

		glTranslatef(0,0,-2);
		glColor3f(1,1,1);
		//glRotatef(90,0,1,0);
		glScalef(5000,5000,1);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, grassimg);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();
}

void drawSky(){
	double equation[4];
	equation[0] = 0;
	equation[1] = 0;
	equation[2] = 1;
	equation[3] = 0;
	glPushMatrix();
	{
		glClipPlane(GL_CLIP_PLANE0, equation);
		glEnable(GL_CLIP_PLANE0);
		glEnable(GL_TEXTURE_2D);
		{
			glRotatef(180, 1, 0, 0);
			glBindTexture(GL_TEXTURE_2D, skyImg);

			glPushMatrix();
			{
				glColor3f(1, 1, 1);
				GLUquadricObj *obj = gluNewQuadric();
				gluQuadricTexture(obj, GL_TRUE);
				gluSphere(obj, 3000, 20, 20);

			}
			glPopMatrix();
		}
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_CLIP_PLANE0);
	}
	glPopMatrix();
}

void drawWall(){


	///// back left walll /////////


	glPushMatrix();{

		glTranslatef(-400-200,100,75);
		glColor3f(1,1,1);
		//glRotatef(90,0,1,0);
		glScalef(400,3,150);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, w6img);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();


	glPushMatrix();{
		glTranslatef(-400-200,100-34,110);
		glColor3f(1,1,1);
		//glRotatef(90,0,1,0);
		glScalef(400,64,3);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, roof3img);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	glPushMatrix();{
		glTranslatef(-400-200,30,75);
		glColor3f(1,1,1);
		//glRotatef(90,0,1,0);
		glScalef(400,3,150);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, w6img);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();


	////// back right wall ////////
	glPushMatrix();{
		glTranslatef(400+200,100,75);
		glColor3f(1,1,1);
		//glRotatef(90,0,1,0);
		glScalef(400,3,150);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, w6img);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();


	glPushMatrix();{
		glTranslatef(400+200,100-34,110);
		glColor3f(1,1,1);
		//glRotatef(90,0,1,0);
		glScalef(400,64,3);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, roof3img);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();



	glPushMatrix();{
		glTranslatef(400+200,30,75);
		glColor3f(1,1,1);
		//glRotatef(90,0,1,0);
		glScalef(400,3,150);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, w6img);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();




	//glPushMatrix();{
	//glTranslatef(54,15,80);

	////////////////// left wall ////////////////////
	glPushMatrix();{

		glTranslatef(-800,-450,75);
		glColor3f(1,1,1);
		//glRotatef(90,0,1,0);
		glScalef(3,1100,150);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, w6img);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();


	glPushMatrix();{
		glTranslatef(-730-34,-450,110);
		glColor3f(1,1,1);
		//glRotatef(90,0,1,0);
		glScalef(64,1100,3);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, roof3img);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	glPushMatrix();{
		glTranslatef(-730,-450,75);
		glColor3f(1,1,1);
		//glRotatef(90,0,1,0);
		glScalef(3,1100,150);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, w6img);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();
	//}glPopMatrix();

	//glPushMatrix();{
	//glTranslatef(54,15,80);

	////////////////// right wall ////////////////////
	glPushMatrix();{

		glTranslatef(800,-450,75);
		glColor3f(1,1,1);
		//glRotatef(90,0,1,0);
		glScalef(3,1100,150);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, w6img);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();


	glPushMatrix();{

		glTranslatef(730+34,-450,110);
		glColor3f(1,1,1);
		//glRotatef(90,0,1,0);
		glScalef(64,1100,3);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, roof3img);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();


	glPushMatrix();{

		glTranslatef(730,-450,75);
		glColor3f(1,1,1);
		//glRotatef(90,0,1,0);
		glScalef(3,1100,150);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, w6img);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();
	//}glPopMatrix();

	////////////////// front wall ////////////////////

	//glPushMatrix();{
	//glTranslatef(54,15,80);
	glPushMatrix();{

		glTranslatef(0,-1000,75);
		glColor3f(1,1,1);
		//glRotatef(90,0,1,0);
		glScalef(1600,3,150);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, w6img);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();


	glPushMatrix();{
		glTranslatef(0,-1000+32,110);
		glColor3f(1,1,1);
		//glRotatef(90,0,1,0);
		glScalef(1600,64,3);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, roof3img);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();


	glPushMatrix();{

		glTranslatef(0,-930,75);
		glColor3f(1,1,1);
		//glRotatef(90,0,1,0);
		glScalef(1600,3,150);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, w6img);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();
	//}glPopMatrix();


	/////// place 4 wall minar Large /////////////

	////////////  left back wall minar //////////

	glPushMatrix();{
		glTranslatef(-770,50,0);
		drawminar();
	}glPopMatrix();

	////////////  right back wall minar //////////

	glPushMatrix();{
		glTranslatef(770,50,0);
		drawminar();
	}glPopMatrix();

	////////////  left front wall minar //////////

	glPushMatrix();{
		glTranslatef(-770,-970,0);
		drawminar();
	}glPopMatrix();

	////////////  right back wall minar //////////

	glPushMatrix();{
		glTranslatef(770,-970,0);
		drawminar();
	}glPopMatrix();

	///// eng of placing minars ////////////


}

void drawFront(){

	/*
	glPushMatrix();{
	glTranslatef(0,-6,0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, w1);
	glColor3f(1,1,1);
	glBegin(GL_QUADS);
	glTexCoord2f(0,1);
	glVertex3f(-10,0,0);
	glTexCoord2f(0,0);
	glVertex3f(40,0,0);
	glTexCoord2f(1,0);
	glVertex3f(40,0,30);
	glTexCoord2f(1,1);
	glVertex3f(-10,0,30);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	}glPopMatrix();


	glPushMatrix();{
	glTranslatef(0,-4,0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, w6img);
	glColor3f(1,1,1);
	glBegin(GL_QUADS);
	glTexCoord2f(0,1);
	glVertex3f(-10,0,0);
	glTexCoord2f(0,0);
	glVertex3f(0,0,0);
	glTexCoord2f(1,0);
	glVertex3f(0,0,50);
	glTexCoord2f(1,1);
	glVertex3f(-10,0,50);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	}glPopMatrix();
	*/
	/*
	glPushMatrix();{

	glTranslatef(-400-200,100,75);
	glColor3f(1,1,1);
	//glRotatef(90,0,1,0);
	glScalef(400,3,150);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, mBM);
	customSolidCube(1);
	glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	*/
	int x=-80;

	//////// lomba part ta korchechi ekhane ////////////
	glPushMatrix();{
		glScalef(.3,1,1);
		glPushMatrix();{
			glTranslatef(0,0,50);
			glColor3f(1,1,1);
			glRotatef(90,0,1,0);
			glScalef(100,3,50);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, mBM);
			customSolidCube(1);
			glDisable(GL_TEXTURE_2D);
			/*
			glTranslatef(-65,0,10);
			glColor3f(1,1,1);
			glScalef(10,5,30);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, mBM);
			glutSolidCube(1);
			glDisable(GL_TEXTURE_2D);
			*/
		}glPopMatrix();
	}glPopMatrix();
	////////////// end of the part /////////////

	glPushMatrix();{
		glScalef(.5,1,1);
		glPushMatrix();{
			glTranslatef(58,0,75);
			glColor3f(1,1,1);
			glRotatef(90,0,1,0);
			glScalef(50,3,100);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, mBM1);
			customSolidCube(1);
			glDisable(GL_TEXTURE_2D);
			/*
			glTranslatef(-65,0,10);
			glColor3f(1,1,1);
			glScalef(10,5,30);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, mBM);
			glutSolidCube(1);
			glDisable(GL_TEXTURE_2D);
			*/
		}glPopMatrix();
	}glPopMatrix();

	//////// 2nd part ta ///////
	glPushMatrix();{

		glTranslatef(60,0,0);
		//////// lomba part ta korchechi ekhane ////////////
		glPushMatrix();{
			glScalef(.3,1,1);
			glPushMatrix();{
				glTranslatef(0,0,50);
				glColor3f(1,1,1);
				glRotatef(90,0,1,0);
				glScalef(100,3,50);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, mBM);
				customSolidCube(1);
				glDisable(GL_TEXTURE_2D);

			}glPopMatrix();
		}glPopMatrix();
		////////////// end of the part /////////////
		glPushMatrix();{
			glScalef(.5,1,1);
			glPushMatrix();{
				glTranslatef(58,0,75);
				glColor3f(1,1,1);
				glRotatef(90,0,1,0);
				glScalef(50,3,100);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, mBM1);
				customSolidCube(1);
				glDisable(GL_TEXTURE_2D);
			}glPopMatrix();
		}glPopMatrix();
	}glPopMatrix();


	//////// 3nd part ta ///////
	glPushMatrix();{

		glTranslatef(120,0,0);
		//////// lomba part ta korchechi ekhane ////////////
		glPushMatrix();{
			glScalef(.3,1,1);
			glPushMatrix();{
				glTranslatef(0,0,50);
				glColor3f(1,1,1);
				glRotatef(90,0,1,0);
				glScalef(100,3,50);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, mBM);
				customSolidCube(1);
				glDisable(GL_TEXTURE_2D);

			}glPopMatrix();
		}glPopMatrix();
		////////////// end of the part /////////////
		glPushMatrix();{
			glScalef(.5,1,1);
			glPushMatrix();{
				glTranslatef(58,0,75);
				glColor3f(1,1,1);
				glRotatef(90,0,1,0);
				glScalef(50,3,100);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, mBM1);
				customSolidCube(1);
				glDisable(GL_TEXTURE_2D);
			}glPopMatrix();
		}glPopMatrix();
	}glPopMatrix();

	//////// 4th part ta ///////
	glPushMatrix();{

		glTranslatef(180,0,0);
		//////// lomba part ta korchechi ekhane ////////////
		glPushMatrix();{
			glScalef(.3,1,1);
			glPushMatrix();{
				glTranslatef(0,0,50);
				glColor3f(1,1,1);
				glRotatef(90,0,1,0);
				glScalef(100,3,50);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, mBM);
				customSolidCube(1);
				glDisable(GL_TEXTURE_2D);

			}glPopMatrix();
		}glPopMatrix();
		////////////// end of the part /////////////
		glPushMatrix();{
			glScalef(.5,1,1);
			glPushMatrix();{
				glTranslatef(58,0,75);
				glColor3f(1,1,1);
				glRotatef(90,0,1,0);
				glScalef(50,3,100);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, mBM1);
				customSolidCube(1);
				glDisable(GL_TEXTURE_2D);
			}glPopMatrix();
		}glPopMatrix();
	}glPopMatrix();


	glPushMatrix();{

		glTranslatef(240,0,0);

	//////// lomba part ta korchechi ekhane ////////////
		glPushMatrix();{
			glScalef(.3,1,1);
			glPushMatrix();{
				glTranslatef(0,0,50);
				glColor3f(1,1,1);
				glRotatef(90,0,1,0);
				glScalef(100,3,50);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, mBM);
				customSolidCube(1);
				glDisable(GL_TEXTURE_2D);

			}glPopMatrix();
		}glPopMatrix();
		}glPopMatrix();


	/*
	for(int i=0;i<5;i++){
	glPushMatrix();{
	x=x+30;
	glTranslatef(x,0,0);
	glPushMatrix();
	{
	glColor3f(.3,.3,.3);
	glTranslatef(0,0,0);
	glRotatef(90,1,0,0);
	double equ[4];
	equ[0]=0;
	equ[1]=1;
	equ[2]=0;
	equ[3]=0;
	glClipPlane(GL_CLIP_PLANE0,equ);
	glEnable(GL_CLIP_PLANE0);{
	glScalef(.4,1,1);
	glutSolidTorus(4,30,50,50);
	}glDisable(GL_CLIP_PLANE0);

	}glPopMatrix();
	glPushMatrix();{
	glTranslatef(15,0,10);
	glScalef(4,5,20);
	glutSolidCube(1);
	}glPopMatrix();
	}glPopMatrix();
	}
	*/
}

void drawMiddlebody(){

	/// dummy cube ////////
	/*
	glPushMatrix();{
	glColor3f(1,1,1);
	glTranslatef(0,0,125);
	glScalef(200,200,250);
	glutSolidCube(1);
	}glPopMatrix();
	*/


	/// front top part /////
	glPushMatrix();{
		glTranslatef(0,-142,132);
		glColor3f(1,1,1);
		glScalef(200,2,264);
		glRotatef(90,0,1,0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, ft);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();



	///// back side //////////////
	glPushMatrix();{
		glTranslatef(0,99.5,125);
		glColor3f(1,1,1);
		glScalef(200,2,250);
		glRotatef(90,0,1,0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, backWall);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	////
	glPushMatrix();{
		//glTranslatef(0,99.5,248);
		//glScalef(.5,1,1);
		drawtop();
	}glPopMatrix();

	///// roof side //////////////

	glPushMatrix();{
		glTranslatef(0,-22,249);
		glColor3f(1,1,1);
		glScalef(200,240,2);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, floorimg);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	///roof side left ///////
	glPushMatrix();{
		glTranslatef(-99,-23,225);
		glColor3f(1,1,1);
		glScalef(2,245,59);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, floorimg);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();


	//roof side right /////
	glPushMatrix();{
		glTranslatef(99,-23,225);
		glColor3f(1,1,1);
		glScalef(2,245,59);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, floorimg);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	//// 4 middle minar in front of mosque //////

	//back left middle minar /////
	glPushMatrix();{
		glTranslatef(-100,100,0);
		glScalef(.5,.5,.7);
		drawMiddlebodyMinar();
	}glPopMatrix();

	////back right middle minar ////
	glPushMatrix();{
		glTranslatef(100,100,0);
		glScalef(.5,.5,.7);
		drawMiddlebodyMinar();
	}glPopMatrix();

	//// front left middle minar //////
	glPushMatrix();{
		glTranslatef(-100,-150,0);
		glScalef(.5,.5,.7);
		drawMiddlebodyMinar();
	}glPopMatrix();

	/// front right middle minar /////
	glPushMatrix();{
		glTranslatef(100,-150,0);
		glScalef(.5,.5,.7);
		drawMiddlebodyMinar();
	}glPopMatrix();

	///// call main gombuj /////////////////////
	glPushMatrix();{
		glTranslatef(0,0,250);
		glScalef(1.4,1.4,1.8);
		drawGambuj();
	}glPopMatrix();



	//// most left back Minar of mosque ////
	glPushMatrix();{
		glTranslatef(-400,100,0);
		//glScalef(.5,.5,.5);
		drawbuildingMinar();
	}glPopMatrix();

	//// most right back Minar of mosque ////
	glPushMatrix();{
		glTranslatef(400,100,0);
		//glScalef(.5,.5,.5);
		drawbuildingMinar();
	}glPopMatrix();

	//// most left front Minar of mosque ////
	glPushMatrix();{
		glTranslatef(-400,-150,0);
		//glScalef(.5,.5,.5);
		drawbuildingMinar();
	}glPopMatrix();

	//// most right front Minar of mosque ////
	glPushMatrix();{
		glTranslatef(400,-150,0);
		//glScalef(.5,.5,.5);
		drawbuildingMinar();
	}glPopMatrix();

	///// left part of mosque //////
	glPushMatrix();{
		glTranslatef(-250,0,0);
		drawbuildingLeft();
	}glPopMatrix();

	///// right part of mosque //////
	glPushMatrix();{
		glTranslatef(250,0,0);
		drawbuildingRight();
	}glPopMatrix();

}

void drawMiddlebodyMinar(){

	////////minar body
	glPushMatrix();{
		glTranslatef(0,0,0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,bwu);
		glColor3f(1,1,1);
		GLUquadric *obj = gluNewQuadric();
		gluQuadricTexture(obj, GL_TRUE);
		gluCylinder(obj,20,20,500,6,8);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	//// jekhane gombuj ta bose tar bakano nicher part
	glPushMatrix();
	{
		glTranslatef(0,0,480);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,bwu);
		glColor3f(1,1,1);
		GLUquadric *obj = gluNewQuadric();
		gluQuadricTexture(obj, GL_TRUE);
		//glScalef(.65,.65,.2);
		gluCylinder(obj,20,40,35,6,8);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	//// jekhane gombuj ta bosbe
	glPushMatrix();
	{
		glTranslatef(0,0,515);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,largeMinar);
		glColor3f(1,1,1);
		glScalef(.8,.8,.1);
		glutSolidTorus(23,23,6,8);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();
	//calling gambuj part
	glPushMatrix();{
		glTranslatef(0,0,500);
		glScalef(.8,.8,1);
		piller();
	}glPopMatrix();

}

void piller(){


	//// uprer gombuj
	glPushMatrix();{
		glTranslatef(0,0,72);
		glScalef(.5,.5,.5);
		drawGambuj();

	}glPopMatrix();

	///pillar er uporer 8 kona part..
	glPushMatrix();
	{
		glTranslatef(0,0,68);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,w6img);
		glColor3f(1,1,1);
		glScalef(1,1,.2);
		glutSolidTorus(24,24,8,8);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	glPushMatrix();{

		//pillar 1 complete ///////////////////
		glPushMatrix();{
			//pillar base
			glPushMatrix();{
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,w6img);
				glColor3f(1,1,1);
				glTranslatef(0,-25,5);
				glScalef(11,11,10);
				glutSolidCube(1);
				glDisable(GL_TEXTURE_2D);
			}glPopMatrix();

			//pillar top
			glPushMatrix();{
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,w6img);
				glColor3f(1,1,1);
				glTranslatef(0,-25,60);
				glScalef(11,11,10);
				glutSolidCube(1);
				glDisable(GL_TEXTURE_2D);
			}glPopMatrix();

			//pillar 1
			glPushMatrix();{
				glTranslatef(0,-25,0);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,w6img);
				glColor3f(1,1,1);
				GLUquadric *obj = gluNewQuadric();
				//gluQuadricTexture(obj, GL_TRUE);
				gluCylinder(obj,4,4,60,20,20);
				glDisable(GL_TEXTURE_2D);

			}glPopMatrix();
		}glPopMatrix();
		/// end pillar 1 here //////////



		//pillar 4 complete  start///////////////////
		glPushMatrix();{
			//pillar base
			glPushMatrix();{
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,w6img);
				glColor3f(1,1,1);
				glTranslatef(0,25,5);
				glScalef(11,11,10);
				glutSolidCube(1);
				glDisable(GL_TEXTURE_2D);
			}glPopMatrix();

			//pillar top
			glPushMatrix();{
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,w6img);
				glColor3f(1,1,1);
				glTranslatef(0,25,60);
				glScalef(11,11,10);
				glutSolidCube(1);
				glDisable(GL_TEXTURE_2D);
			}glPopMatrix();

			//pillar 4
			glPushMatrix();{
				glTranslatef(0,25,0);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,w6img);
				glColor3f(1,1,1);
				GLUquadric *obj = gluNewQuadric();
				//gluQuadricTexture(obj, GL_TRUE);
				gluCylinder(obj,4,4,60,20,20);
				glDisable(GL_TEXTURE_2D);

			}glPopMatrix();
		}glPopMatrix();
		/// end pillar 4 here //////////


		//pillar 2 complete start ///////////////////
		glPushMatrix();{
			//pillar base
			glPushMatrix();{
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,w6img);
				glColor3f(1,1,1);
				glTranslatef(25,-12.5,5);
				glScalef(11,11,10);
				glutSolidCube(1);
				glDisable(GL_TEXTURE_2D);
			}glPopMatrix();

			//pillar top
			glPushMatrix();{
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,w6img);
				glColor3f(1,1,1);
				glTranslatef(25,-12.5,60);
				glScalef(11,11,10);
				glutSolidCube(1);
				glDisable(GL_TEXTURE_2D);
			}glPopMatrix();

			//pillar 2


			glPushMatrix();{

				glTranslatef(25,-12.5,0);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,w6img);
				glColor3f(1,1,1);
				GLUquadric *obj = gluNewQuadric();
				//gluQuadricTexture(obj, GL_TRUE);
				gluCylinder(obj,4,4,60,20,20);
				glDisable(GL_TEXTURE_2D);

			}glPopMatrix();
		}glPopMatrix();
		/// end pillar 2 here //////////

		//pillar 2 complete start ///////////////////
		glPushMatrix();{
			//pillar base
			glPushMatrix();{
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,w6img);
				glColor3f(1,1,1);
				glTranslatef(-25,-12.5,5);
				glScalef(11,11,10);
				glutSolidCube(1);
				glDisable(GL_TEXTURE_2D);
			}glPopMatrix();

			//pillar top
			glPushMatrix();{
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,w6img);
				glColor3f(1,1,1);
				glTranslatef(-25,-12.5,60);
				glScalef(11,11,10);
				glutSolidCube(1);
				glDisable(GL_TEXTURE_2D);
			}glPopMatrix();

			//pillar 4

			glPushMatrix();{

				glTranslatef(-25,-12.5,0);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,w6img);
				glColor3f(1,1,1);
				GLUquadric *obj = gluNewQuadric();
				//gluQuadricTexture(obj, GL_TRUE);
				gluCylinder(obj,4,4,60,20,20);
				glDisable(GL_TEXTURE_2D);

			}glPopMatrix();
		}glPopMatrix();
		/// end pillar 4 here //////////



		//pillar 3 complete start ///////////////////
		glPushMatrix();{
			//pillar base
			glPushMatrix();{
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,w6img);
				glColor3f(1,1,1);
				glTranslatef(25,12.5,5);
				glScalef(11,11,10);
				glutSolidCube(1);
				glDisable(GL_TEXTURE_2D);
			}glPopMatrix();

			//pillar top
			glPushMatrix();{
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,w6img);
				glColor3f(1,1,1);
				glTranslatef(25,12.5,60);
				glScalef(11,11,10);
				glutSolidCube(1);
				glDisable(GL_TEXTURE_2D);
			}glPopMatrix();

			//pillar 3
			glPushMatrix();{

				glTranslatef(25,12.5,0);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,w6img);
				glColor3f(1,1,1);
				GLUquadric *obj = gluNewQuadric();
				//gluQuadricTexture(obj, GL_TRUE);
				gluCylinder(obj,4,4,60,20,20);
				glDisable(GL_TEXTURE_2D);

			}glPopMatrix();
		}glPopMatrix();
		/// end pillar 5 here //////////

		//pillar 5 complete start ///////////////////
		glPushMatrix();{
			//pillar base
			glPushMatrix();{
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,w6img);
				glColor3f(1,1,1);
				glTranslatef(-25,12.5,5);
				glScalef(11,11,10);
				glutSolidCube(1);
				glDisable(GL_TEXTURE_2D);
			}glPopMatrix();

			//pillar top
			glPushMatrix();{
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,w6img);
				glColor3f(1,1,1);
				glTranslatef(-25,12.5,60);
				glScalef(11,11,10);
				glutSolidCube(1);
				glDisable(GL_TEXTURE_2D);
			}glPopMatrix();

			//pillar 5

			glPushMatrix();{

				glTranslatef(-25,12.5,0);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,w6img);
				glColor3f(1,1,1);
				GLUquadric *obj = gluNewQuadric();
				//gluQuadricTexture(obj, GL_TRUE);
				gluCylinder(obj,4,4,60,20,20);
				glDisable(GL_TEXTURE_2D);

			}glPopMatrix();
		}glPopMatrix();
		/// end pillar 5 here //////////



	}glPopMatrix();


	/*
	//// uprer gombuj
	glPushMatrix();{
	glTranslatef(0,25,74);
	glScalef(.4,.4,.4);
	drawGambuj();

	}glPopMatrix();


	////gombuj jekhane bosbe seta
	/*
	glPushMatrix();{
	glTranslatef(0,25,70);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,w6img);
	glColor3f(1,1,1);
	GLUquadric *obj = gluNewQuadric();
	//gluQuadricTexture(obj, GL_TRUE);
	gluCylinder(obj,30,30,8,20,20);
	glDisable(GL_TEXTURE_2D);
	}glPopMatrix();



	///pillar er uporer 8 kona part..
	glPushMatrix();
	{
	glTranslatef(0,25,68);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,w6img);
	glColor3f(1,1,1);
	glScalef(1,1,.2);
	glutSolidTorus(23,23,8,8);
	glDisable(GL_TEXTURE_2D);
	}glPopMatrix();


	double equation[4];
	equation[0] = 0;
	equation[1] = 0;
	equation[2] = 1;
	equation[3] = -20;

	glPushMatrix();
	{
	glClipPlane(GL_CLIP_PLANE0, equation);

	glEnable(GL_CLIP_PLANE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,minar);
	glColor3f(1,1,1);
	GLUquadric *qobj  = gluNewQuadric();
	gluQuadricTexture(qobj, GL_TRUE);
	gluSphere(qobj,70, 20, 20);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_CLIP_PLANE0);
	}
	glPopMatrix();


	//dui piller er bakano part

	glPushMatrix();
	{
	glColor3f(.3,.3,.3);
	glTranslatef(0,10,0);
	glRotatef(90,1,0,0);
	double equ[4];
	equ[0]=0;
	equ[1]=1;
	equ[2]=0;
	equ[3]=0;
	glClipPlane(GL_CLIP_PLANE0,equ);
	glEnable(GL_CLIP_PLANE0);{
	glScalef(.5,1,.2);
	glutSolidTorus(4,30,50,50);
	}glDisable(GL_CLIP_PLANE0);

	}glPopMatrix();


	glPushMatrix();
	{

	glColor3f(0,1,1);
	GLUquadric *qobj  = gluNewQuadric();
	//glutWireSphere(10,10,5);
	//glScalef(2,2,2);
	glutSolidTorus(3,6,20,20);
	//glutSolidDodecahedron();
	//gluSphere(qobj,70, 20, 20);
	}
	glPopMatrix();

	//for(int i=0; i<=5; i++){

	//pillar base





	glPushMatrix();{
	//glRotatef(54,0,0,1);


	glPushMatrix();{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,w6img);
	glColor3f(1,1,1);
	glTranslatef(0,0,5);
	glScalef(15,15,10);
	glutSolidCube(1);
	glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	//pillar top

	glPushMatrix();{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,w6img);
	glColor3f(1,1,1);
	glTranslatef(0,0,60);
	glScalef(15,15,10);
	glutSolidCube(1);
	glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	//main pillar

	glPushMatrix();{
	glTranslatef(0,0,4);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,w6img);
	glColor3f(1,1,1);
	GLUquadric *obj = gluNewQuadric();
	//gluQuadricTexture(obj, GL_TRUE);
	gluCylinder(obj,4,4,60,20,20);
	glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	//dui pillar er bakano part

	glPushMatrix();{
	//glRotatef(45,1,1,0);
	glTranslatef(15,0,50);
	glPushMatrix();
	{
	//glColor3f(.3,.3,.3);
	glColor3f(.8,.4,0);
	glTranslatef(0,0,0);
	glRotatef(90,1,0,0);
	double equ[4];
	equ[0]=0;
	equ[1]=1;
	equ[2]=0;
	equ[3]=0;
	glClipPlane(GL_CLIP_PLANE0,equ);
	glEnable(GL_CLIP_PLANE0);{
	glScalef(.3,.5,.5);
	glutSolidTorus(8,30,50,50);
	}glDisable(GL_CLIP_PLANE0);

	}glPopMatrix();
	}
	glPopMatrix();

	//2nd pillar
	glPushMatrix();{
	glTranslatef(30,0,0);
	//pillar base

	glPushMatrix();{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,w6img);
	glColor3f(1,1,1);
	glTranslatef(0,0,5);
	glScalef(15,15,10);
	glutSolidCube(1);
	glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	//pillar top

	glPushMatrix();{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,w6img);
	glColor3f(1,1,1);
	glTranslatef(0,0,60);
	glScalef(15,15,10);
	glutSolidCube(1);
	glDisable(GL_TEXTURE_2D);
	}glPopMatrix();
	glPushMatrix();{
	glTranslatef(0,0,4);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,w6img);
	glColor3f(1,1,1);
	GLUquadric *obj = gluNewQuadric();
	//gluQuadricTexture(obj, GL_TRUE);
	gluCylinder(obj,4,4,60,20,20);
	glDisable(GL_TEXTURE_2D);
	}glPopMatrix();
	}
	glPopMatrix();
	/*
	glPushMatrix();{
	//glRotatef(45,1,1,0);
	glTranslatef(45,0,50);
	glPushMatrix();
	{
	glColor3f(.3,.3,.3);
	glTranslatef(0,0,0);
	glRotatef(90,1,0,0);
	double equ[4];
	equ[0]=0;
	equ[1]=1;
	equ[2]=0;
	equ[3]=0;
	glClipPlane(GL_CLIP_PLANE0,equ);
	glEnable(GL_CLIP_PLANE0);{
	glScalef(.3,.5,.5);
	glutSolidTorus(8,30,50,50);
	}glDisable(GL_CLIP_PLANE0);

	}glPopMatrix();
	}
	glPopMatrix();

	}glPopMatrix();


	//////// left 2 piller /////////////////////////////////////////////////

	glPushMatrix();{
	glTranslatef(0,50,0);
	glRotatef(-45,0,0,1);

	///pillar base
	glPushMatrix();{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,w6img);
	glColor3f(1,1,1);
	glTranslatef(0,0,5);
	glScalef(15,15,10);
	glutSolidCube(1);
	glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	//pillar top

	glPushMatrix();{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,w6img);
	glColor3f(1,1,1);
	glTranslatef(0,0,60);
	glScalef(15,15,10);
	glutSolidCube(1);
	glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	//main pillar

	glPushMatrix();{
	glTranslatef(0,0,4);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,w6img);
	glColor3f(1,1,1);
	GLUquadric *obj = gluNewQuadric();
	//gluQuadricTexture(obj, GL_TRUE);
	gluCylinder(obj,4,4,60,20,20);
	glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	//dui pillar er bakano part

	glPushMatrix();{
	//glRotatef(45,1,1,0);
	glTranslatef(15,0,50);
	glPushMatrix();
	{
	//glColor3f(.3,.3,.3);
	glColor3f(.8,.4,0);
	glTranslatef(0,0,0);
	glRotatef(90,1,0,0);
	double equ[4];
	equ[0]=0;
	equ[1]=1;
	equ[2]=0;
	equ[3]=0;
	glClipPlane(GL_CLIP_PLANE0,equ);
	glEnable(GL_CLIP_PLANE0);{
	glScalef(.3,.5,.5);
	glutSolidTorus(8,30,50,50);
	}glDisable(GL_CLIP_PLANE0);

	}glPopMatrix();
	}
	glPopMatrix();
	/*
	//2nd pillar
	glPushMatrix();{
	glTranslatef(30,0,0);
	//pillar base

	glPushMatrix();{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,w6img);
	glColor3f(1,1,1);
	glTranslatef(0,0,5);
	glScalef(15,15,10);
	glutSolidCube(1);
	glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	//pillar top

	glPushMatrix();{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,w6img);
	glColor3f(1,1,1);
	glTranslatef(0,0,60);
	glScalef(15,15,10);
	glutSolidCube(1);
	glDisable(GL_TEXTURE_2D);
	}glPopMatrix();
	glPushMatrix();{
	glTranslatef(0,0,4);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,w6img);
	glColor3f(1,1,1);
	GLUquadric *obj = gluNewQuadric();
	//gluQuadricTexture(obj, GL_TRUE);
	gluCylinder(obj,6,6,60,20,20);
	glDisable(GL_TEXTURE_2D);
	}glPopMatrix();
	}
	glPopMatrix();
	glPushMatrix();{
	//glRotatef(45,1,1,0);
	glTranslatef(45,0,50);
	glPushMatrix();
	{
	glColor3f(.3,.3,.3);
	glTranslatef(0,0,0);
	glRotatef(90,1,0,0);
	double equ[4];
	equ[0]=0;
	equ[1]=1;
	equ[2]=0;
	equ[3]=0;
	glClipPlane(GL_CLIP_PLANE0,equ);
	glEnable(GL_CLIP_PLANE0);{
	glScalef(.3,.5,.5);
	glutSolidTorus(8,30,50,50);
	}glDisable(GL_CLIP_PLANE0);

	}glPopMatrix();
	}
	glPopMatrix();


	}glPopMatrix();



	////// left pillar between two pillar ///////////////////////////////


	//////total left  ///////////
	glPushMatrix();{

	glTranslatef(-25,25,0);
	glRotatef(54,0,0,1);

	////bakano ////////////////////////////
	glPushMatrix();{
	//glRotatef(45,1,1,0);
	glTranslatef(18.6,0,50);
	glPushMatrix();
	{
	//glColor3f(.3,.3,.3);
	glColor3f(.8,.4,0);
	glTranslatef(0,0,0);
	glRotatef(90,1,0,0);
	double equ[4];
	equ[0]=0;
	equ[1]=1;
	equ[2]=0;
	equ[3]=0;
	glClipPlane(GL_CLIP_PLANE0,equ);
	glEnable(GL_CLIP_PLANE0);{
	glScalef(.4,.5,.5);
	glutSolidTorus(8,30,50,50);
	}glDisable(GL_CLIP_PLANE0);

	}glPopMatrix();
	}glPopMatrix();

	////base //////////////////
	glPushMatrix();{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,w6img);
	glColor3f(1,1,1);
	glTranslatef(0,0,5);
	glScalef(15,15,10);
	glutSolidCube(1);
	glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	//pillar top

	glPushMatrix();{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,w6img);
	glColor3f(1,1,1);
	glTranslatef(0,0,60);
	glScalef(15,15,10);
	glutSolidCube(1);
	glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	//main pillar

	glPushMatrix();{
	glTranslatef(0,0,4);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,w6img);
	glColor3f(1,1,1);
	GLUquadric *obj = gluNewQuadric();
	//gluQuadricTexture(obj, GL_TRUE);
	gluCylinder(obj,4,4,60,20,20);
	glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	}glPopMatrix();

	/// baka ////////////////
	glPushMatrix();{
	glTranslatef(-13.7,12,50);
	glRotatef(-45,0,0,1);

	glPushMatrix();
	{
	//glColor3f(.3,.3,.3);
	glColor3f(.8,.4,0);
	glTranslatef(0,0,0);
	glRotatef(90,1,0,0);
	double equ[4];
	equ[0]=0;
	equ[1]=1;
	equ[2]=0;
	equ[3]=0;
	glClipPlane(GL_CLIP_PLANE0,equ);
	glEnable(GL_CLIP_PLANE0);{
	glScalef(.36,.5,.5);
	glutSolidTorus(8,30,50,50);
	}glDisable(GL_CLIP_PLANE0);

	}glPopMatrix();
	}
	glPopMatrix();

	/*
	if(i==0)
	glTranslatef(25,25,0);
	else if(i==1)
	glTranslatef(0,25,0);
	else if(i==2)
	glTranslatef(-25,25,0);
	else if(i==3)
	glTranslatef(-25,-25,0);
	else if(i==4)
	glTranslatef(0,-25,0);
	else
	glTranslatef(0,0,0);
	//glRotatef();
	}

	*/

}

void drawbuildingMinar(){

	// nicher part
	glPushMatrix();{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,bw2);
		glColor3f(1,1,1);
		GLUquadric *obj = gluNewQuadric();
		gluQuadricTexture(obj, GL_TRUE);
		gluCylinder(obj,35,35,200,8,8);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	//uporer part
	glPushMatrix();{
		glTranslatef(0,0,200);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,bwu);
		glColor3f(1,1,1);
		GLUquadric *obj = gluNewQuadric();
		gluQuadricTexture(obj, GL_TRUE);
		gluCylinder(obj,35,33,75,6,8);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();


	//// jekhane gombuj ta bosbe
	glPushMatrix();
	{
		glTranslatef(0,0,275);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,bw2);
		glColor3f(1,1,1);
		glScalef(.7,.7,.1);
		glutSolidTorus(30,30,6,8);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	//// jekhane gombuj ta bose tar bakano nicher part
	glPushMatrix();
	{
		glTranslatef(0,0,263);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,bwu);
		glColor3f(1,1,1);
		GLUquadric *obj = gluNewQuadric();
		gluQuadricTexture(obj, GL_TRUE);
		//glScalef(.65,.65,.2);
		gluCylinder(obj,35,43,10,6,8);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	//calling gambuj part
	glPushMatrix();{
		glTranslatef(0,0,250);
		glScalef(.8,.8,1);
		piller();
	}glPopMatrix();


}

void drawbuildingRight(){
	//building back wall
	/*glPushMatrix();{
	glColor3f(1,1,1);
	glTranslatef(0,0,100);
	glScalef(300,200,200);
	glutSolidCube(1);
	}glPopMatrix();
	*/
	///front top little most part /////////////
	glPushMatrix();{
		glTranslatef(0,-140,200+10);
		glColor3f(1,1,1);
		glScalef(300,1.5,26);
		glRotatef(90,0,1,0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tmp);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	//// buinding front top part ////
	glPushMatrix();{
		glTranslatef(0,-140,100+50);
		glColor3f(1,1,1);
		glScalef(300,1.5,100);
		glRotatef(90,0,1,0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, lrt);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	//// buinding front bottom part ////
	glPushMatrix();{
		glTranslatef(-135,-140,0);
		glScalef(1.03,1,1);
		drawFront();
	}glPopMatrix();

	//// building side wall ///////
	glPushMatrix();{

		glTranslatef(-650,-10,119);
		glColor3f(1,1,1);
		glScalef(1.5,212,238);
		glRotatef(90,0,1,0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, sideWall);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	//building back wall
	glPushMatrix();{
		glTranslatef(0,99.5,100);
		glColor3f(1,1,1);
		glScalef(300,1.5,200);
		glRotatef(90,0,1,0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, backWall);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();


	glPushMatrix();{
		glTranslatef(0,99.5,198);
		glScalef(.5,1,1);
		glPushMatrix();{

			glTranslatef(0,0,25);
			glColor3f(1,1,1);
			glRotatef(90,0,1,0);
			glScalef(50,2,600);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, topMost);
			customSolidCube(1);
			glDisable(GL_TEXTURE_2D);
		}glPopMatrix();
	}glPopMatrix();

	///// roof side //////////////

	glPushMatrix();{
		glTranslatef(0,-20,199);
		glColor3f(1,1,1);
		glScalef(300,239,2);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, floorimg);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();


	//building roof where gombuj will be placed
	glPushMatrix();{
		glColor3f(.8,.8,.8);
		glTranslatef(0,0,204);
		glScalef(190,190,8);
		glutSolidCube(1);
	}glPopMatrix();

	//right roof top place gombuj //////////////
	glPushMatrix();{
		glTranslatef(0,0,208);
		glScalef(1.2,1.2,1.5);
		drawGambuj();
	}glPopMatrix();
}

void drawbuildingLeft(){


	//building back wall
	/*
	glPushMatrix();{
	glColor3f(1,1,1);
	glTranslatef(0,0,100);
	glScalef(300,200,200);
	glutSolidCube(1);
	}glPopMatrix();
	*/

	//building back wall
	/*
	glPushMatrix();{
	glColor3f(1,.4,0);
	glTranslatef(0,0,100);
	glScalef(300,3,200);
	glutSolidCube(1);
	}glPopMatrix();

	//building left wall
	glPushMatrix();{
	glColor3f(1,.4,0);
	glTranslatef(-149,-133.5,100);
	glScalef(2,267,200);
	glutSolidCube(1);
	}glPopMatrix();

	//building front wall
	glPushMatrix();{
	glColor3f(1,.4,0);
	glTranslatef(0,-267,100);
	glScalef(300,3,200);
	glutSolidCube(1);
	}glPopMatrix();

	//building roof
	glPushMatrix();{
	glColor3f(.9,.9,.9);
	glTranslatef(0,-133.5,200);
	glScalef(300,267,2);
	glutSolidCube(1);
	}glPopMatrix();
	*/
	///front top little most part /////////////
	glPushMatrix();{
		glTranslatef(0,-140,200+10);
		glColor3f(1,1,1);
		glScalef(300,1.5,26);
		glRotatef(90,0,1,0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tmp);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	//// buinding front top part ////
	glPushMatrix();{
		glTranslatef(0,-140,100+50);
		glColor3f(1,1,1);
		glScalef(300,1.5,100);
		glRotatef(90,0,1,0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, lrt);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	//// buinding front bottom part ////
	glPushMatrix();{
		glTranslatef(-110,-140,0);
		glScalef(1.03,1,1);
		drawFront();
	}glPopMatrix();


	//// building side wall ///////
	glPushMatrix();{

		glTranslatef(650,-10,119);
		glColor3f(1,1,1);
		glScalef(1.5,212,238);
		glRotatef(90,0,1,0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, sideWall);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();


	//building back wall
	glPushMatrix();{
		glTranslatef(0,99.5,100);
		glColor3f(1,1,1);
		glScalef(300,1.5,200);
		glRotatef(90,0,1,0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, backWall);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();



	glPushMatrix();{
		glTranslatef(0,99.5,198);
		glScalef(.5,1,1);
		glPushMatrix();{

			glTranslatef(0,0,25);
			glColor3f(1,1,1);
			glRotatef(90,0,1,0);
			glScalef(50,2,600);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, topMost);
			customSolidCube(1);
			glDisable(GL_TEXTURE_2D);
		}glPopMatrix();
	}glPopMatrix();

	///// roof side //////////////

	glPushMatrix();{
		glTranslatef(0,-20,199);
		glColor3f(1,1,1);
		glScalef(300,239,2);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, floorimg);
		customSolidCube(1);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();
	//building roof where gombuj will be placed
	/*glPushMatrix();{
	glColor3f(.8,.8,.8);
	glTranslatef(0,-93,204);
	glScalef(190,190,8);
	glutSolidCube(1);
	}glPopMatrix();
	*/

	glPushMatrix();{
		glColor3f(.8,.8,.8);
		//glColor3f(1,1,1);
		glTranslatef(0,0,204);
		glScalef(190,190,8);
		glutSolidCube(1);
	}glPopMatrix();


	//left roof top side gombuj //////////////
	glPushMatrix();{
		glTranslatef(0,0,208);
		glScalef(1.2,1.2,1.5);
		drawGambuj();
	}glPopMatrix();


	/*
	glPushMatrix();{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, top);
	glColor3f(1,1,1);
	glTranslatef(-240,-5,0);
	//glRotatef(90,0,0,1);
	glBegin(GL_QUADS);
	glTexCoord2f(0,0);
	glVertex3f(0,0,300);
	glTexCoord2f(1,0);
	glVertex3f(480,0,300);
	glTexCoord2f(1,1);
	glVertex3f(480,0,0);
	glTexCoord2f(0,1);
	glVertex3f(0,0,0);
	glEnd();

	glDisable(GL_TEXTURE_2D);
	}glPopMatrix();
	*/

}

void drawtestCylinder(){
	/*
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,w6img);
	glColor3f(1,1,1);
	GLUquadric *obj = gluNewQuadric();
	gluQuadricTexture(obj, GL_TRUE);
	gluCylinder(obj,30,10,100,4,10);
	glDisable(GL_TEXTURE_2D);
	*/




	glPushMatrix();{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,w6img);
		glColor3f(1,1,1);
		GLUquadric *obj = gluNewQuadric();
		gluQuadricTexture(obj, GL_TRUE);
		glutSolidTorus(10,15,5,10);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	/*
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,cylimg);
	glColor3f(1,1,1);
	GLUquadric *cyl  = gluNewQuadric();
	gluQuadricTexture(cyl, GL_TRUE);
	gluCylinder(cyl,20,20,30,6,20);
	glDisable(GL_TEXTURE_2D);
	*/
}



void drawminar(){

	// calling pillar with gombuj //////////////
	glPushMatrix();{
		glTranslatef(0,0,655);
		glScalef(1.2,1.2,1);
		piller();
	}glPopMatrix();

	//// jekhane gombuj ta bosbe
	glPushMatrix();
	{
		glTranslatef(0,0,655);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,largeMinar);
		glColor3f(1,1,1);
		glScalef(.8,.8,.1);
		glutSolidTorus(30,30,6,8);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	//// jekhane gombuj ta bose tar bakano nicher part
	glPushMatrix();
	{
		glTranslatef(0,0,634);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,largeMinar);
		glColor3f(1,1,1);
		GLUquadric *obj = gluNewQuadric();
		gluQuadricTexture(obj, GL_TRUE);
		//glScalef(.65,.65,.2);
		gluCylinder(obj,35,55,20,6,8);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();


	//// base er uprer part ta /////////////////////
	glPushMatrix();
	{
		glTranslatef(0,0,250);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,largeMinar);
		glColor3f(1,1,1);
		GLUquadric *obj = gluNewQuadric();
		gluQuadricTexture(obj, GL_TRUE);
		glScalef(1,1,2);
		gluCylinder(obj,50,90,20,6,8);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	/////////////  middle part ta  //////////////////
	glPushMatrix();
	{
		glTranslatef(0,0,480);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,largeMinar);
		glColor3f(1,1,1);
		GLUquadric *obj = gluNewQuadric();
		gluQuadricTexture(obj, GL_TRUE);
		glScalef(1,1,2);
		gluCylinder(obj,40,75,20,6,8);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	// body /////////////////
	glPushMatrix();{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,largeMinar);
		glColor3f(1,1,1);
		GLUquadric *obj = gluNewQuadric();
		gluQuadricTexture(obj, GL_TRUE);
		gluCylinder(obj,80,40,650,6,8);
		glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	////////////////// test part
	/*
	glPushMatrix();{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,wallLight);
	glColor3f(1,1,1);
	//glColor3f(0,0,1);
	//glTranslatef(0,0,100);
	//glScalef(2,2,10);

	//glColor3f(1,1,1);
	//GLUquadric *qobj  = gluNewQuadric();
	//gluQuadricTexture(qobj, GL_TRUE);

	GLUquadricObj *object = gluNewQuadric();
	gluCylinder(object, 62, 60, 20, 20, 20);
	//gluSphere(qobj,65, 20, 20);
	//glutSolidCone(qobj, 20, 20, 30, 10);
	//glutSolidTorus(qobj,20,140,20,6);
	glDisable(GL_TEXTURE_2D);

	/*
	GLUquadric *qobj  = gluNewQuadric();
	gluQuadricTexture(qobj, GL_TRUE);
	glutSolidTorus(qobj,20,140,20,6);
	glDisable(GL_TEXTURE_2D);
	}glPopMatrix();

	/// end of test part

	/*
	glPushMatrix();{
	//glTranslatef(0,0,5200);
	glColor3f(0,0,1);
	glutSolidCone(50,100,10,20);
	}glPopMatrix();


	glPushMatrix();{
	glTranslatef(0,0,5120);
	glColor3f(1,0,0);
	//glScalef(3,4,3.5);
	glutSolidSphere(45,30,30);
	}glPopMatrix();

	glPushMatrix();{
	glTranslatef(0,0,5010);
	glColor3f(0,0,1);
	GLUquadricObj *obj = gluNewQuadric();
	gluCylinder(obj,20,20,250,30,30);
	}glPopMatrix();

	glPushMatrix();{
	glTranslatef(0,0,5000);
	glColor3f(0,0,1);
	glScalef(1.5,1.5,1);
	glutSolidSphere(50,10,30);
	}glPopMatrix();


	///////////////////////////

	glPushMatrix();{
	double equ[4];
	equ[0]=0;
	equ[1]=0;
	equ[2]=1;
	equ[3]=-400;
	glClipPlane(GL_CLIP_PLANE0,equ);
	glEnable(GL_CLIP_PLANE0);{
	//glTranslatef(0,0,30);
	glColor3f(1,0,0);
	glTranslatef(0,0,4700);
	glutSolidSphere(300,30,30);

	glColor3f(0,0,1);
	//glTranslatef(0,0,0.6);
	glutWireSphere(300,10,30);
	}glDisable(GL_CLIP_PLANE0);
	}glPopMatrix();

	glPushMatrix();{

	glColor3f(0,0,1);
	glTranslatef(0,0,4500);
	//glRotatef(180,1,0,0);
	glutSolidCone(550,400,6,20);
	glColor3f(1,0,1);
	glutWireCone(550,400,6,1);

	}glPopMatrix();

	glPushMatrix();{
	glColor3f(1,0,0);
	glTranslatef(0,0,4300);

	glScalef(2,2,15);
	glutSolidTorus(15,140,20,6);

	}glPopMatrix();

	glPushMatrix();{
	glColor3f(1,0,1);

	glTranslatef(0,0,2010);
	glRotatef(-90,0,1,0);

	glScalef(20,4.5,4.5);

	glutSolidSphere(40,2,3);

	}glPopMatrix();


	glPushMatrix();{

	glColor3f(1,0,1);
	glTranslatef(0,0,4200);
	glRotatef(180,1,0,0);
	glutSolidCone(550,650,6,20);
	glColor3f(1,0,1);
	glutWireCone(550,650,6,1);

	}glPopMatrix();

	//Diamond shape of minar (in middle)
	glPushMatrix();{
	glColor3f(1,0,0);
	glTranslatef(0,0,3900);
	glScalef(2,2,2);
	glutSolidTorus(15,170,20,6);

	}glPopMatrix();


	glPushMatrix();{
	glColor3f(1,0,1);
	glTranslatef(0,0,3100);
	glRotatef(180,1,0,0);
	glutSolidCone(550,650,6,20);
	glColor3f(1,0,1);
	glutWireCone(550,650,6,1);
	}glPopMatrix();


	//Diamond shape of minar (in middle)
	glPushMatrix();{
	glColor3f(1,0,0);
	glTranslatef(0,0,2800);

	glScalef(2,2,2);
	glutSolidTorus(15,170,20,6);
	}glPopMatrix();

	/// ring in the diamond shape

	glPushMatrix();{
	glColor3f(1,0,1);
	glTranslatef(0,0,1800);
	glRotatef(180,1,0,0);
	glutSolidCone(550,650,6,20);
	glColor3f(1,0,1);
	glutWireCone(550,650,6,1);

	}glPopMatrix();


	//Diamond shape of minar (in middle)

	glPushMatrix();{
	glColor3f(1,0,0);
	glTranslatef(0,0,1500);

	glScalef(2,2,2);
	glutSolidTorus(15,170,20,6);

	}glPopMatrix();


	/// minar main body

	glPushMatrix();{
	//glColor3f(0,0,1);
	glColor3f(.8 ,.4,0);
	glTranslatef(0,0,2200);

	glScalef(2,2,100);
	glutSolidTorus(20,140,20,6);
	}glPopMatrix();




	//Diamond shape of minar (in middle)

	glPushMatrix();{
	glColor3f(1,0,0);
	glTranslatef(0,0,350);
	glScalef(2,2,2);
	glutSolidTorus(15,170,20,6);

	}glPopMatrix();


	////base of the minar

	glPushMatrix();{
	glTranslatef(0,0,185);
	glColor3f(1,0,1);
	glScalef(2,2,8);
	glutSolidTorus(20,170,20,6);

	}glPopMatrix();



	//no use
	/
	glPushMatrix();{
	glColor3f(0,0,1);
	glTranslatef(0,0,160);
	glRotatef(-90,0,1,0);
	glScalef(4,4,4);
	glutSolidSphere(130,2,3);

	}glPopMatrix();

	glPushMatrix();{
	glColor3f(1,0,0);
	glTranslatef(0,0,150);
	glRotatef(-90,0,1,0);

	glScalef(4,4,4);

	glutSolidSphere(130,2,3);
	}glPopMatrix();


	glPushMatrix();{
	glColor3f(1,0,1);
	glTranslatef(0,0,90);

	glScalef(2,2,4);
	glutSolidTorus(20,190,20,6);

	}glPopMatrix();

	glPushMatrix();{
	glColor3f(1,0,0);
	glTranslatef(0,0,10);
	glRotatef(-90,0,1,0);
	glScalef(4,4,4);
	glutSolidSphere(130,2,3);
	}glPopMatrix();



	glPushMatrix();{
	glColor3f(0,0,1);
	glTranslatef(0,0,30);
	glRotatef(-90,0,1,0);

	glScalef(4,4,4);

	glutSolidSphere(130,2,3);

	}glPopMatrix();
	*/


}

/****************************** GridLines and Axes ***********************************/

void gridAndAxis(){
	// draw the three major AXES
	if(canDrawAxis==1){
		glBegin(GL_LINES);
			//X axis
			glColor3f(0, 1, 0);	//100% Green
			glVertex3f(-150, 0, 0);
			glVertex3f( 150, 0, 0);

			//Y axis
			glColor3f(0, 0, 1);	//100% Blue
			glVertex3f(0, -150, 0);	// intentionally extended to -150 to 150, no big deal
			glVertex3f(0,  150, 0);

			//Z axis
			glColor3f(1, 1, 1);	//100% White
			glVertex3f( 0, 0, -150);
			glVertex3f(0, 0, 150);
		glEnd();
	}

	if(canDrawGrid == 1){
	//some gridlines along the field
		int i;

		glColor3f(0.5, 0.5, 0.5);	//grey
		glBegin(GL_LINES);
			for(i=-10;i<=10;i++){

				if(i==0)
					continue;	//SKIP the MAIN axes

				//lines parallel to Y-axis
				glVertex3f(i*10, -100, 0);
				glVertex3f(i*10,  100, 0);

				//lines parallel to X-axis
				glVertex3f(-100, i*10, 0);
				glVertex3f( 100, i*10, 0);
			}
		glEnd();

	}
}

/********************************************************************************/

void display(){
	//codes for Models, Camera

	//clear the display
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0,0,0,0);	//color black
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		//clear buffers to preset values

	/***************************
	/ set-up camera (view) here
	****************************/
	//load the correct matrix -- MODEL-VIEW matrix
	glMatrixMode(GL_MODELVIEW);		//specify which matrix is the current matrix

	//initialize the matrix
	glLoadIdentity();				//replace the current matrix with the identity matrix [Diagonals have 1, others have 0]

	//now give three info
	//1. where is the camera (viewer)?
	//2. where is the camera looking?
	//3. Which direction is the camera's UP direction?

	//gluLookAt(0,-150,20,	0,0,0,	0,0,1);
	//gluLookAt(cameraRadius*sin(cameraAngle), -cameraRadius*cos(cameraAngle), cameraHeight,		0,0,0,		0,0,1);

	V to=loc+dir;
	gluLookAt(loc.x, loc.y, loc.z,		to.x,to.y,to.z,		perp.x,perp.y,perp.z);

	//again select MODEL-VIEW
	glMatrixMode(GL_MODELVIEW);


	/**************************************************
	/ Grid and axes Lines(You can remove them if u want)
	***************************************************/

	gridAndAxis();


	/****************************
	/ Add your objects from here
	****************************/
	//// below al for kajer

	carpetInside();
	drawMiddlebody();
	drawWall();
	drawFloor();
	drawSky();


	//ADD this line in the end --- if you use double buffer (i.e. GL_DOUBLE)
	glFlush();
	glutSwapBuffers();
}

void animate(){
	//codes for any changes in Models, Camera

	//cameraAngle += cameraAngleDelta;	// camera will rotate at 0.002 radians per frame.

	//codes for any changes in Models

	//MISSING SOMETHING? -- YES: add the following
	glutPostRedisplay();	//this will call the display AGAIN

}

void init(){
	//codes for initialization
	loadImage();

	cameraAngle = 0;	//angle in radian
	move_X = 0;
	move_Y = 0;
	move_Z = 0;
	canDrawGrid = 1;
	canDrawAxis = 1;

	cameraAngleDelta = .001;

	cameraRadius = 150;
	cameraHeight = 50;


	loc=_L,dir=_D,perp=_P;
	//clear the screen
	glClearColor(0,0,0, 0);

	/************************
	/ set-up projection here
	************************/
	//load the PROJECTION matrix
	glMatrixMode(GL_PROJECTION);

	//initialize the matrix
	glLoadIdentity();

	/*
		gluPerspective() — set up a perspective projection matrix

		fovy -         Specifies the field of view angle, in degrees, in the y direction.
        aspect ratio - Specifies the aspect ratio that determines the field of view in the x direction. The aspect ratio is the ratio of x (width) to y (height).
        zNear -        Specifies the distance from the viewer to the near clipping plane (always positive).
        zFar  -        Specifies the distance from the viewer to the far clipping plane (always positive).
    */

	gluPerspective(70,	1,	0.1,	10000.0);

}

void keyboardListener(unsigned char key, int x,int y){
	switch(key){

	case 'w':case 'W':
		loc=loc+speed*dir;
		break;

	case 'a':case 'A':
		loc=loc+speed*((perp*dir).unit());
		break;

	case 's':case 'S':
		loc=loc-speed*dir;
		break;

	case 'd':case 'D':
		loc=loc+speed*((dir*perp).unit());
		break;

	case 'q':case 'Q':
		perp=perp.rot(dir,ang_speed);
		break;

	case 'e':case 'E':
		perp=perp.rot(-dir,ang_speed);
		break;


	default:
		break;
	}
}

void specialKeyListener(int key, int x,int y){
	V p;

	switch(key){
	case GLUT_KEY_UP:
		p=(dir*perp).unit();
		dir=dir.rot(p,ang_speed);
		perp=perp.rot(p,ang_speed);
		break;
	case GLUT_KEY_DOWN:
		p=(perp*dir).unit();
		dir=dir.rot(p,ang_speed);
		perp=perp.rot(p,ang_speed);
		break;

	case GLUT_KEY_LEFT:
		dir=dir.rot(-perp,ang_speed);
		break;
	case GLUT_KEY_RIGHT:
		dir=dir.rot(perp,ang_speed);
		break;

	case GLUT_KEY_PAGE_UP:
		loc=loc+speed*perp;
		break;
	case GLUT_KEY_PAGE_DOWN:
		loc=loc-speed*perp;
		break;

	case GLUT_KEY_INSERT:
		break;

	case GLUT_KEY_HOME:
		loc=_L,dir=_D,perp=_P;
		break;
	case GLUT_KEY_END:
		break;

	default:
		break;
	}
}

void mouseListener(int button, int state, int x, int y){	//x, y is the x-y of the screen (2D)
	switch(button){
		case GLUT_LEFT_BUTTON:
			if(state == GLUT_DOWN){		// 2 times?? in ONE click? -- solution is checking DOWN or UP

			}
			break;

		case GLUT_RIGHT_BUTTON:
			//........
			break;

		case GLUT_MIDDLE_BUTTON:
			//........
			break;

		default:
			break;
	}
}

int main(int argc, char **argv){

	glutInit(&argc,argv);							//initialize the GLUT library

	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);

	/*
		glutInitDisplayMode - inits display mode
		GLUT_DOUBLE - allows for display on the double buffer window
		GLUT_RGBA - shows color (Red, green, blue) and an alpha
		GLUT_DEPTH - allows for depth buffer
	*/
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);

	glutCreateWindow("Some Title");

	printf("Camera Control\n");
	printf("_____________\n");
	printf("Roll: UP and DOWN arrow\n");
	printf("Pitch: Q and E\n");
	printf("Yaw: LEFT and RIGHT arrow\n");
	printf("Up-Down: PAGEUP and PAGEDOWN\n");
	printf("Left-Right: A and D\n");
	printf("Zoom in-out: W and S\n");
	printf("Reset Camera: HOME\n");

	init();						//codes for initialization

	glEnable(GL_DEPTH_TEST);	//enable Depth Testing

	glutDisplayFunc(display);	//display callback function
	glutIdleFunc(animate);		//what you want to do in the idle time (when no drawing is occuring)

	glutKeyboardFunc(keyboardListener);
	glutSpecialFunc(specialKeyListener);

	glutMouseFunc(mouseListener);

	glutMainLoop();		//The main loop of OpenGL

	return 0;
}
