/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

by Jeffery Myers is marked with CC0 1.0. To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/

*/

#include "raylib.h"
#include "raymath.h"
#include "stdio.h"

#define RENDER_WIDTH 400
#define RENDER_HEIGHT 300

#define WIDTH 800
#define HEIGHT 600

typedef struct Matrix3
{
	float m0, m3, m6; // Matrix first row (3 components)
	float m1, m4, m7; // Matrix second row (3 components)
	float m2, m5, m8; // Matrix third row (3 components)
} Matrix3;

Vector3 Matrix3ByVector3(Matrix3 mat3, Vector3 vec3) {

	Vector3 result = {0};

	result.x = mat3.m0 * vec3.x + mat3.m3 * vec3.y + mat3.m6 * vec3.z;
	result.y = mat3.m1 * vec3.x + mat3.m4 * vec3.y + mat3.m7 * vec3.z;
	result.z = mat3.m2 * vec3.x + mat3.m5 * vec3.y + mat3.m8 * vec3.z;

	return result;
}

Matrix3 Matrix3Multiply(Matrix3 left, Matrix3 right) {

	Matrix3 result = {0};

	result.m0 = left.m0 * right.m0 + left.m3 * right.m1 + left.m6 * right.m2;
	result.m3 = left.m0 * right.m3 + left.m3 * right.m4 + left.m6 * right.m5;
	result.m6 = left.m0 * right.m6 + left.m3 * right.m7 + left.m6 * right.m8;
	
	result.m1 = left.m1 * right.m0 + left.m4 * right.m1 + left.m7 * right.m2;
	result.m4 = left.m1 * right.m3 + left.m4 * right.m4 + left.m7 * right.m5;
	result.m7 = left.m1 * right.m6 + left.m4 * right.m7 + left.m7 * right.m8;
	
	result.m2 = left.m2 * right.m0 + left.m5 * right.m1 + left.m8 * right.m2;
	result.m5 = left.m2 * right.m3 + left.m5 * right.m4 + left.m8 * right.m5;
	result.m8 = left.m2 * right.m6 + left.m5 * right.m7 + left.m8 * right.m8;

	return result;
}


float Matrix3Determinant(Matrix3 mat)
{
	float result = 1.0f;

	// Using Laplace expansion (https://en.wikipedia.org/wiki/Laplace_expansion),
	// size from 3x3 to 2x2 using minors

	// Cache the matrix values (speed optimization)
	float m0 = mat.m0, m1 = mat.m1, m2 = mat.m2;
	float m3 = mat.m3, m4 = mat.m4, m5 = mat.m5;
	float m6 = mat.m6, m7 = mat.m7, m8 = mat.m8;

	result = m0 * (m4 * m8 - m7 * m5) - m3 * (m1 * m8 - m7 * m2) + m6 * (m1 * m5 - m4 * m2);

	return result;
}


Vector3 CramersRule(Matrix3 A, Vector3 constants)
{

	Matrix3 A1 = {
		constants.x, A.m3, A.m6,
		constants.y, A.m4, A.m7,
		constants.z, A.m5, A.m8
	};

	Matrix3 A2 = {
		A.m0, constants.x, A.m6,
		A.m1, constants.y, A.m7,
		A.m2, constants.z, A.m8,
	};

	Matrix3 A3 = {
		A.m0, A.m3, constants.x,
		A.m1, A.m4, constants.y,
		A.m2, A.m5, constants.z
	};

	float detA = Matrix3Determinant(A);
	float detA1 = Matrix3Determinant(A1);
	float detA2 = Matrix3Determinant(A2);
	float detA3 = Matrix3Determinant(A3);

	if (detA == 0)
	{
		return Vector3Zero();
	}

	float t = detA1 / detA;
	float u = detA2 / detA;
	float v = detA3 / detA;

	return (Vector3){t, u, v};
}

bool RayIntersectsPlane(Vector3 v1, Vector3 v2, Vector3 v3, Vector3 ray, Vector3 origin)
{

	Vector3 e1 = Vector3Subtract(v2, v1);
	Vector3 e2 = Vector3Subtract(v3, v1);

	Vector3 planeNormal = Vector3CrossProduct(e1, e2);

	Matrix3 vectorMatrix = {
		ray.x, e1.x, e2.x,
		ray.y, e1.y, e2.y,
		ray.z, e1.z, e2.z,
	};

	Vector3 constants = Vector3Subtract(origin, v1);

	if (Vector3DotProduct(ray, planeNormal) == 0 || (planeNormal.x == 0 && planeNormal.y == 0 && planeNormal.z == 0))
	{
		return false;
	}

	Vector3 unknowns = CramersRule(vectorMatrix, constants);

	float t = unknowns.x;
	float u = unknowns.y;
	float v = unknowns.z;

	if (0 <= u && u <= 1 && 0 <= v && v <= 1 && u + v <= 1 && t > 0)
	{
		return true;
	}
	else
	{
		return false;
	}
	// Vector3 intersectionPoint = Vector3Add(origin, Vector3Scale(ray, t));
}

float theta = PI / 2; // FOV in range [0, pi]
float m = HEIGHT;
float k = WIDTH;
int i, j = 1;
int d = 1;

Vector3 EYE_POSITION = {0, 0, 0};
Vector3 TARGET_POSITION = {0, 0, 2};

Vector3 q_x, q_y, p_1m = {0};

void update() {

	Vector3 v = {0, 1, 0};

	Vector3 t = Vector3Subtract(TARGET_POSITION, EYE_POSITION);
	Vector3 b = Vector3CrossProduct(v, t);

	Vector3 t_normal = Vector3Normalize(t);
	Vector3 b_normal = Vector3Normalize(b);
	Vector3 v_normal = Vector3Normalize(v);

	Vector3 CENTER_VIEWPORT = Vector3Add(EYE_POSITION, Vector3Scale(t_normal, d));

	// Viewport width -> Converting pixels to points in space [g_x/g_y are half the viewport]
	float g_x = d * tan(theta / 2);
	float g_y = g_x * ((m - 1) / (k - 1));
	float h_x = 2 * g_x;
	float h_y = 2 * g_y;

	// next-pixel shifting vectors + bottom left pixel p1m
	q_x = Vector3Scale(b_normal, (h_x / (k - 1)));
	q_y = Vector3Scale(v_normal, (h_y / (m - 1)));
	p_1m = Vector3Subtract(Vector3Subtract(Vector3Scale(t_normal, d), Vector3Scale(b_normal, g_x)), Vector3Scale(v_normal, g_y));
}

float totalYaw = 0.0f;
float totalPitch = 0.0f;

void UpdateTargetPosition() {

	float dx = GetMouseDelta().x;
	float dy = GetMouseDelta().y;

	float sensitivity = 0.001f;

	totalYaw += round(dx) * sensitivity;
	totalPitch += round(dy) * sensitivity;

	if(totalPitch > PI/2 - 0.01f) totalPitch = PI/2 -0.01f;
	if(totalPitch < -PI/2 + 0.0f) totalPitch = -PI/2 +0.01f;


	Vector3 direction = {
		cos(totalPitch) * sin(totalYaw),
		sin(totalPitch),
		cos(totalPitch) * cos(totalYaw)	
	};

	TARGET_POSITION = Vector3Add(EYE_POSITION, direction);
}


int main()
{

	/* OBJECT DEFINITION */

	Vector3 indices[] = {
		{-0.5, 0, 5},
		{0.5, 0, 5},
		{0.5, 0.5, 5}};

	/**/

	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	// Create the window and OpenGL context
	InitWindow(WIDTH, HEIGHT, "Hello Raylib");

	printf("Initializing!\n");
	
	SetTargetFPS(165);
	
	Image canvas = GenImageColor(RENDER_WIDTH, RENDER_HEIGHT, BLACK);
	Texture2D texture = LoadTextureFromImage(canvas);
	Color* pixels = (Color*) canvas.data;

	// game loop
	while (!WindowShouldClose()) // run the loop until the user presses ESCAPE or presses the Close button on the window
	{

		if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
			
			UpdateTargetPosition(EYE_POSITION, TARGET_POSITION);

			SetMousePosition(WIDTH / 2, HEIGHT / 2);
			HideCursor();
			update();

		}

		for (i = 0; i < RENDER_WIDTH; i++)
		{
			for (j = 0; j < RENDER_HEIGHT; j++)
			{

				Vector3 p_ij = Vector3Add(Vector3Add(p_1m, Vector3Scale(q_x, i - 1)), Vector3Scale(q_y, (j - 1))); // Vector from Eye Position to Viewport Intersection Point P_ij or Ray Direcition
				Vector3 r_ij = Vector3Normalize(Vector3Subtract(p_ij, EYE_POSITION));								   // Normalized Ray Direction

				pixels[j * RENDER_WIDTH + i] = RayIntersectsPlane(indices[0], indices[1], indices[2], r_ij, EYE_POSITION) ? WHITE: GRAY;

				// printf("p_ij: %f, %f, %f \n", p_ij.x, p_ij.y, p_ij.z);
				// printf("r_ij: %f, %f, %f \n", r_ij.x, r_ij.y, r_ij.z);
			}
		}
		UpdateTexture(texture, canvas.data);

		// drawing
		BeginDrawing();
			ClearBackground(BLACK);
			DrawTexturePro(
				texture,
				(Rectangle) {0, 0, RENDER_WIDTH, RENDER_HEIGHT},
				(Rectangle) {0, 0, WIDTH, HEIGHT},
				(Vector2) {0, 0},
				0.0f,
				WHITE
			);
			DrawFPS(10, 10);
		EndDrawing();
	}

	UnloadTexture(texture);
	UnloadImage(canvas);

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}
