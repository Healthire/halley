/*****************************************************************\
           __
          / /
		 / /                     __  __
		/ /______    _______    / / / / ________   __       __
	   / ______  \  /_____  \  / / / / / _____  | / /      / /
	  / /      | / _______| / / / / / / /____/ / / /      / /
	 / /      / / / _____  / / / / / / _______/ / /      / /
	/ /      / / / /____/ / / / / / / |______  / |______/ /
   /_/      /_/ |________/ / / / /  \_______/  \_______  /
                          /_/ /_/                     / /
			                                         / /
		       High Level Game Framework            /_/

  ---------------------------------------------------------------

  Copyright (c) 2007-2011 - Rodrigo Braz Monteiro.
  This file is subject to the terms of halley_license.txt.

\*****************************************************************/


#include "polygon.h"
#include <float.h>
using namespace Halley;


///////////////
// Constructor
Polygon::Polygon()
{
	outerRadius = 0.0f;
}

Polygon::Polygon(const VertexList& _vertices, Vertex _origin)
	: vertices(_vertices), origin(_origin)
{
	realize();
}


//////////////////////////////////////////////
// Realize that the polygon has changed shape
void Polygon::realize()
{
	// Calculate the outer radius and aabb
	float x1 = FLT_MAX;
	float x2 = -FLT_MAX;
	float y1 = FLT_MAX;
	float y2 = -FLT_MAX;

	outerRadius = 0.0f;
	size_t len = vertices.size();
	float cur;
	for (size_t i=0;i<len;i++) {
		Vector2f v = vertices[i];
		if (v.x < x1) x1 = v.x;
		if (v.x > x2) x2 = v.x;
		if (v.y < y1) y1 = v.y;
		if (v.y > y2) y2 = v.y;
		cur = v.squaredLength();
		if (cur > outerRadius) outerRadius = cur;
	}
	aabb.set(Vector2f(x1, y1), Vector2f(x2, y2));
	outerRadius = sqrt(outerRadius);
}


///////////////////////////////////////////////////////
// Checks if a particular point is inside the polygon
bool Polygon::isPointInside(const Vector2f &point) const
{
	Vector2f p = point - origin;

	// Fast fail
	if (p.squaredLength() > outerRadius*outerRadius) return false;

	// Do cross product with all the segments, except for last
	Vector2f a,b;
	size_t len = vertices.size();
	for (size_t i=0; i<len-1; i++) {
		a = p - vertices[i];
		b = vertices[i+1] - vertices[i];
		if (a.cross(b) > 0) return false;
	}

	// Try last segment (duplicated code just to avoid a %, fuck yeah)
	len--;
	a = p - vertices[len];
	b = vertices[0] - vertices[len];
	if (a.cross(b) > 0) return false;

	// Nothing failed, so it's inside
	return true;
}


///////////////////////////////////////////////////////
// Checks if any of the points are inside the polygon
// Using the separating axis theorem here
Vector2f average(std::vector<Vector2f>& v)
{
	Vector2f result;
	size_t len = v.size();
	for (size_t i=0; i<len; i++) {
		result += v[i];
	}
	result /= (float)len;
	return result;
}

bool Polygon::overlaps(const Polygon &param,Vector2f *translation,Vector2f *collisionPoint) const
{
	// Check if they are within overlap range
	float maxDist = outerRadius + param.outerRadius;
	if ((origin-param.origin).squaredLength() >= maxDist*maxDist) return false;

	// AABB test
	//if (!aabb.overlaps(param.aabb, param.getOrigin()-getOrigin())) return false;

	// Prepare
	float minDist = -999999.0f;
	Vector2f bestAxis;
	bool hasBestAxis = false;
	float bmin1=0, bmax1=0, bmin2=0, bmax2=0;

	// For each edge
	size_t len1 = vertices.size();
	size_t len2 = param.vertices.size();
	for (size_t i=0; i<len1+len2; i++) {
		// Find the orthonormal axis
		Vector2f axis;
		if (i < len1) axis = (vertices[(i+1)%len1] - vertices[i]).orthoLeft().unit();
		else axis = (param.vertices[(i-len1+1)%len2] - param.vertices[i-len1]).orthoLeft().unit();

		// Project both polygons there
		float min1, max1, min2, max2;
		project(axis,min1,max1);
		param.project(axis,min2,max2);

		// Find the distance between the projections
		float dist = min1<min2 ? min2 - max1 : min1 - max2;
		if (dist >= 0) {
			// This axis separates them
			return false;
		} else {
			if (translation && dist > minDist) {
				bestAxis = axis;
				hasBestAxis = true;
				minDist = dist;
				bmin1 = min1;
				bmin2 = min2;
				bmax1 = max1;
				bmax2 = max2;
			}
		}
	}

	// Gather additional data based on best axis
	if (hasBestAxis) {
		// Find all vertices possibly involved in the collision
		float dist;
		int sign;
		std::vector<Vector2f> v1,v2;
		if (bmin1 < bmin2) {
			dist = bmin2 - bmax1;
			sign = 1;
			if (collisionPoint) {
				unproject(bestAxis,bmax1,v1);
				param.unproject(bestAxis,bmin2,v2);
			}
		}
		else {
			dist = bmin1 - bmax2;
			sign = -1;
			if (collisionPoint) {
				unproject(bestAxis,bmin1,v1);
				param.unproject(bestAxis,bmax2,v2);
			}
		}

		// Find the collision point
		if (collisionPoint) {
			Vector2f colPoint = (origin + param.origin)/2.0f;
			if (v1.size() == 1) colPoint = v1[0];
			else if (v2.size() == 1) colPoint = v2[0];
			else if (!v1.empty()) colPoint = average(v1); //v1[0];
			else if (!v2.empty()) colPoint = average(v2); //v2[0];
			*collisionPoint = colPoint;
		}

		// Find the translation vector
		*translation = bestAxis*(dist*sign);
	}

	// Done
	return true;
}


////////////////////////////////
// Project polygon into an axis
void Polygon::project(const Vector2f &_axis,float &_min,float &_max) const
{
	Vector2f axis = _axis;
	float dot = axis.dot(vertices[0]+origin);
	float min = dot;
	float max = dot;
	size_t len = vertices.size();
	for (size_t i=1;i<len;i++) {
		dot = axis.dot(vertices[i]+origin);
		if (dot < min) min = dot;
		else if (dot > max) max = dot;
	}
	_min = min;
	_max = max;
}


/////////////
// Unproject
// Finds all vertices whose projection on a given axis is the value given
void Polygon::unproject(const Vector2f &axis,const float point,std::vector<Vector2f> &ver) const
{
	size_t len = vertices.size();
	float dot;
	for (size_t i=0;i<len;i++) {
		dot = axis.dot(vertices[i]+origin);
		if (dot == point) ver.push_back(vertices[i] + origin);
	}
}


//////////
// Rotate
// Return this polygon rotated by angle
void Polygon::rotate(Angle<float> angle)
{
	size_t len = vertices.size();
	for (size_t i=0;i<len;i++) {
		vertices[i] = vertices[i].rotate(angle);
	}
	realize();
}


void Halley::Polygon::rotateAndScale(Angle<float> angle, Vector2f scale)
{
	size_t len = vertices.size();
	for (size_t i=0;i<len;i++) {
		vertices[i] = (vertices[i] * scale).rotate(angle);
	}
	realize();
}


Halley::Polygon Polygon::makePolygon(Vector2f origin, float w, float h)
{
	const float x = origin.x;
	const float y = origin.y;
	VertexList list;
	list.push_back(Vertex(x, y));
	list.push_back(Vertex(x+w, y));
	list.push_back(Vertex(x+w, y+h));
	list.push_back(Vertex(x, y+h));
	return Halley::Polygon(list);
}


bool Polygon::isClockwise() const
{
	if (vertices.size() < 3) return true;

	return ((vertices[1]-vertices[0]).cross(vertices[2]-vertices[1]) > 0);
}

void Polygon::setVertices(const VertexList& _vertices)
{
	vertices = _vertices;
	realize();
}

float Polygon::getRadius() const
{
	return outerRadius;
}