#include "pch.h"
#include "CustomGl.h"


void GL::SetupOrtho()
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushMatrix();
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glViewport(0, 0, viewport[2], viewport[3]);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, viewport[2], viewport[3], 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
}
void GL::RestoreGl()
{
    glPopMatrix();
    glPopAttrib();
}

void GL::DrawFilledRect(float x, float y, float width, float height, const GLubyte color[3])
{
    glColor3ub(color[0], color[1], color[2]);
    glBegin(GL_QUADS);
    glVertex2f(x, y); // Top Left
    glVertex2f(x + width, y); // Top right
    glVertex2f(x + width, y + height); // Bottom right
    glVertex2f(x, y + height); // Bottom left
    glEnd();
}
void GL::DrawOutLine(float x, float y, float width, float height, float lineWidth, const GLubyte color[3])
{
    glLineWidth(lineWidth);  // Set line thickness
    glBegin(GL_LINE_STRIP);  // Start drawing a connected line strip
    glColor3ub(color[0], color[1], color[2]);  // Set line color

    // Define rectangle outline (slightly offset by -0.5 and +0.5)
    glVertex2f(x - 0.5f, y - 0.5f);            // Top-left
    glVertex2f(x + width + 0.5f, y - 0.5f);    // Top-right
    glVertex2f(x + width + 0.5f, y + height + 0.5f);  // Bottom-right
    glVertex2f(x - 0.5f, y + height + 0.5f);   // Bottom-left
    glVertex2f(x - 0.5f, y - 0.5f);            // Back to top-left (closing the shape)

    glEnd();  // Finish drawing
}
void GL::DrawLine(float fromX, float fromY, float toX, float toY, float lineWidth, const GLubyte color[3])
{
    glLineWidth(lineWidth);
    glBegin(GL_LINES);
    glColor3ub(color[0], color[1], color[2]);
    glVertex2f(fromX, fromY);
    glVertex2f(toX, toY);
    glEnd();
}
void GL::DrawESPBox(float posX, float posY, float distance, const GLubyte color[3], const int health)
{
    float lineWidth = 0.5f;
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    float height = (viewport[3] / distance) * 4;
    float width = (viewport[2] / distance) * 2.5;
    GL::DrawOutLine(posX - (width / 2), posY - height, width, height, lineWidth, color);
    GL::DrawLine(viewport[2] / 2, viewport[3], posX, posY, lineWidth, color);
    if (health != -1)
    {
        float perc = (width / 100);
        float curr = perc * health;

        GLubyte Hcolor[3] = { 255, 123, 99 };
        GL::DrawFilledRect(posX - (width / 2), posY - (height / 10), curr, height / 10, Hcolor);
    }
}

float GL::Vector3::distance(const Vector3& other)
{
    return sqrt(
        (this->x - other.x) * (this->x - other.x) //x^2
        + (this->y - other.y) * (this->y - other.y) //y^2
        + (this->z - other.z)* (this->z- other.z) //z^2
    );
}

bool GL::WorldToScreen(Vector3 pos, Vector3& screen, float matrix[16])
{
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
   int windowWidth = viewport[2];
   int windowHeight = viewport[3];

    #ifdef _DEBUG
   std::cout << "Matrix 4x4:\n";
   std::cout << matrix[0] << " " << matrix[1] << " " << matrix[2] << " " << matrix[3] << "\n";
   std::cout << matrix[4] << " " << matrix[5] << " " << matrix[6] << " " << matrix[7] << "\n";
   std::cout << matrix[8] << " " << matrix[9] << " " << matrix[10] << " " << matrix[11] << "\n";
   std::cout << matrix[12] << " " << matrix[13] << " " << matrix[14] << " " << matrix[15] << "\n";
   #endif
    #ifdef _DEBUG
   std::cout << "Matrix 4x4 Adresses:\n";
   std::cout << &matrix[0] << " " << &matrix[1] << " " << &matrix[2] << " " << &matrix[3] << "\n";
   std::cout << &matrix[4] << " " << &matrix[5] << " " << &matrix[6] << " " << &matrix[7] << "\n";
   std::cout << &matrix[8] << " " << &matrix[9] << " " << &matrix[10] << " " << &matrix[11] << "\n";
   std::cout << &matrix[12] << " " << &matrix[13] << " " << &matrix[14] << " " << &matrix[15] << "\n";
    #endif


   Vector4 clipBoards;
   clipBoards.x = pos.x * matrix[0] + pos.y * matrix[4] + pos.z * matrix[8] + matrix[12];
   clipBoards.y = pos.x * matrix[1] + pos.y * matrix[5] + pos.z * matrix[9] + matrix[13];
   clipBoards.z = pos.x * matrix[2] + pos.y * matrix[6] + pos.z * matrix[10] + matrix[14];
   clipBoards.w = pos.x * matrix[3] + pos.y * matrix[7] + pos.z * matrix[11] + matrix[15];

   if (clipBoards.w < 0.1f) return false;

   Vector3 NDC;
   NDC.x = clipBoards.x / clipBoards.w;
   NDC.y = clipBoards.y / clipBoards.w;
   NDC.z = clipBoards.z / clipBoards.w;



   screen.x = (windowWidth / 2 * NDC.x) + (windowWidth / 2);
   screen.y = (windowHeight / 2 * -NDC.y) + (windowHeight / 2);

   //screen.x = (windowWidth / 2) * (NDC.x + 1);
   //screen.y = (windowHeight / 2) * (1 - NDC.y);
   //screen.x = (windowWidth / 2 * NDC.x) + (windowWidth + NDC.x / 2);
   //screen.y = -(windowHeight / 2 * NDC.y) + (windowHeight + NDC.y / 2);
   return true;
}
