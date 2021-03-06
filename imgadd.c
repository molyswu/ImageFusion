/** @file imgadd.c - Implementation
 ** @brief Image add
 ** @author Zhiwei Zeng
 ** @date 2018.04.25
 **/

/*
Copyright (C) 2018 Zhiwei Zeng.
Copyright (C) 2018 Chengdu ZLT Technology Co., Ltd.
All rights reserved.

This file is part of the railway monitor toolkit and is made available under
the terms of the BSD license (see the COPYING file).
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef __WIN_SSE__	
#	include <smmintrin.h>
#endif

#ifdef __WIN_AVX__
#	include <immintrin.h>
#endif

#include "imgadd.h"

/** @name Some local functions.
 ** @{ */
static void img_add_kr_sse(const unsigned char *A, unsigned int width,
                           unsigned int height, const unsigned char *B,
			               unsigned char *C);
static void img_add_kr_avx(const unsigned char *A, unsigned int width,
                           unsigned int height, const unsigned char *B,
			               unsigned char *C);
static void img_add_kr_nsu(const unsigned char *A, unsigned int width,
                           unsigned int height, const unsigned char *B,
			               unsigned char *C);
static void img_add_sse(const unsigned char *A, unsigned int width,
                        unsigned int height, const unsigned char *B,
			            unsigned short *C);
static void img_add_avx(const unsigned char *A, unsigned int width,
                        unsigned int height, const unsigned char *B,
			            unsigned short *C);
static void img_add_nsu(const unsigned char *A, unsigned int width,
                        unsigned int height, const unsigned char *B,
			            unsigned short *C);
/** @} */

/** @brief Add two images and keep gray range no changed.
 ** @param A one gray image.
 ** @param width image width.
 ** @param height image height.
 ** @param B another gray image.
 ** @param C the sum image.
 **/
void img_add_kr(const unsigned char *A, unsigned int width,
                unsigned int height, const unsigned char *B,
			    unsigned char *C)
{
	assert(A);
	assert(B);
	assert(C);
	
#ifdef __WIN_SSE__
	img_add_kr_sse(A, width, height, B, C);
#elif __WIN_AVX__
	img_add_kr_avx(A, width, height, B, C);
#else
	img_add_kr_nsu(A, width, height, B, C);
#endif
}

/** @brief Add two images.
 ** @param A one gray image.
 ** @param width image width.
 ** @param height image height.
 ** @param B another gray image.
 ** @param C the sum image.
 **/
void img_add(const unsigned char *A, unsigned int width,
             unsigned int height, const unsigned char *B,
			 unsigned short *C)
{
	assert(A);
	assert(B);
	assert(C);
	
#ifdef __WIN_SSE__
	img_add_sse(A, width, height, B, C);
#elif __WIN_AVX__
	img_add_avx(A, width, height, B, C);
#else
	img_add_nsu(A, width, height, B, C);
#endif	
}

/** @brief Add two images with SSE.
 **        keep gray range no changed.
 ** @param A one gray image.
 ** @param width image width.
 ** @param height image height.
 ** @param B another gray image.
 ** @param C the sum image.
 **/
#ifdef __WIN_SSE__
void img_add_kr_sse(const unsigned char *A, unsigned int width,
                    unsigned int height, const unsigned char *B,
			        unsigned char *C)
{
	unsigned int i;
	unsigned int npixels;
	const unsigned int ppl = 16;
	
	__m128i X;
	__m128i Y;
	__m128i S;
	
	npixels = width * height;
	
	for (i = 0; i < npixels; i += ppl) {
		X = _mm_loadu_si128((__m128i *)(A + i));
		Y = _mm_loadu_si128((__m128i *)(B + i));
		S = _mm_adds_epu8(X, Y);
		_mm_storeu_si128((__m128i *)(C + i), S);
	}
}	
#endif
	
/** @brief Add two images with AVX.
 **        keep gray range no changed.
 ** @param A one gray image.
 ** @param width image width.
 ** @param height image height.
 ** @param B another gray image.
 ** @param C the sum image.
 **/	
#ifdef __WIN_AVX__		
void img_add_kr_avx(const unsigned char *A, unsigned int width,
                    unsigned int height, const unsigned char *B,
			        unsigned char *C)
{
	unsigned int i;
	unsigned int npixels;
	const unsigned int ppl = 32;
	
	__m256i X;
	__m256i Y;
	__m256i S;
	
	npixels = width * height;
	
	for (i = 0; i < npixels; i += ppl) {
		X = _mm256_loadu_si256((__m256i *)(A + i));
		Y = _mm256_loadu_si256((__m256i *)(B + i));
		S = _mm256_adds_epu8(X, Y);
		_mm256_storeu_si256((__m256i *)(C + i), S);
	}
}	
#endif

/** @brief Add two images no speed up.
 **        keep gray range no changed.
 ** @param A one gray image.
 ** @param width image width.
 ** @param height image height.
 ** @param B another gray image.
 ** @param C the sum image.
 **/					
void img_add_kr_nsu(const unsigned char *A, unsigned int width,
                    unsigned int height, const unsigned char *B,
			        unsigned char *C)
{
	unsigned int i;
	unsigned int npixels;
	unsigned int sum;
	
	npixels = width * height;
	
	for (i = 0; i < npixels; i++) {
		sum = A[i] + B[i];
		if (sum > 255) {
			sum = 255;
		}
		
		C[i] = sum;
	}
}

/** @brief Add two images with SSE.
 ** @param A one gray image.
 ** @param width image width.
 ** @param height image height.
 ** @param B another gray image.
 ** @param C the sum image.
 **/
#ifdef __WIN_SSE__
void img_add_sse(const unsigned char *A, unsigned int width,
                 unsigned int height, const unsigned char *B,
			     unsigned short *C)
{	
	unsigned int i;
	unsigned int j;
	unsigned int npixels;
	const unsigned int ppl = 16;
	const unsigned int pps = 8;
	
	__m128i Zero;
	__m128i X;
	__m128i XL;
	__m128i XH;
	__m128i Y;
	__m128i YL;
	__m128i YH;
	__m128i SL;
	__m128i SH;
	
	j = 0;
	npixels = width * height;
	Zero = _mm_setzero_si128();
	
	for (i = 0; i < npixels; i += ppl) {
		X = _mm_loadu_si128((__m128i *)(A + i));
		XL = _mm_unpacklo_epi8(X, Zero);
		XH = _mm_unpackhi_epi8(X, Zero);
		
		Y = _mm_loadu_si128((__m128i *)(B + i));
		YL = _mm_unpacklo_epi8(Y, Zero);
		YH = _mm_unpackhi_epi8(Y, Zero);
		
		SL = _mm_adds_epu16(XL, YL);
		SH = _mm_adds_epu16(XH, YH);
		
		_mm_storeu_si128((__m128i *)(C + j), SL);
		j += pps;
		_mm_storeu_si128((__m128i *)(C + j), SH);
		j += pps;
	}
}
#endif

/** @brief Add two images with AVX.
 ** @param A one gray image.
 ** @param width image width.
 ** @param height image height.
 ** @param B another gray image.
 ** @param C the sum image.
 **/
#ifdef __WIN_AVX__
void img_add_avx(const unsigned char *A, unsigned int width,
                 unsigned int height, const unsigned char *B,
			     unsigned short *C)
{
	unsigned int i;
	unsigned int j;
	unsigned int npixels;
	const unsigned int ppl = 32;
	const unsigned int pps = 16;
	
	__m256i Zero;
	__m256i X;
	__m256i XL;
	__m256i XH;
	__m256i Y;
	__m256i YL;
	__m256i YH;
	__m256i SL;
	__m256i SH;
	
	j = 0;
	npixels = width * height;
	Zero = _mm256_setzero_si256();
	
	for (i = 0; i < npixels; i += ppl) {
		X = _mm256_loadu_si256((__m256i *)(A + i));
		XL = _mm256_unpacklo_epi8(X, Zero);
		XH = _mm256_unpackhi_epi8(X, Zero);
		
		Y = _mm256_loadu_si256((__m256i *)(B + i));
		YL = _mm256_unpacklo_epi8(Y, Zero);
		YH = _mm256_unpackhi_epi8(Y, Zero);
		
		SL = _mm256_adds_epu16(XL, YL);
		SH = _mm256_adds_epu16(XH, YH);
		
		_mm256_storeu_si256((__m256i *)(C + j), SL);
		j += pps;
		_mm256_storeu_si256((__m256i *)(C + j), SH);
		j += pps;
	}
}
#endif

/** @brief Add two images no speed up.
 ** @param A one gray image.
 ** @param width image width.
 ** @param height image height.
 ** @param B another gray image.
 ** @param C the sum image.
 **/
void img_add_nsu(const unsigned char *A, unsigned int width,
                 unsigned int height, const unsigned char *B,
		         unsigned short *C)	
{
	unsigned int i;
	unsigned int npixels;
	
	npixels = width * height;
	
	for (i = 0; i < npixels; i++) {
		C[i] = A[i] + B[i];
	}
}		   