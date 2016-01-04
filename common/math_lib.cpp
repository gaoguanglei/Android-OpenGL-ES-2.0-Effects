

#include <math_lib.h>



//#define EPSILON 1e-6f
#define PI 3.14159265358979323846f
#define DEG2RAD (PI / 180.0f)
#define RAD2DEG (180.0f / PI)

/*-------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------*/
	mat4::mat4()
	{
		mat[0] = 1.0; mat[4] = 0.0; mat[8] = 0.0; mat[12] = 0.0;
		mat[1] = 0.0; mat[5] = 1.0; mat[9] = 0.0; mat[13] = 0.0;
		mat[2] = 0.0; mat[6] = 0.0; mat[10] = 1.0; mat[14] = 0.0;
		mat[3] = 0.0; mat[7] = 0.0; mat[11] = 0.0; mat[15] = 1.0;
	}

	mat4::mat4(const float *m) 
	{
		mat[0] = m[0]; mat[4] = m[4]; mat[8] = m[8]; mat[12] = m[12];
		mat[1] = m[1]; mat[5] = m[5]; mat[9] = m[9]; mat[13] = m[13];
		mat[2] = m[2]; mat[6] = m[6]; mat[10] = m[10]; mat[14] = m[14];
		mat[3] = m[3]; mat[7] = m[7]; mat[11] = m[11]; mat[15] = m[15];
	}

	mat4::mat4(const mat4 &m) 
	{
		mat[0] = m[0]; mat[4] = m[4]; mat[8] = m[8]; mat[12] = m[12];
		mat[1] = m[1]; mat[5] = m[5]; mat[9] = m[9]; mat[13] = m[13];
		mat[2] = m[2]; mat[6] = m[6]; mat[10] = m[10]; mat[14] = m[14];
		mat[3] = m[3]; mat[7] = m[7]; mat[11] = m[11]; mat[15] = m[15];
	}	

	mat4::mat4(float m0, float m4, float m8, float m12,
			   float m1, float m5, float m9, float m13,
		       float m2, float m6, float m10,float m14,
		       float m3, float m7, float m11,float m15)
	{
		mat[0] = m0;  mat[4] = m4;  mat[8] = m8;  mat[12] = m12;
		mat[1] = m1;  mat[5] = m5;  mat[9] = m9;  mat[13] = m13;
		mat[2] = m2;  mat[6] = m6;  mat[10] = m10;mat[14] = m14;
		mat[3] = m3;  mat[7] = m7;  mat[11] = m11;mat[15] = m15;
	}

	vec3 mat4::operator*(const vec3 &v) const 
	{
		vec3 ret;
		ret[0] = mat[0] * v[0] + mat[4] * v[1] + mat[8] * v[2] + mat[12];
		ret[1] = mat[1] * v[0] + mat[5] * v[1] + mat[9] * v[2] + mat[13];
		ret[2] = mat[2] * v[0] + mat[6] * v[1] + mat[10] * v[2] + mat[14];
		return ret;
	}

	vec3 mat4::mult_normal(const vec3 &v)
	{
		vec3 ret;
		ret[0] = mat[0] * v[0] + mat[4] * v[1] + mat[8] * v[2];
		ret[1] = mat[1] * v[0] + mat[5] * v[1] + mat[9] * v[2];
		ret[2] = mat[2] * v[0] + mat[6] * v[1] + mat[10] * v[2];
		return ret;
	}

	mat4 mat4::operator*(float f) const 
	{
		mat4 ret;
		ret[0] = mat[0] * f; ret[4] = mat[4] * f; ret[8] = mat[8] * f; ret[12] = mat[12] * f;
		ret[1] = mat[1] * f; ret[5] = mat[5] * f; ret[9] = mat[9] * f; ret[13] = mat[13] * f;
		ret[2] = mat[2] * f; ret[6] = mat[6] * f; ret[10] = mat[10] * f; ret[14] = mat[14] * f;
		ret[3] = mat[3] * f; ret[7] = mat[7] * f; ret[11] = mat[11] * f; ret[15] = mat[15] * f;
		return ret;
	}

	mat4 mat4::operator*(const mat4 &m) const
	{
		mat4 ret;
		ret[0] = mat[0] * m[0] + mat[4] * m[1] + mat[8] * m[2] + mat[12] * m[3];
		ret[1] = mat[1] * m[0] + mat[5] * m[1] + mat[9] * m[2] + mat[13] * m[3];
		ret[2] = mat[2] * m[0] + mat[6] * m[1] + mat[10] * m[2] + mat[14] * m[3];
		ret[3] = mat[3] * m[0] + mat[7] * m[1] + mat[11] * m[2] + mat[15] * m[3];
		ret[4] = mat[0] * m[4] + mat[4] * m[5] + mat[8] * m[6] + mat[12] * m[7];
		ret[5] = mat[1] * m[4] + mat[5] * m[5] + mat[9] * m[6] + mat[13] * m[7];
		ret[6] = mat[2] * m[4] + mat[6] * m[5] + mat[10] * m[6] + mat[14] * m[7];
		ret[7] = mat[3] * m[4] + mat[7] * m[5] + mat[11] * m[6] + mat[15] * m[7];
		ret[8] = mat[0] * m[8] + mat[4] * m[9] + mat[8] * m[10] + mat[12] * m[11];
		ret[9] = mat[1] * m[8] + mat[5] * m[9] + mat[9] * m[10] + mat[13] * m[11];
		ret[10] = mat[2] * m[8] + mat[6] * m[9] + mat[10] * m[10] + mat[14] * m[11];
		ret[11] = mat[3] * m[8] + mat[7] * m[9] + mat[11] * m[10] + mat[15] * m[11];
		ret[12] = mat[0] * m[12] + mat[4] * m[13] + mat[8] * m[14] + mat[12] * m[15];
		ret[13] = mat[1] * m[12] + mat[5] * m[13] + mat[9] * m[14] + mat[13] * m[15];
		ret[14] = mat[2] * m[12] + mat[6] * m[13] + mat[10] * m[14] + mat[14] * m[15];
		ret[15] = mat[3] * m[12] + mat[7] * m[13] + mat[11] * m[14] + mat[15] * m[15];
		return ret;
	}

	mat4 mat4::operator+(const mat4 &m) const
	{
		mat4 ret;
		ret[0] = mat[0] + m[0]; ret[4] = mat[4] + m[4]; ret[8] = mat[8] + m[8]; ret[12] = mat[12] + m[12];
		ret[1] = mat[1] + m[1]; ret[5] = mat[5] + m[5]; ret[9] = mat[9] + m[9]; ret[13] = mat[13] + m[13];
		ret[2] = mat[2] + m[2]; ret[6] = mat[6] + m[6]; ret[10] = mat[10] + m[10]; ret[14] = mat[14] + m[14];
		ret[3] = mat[3] + m[3]; ret[7] = mat[7] + m[7]; ret[11] = mat[11] + m[11]; ret[15] = mat[15] + m[15];
		return ret;
	}

	mat4 mat4::operator-(const mat4 &m) const 
	{
		mat4 ret;
		ret[0] = mat[0] - m[0]; ret[4] = mat[4] - m[4]; ret[8] = mat[8] - m[8]; ret[12] = mat[12] - m[12];
		ret[1] = mat[1] - m[1]; ret[5] = mat[5] - m[5]; ret[9] = mat[9] - m[9]; ret[13] = mat[13] - m[13];
		ret[2] = mat[2] - m[2]; ret[6] = mat[6] - m[6]; ret[10] = mat[10] - m[10]; ret[14] = mat[14] - m[14];
		ret[3] = mat[3] - m[3]; ret[7] = mat[7] - m[7]; ret[11] = mat[11] - m[11]; ret[15] = mat[15] - m[15];
		return ret;
	}
	

	
	mat4 mat4::inverse() const 
	{
		mat4 ret;
		ret[0]=mat[0];
		ret[1]=mat[4];
		ret[2]=mat[8];
		ret[3]=0.0;
		ret[4]=mat[1];
		ret[5]=mat[5];
		ret[6]=mat[9];
		ret[7]=0.0;
		ret[8]=mat[2];
		ret[9]=mat[6];
		ret[10]=mat[10];
		ret[11]=0.0;
		ret[12]=-mat[0]*mat[12]-mat[1]*mat[13]-mat[2]*mat[14];
		ret[13]=-mat[4]*mat[12]-mat[5]*mat[13]-mat[6]*mat[14];
		ret[14]=-mat[8]*mat[12]-mat[9]*mat[13]-mat[10]*mat[14];
		ret[15]=1.0;
		return ret;
	}
	
	void mat4::zero() 
	{
		mat[0] = 0.0; mat[4] = 0.0; mat[8] = 0.0; mat[12] = 0.0;
		mat[1] = 0.0; mat[5] = 0.0; mat[9] = 0.0; mat[13] = 0.0;
		mat[2] = 0.0; mat[6] = 0.0; mat[10] = 0.0; mat[14] = 0.0;
		mat[3] = 0.0; mat[7] = 0.0; mat[11] = 0.0; mat[15] = 0.0;
	}

	void mat4::identity() 
	{
		mat[0] = 1.0; mat[4] = 0.0; mat[8] = 0.0; mat[12] = 0.0;
		mat[1] = 0.0; mat[5] = 1.0; mat[9] = 0.0; mat[13] = 0.0;
		mat[2] = 0.0; mat[6] = 0.0; mat[10] = 1.0; mat[14] = 0.0;
		mat[3] = 0.0; mat[7] = 0.0; mat[11] = 0.0; mat[15] = 1.0;
	}

	void mat4::rotate(float angle,const vec3 &axis) 
	{
		float rad = angle * DEG2RAD;
		float c = (float)cos(rad);
		float s = (float)sin(rad);
		vec3 v = axis;
		v.normalize();
		float xy = v.x * v.y;
		float yz = v.y * v.z;
		float zx = v.z * v.x;
		float xs = v.x * s;
		float ys = v.y * s;
		float zs = v.z * s;

		mat[0] = (1.0f - c) * v.x * v.x + c; 
		mat[4] = (1.0f - c) * xy - zs; 
		mat[8] = (1.0f - c) * zx + ys; 
		mat[12] = 0.0;

		mat[1] = (1.0f - c) * xy + zs; 
		mat[5] = (1.0f - c) * v.y * v.y + c; 
		mat[9] = mat[6] = (1.0f - c) * yz - xs; 
		mat[13] = 0.0;

		mat[2] = (1.0f - c) * zx - ys; 
		mat[6] = (1.0f - c) * yz + xs; 
		mat[10] = (1.0f - c) * v.z * v.z + c; 
		mat[14] = 0.0;

		mat[3] = 0.0; 
		mat[7] = 0.0; 
		mat[11] = 0.0; 
		mat[15] = 1.0;
	}

	 void mat4::rotate(float angle,float x,float y,float z) 
	{
		rotate(angle,vec3(x,y,z));
	}

	void mat4::rotate_x(float angle) 
	{
		float rad = angle * DEG2RAD;
		float c = (float)cos(rad);
		float s = (float)sin(rad);
		mat[0] = 1.0; mat[4] = 0.0; mat[8] = 0.0; mat[12] = 0.0;
		mat[1] = 0.0; mat[5] = c; mat[9] = -s; mat[13] = 0.0;
		mat[2] = 0.0; mat[6] = s; mat[10] = c; mat[14] = 0.0;
		mat[3] = 0.0; mat[7] = 0.0; mat[11] = 0.0; mat[15] = 1.0;
	}

	void mat4::rotate_y(float angle) 
	{
		float rad = angle * DEG2RAD;
		float c = (float)cos(rad);
		float s = (float)sin(rad);
		mat[0] = c; mat[4] = 0.0; mat[8] = s; mat[12] = 0.0;
		mat[1] = 0.0; mat[5] = 1.0; mat[9] = 0.0; mat[13] = 0.0;
		mat[2] = -s; mat[6] = 0.0; mat[10] = c; mat[14] = 0.0;
		mat[3] = 0.0; mat[7] = 0.0; mat[11] = 0.0; mat[15] = 1.0;
	}

	void mat4::rotate_z(float angle) 
	{
		float rad = angle * DEG2RAD;
		float c = (float)cos(rad);
		float s = (float)sin(rad);
		mat[0] = c; mat[4] = -s; mat[8] = 0.0; mat[12] = 0.0;
		mat[1] = s; mat[5] = c; mat[9] = 0.0; mat[13] = 0.0;
		mat[2] = 0.0; mat[6] = 0.0; mat[10] = 1.0; mat[14] = 0.0;
		mat[3] = 0.0; mat[7] = 0.0; mat[11] = 0.0; mat[15] = 1.0;
	}

	void mat4::scale(const vec3 &v) 
	{
		mat[0] = v.x; mat[4] = 0.0; mat[8] = 0.0; mat[12] = 0.0;
		mat[1] = 0.0; mat[5] = v.y; mat[9] = 0.0; mat[13] = 0.0;
		mat[2] = 0.0; mat[6] = 0.0; mat[10] = v.z; mat[14] = 0.0;
		mat[3] = 0.0; mat[7] = 0.0; mat[11] = 0.0; mat[15] = 1.0;
	}

	 void mat4::scale(float x,float y,float z) 
	{
		scale(vec3(x,y,z));
	}

	void mat4::translate(const vec3 &v) 
	{
		mat[0] = 1.0; mat[4] = 0.0; mat[8] = 0.0; mat[12] = v.x;
		mat[1] = 0.0; mat[5] = 1.0; mat[9] = 0.0; mat[13] = v.y;
		mat[2] = 0.0; mat[6] = 0.0; mat[10] = 1.0; mat[14] = v.z;
		mat[3] = 0.0; mat[7] = 0.0; mat[11] = 0.0; mat[15] = 1.0;
	}

	 void mat4::translate(float x,float y,float z) 
	{
		translate(vec3(x,y,z));
	}

	 void mat4::ortho(float left, float right, float bottom, float top, float zNear, float zFar)
	 {
		 identity();

		 mat[0] = 2.0f / (right - left);
		 mat[5] = 2.0f / (top - bottom);
		 mat[10] = - 2.f / (zFar - zNear);
		 mat[12] = - (right + left) / (right - left);
		 mat[13] = - (top + bottom) / (top - bottom);
		 mat[14] = - (zFar + zNear) / (zFar - zNear);
	 }

	void mat4::perspective(float fov,float aspect,float znear,float zfar) 
	{
		float y = (float)tan(fov * PI / 360.0f);
		float x = y * aspect;
		mat[0] = 1.0f / x; mat[4] = 0.0; mat[8] = 0.0; mat[12] = 0.0;
		mat[1] = 0.0; mat[5] = 1.0f / y; mat[9] = 0.0; mat[13] = 0.0;
		mat[2] = 0.0; mat[6] = 0.0; mat[10] = -(zfar + znear) / (zfar - znear); mat[14] = -(2.0f * zfar * znear) / (zfar - znear);
		mat[3] = 0.0; mat[7] = 0.0; mat[11] = -1.0; mat[15] = 0.0;
	}

	void mat4::look_at(const vec3 &eye,const vec3 &dir,const vec3 &up) 
	{
		vec3 x,y,z;
		mat4 m0,m1;
		z = eye - dir;
		z.normalize();
//		x=up.cross(z);
		x=cross(up,z);
		x.normalize();
//		y=z.cross(x);
		y=cross(z,x);
		y.normalize();
		m0[0] = x.x; m0[4] = x.y; m0[8] = x.z; m0[12] = 0.0;
		m0[1] = y.x; m0[5] = y.y; m0[9] = y.z; m0[13] = 0.0;
		m0[2] = z.x; m0[6] = z.y; m0[10] = z.z; m0[14] = 0.0;
		m0[3] = 0.0; m0[7] = 0.0; m0[11] = 0.0; m0[15] = 1.0;
		m1.translate(-eye);
		*this = m0 * m1;
	}

	 void mat4::look_at(const float *eye,const float *dir,const float *up) 
	{
		look_at(vec3(eye),vec3(dir),vec3(up));
	}
/*-------------------------------------------------------------------------------------*/

	 quat::quat() : x(0), y(0), z(0), w(1) 
	{ }

	 quat::quat(float tx,float ty,float tz,float tw)
	{
		x=tx;
		y=ty;
		z=tz;
		w=tw;
	}

	 quat::quat(float tx,float ty,float tz)
	
	{
		x=tx;
		y=ty;
		z=tz;
		computer_w();
	}

	 quat quat::operator +(const quat &q)const 
	{
		quat tq;
		tq.x=x+q.x;
		tq.y=y+q.y;
		tq.z=z+q.z;
		tq.w=w+q.w;
		return tq;
	}

	quat quat::operator -(const quat &q)const 
	{
		quat tq;
		tq.x=x+q.x;
		tq.y=y+q.y;
		tq.z=z+q.z;
		tq.w=w+q.w;
		return tq;
	}

	quat quat::operator*(const quat &q) const 
	{
		quat ret;
		ret.x = w * q.x + x * q.w + y * q.z - z * q.y;
		ret.y = w * q.y + y * q.w + z * q.x - x * q.z;
		ret.z = w * q.z + z * q.w + x * q.y - y * q.x;
		ret.w = w * q.w - x * q.x - y * q.y - z * q.z;
		return ret;
	}

	quat quat::operator*(const vec3 &q) const 
	{
		quat ret;
		ret.x = w * q.x + y * q.z - z * q.y;
		ret.y = w * q.y + z * q.x - x * q.z;
		ret.z = w * q.z + x * q.y - y * q.x;
		ret.w = - x * q.x - y * q.y - z * q.z;
		return ret;
	}

	quat quat::operator *(const float &tf)const
	{
		quat ret;
		ret.x*=tf;
		ret.y*=tf;
		ret.z*=tf;
		ret.w*=tf;
		return ret;
	}
	
	 quat quat::operator /(const float & tf)const
	{
		float inver=(float)1.0/tf;
		return (*this)*inver;
	}
	
	 quat& quat::operator+=(const quat &tq)
	{
		x+=tq.x;
		y+=tq.y;
		z+=tq.z;
		w+=tq.w;
		return *this;
	}

	 quat& quat::operator-=(const quat &tq)
	{
		x-=tq.x;
		y-=tq.y;
		z-=tq.z;
		w-=tq.w;
		return *this;
	}

	quat& quat::operator *=(const quat &q)
	{
		quat ret;
		ret.x = w * q.x + x * q.w + y * q.z - z * q.y;
		ret.y = w * q.y + y * q.w + z * q.x - x * q.z;
		ret.z = w * q.z + z * q.w + x * q.y - y * q.x;
		ret.w = w * q.w - x * q.x - y * q.y - z * q.z;
		this->set(ret);
		return *this; 
	}

	 quat& quat::operator /=(const float &tf)
	{
		x/=tf;
		y/=tf;
		z/=tf;
		w/=tf;
		return *this;
	}

	 float& quat::operator[](int i) 
	{ 
		return ((float*)&x)[i]; 
	}

	 const float quat::operator[](int i) const 
	{
		return ((float*)&x)[i]; 
	}

	 void quat::computer_w()
	{
		float temp=1.0f -(x*x+y*y+z*z);
		if( temp>0.0f )	w=-(float)sqrt(temp);
		else w=0.0f;
	}

	void quat::normalize()
	{
		float temp=(float)sqrt(x*x+y*y+z*z+w*w);
		temp=(float)1.0/temp;
		x*=temp;
		y*=temp;
		z*=temp;
		w*=temp;
	}

	 void quat::set(quat &tq)
	{
		x=tq.x;
		y=tq.y;
		z=tq.z;
		w=tq.w;
	}
	
	 void quat::set(float tx,float ty,float tz,float tw)
	{
		x=tx;
		y=ty;
		z=tz;
		w=tw;
	}
	
	 void quat::set(float tx,float ty,float tz)
	{
		x=tx;
		y=ty;
		z=tz;
		computer_w();
	}

	 quat quat::inverse()
	{
		return quat(-x,-y,-z,w);
	}

	 void quat::zero()
	{
		x=0.0;
		y=0.0;
		z=0.0;
		w=0.0;
	}

	 void quat::set(const vec3 &v)
	{
		set(v.x,v.y,v.z);
	}

	void quat::set_rotation(const vec3 &dir,float angle) 
	{
		float length = dir.length();
		if(length != 0.0) {
			length = 1.0f / length;
			float sinangle = (float)sin(angle * DEG2RAD / 2.0f);
			x = dir[0] * length * sinangle;
			y = dir[1] * length * sinangle;
			z = dir[2] * length * sinangle;
			w = (float)cos(angle * DEG2RAD / 2.0f);
		} else {
			x = y = z = 0.0;
			w = 1.0;
		}
	}

	vec3 quat::inverse_rotate(const vec3& p)
	{
		vec3 v(-x,-y,-z);
		vec3 result=p*(w*w-dot(v,v))+cross(v,p)*w*2+v*dot(v,p)*2;
		return result;
	}
	vec3 quat::rotate(const vec3& p)
	{
		vec3 v(x,y,z);
		vec3 result=p*(w*w-dot(v,v))+cross(v,p)*w*2+v*dot(v,p)*2;
		return result;
		
	}
	 void quat::set_rotation(float x,float y,float z,float angle) 
	{
		set_rotation(vec3(x,y,z),angle);
	}

	void quat::slerp(const quat &q0,const quat &q1,float t) 
	{
		float k0,k1,cosomega = q0.x * q1.x + q0.y * q1.y + q0.z * q1.z + q0.w * q1.w;
		quat q;
		if(cosomega < 0.0) {
			cosomega = -cosomega;
			q.x = -q1.x;
			q.y = -q1.y;
			q.z = -q1.z;
			q.w = -q1.w;
		} else {
			q.x = q1.x;
			q.y = q1.y;
			q.z = q1.z;
			q.w = q1.w;
		}
		if(1.0 - cosomega > 1e-6) {
			float omega = (float)acos(cosomega);
			float sinomega = (float)sin(omega);
			k0 = (float)sin((1.0f - t) * omega) / sinomega;
			k1 = (float)sin(t * omega) / sinomega;
		} else {
			k0 = 1.0f - t;
			k1 = t;
		}
		x = q0.x * k0 + q.x * k1;
		y = q0.y * k0 + q.y * k1;
		z = q0.z * k0 + q.z * k1;
		w = q0.w * k0 + q.w * k1;
	}

	mat4 quat::to_matrix() const 
	{
		mat4 ret;
		float x2 = x + x;
		float y2 = y + y;
		float z2 = z + z;
		float xx = x * x2;
		float yy = y * y2;
		float zz = z * z2;
		float xy = x * y2;
		float yz = y * z2;
		float xz = z * x2;
		float wx = w * x2;
		float wy = w * y2;
		float wz = w * z2;
		ret[0] = 1.0f - (yy + zz); ret[4] = xy - wz;          ret[8] = xz + wy;           ret[12]=0.0;
		ret[1] = xy + wz;          ret[5] = 1.0f - (xx + zz); ret[9] = yz - wx;           ret[13]=0.0;
		ret[2] = xz - wy;          ret[6] = yz + wx;          ret[10]= 1.0f - (xx + yy);  ret[14]=0.0;
		ret[3] = 0.0;              ret[7]=  0.0;              ret[11]= 0.0;               ret[15]=1.0;
		return ret;
	}

	mat4 quat::to_matrix(const vec3 &pos) const 
	{
		mat4 ret;
		float x2 = x + x;
		float y2 = y + y;
		float z2 = z + z;
		float xx = x * x2;
		float yy = y * y2;
		float zz = z * z2;
		float xy = x * y2;
		float yz = y * z2;
		float xz = z * x2;
		float wx = w * x2;
		float wy = w * y2;
		float wz = w * z2;
		ret[0] = 1.0f - (yy + zz); ret[4] = xy - wz;          ret[8] = xz + wy;           ret[12]=pos.x;       
		ret[1] = xy + wz;          ret[5] = 1.0f - (xx + zz); ret[9] = yz - wx;           ret[13]=pos.y;
		ret[2] = xz - wy;          ret[6] = yz + wx;          ret[10] = 1.0f - (xx + yy); ret[14]=pos.z;
		ret[3] = 0.0;              ret[7] = 0.0;              ret[11]=0.0;                ret[15]=1.0;
		return ret;
	}
	
/*-------------------------------------------------------------------------------------*/

float dot(const vec3& v1,const vec3& v2)
{
	return v1.x*v2.x+v1.y*v2.y+v1.z*v2.z;
}

vec3 cross(const vec3& v1,const vec3& v2)
{
	vec3 tv;
	tv.x=v1.y*v2.z-v1.z*v2.y;
	tv.y=v1.z*v2.x-v1.x*v2.z;
	tv.z=v1.x*v2.y-v1.y*v2.x;
	return tv;
}

vec3 computer_normal(const vec3& v1,const vec3& v2,const vec3& v3)
{
	vec3 v12=v2-v1;
	vec3 v13=v3-v1;
	vec3 tv=cross(v12,v13);
	tv.normalize();
	return tv;
}
