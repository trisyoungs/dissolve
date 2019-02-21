/*
	*** Rendering Primitive
	*** src/gui/viewer/render/primitive.h
	Copyright T. Youngs 2013-2019

	This file is part of Dissolve.

	Dissolve is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Dissolve is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Dissolve.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DISSOLVE_PRIMITIVE_H
#define DISSOLVE_PRIMITIVE_H

#include "gui/viewer/render/primitiveinstance.h"
#include "math/matrix4.h"
#include "templates/list.h"
#include "templates/array.h"
#include <QOpenGLFunctions>

// Forward Declarations
/* none */

// Rendering Primitive
class Primitive : public ListItem<Primitive>
{
	public:
	// Constructor / Destructor
	Primitive();
	~Primitive();


	/*
	 * Data
	 */
	private:
	// Vertex data array
	Array<GLfloat> vertexData_;
	// Number of vertices defined in vertexData_
	int nDefinedVertices_;
	// Index data array
	Array<GLuint> indexData_;
	// GL primitive type (GL_TRIANGLES, GL_LINES etc.)
	GLenum type_;
	// Number of vertices per primitive type
	int verticesPerType_;
	// Number of data points per vertex
	int dataPerVertex_;
	// Whether vertex data array also contains colour information
	bool colouredVertexData_;

	public:
	// Initialise primitive storage
	void initialise(GLenum type, bool colourData);
	// Forget all data, leaving arrays intact
	void forgetAll();
	// Return number of vertices currently defined in primitive
	int nDefinedVertices() const;
	// Return number of indices currently defined in primitive
	int nDefinedIndices() const;
	// Return whether vertex data contains colour information
	bool colouredVertexData() const;


	/*
	 * Instances
	 */
	private:
	// Stack of OpenGL VBO or display list IDs and the contexts in which they were created
	List<PrimitiveInstance> instances_;
	// Flag stating whether or not instances should be used for this primitive
	bool useInstances_;

	public:
	// Flag that this primitive should not use instances (rendering will use vertex arrays)
	void setNoInstances();
	// Push instance layer from current vertex chunk list
	void pushInstance(const QOpenGLContext* context);
	// Pop topmost instance layer
	void popInstance(const QOpenGLContext* context);
	// Return number of instances available
	int nInstances();
	// Send to OpenGL (i.e. render)
	void sendToGL();


	/*
	 * Vertex / Index Generation
	 */
	public:
	// Define next vertex and normal
	GLuint defineVertex(GLfloat x, GLfloat y, GLfloat z, GLfloat nx, GLfloat ny, GLfloat nz);
	// Define next vertex and normal (as Vec3<double>)
	GLuint defineVertex(Vec3<double> vertex, Vec3<double> normal);
	// Define next vertex, normal, and colour
	GLuint defineVertex(GLfloat x, GLfloat y, GLfloat z, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat r, GLfloat g, GLfloat b, GLfloat a);
	// Define next vertex and normal
	GLuint defineVertex(GLfloat x, GLfloat y, GLfloat z, GLfloat nx, GLfloat ny, GLfloat nz, Vec4<GLfloat>& colour);
	// Define next vertex, normal, and colour
	GLuint defineVertex(GLfloat x, GLfloat y, GLfloat z, Vec3<double>& normal, Vec4<GLfloat>& colour);
	// Define next vertex, normal, and colour (as Vec3<double>s and array)
	GLuint defineVertex(Vec3<double>& v, Vec3<double>& u, Vec4<GLfloat>& colour);
	// Define next index double
	void defineIndices(GLuint a, GLuint b);
	// Define next index triple
	void defineIndices(GLuint a, GLuint b, GLuint c);


	/*
	 * Geometric Primitive Generation
	 */
	public:
	// Draw line
	void line(double x1, double y1, double z1, double x2, double y2, double z2);
	// Draw line
	void line(Vec3<double> v1, Vec3<double> v2);
	// Create vertices of sphere with specified radius and quality
	void sphere(double radius, int nstacks, int nslices);
	// Plot cylinder vertices from origin {ox,oy,oz}, following vector {vx,vy,vz}, with radii and quality specified
	void cylinder(GLfloat ox, GLfloat oy, GLfloat oz, GLfloat vx, GLfloat vy, GLfloat vz, double startRadius, double endRadius, int nStacks, int nSlices, bool capStart = false, bool capEnd = false);
	// Plot tube ring of specified radius and tube width
	void ring(double radius, double width, int nStacks, int nSlices, int nSegments, bool segmented);
	// Plot circle of specified radius
	void circle(double radius, int nStacks, int nSegments, bool segmented);
	// Create vertices of cross with specified width
	void cross(double halfWidth, Matrix4& transform, GLfloat colour[4]);
	// Plot solid cube of specified size at specified origin, and with sides subdivided into triangles ( ntriangles = 2*nSubs )
	void cube(double size, int nSubs, double ox, double oy, double oz);
	// Plot solid orthorhomboid of specified size at specified origin, and with sides subdivided into triangles ( ntriangles = 2*nSubs )
	void orthorhomboid(double sizex, double sizey, double sizez, int nSubs, double ox, double oy, double oz);
};

#endif
