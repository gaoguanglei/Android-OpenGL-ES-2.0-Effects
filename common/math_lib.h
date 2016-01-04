/************************************************************************************************************
 * Added by gaoguanglei 2012-3-28
 ************************************************************************************************************/

/*
author:gaoguanglei
*/

#ifndef MATHLIB_H
#define MATHLIB_H

#include <math.h>

struct vec3;
struct mat4;
struct quat;

/*---------------------------------------------------------------------------*/
/*                                                                           */
/* vec3                                                                      */
/*                                                                           */
/*---------------------------------------------------------------------------*/
struct vec3 
{
	vec3() : x(0), y(0), z(0) { }
	vec3(float x,float y,float z) : x(x), y(y), z(z) { }
	vec3(const float *v) : x(v[0]), y(v[1]), z(v[2]) { }
	vec3(const vec3 &v) : x(v.x), y(v.y), z(v.z) { }
	
	void set(float tx,float ty,float tz){x=tx;y=ty;z=tz;}
	void set(const vec3 &v){x=v.x;y=v.y;z=v.z;}
	void set(const float*v){x=v[0];y=v[1];z=v[2];}
	void zero(){x=0;y=0;z=0;};

	 vec3  operator*(float f) const { return vec3(x * f,y * f,z * f); }
	 vec3  operator/(float f) const { return vec3(x / f,y / f,z / f); }
	 vec3  operator+(const vec3 &v) const { return vec3(x + v.x,y + v.y,z + v.z); }
	 vec3  operator-() const { return vec3(-x,-y,-z); }
	 vec3  operator-(const vec3 &v) const { return vec3(x - v.x,y - v.y,z - v.z); }	
	 vec3 &operator*=(float f) { return *this = *this * f; }
	 vec3 &operator/=(float f) { return *this = *this / f; }
	 vec3 &operator+=(const vec3 &v) { return *this = *this + v; }
	 vec3 &operator-=(const vec3 &v) { return *this = *this - v; }	

	float &operator[](int i) { return ((float*)&x)[i]; }
	const float operator[](int i) const { return ((float*)&x)[i]; }
	
	float length() const { return (float)sqrt(x * x + y * y + z * z); }
	vec3 normalize() {
		float inv,length = (float)sqrt(x * x + y * y + z * z);
		inv = 1.0f / length;
		x *= inv;
		y *= inv;
		z *= inv;
		return *this;
	}
	union {
		struct {
			float x,y,z;
		};
		float v[3];
	};
};
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* mat4                                                                     */
/*                                                                           */
/*---------------------------------------------------------------------------*/

struct mat4
{
	mat4() ;
	mat4(const float *m);
	mat4(const mat4 &m) ;
	mat4(float m0, float m4, float m8, float m12,
		 float m1, float m5, float m9, float m13,
		 float m2, float m6, float m10,float m14,
		 float m3, float m7, float m11,float m15);
	
	vec3 operator*(const vec3 &v) const ;
	vec3 mult_normal(const vec3 &v);
	mat4 operator*(float f) const ;
	mat4 operator*(const mat4 &m) const;
	mat4 operator+(const mat4 &m) const ;
	mat4 operator-(const mat4 &m) const ;

	mat4& operator*=(float f)     
	{
		return *this = *this * f;
	}

	mat4& operator*=(const mat4 &m)
	{ 
		return *this = *this * m; 
	}

	mat4& operator+=(const mat4 &m) 
	{ 
		return *this = *this + m; 
	}

	mat4& operator-=(const mat4 &m) 
	{ 
		return *this = *this - m; 
	}
	
	float& operator[](int i)            
	{ 
		return mat[i];
	}

	const float operator[](int i) const 
	{
		return mat[i]; 
	}

	mat4 inverse() const ;
	void zero() ;
	void identity();

	void rotate(float angle,const vec3 &axis);
	void rotate(float angle,float x,float y,float z) ;
	void rotate_x(float angle) ;
	void rotate_y(float angle) ;
	void rotate_z(float angle) ;

	void scale(const vec3 &v) ;
	void scale(float x,float y,float z) ;

	void translate(const vec3 &v) ;
	void translate(float x,float y,float z);

	void ortho(float l, float r, float t, float b, float n, float f);
	void perspective(float fov,float aspect,float znear,float zfar) ;
	void look_at(const vec3 &eye,const vec3 &dir,const vec3 &up) ;

	void look_at(const float *eye,const float *dir,const float *up);

	float mat[16];
};

/*---------------------------------------------------------------------------*/
/*                                                                           */
/* quat                                                                      */
/*                                                                           */
/*---------------------------------------------------------------------------*/

struct quat
{
	quat();
	quat(float tx,float ty,float tz,float tw);
	quat(float tx,float ty,float tz);

	quat operator +(const quat &q)const;
	quat operator -(const quat &q)const ;
	quat operator*(const quat &q) const ;
	quat operator*(const vec3 &q) const ;
	quat operator *(const float &tf)const;
	quat operator /(const float & tf)const;

	quat & operator+=(const quat &tq);
	quat & operator-=(const quat &tq);
	quat & operator *=(const quat &q);
	quat &operator /=(const float &tf);

	float &operator[](int i) ;
	const float operator[](int i) const;

	void computer_w();
	void normalize();

	void set(quat &tq);
	void set(float tx,float ty,float tz,float tw);
	void set(float tx,float ty,float tz);

	quat inverse();
	void zero();
	void set(const vec3 &v);

	void set_rotation(const vec3 &dir,float angle);
	vec3 inverse_rotate(const vec3& p);
	vec3 rotate(const vec3& p);
	void set_rotation(float x,float y,float z,float angle);

	void slerp(const quat &q0,const quat &q1,float t) ;

	mat4 to_matrix() const ;
	mat4 to_matrix(const vec3 &pos) const ;

	union
	{
		struct 
		{
			float x,y,z,w;
		};
		float q[4];
	};
};
/*----------------------------------------------------------------------------------*/

float dot(const vec3& v1,const vec3& v2);

vec3 cross(const vec3& v1,const vec3& v2);

vec3 computer_normal(const vec3& v1,const vec3& v2,const vec3& v3);

 
/*----------------------------------------------------------------------------------*/

#endif 
