/****************************************************************************
* VCGLib                                                            o o     *
* Visual and Computer Graphics Library                            o     o   *
*                                                                _   O  _   *
* Copyright(C) 2004                                                \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *   
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
*                                                                           *
****************************************************************************/
/****************************************************************************
  History

$Log: not supported by cvs2svn $
Revision 1.6  2004/05/12 20:55:18  ponchio
*** empty log message ***

Revision 1.5  2004/05/07 12:46:08  cignoni
Restructured and adapted in a better way to opengl

Revision 1.4  2004/04/07 10:54:11  cignoni
Commented out unused parameter names and other minor warning related issues

Revision 1.3  2004/03/31 15:07:37  ponchio
CAMERA_H -> VCG_CAMERA_H

Revision 1.2  2004/03/25 14:55:25  ponchio
Adding copyright.


****************************************************************************/

#ifndef VCG_CAMERA_H
#define VCG_CAMERA_H

/**********************
WARNING
Everything here assumes the opengl window coord system.
the 0,0 is bottom left
y is upward!
**********************/

#include <vcg/space/point3.h>
#include <vcg/space/plane3.h>
#include <vcg/space/line3.h>
#include <vcg/math/matrix44.h>
#include <GL/glew.h>

namespace vcg {
/**
This class represent the viewing parameters under opengl. 
Mainly it stores the projection and modelview matrix and the viewport
and it is used to simply project back and forth points, computing line of sight, planes etc.
*/

template <class T> class View {
public:
  void GetView();
  void SetView();                      
  Point3<T> Project(const Point3<T> &p) const;
  Point3<T> UnProject(const Point3<T> &p) const;
  Point3<T> ViewPoint() const;

  /// Return the plane perpendicular to the view axis and passing through point P.
  Plane3<T> ViewPlaneFromModel(const Point3<T> &p);

  /// Return the line of view passing through point P.
  Line3<T> ViewLineFromModel(const Point3<T> &p);
  
  /// Return the line passing through the point p and the observer.
  Line3<T> ViewLineFromWindow(const Point3<T> &p);
  
  /// Convert coordinates from the range -1..1 of Normalized Device Coords to range 0 viewport[2]
  Point3<T> NormDevCoordToWindowCoord(const Point3<T> &p) const;
  
  /// Convert coordinates from 0--viewport[2] to the range -1..1 of Normalized Device Coords to
  Point3<T> WindowCoordToNormDevCoord(const Point3<T> &p) const;

  Matrix44<T> proj;
  Matrix44<T> model;
  Matrix44<T> matrix;
  Matrix44<T> inverse;  
  int viewport[4];    
};

template <class T> void View<T>::GetView() {
  double m[16];
  glGetDoublev(GL_TRANSPOSE_PROJECTION_MATRIX_ARB, m);
  proj.Import(Matrix44d(m));
	glGetDoublev(GL_TRANSPOSE_MODELVIEW_MATRIX_ARB, m);
  model.Import(Matrix44d(m));
	
	glGetIntegerv(GL_VIEWPORT, viewport);
  
  matrix = proj*model;
  inverse = matrix;
  Invert(inverse);
}

template <class T> void View<T>::SetView() {
  
}

template <class T> Point3<T> View<T>::ViewPoint() const {
  Matrix44<T> mi=model;
  Invert(mi);
  return mi* Point3<T>(0, 0, 0);
}
// Note that p it is assumed to be in model coordinate.
template <class T> Plane3<T> View<T>::ViewPlaneFromModel(const Point3<T> &p)
{
  Point3<T> vp=ViewPoint();
  Plane3<T> pl;  // plane perpedicular to view direction and passing through manip center
  pl.n=(vp-p);
  pl.d=pl.n*p;
  return pl;
}

// Note that p it is assumed to be in model coordinate.
template <class T> Line3<T> View<T>::ViewLineFromModel(const Point3<T> &p)
{
  Point3<T> vp=ViewPoint();
  Line3<T> line;
  line.SetOrigin(vp);
  line.SetDirection(p - vp);
  return line;
}

// Note that p it is assumed to be in window coordinate.
template <class T> Line3<T> View<T>::ViewLineFromWindow(const Point3<T> &p)
{
  Point3<T> vp=ViewPoint();
	Line3<T> ln;  // plane perpedicular to view direction and passing through manip center
  /*Matrix44<T> mi=model;
  Invert(mi);
  */Point3f pp=UnProject(p);
	ln.SetOrigin(vp);
	ln.SetDirection(pp-vp);
  return ln;
}

template <class T> Point3<T> View<T>::Project(const Point3<T> &p) const {
  Point3<T> r;
  r = matrix * p;  	
  return NormDevCoordToWindowCoord(r);
 }

template <class T> Point3<T> View<T>::UnProject(const Point3<T> &p) const {
  Point3<T> s = WindowCoordToNormDevCoord(p);
  s =  inverse * s ;
  return s;	    
}

// Come spiegato nelle glspec
// dopo la perspective division le coordinate sono dette normalized device coords ( NDC ). 
// Per passare alle window coords si deve fare la viewport transformation.
// Le coordinate di viewport stanno tra -1 e 1

template <class T> Point3<T> View<T>::NormDevCoordToWindowCoord(const Point3<T> &p) const {
  Point3<T> a;
  a[0] = (p[0]+1)*(viewport[2]/(T)2.0)+viewport[0];
	a[1] = (p[1]+1)*(viewport[3]/(T)2.0)+viewport[1];
  //a[1] = viewport[3] - a[1];
  a[2] = p[2];
  return a;
}


template <class T> Point3<T> View<T>::WindowCoordToNormDevCoord(const Point3<T> &p) const {
  Point3<T> a;
  a[0] = (p[0]- viewport[0])/ (viewport[2]/(T)2.0) - 1;
	a[1] = (p[1]- viewport[1])/ (viewport[3]/(T)2.0) - 1;
  //a[1] = -a[1];
	a[2] = p[2];
  return a;
}


}//namespace

#endif
