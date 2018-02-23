#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>

#define RGB_LED_BARGRAPH_0_BASEADDR 0x00000000
#define BARGRAPH_ADDR_REG 			(*((volatile uint32_t *)(RGB_LED_BARGRAPH_0_BASEADDR + 0x00)))
#define BARGRAPH_DATA_REG 			(*((volatile uint32_t *)(RGB_LED_BARGRAPH_0_BASEADDR + 0x04)))
#define BARGRAPH_BUFFER_REG 		(*((volatile uint32_t *)(RGB_LED_BARGRAPH_0_BASEADDR + 0x08)))
#define BARGRAPH_BRIGHTNESS_REG		(*((volatile uint32_t *)(RGB_LED_BARGRAPH_0_BASEADDR + 0x0C)))
#define BARGRAPH_TIMER_REG 			(*((volatile uint32_t *)(RGB_LED_BARGRAPH_0_BASEADDR + 0x10)))
#define BARGRAPH_TIMER_FLAG_REG 	(*((volatile uint32_t *)(RGB_LED_BARGRAPH_0_BASEADDR + 0x14)))
#define BARGRAPH_TEST_LEDS_REG		(*((volatile uint32_t *)(RGB_LED_BARGRAPH_0_BASEADDR + 0x18)))

void SetLevels (void);
void SetDot (uint16_t base, uint8_t n, uint8_t r, uint8_t g, uint8_t b);
void TranslateHue (int16_t hue, uint8_t *pr, uint8_t *pg, uint8_t *pb);
void InitPerlin (void);
void NextPerlin (void);
int32_t noise (uint16_t x, uint16_t y, uint16_t z);

const uint8_t gamma8[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

uint8_t buffer = 0;
uint8_t levels[1][48][3];

int main()
{
	uint32_t a, b, c, d;

	a = *((uint32_t *)0x00000000);
	b = *((uint32_t *)0x00000004);
	c = *((uint32_t *)0x00000008);
	d = *((uint32_t *)0x0000000c);

	int i;

    // blank both display buffers
    BARGRAPH_ADDR_REG = 0x00000000;
    for (i = 0; i < 512; i++) {
        BARGRAPH_DATA_REG = 0x00000000;
    }

    BARGRAPH_TEST_LEDS_REG = 1;

    // select display buffer 0
    BARGRAPH_BUFFER_REG = 0x0;

    BARGRAPH_TEST_LEDS_REG = 2;

    // set global brightness to full
    BARGRAPH_BRIGHTNESS_REG = 0x100;

    BARGRAPH_TEST_LEDS_REG = 3;

    // set periodic interval timer to 30Hz w/ a 100MHz clock
    BARGRAPH_TIMER_REG = 3333333;

    BARGRAPH_TEST_LEDS_REG = 4;

    // init pattern
    InitPerlin ();

    BARGRAPH_TEST_LEDS_REG = 5;

    // clear timer flag
	BARGRAPH_TIMER_FLAG_REG = 0x1;

    BARGRAPH_TEST_LEDS_REG = 6;

    // wait for timer flag to be set
	while (BARGRAPH_TIMER_FLAG_REG == 0) {
	}

    BARGRAPH_TEST_LEDS_REG = 7;

    i = 8;

    while (1) {
    	// calculate next step in pattern
    	NextPerlin ();

		// wait for timer flag to be set
		while (BARGRAPH_TIMER_FLAG_REG == 0) {
		}

    	// clear timer flag
		BARGRAPH_TIMER_FLAG_REG = 0x1;

    	// write levels to bar graph
    	SetLevels ();

        BARGRAPH_TEST_LEDS_REG = i++;
    }

    return 0;
}


void SetLevels (void)
{
	int n;
	uint16_t base;

	if (buffer == 0) {
		base = 0x000;
	} else {
		base = 0x100;
	}

	for (n = 0; n < 48; n++) {
		SetDot (base, n, levels[0][n][0], levels[0][n][1], levels[0][n][2]);
	}

	if (buffer == 0) {
		BARGRAPH_BUFFER_REG = 0x0;
		buffer = 1;
	} else {
		BARGRAPH_BUFFER_REG = 0x1;
		buffer = 0;
	}
}


void SetDot (uint16_t base, uint8_t n, uint8_t r, uint8_t g, uint8_t b)
{
	uint8_t row = 16*(n/3);
	uint8_t col = 13 - 3*(n%3);
	uint16_t address = base + row + col;

    BARGRAPH_ADDR_REG = address;
	BARGRAPH_DATA_REG = gamma8[b];
	BARGRAPH_DATA_REG = gamma8[g];
	BARGRAPH_DATA_REG = gamma8[r];
}


void TranslateHue (int16_t hue, uint8_t *pr, uint8_t *pg, uint8_t *pb)
{
    uint8_t hi, lo;
    uint8_t r = 0, g = 0, b = 0;

    hi = hue >> 8;
    lo = hue & 0xff;

    switch (hi) {
        case 0: r = 0xff;    g = 0;       b = lo;      break;
        case 1: r = 0xff-lo, g = 0,       b = 0xff;    break;
        case 2: r = 0,       g = lo,      b = 0xff;    break;
        case 3: r = 0,       g = 0xff,    b = 0xff-lo; break;
        case 4: r = lo,      g = 0xff,    b = 0;       break;
        case 5: r = 0xff,    g = 0xff-lo, b = 0;       break;
    }

    *pr = r;
    *pg = g;
    *pb = b;
}


int32_t m_width;
int32_t m_height;
int32_t m_mode;
int32_t m_xy_scale;
float m_z_step;
float m_z_depth;
float m_hue_options;
float m_z_state;
float m_hue_state;
float m_min, m_max;


void InitPerlin (void)
{
    // new Perlin (DISPLAY_WIDTH, DISPLAY_HEIGHT, 2, 8.0/64.0, 1.0/64.0, 256.0, 0.005);
	m_width = 48;
	m_height = 1;
	m_mode = 2;
	m_xy_scale = 2.0/64.0*256.0; // normally use 8 here
	m_z_step = 1.0/64.0;
	m_z_depth = 256.0;
	m_hue_options = 0.005;
	m_z_state = 0;
	m_hue_state = 0;
	m_min = 1;
	m_max = 1;
}


void NextPerlin (void)
{
    int32_t x, y;
    uint16_t sx, sy;
	int32_t n1, n2;
	float n;
    int32_t hue;

	int16_t sz1 = (float)m_z_state * 256.0;
	int16_t sz2 = (float)(m_z_state - m_z_depth) * 256.0;

    // row
    for (y = 0; y < m_height; y++) {

        // scale y
        sy = y * m_xy_scale;

        // column
        for (x = 0; x < m_width; x++) {

            // scale x
            sx = x * m_xy_scale;

            // generate noise at plane z_state
            n1 = noise (sx, sy, sz1);

            // generate noise at plane z_state - z_depth
            n2 = noise (sx, sy, sz2);

            // combine noises to make a seamless transition from plane
            // at z = z_depth back to plane at z = 0
            n = ((m_z_depth - m_z_state) * (float)n1 + (m_z_state) * (float)n2) / m_z_depth;

            // normalize combined noises to a number between 0 and 1
            if (n > m_max) m_max = n;
            if (n < m_min) m_min = n;
            n = n + fabs (m_min);               // make noise a positive value
            n = n / (m_max + fabs (m_min));     // scale noise to between 0 and 1

            // set hue and/or brightness based on mode
            switch (m_mode) {

                // base hue fixed, varies based on noise
                case 1:
                    hue = (m_hue_options + n)*1536.0 + 0.5;
                    hue = hue % 1536;
                    TranslateHue (hue, &levels[y][x][0], &levels[y][x][1], &levels[y][x][2]);
                    break;

                // hue rotates at constant velocity, varies based on noise
                case 2:
                    hue = (m_hue_state + n)*1536.0 + 0.5;
                    hue = hue % 1536;
                    TranslateHue (hue, &levels[y][x][0], &levels[y][x][1], &levels[y][x][2]);
                    break;

                // hue rotates at constant velocity, brightness varies based on noise
                case 3:
                    hue = (m_hue_state)*1536.0 + 0.5;
                    hue = hue % 1536;
                    TranslateHue (hue, &levels[y][x][0], &levels[y][x][1], &levels[y][x][2]);
                    break;

                // undefined mode, blank display
                default:
                    levels[y][x][0] = 0;
                    levels[y][x][1] = 0;
                    levels[y][x][2] = 0;
                    break;

            }
        }
    }

    // update state variables
    m_z_state = fmod (m_z_state + m_z_step, m_z_depth);
    m_hue_state = fmod (m_hue_state + m_hue_options, 1.0);
}


//---------------------------------------------------------------------------------------------
// noise
//

#define lerp1(t, a, b) (((a)<<12) + (t) * ((b) - (a)))

static const int8_t GRAD3[16][3] = {
    {1,1,0},{-1,1,0},{1,-1,0},{-1,-1,0},
    {1,0,1},{-1,0,1},{1,0,-1},{-1,0,-1},
    {0,1,1},{0,-1,1},{0,1,-1},{0,-1,-1},
    {1,0,-1},{-1,0,-1},{0,-1,1},{0,1,1}};

static const uint8_t PERM[512] = {
  151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140,
  36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120,
  234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
  88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71,
  134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133,
  230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161,
  1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196, 135, 130,
  116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250,
  124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227,
  47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44,
  154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9, 129, 22, 39, 253, 19, 98,
  108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228, 251, 34,
  242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14,
  239, 107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121,
  50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243,
  141, 128, 195, 78, 66, 215, 61, 156, 180, 151, 160, 137, 91, 90, 15, 131,
  13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37,
  240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252,
  219, 203, 117, 35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125,
  136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158,
  231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245,
  40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187,
  208, 89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198,
  173, 186, 3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126,
  255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223,
  183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167,
  43, 172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185,
  112, 104, 218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179,
  162, 241, 81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199,
  106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236,
  205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156,
  180};

static const uint16_t easing_function_lut[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 3, 4, 6, 7,
    9, 10, 12, 14, 17, 19, 22, 25, 29, 32, 36, 40, 45, 49, 54, 60,
    65, 71, 77, 84, 91, 98, 105, 113, 121, 130, 139, 148, 158, 167, 178, 188,
    199, 211, 222, 234, 247, 259, 273, 286, 300, 314, 329, 344, 359, 374, 390, 407,
    424, 441, 458, 476, 494, 512, 531, 550, 570, 589, 609, 630, 651, 672, 693, 715,
    737, 759, 782, 805, 828, 851, 875, 899, 923, 948, 973, 998, 1023, 1049, 1074, 1100,
    1127, 1153, 1180, 1207, 1234, 1261, 1289, 1316, 1344, 1372, 1400, 1429, 1457, 1486, 1515,
    1543, 1572, 1602, 1631, 1660, 1690, 1719, 1749, 1778, 1808, 1838, 1868, 1898, 1928, 1958,
    1988, 2018, 2048, 2077, 2107, 2137, 2167, 2197, 2227, 2257, 2287, 2317, 2346, 2376, 2405,
    2435, 2464, 2493, 2523, 2552, 2580, 2609, 2638, 2666, 2695, 2723, 2751, 2779, 2806, 2834,
    2861, 2888, 2915, 2942, 2968, 2995, 3021, 3046, 3072, 3097, 3122, 3147, 3172, 3196, 3220,
    3244, 3267, 3290, 3313, 3336, 3358, 3380, 3402, 3423, 3444, 3465, 3486, 3506, 3525, 3545,
    3564, 3583, 3601, 3619, 3637, 3654, 3672, 3688, 3705, 3721, 3736, 3751, 3766, 3781, 3795,
    3809, 3822, 3836, 3848, 3861, 3873, 3884, 3896, 3907, 3917, 3928, 3937, 3947, 3956, 3965,
    3974, 3982, 3990, 3997, 4004, 4011, 4018, 4024, 4030, 4035, 4041, 4046, 4050, 4055, 4059,
    4063, 4066, 4070, 4073, 4076, 4078, 4081, 4083, 4085, 4086, 4088, 4089, 4091, 4092, 4092,
    4093, 4094, 4094, 4095, 4095, 4095, 4095, 4095, 4095, 4095
};

static inline int16_t grad3(const uint8_t h, const int16_t x, const int16_t y, const int16_t z)
{
    return x * GRAD3[h][0] + y * GRAD3[h][1] + z * GRAD3[h][2];
}

int32_t noise (uint16_t x, uint16_t y, uint16_t z)
{
    uint8_t i0, j0, k0;     // integer part of (x, y, z)
    uint8_t i1, j1, k1;     // integer part plus one of (x, y, z)
    uint8_t xx, yy, zz;     // fractional part of (x, y, z)
    uint16_t fx, fy, fz;    // easing function result, add 4 LS bits

    // drop fractional part of each input
    i0 = x >> 8;
    j0 = y >> 8;
    k0 = z >> 8;

    // integer part plus one, wrapped between 0x00 and 0xff
    i1 = i0 + 1;
    j1 = j0 + 1;
    k1 = k0 + 1;

    // fractional part of each input
    xx = x & 0xff;
    yy = y & 0xff;
    zz = z & 0xff;

    // apply easing function
    fx = easing_function_lut[xx];
    fy = easing_function_lut[yy];
    fz = easing_function_lut[zz];

    uint8_t A, AA, AB, B, BA, BB;
    uint8_t CA, CB, CC, CD, CE, CF, CG, CH;

    // apply permutation functions
    A = PERM[i0];
    AA = PERM[A + j0];
    AB = PERM[A + j1];
    B = PERM[i1];
    BA = PERM[B + j0];
    BB = PERM[B + j1];
    CA = PERM[AA + k0] & 0xf;
    CB = PERM[BA + k0] & 0xf;
    CC = PERM[AB + k0] & 0xf;
    CD = PERM[BB + k0] & 0xf;
    CE = PERM[AA + k1] & 0xf;
    CF = PERM[BA + k1] & 0xf;
    CG = PERM[AB + k1] & 0xf;
    CH = PERM[BB + k1] & 0xf;

    // subtract 1.0 from xx, yy, zz
    int16_t xxm1 = xx - 256;
    int16_t yym1 = yy - 256;
    int16_t zzm1 = zz - 256;

    // result is -2 to exactly +2
    int16_t g1 = grad3 (CA, xx,   yy,   zz  );
    int16_t g2 = grad3 (CB, xxm1, yy,   zz  );
    int16_t g3 = grad3 (CC, xx,   yym1, zz  );
    int16_t g4 = grad3 (CD, xxm1, yym1, zz  );
    int16_t g5 = grad3 (CE, xx,   yy,   zzm1);
    int16_t g6 = grad3 (CF, xxm1, yy,   zzm1);
    int16_t g7 = grad3 (CG, xx,   yym1, zzm1);
    int16_t g8 = grad3 (CH, xxm1, yym1, zzm1);

    // linear interpolations
    int32_t l1 = lerp1(fx, g1, g2) >> 6;
    int32_t l2 = lerp1(fx, g3, g4) >> 6;
    int32_t l3 = lerp1(fx, g5, g6) >> 6;
    int32_t l4 = lerp1(fx, g7, g8) >> 6;

    int32_t l5 = lerp1(fy, l1, l2) >> 12;
    int32_t l6 = lerp1(fy, l3, l4) >> 12;

    int32_t l7 = lerp1(fz, l5, l6) >> 12;

	return l7;
}

