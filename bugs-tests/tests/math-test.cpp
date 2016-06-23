/*
 * math-test.cpp
 *
 *  Created on: Dec 29, 2015
 *      Author: bog
 */

#include "../../bugs/math/math2D.h"
#include "../../bugs/math/box2glm.h"

#include <easyunit/test.h>
#include <sstream>

using namespace easyunit;

float flterr = 1.e-5f;	// accepted float error in comparisons

TEST(math, sqr) {
	float x = PI;
	ASSERT_EQUALS_DELTA(PI*PI, sqr(x), flterr);
}

TEST(math, xchg) {
	int a=3, b=17;
	xchg(a,b);
	ASSERT_EQUALS(a, 17);
	ASSERT_EQUALS(b, 3);
}

TEST(math, min) {
	float x=1.798505, y=56.12314;
	ASSERT_EQUALS(min(x, y), min(y, x));
	ASSERT_EQUALS(min(x, y), x);
}

TEST(math, max) {
	float x=1.798505, y=56.12314;
	ASSERT_EQUALS(max(x, y), max(y, x));
	ASSERT_EQUALS(max(x, y), y);
}

TEST(math, sign) {
	ASSERT_EQUALS(sign(-7.2323), -1);
	ASSERT_EQUALS(sign(15.1235), +1);
	ASSERT_EQUALS(sign(0), 0);
}

TEST(math, abs) {
	float x=-56543.123;
	ASSERT_EQUALS(abs(x), abs(-x));
	ASSERT_EQUALS(abs(x), -x);
}

TEST(math, eqEps) {
	ASSERT_TRUE(eqEps(5, 5+EPS*0.9f));
	ASSERT_TRUE(eqEps(7, 7+0.023, 0.03));
}

TEST(math, circularPrev) {
	ASSERT_EQUALS(circularPrev(3, 4), 2);
	ASSERT_EQUALS(circularPrev(0, 4), 3);
}

TEST(math, circularNext) {
	ASSERT_EQUALS(circularNext(3, 4), 0);
	ASSERT_EQUALS(circularNext(1, 4), 2);
}

TEST(math, pointDirectionNormalized) {
	float angle1 = PI/2.3f;
	glm::vec2 point1(cosf(angle1), sinf(angle1));
	ASSERT_EQUALS_DELTA(angle1, pointDirectionNormalized(point1), flterr);
	float angle2 = PI/2 + angle1;
	glm::vec2 point2(cosf(angle2), sinf(angle2));
	ASSERT_EQUALS_DELTA(angle2, pointDirectionNormalized(point2), flterr);
	float angle3 = -PI + angle1;
	glm::vec2 point3(cosf(angle3), sinf(angle3));
	ASSERT_EQUALS_DELTA(angle3, pointDirectionNormalized(point3), flterr);
	float angle4 = PI/2 + angle3;
	glm::vec2 point4(cosf(angle4), sinf(angle4));
	ASSERT_EQUALS_DELTA(angle4, pointDirectionNormalized(point4), flterr);
}

TEST(math, limitAngle) {
	for (float bis=0.3f; bis<2*PI; bis += PI/2)
		for (float a=-2*PI+0.1f; a<2*PI; a+=PI/2.1f) {
			float alim = limitAngle(a, bis);
			std::stringstream ss;
			ss << "a="<<a<<"; alim="<<alim<<"; bis="<<bis;
			ASSERT_TRUE_M(alim < bis, ss.str().c_str());
			ASSERT_TRUE_M(alim >= bis-2*PI, ss.str().c_str());
			ASSERT_TRUE_M(eqEps(alim,a,flterr) || eqEps(alim+2*PI,a,flterr) || eqEps(alim+4*PI,a,flterr)
					|| eqEps(alim-2*PI,a,flterr) || eqEps(alim-4*PI,a,flterr), ss.str().c_str());
		}
}

TEST(math, angleDiff) {
	for (float a=PI/4-2*PI; a<2*PI; a+=PI/2)
		for (float b=PI/8-2*PI; b<2*PI; b+=PI/4) {
			float dif = angleDiff(a, b);
			std::stringstream s;
			s << "a="<<a<<"; b="<<b<<"; dif="<<dif;
			ASSERT_TRUE_M(dif<=PI && dif>-PI, s.str().c_str());
			float al = limitAngle(a, 2*PI);
			float bl = limitAngle(b, 2*PI);
			ASSERT_TRUE_M(((bl<al && bl > al-PI) || bl>al+PI) == dif<0, s.str().c_str());
			ASSERT_TRUE_M(((bl>al && bl<al+PI) || bl<al-PI) == dif>0, s.str().c_str());
			ASSERT_TRUE_M((bl==al) == (dif==0), s.str().c_str());
		}
}

TEST(math, absAngleDiff) {
	for (float a=PI/4-2*PI; a<2*PI; a+=PI/2)
		for (float b=PI/8-2*PI; b<2*PI; b+=PI/4) {
			float dif = absAngleDiff(a, b);
			std::stringstream s;
			s << "a="<<a<<"; b="<<b<<"; dif="<<dif;
			ASSERT_TRUE_M(dif<PI && dif>=0, s.str().c_str());
			float al = limitAngle(a, 2*PI);
			float bl = limitAngle(b, 2*PI);
			ASSERT_TRUE_M(eqEps(limitAngle(al+dif, 2*PI), bl, flterr) || eqEps(limitAngle(bl+dif, 2*PI), al, flterr), s.str().c_str());
		}
}


