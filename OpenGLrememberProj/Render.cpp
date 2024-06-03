#include "Render.h"
#include <cmath>
#include <sstream>
#include <iostream>

#include <Windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

bool textureMode = true;
bool lightMode = true;

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
	if (key == 'V')
	{
		
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



GLuint texId;

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}





void Render(OpenGL *ogl)
{

	

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);



	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  
	double A[] = { 0,0,0 };
	double B[] = { 3,8,0 };
	double C[] = { 8,8,0 };
	double D[] = { 8,1,0 };
	double E[] = { 10,4,0 };
	double F[] = { 14,4,0 };
	double G[] = { 16,0,0 };
	double H[] = { 11,-5,0 };
	double A1[] = { 0,0,5 };
	double B1[] = { 3,8,5 };
	double C1[] = { 8,8,5 };
	double D1[] = { 8,1,5 };
	double E1[] = { 10,4,5 };
	double F1[] = { 14,4,5 };
	double G1[] = { 16,0,5 };
	double H1[] = { 11,-5,5 };
	//Нижняя грань нач
	


	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, -1);
	glColor3d(0, 0, 1);
	glVertex3dv(B);
	glVertex3dv(C);
	glVertex3dv(D);
	glEnd();

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, -1);
	glColor3d(0, 0, 1);
	glVertex3dv(C);
	glVertex3dv(D);
	glVertex3dv(E);
	glEnd();

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, -1);
	glColor3d(0, 0, 1);
	glVertex3dv(E);
	glVertex3dv(D);
	glVertex3dv(F);
	glEnd();

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, -1);
	glColor3d(0, 0, 1);
	glVertex3dv(F);
	glVertex3dv(D);
	glVertex3dv(H);
	glEnd();

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, -1);
	glColor3d(0, 0, 1);
	glVertex3dv(F);
	glVertex3dv(H);
	glVertex3dv(G);
	glEnd();
	//Нижняя грань кон

	//Боковые грани нач

	glBegin(GL_QUADS);
	glNormal3d(0, 1, 0);
	glColor3d(1, 0, 0);
	glVertex3dv(B);
	glVertex3dv(C);
	glVertex3dv(C1);
	glVertex3dv(B1);
	glEnd();

	glBegin(GL_QUADS);
	glNormal3d((20)/sqrt(20*20+10*10), 10/sqrt(20*20+10*10), 0);
	glColor3d(1, 0, 0);
	glVertex3dv(C);
	glVertex3dv(E);
	glVertex3dv(E1);
	glVertex3dv(C1);
	glEnd();

	glBegin(GL_QUADS);
	glNormal3d(0, 1, 0);
	glColor3d(1, 0, 0);
	glVertex3dv(E);
	glVertex3dv(F);
	glVertex3dv(F1);
	glVertex3dv(E1);
	glEnd();

	glBegin(GL_QUADS);
	glNormal3d((20) / sqrt(20 * 20 + 10 * 10), 10 / sqrt(20 * 20 + 10 * 10), 0);
	glColor3d(1, 0, 0);
	glVertex3dv(F);
	glVertex3dv(G);
	glVertex3dv(G1);
	glVertex3dv(F1);
	glEnd();


	glBegin(GL_QUADS);
	glNormal3d((-30) / sqrt(30 * 30 + 15 * 15), -15 / sqrt(30 * 30 + 15 * 15), 0);
	glColor3d(1, 0, 0);
	glVertex3dv(H);
	glVertex3dv(D);
	glVertex3dv(D1);
	glVertex3dv(H1);
	glEnd();

	glBegin(GL_QUADS);
	glNormal3d((5) / sqrt(5 * 5+ 40 * 40), -40 / sqrt(5 * 5 + 40 * 40), 0);
	glColor3d(1, 0, 0);
	glVertex3dv(D);
	glVertex3dv(A);
	glVertex3dv(A1);
	glVertex3dv(D1);
	glEnd();
	//Боковые грани кон



	//Выпуклость нач
	double x, y, m1[1000][3], m2[1000][3];
	int k = 0;
	double r = sqrt(50) / 2;
	for (double i = 225; i <= 405; i += 0.2) {
		x = r * cos(i * 3.14159265 / 180) + 13.5;
		y = r * sin(i * 3.14159265 / 180) - 2.5;

		m1[k][0] = x;
		m1[k][1] = y;
		m1[k][2] = 0;

		m2[k][0] = x;
		m2[k][1] = y;
		m2[k][2] = 5;
		k++;

	}

	for (int i = 0; i < k; i++) {
		double ax1 = m1[i + 1][0] - m1[i][0];
		double ay1 = m1[i + 1][1] - m1[i][1];
		double nx1 = ay1 * 5;
		double ny1 = -ax1 * 5;
		glBegin(GL_QUADS);
		glNormal3d((nx1) / sqrt(nx1 * nx1 + ny1 * ny1), ny1 / sqrt(nx1 * nx1 + ny1 * ny1), 0);
		glColor4f(1.0, 0.8, 0.9, 1.0);
		glVertex3dv(m1[i]);
		glVertex3dv(m1[i + 1]);
		glVertex3dv(m2[i + 1]);
		glVertex3dv(m2[i]);
		glEnd();
	}

	for (int i = 0; i < k; i++) {
		glBegin(GL_TRIANGLES);
		glNormal3d(0, 0, -1);
		glColor3d(1, 0, 0);
		glVertex3dv(m1[i]);
		glVertex3dv(G);
		glVertex3dv(H);
		glEnd();
	}


	//Выпуклость кон

	//Вогнутость нач
	double x1, y1, m3[5000][3], m4[5000][3];
	int k1 = 0;
	double r1 = 6.041522986797286;
	for (double i = 249.44; i <= 429.44; i += 0.1) {
		x1 = r1 * cos(i * 3.14159265 / 180) - 2.5;
		y1 = r1 * sin(i * 3.14159265 / 180) + 5.5;
		if (y1 <= ((8 * x1) / 3)) {
			m3[k1][0] = x1;
			m3[k1][1] = y1;
			m3[k1][2] = 0;

			m4[k1][0] = x1;
			m4[k1][1] = y1;
			m4[k1][2] = 5;
			k1++;
		}
	}

	for (int i = 0; i < k1; i++) {
		double ax = m3[i + 1][0] - m3[i][0];
		double ay = m3[i + 1][1] - m3[i][1];
		double nx = ay * 5;
		double ny = -ax * 5;
		glBegin(GL_QUADS);
		glNormal3d((-nx) / sqrt(nx * nx + ny * ny), -ny / sqrt(nx * nx + ny * ny), 0);
		glColor3d(0, 1, 0);
		glVertex3dv(m3[i]);
		glVertex3dv(m3[i + 1]);
		glVertex3dv(m4[i + 1]);
		glVertex3dv(m4[i]);
		glEnd();
	}

	for (int i = 0; i < k1; i++) {
		glBegin(GL_TRIANGLES);
		glNormal3d(0, 0, -1);
		glColor3d(1, 0, 0);
		glVertex3dv(m3[i]);
		glVertex3dv(m3[i + 1]);
		glVertex3dv(D);
		glEnd();
	}




	//Вогнутость кон
	float xcos, ysin, tx, ty, angle, radian, x11, y11, r11 = 10;
	glBindTexture(GL_TEXTURE_2D, texId);
	glBegin(GL_POLYGON);
	glColor4f(0.7, 0.9, 0.7, 1.0);
	glNormal3d(0, 0, 1);
	for (angle = 0.0; angle < 360.0; angle += 2.0) {
		radian = angle * (3.14159265 / 180.0f);
		xcos = (float)cos(radian);
		ysin = (float)sin(radian);
		x11 = xcos * r11 + 15;
		y11 = ysin * r11 + 25;
		tx = xcos * 0.5 + 0.5;
		ty = ysin * 0.5 + 0.5;
		glTexCoord2f(ty, tx);
		glVertex3f(x11, y11, 2);
	}
	glEnd();
	//Начало рисования квадратика станкина
	double A2[3] = { 3.05, 8.05, 0 };
	double B2[3] = { 3.05, 8.05, 5 };
	double C2[3] = { 8.05, 8.05, 5 };
	double D2[3] = { 8.05, 8.05, 0 };

	glBindTexture(GL_TEXTURE_2D, texId);

	glColor3d(0.6, 0.6, 0.6);
	glBegin(GL_QUADS);

	glNormal3d(0, 1, 0);
	glTexCoord3d(0, 0, 0);
	glVertex3dv(A2);
	glTexCoord3d(1, 0, 0);
	glVertex3dv(B2);
	glTexCoord3d(1, 1, 0);
	glVertex3dv(C2);
	glTexCoord3d(0, 1, 0);
	glVertex3dv(D2);

	glEnd();

	//конец рисования квадратика станкина
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


		//Верхняя грань нач
	//Верхняя грань вогнутости нач
	for (int i = 0; i < k1; i++) {
		glBegin(GL_TRIANGLES);
		glNormal3d(0, 0, 1);
		glColor4d(0.5, 0.4, 0.9, 0.5);
		glVertex3dv(m4[i]);
		glVertex3dv(m4[i + 1]);
		glVertex3dv(D1);
		glEnd();
	}
	//Верхняя грань вогнутости кон

	//Верхняя грань выпуклости нач
	for (int i = 0; i < k; i++) {
		glBegin(GL_TRIANGLES);
		glNormal3d(0, 0, 1);
		glColor4d(0.5, 0.4, 0.9, 0.5);
		glVertex3dv(m2[i]);
		glVertex3dv(G1);
		glVertex3dv(H1);
		glEnd();
	}
	//Верхняя часть выпуклости кон
		//альфаналожение


	glBegin(GL_TRIANGLES);
	glColor4d(0.5, 0.4, 0.9, 0.5);
	glNormal3d(0, 0, 1);
	glVertex3dv(B1);
	glVertex3dv(C1);
	glVertex3dv(D1);
	glEnd();

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, 1);
	glColor4d(0.5, 0.4, 0.9, 0.5);
	glVertex3dv(C1);
	glVertex3dv(D1);
	glVertex3dv(E1);
	glEnd();

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, 1);
	glColor4d(0.5, 0.4, 0.9, 0.5);
	glVertex3dv(E1);
	glVertex3dv(D1);
	glVertex3dv(F1);
	glEnd();

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, 1);
	glColor4d(0.5, 0.4, 0.9, 0.5);
	glVertex3dv(F1);
	glVertex3dv(D1);
	glVertex3dv(H1);
	glEnd();

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, 1);
	glColor4d(0.5, 0.4, 0.9, 0.5);
	glVertex3dv(F1);
	glVertex3dv(H1);
	glVertex3dv(G1);
	glEnd();
	//Верхняя грань кон

   //Сообщение вверху экрана

	
	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	                                //(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}