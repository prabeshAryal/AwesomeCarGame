#include <iostream>
#include <Windows.h>
#include <GL\glew.h>
#include <GL\freeglut.h>


// Function to render the triangle
void render() {
    glClear(GL_COLOR_BUFFER_BIT); // Clear the screen

    glBegin(GL_TRIANGLES);       // Start drawing a triangle
    glColor3f(1.0f, 0.0f, 0.0f); // Red
    glVertex2f(-0.5f, -0.5f);    // Bottom left
    glColor3f(0.0f, 1.0f, 0.0f); // Green
    glVertex2f(0.5f, -0.5f);     // Bottom right
    glColor3f(0.0f, 0.0f, 1.0f); // Blue
    glVertex2f(0.0f, 0.5f);      // Top center
    glEnd();

    glFlush(); // Render the result
}

// Function to initialize OpenGL
void initialize() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set a black background
}

// Main function
int main(int argc, char** argv) {
    glutInit(&argc, argv);                  // Initialize GLUT
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB); // Set display mode
    glutInitWindowSize(800, 600);           // Set window size
    glutCreateWindow("Simple OpenGL Example"); // Create window
    glewInit();                             // Initialize GLEW

    initialize();                           // Call initialization function

    glutDisplayFunc(render);                // Set rendering function
    glutMainLoop();                         // Start the main loop

    return 0;
}