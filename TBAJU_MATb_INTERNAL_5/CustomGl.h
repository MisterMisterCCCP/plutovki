#pragma once
#include <windows.h>
#include <vector>
#include <gl/GL.h>
#include <cstdio>
#include <iostream>

#pragma comment(lib, "opengl32.lib")

namespace GL {

	struct Vector3
	{
		float x, y, z;
		float distance(const Vector3& other);
	};

	struct Vector4 
	{
		float x, y, z, w;
	};
	void SetupOrtho();
	void RestoreGl();
	void DrawFilledRect(float x, float y, float width, float height, const GLubyte color[3]);
	void DrawOutLine(float x, float y, float widht, float hight, float lineWidht, const GLubyte color[3]);
	void DrawLine(float fromX, float fromY, float toX, float toY, float lineWidht, const GLubyte color[3]);
	void DrawESPBox(float posX, float posY, float distance, const GLubyte color[3], const int health);

	bool WorldToScreen(Vector3 pos, Vector3& screen, float matirx[16]);

}