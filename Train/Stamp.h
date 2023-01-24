#pragma once
#ifndef STAMP_H
#define STAMP_H

#ifndef MATH_H
#include "../Base/Math.h"
#endif
#ifndef CTARGAIMAGE_H
#include "../FileIO/CTargaImage.h"
#endif
#ifndef PARSETXT_H
#include "../FileIO/ParseTxt.h"
#endif
#ifndef IMG_H
#include "../Base/Img.h"
#endif

//#define CTARGAIMAGE_IMGFILEPRE "img"
//#define CTARGAIMAGE_IMGFILESUF ".tga"
#define STAMP_DIR "../dDump"
#define STAMP_ROUNDCORN_DIR "Raw"
#define STAMP_KEY "stampkey.txt"/*key file contains information about each numbered image stamp*/
#define STAMP_MASTER_KEY "stampskey.txt"/*master key file that tells info like how many files are there
                                    has form numfiles, num no-rot sig, num back, num rot sig*/
const float stamp_zero_intensity = 0.f;
const float stamp_max_ang_rad = 3.1f;

const int number_of_stamps_per_shape = 1;/*this number changes depending on whether stamps solid or stamps with holes is used */

using namespace std;

struct s_stampsKey {
	float r;/*scale of small r for the img, the base hex size*/
	float Dim;/*dim of the full size of the stamp*/
	float maxDim;/*max size of image, used when image is slid by offset to make sure image is still large enough to account for slide*/

	int N;
	int N_sig;
	int N_bak;
	int N_pre_sig;
	int N_sig_bak_rot;/*number of times each signal should be rotated off to become background*/
	float min_sig_bak_rotang;/*min angle in rad that signal must rotate away to be considere background*/
	float sig_bak_rotang_jitter;/*span in rad the rotated backround signal can miss the exact an rot target*/
	int N_bak_smudge;
	int N_sig_smudge;/*if zero then signal is equalized to the background*/

	float y_sig_bak_rot;

	float smudge_sigma_divisor;/*number like 2 that divides r for the sigma of the gaussian that determines the offset*/
};
struct s_stampKey {
	int   ID;/*stamp number, matches number of image file*/
	float ang;/*angle of line perp to stamp edge vs line directly pointing left, or just reference angle*/
	float y;/* target value for this stamp*/

	float preRot;/*either flag 1 if needs rotation 0 if fixed, or angle img needs to be rotated by before it is processed as stamap*/
	s_2pt offset;

	int train;/*if 0 then drop from training set don't use as signal or background*/
};
struct s_rCornKey {
	s_stampKey key;
	float R;/*radius of rounded corner, negative for inverse*/
	float opening_ang;/*opening ang for rounded corner*/
};
namespace n_stampsKey {
	const int len = 14;
	void clear(s_stampsKey& key);
	unsigned char dumpToDatLine(const s_stampsKey& key, s_datLine& dl);
	int datLineToKey(const s_datLine& dl, s_stampsKey& key);
}
namespace n_stampKey {
	const int len = 6;
	void clear(s_stampKey& key);
	void copy(s_stampKey& key, const s_stampKey& orig);
	unsigned char dumpToDatLine(const s_stampKey& key, s_datLine& dl);
    int datLineToKey(const s_datLine& dl, s_stampKey& key);
}
namespace n_rCornKey {
	const int len = 2;
	void clear(s_rCornKey& key);
	unsigned char dumpToDatLine(const s_rCornKey& key, s_datLine& dl);
	int datLineToKey(const s_datLine& dl, s_rCornKey& key);
}

/* generates the stamp images that represent the aftermath of the col plate*/
class Stamp : public Base {
public:
	Stamp();
	~Stamp();

	unsigned char init(
		float imgDim,
		float scale_r, /*radius that sets the overall scale, should be set to the base radus that the luna pattern is ran on*/
		float openingAngDivisor = 2.f, /*divids PI to get opening angle*/
		float radCmul = 2.f,/*multiplicative factor that increases the circle radius each cycle*/
		float numAngDiv = 12.f,
		float numCircleRadii = 7.f,
		float minCircleRadSF = 2.f,
		float maxFinalOpeningAng= 1.1f,
		int   numfinalOpeningAngs = 3,
		float thickness_in_2Runits=2.f,
		float falloff_scale_factor_2Runits=1.f,
		float gaussSigma_in_thicknessUnits=1.f
		);
	void          release();

	unsigned char run();
protected:
	/*owned helper classes*/
	CTargaImage* m_tgaIO;
	ParseTxt* m_parse;

	/*init parameters*/
	/*inital parameters tht define the corners*/
	float m_cornerOpeningAng;/*PI/openingAngDivisor*/
	float m_radCmul;/*multiplies circle radius to increase it */
	float m_numAngDiv;
	float m_numCircleRadii;
	float m_minCircleRadius;
	/*parameters used by the final series of angles that go down to a rounded line*/
	float m_maxFinalOpeningAng;
	int   m_NFinalOpeningAngs;
	/* used for the corners with hole */
	float m_thickness;
	float m_falloffScale;
	float m_gaussSigma;

	string m_imgFile;/*this will update as each img is written*/

	float       m_imgDim;
	/*owned*/
	Img**       m_stampImgs;
	s_rCornKey* m_Keys;/*this has length m_max_total_number_of_stamps */
	int         m_stampN;
	s_2pt&      m_stampImgCenter;
	s_stampsKey m_masterStampsKey;

	/*working scratch*/
	s_2pt m_UBasis0;
	s_2pt m_UBasis1;
	s_2pt m_UrevBasis0;/*vectors that determine the rotation of the curent pattern*/
	s_2pt m_UrevBasis1;
	s_2pt m_circle_center;
	float m_circle_radius;
	float m_max_circle_radius; 
	s_2pt m_line_intersect;
	s_2pt m_circle_half_pt;/*point in the x direction that is the x coord of the line intercepts*/
	s_2pt m_Uline_perp1;/*points perp to line 1 into region beetween lines*/
	s_2pt m_Uline_perp2;/* "                  2         "                 */
	s_2pt m_UcenterIn;/*points inward from center of where lines would intersect, should be set to 1,0*/
	/*calculated during init*/
	float m_n_DAng;
	int   m_n_ang;
	int   m_n_circleRadii;
	int   m_max_total_num_of_stamps;/*this is the length of the keys array in memory*/

	bool  m_cosFalloff;
	bool  m_linearFalloff;
	bool  m_gaussFalloff;
	bool  m_sharpFalloff;

	/*init/release helpers*/
	unsigned char calcNumOfStamps();/* uses m_NumAngDiv to calculate m_DAng m_n_ang and m_n_circleRadii, also uses m_NFinalOpeningAngs*/
	unsigned char initParse();
	void          releaseParse();
	unsigned char initStampKeys();/*requires calcNumOfStamps to have been run*/
	void          releaseStampKeys();
	/*            */

	unsigned char stampRoundedCornerImgs();
	unsigned char dumpStampImgs();/*run after stampRoundedCorner, dumps images of stamps, with name CTARGAIMAGE_IMGFILEPRE ID CTARGAIMAGE_IMGFILESUF*/
	unsigned char dumpStampKeys();/*run after stamp RoundedCorner Images dumps the keys for each stamp
								     keys dumps the data from s_stampKey first */

	bool stampsSolid(const s_2pt& center, float ang, float circle_scale, float opening_ang);/*stamp solid stamp*/
	bool stampsWHoles(const s_2pt& center, float ang, float circle_scale, float opening_ang);/*either use this function or stampsSolid*/
	bool stampFinalAngSpread(const s_2pt& center, float ang, float circle_scale, float opening_ang_start);

	bool stampImg(const s_2pt& center, float ang, float circle_rad, float opening_ang);
	/*math functions*/
	unsigned char setBasisFromAng(float ang);
	unsigned char setRoundedCorner(const s_2pt& center/*center of rounded middle, in middle of edge*/, float radius, float ang_rad);

	bool isInCircle(const s_2pt& pt) { return vecMath::inCircle(m_circle_radius, m_circle_center, pt); }
	bool isUnderLine(const s_2pt& pt, const s_2pt& Uline_perp);
	bool isInsideCurveHalf(const s_2pt& pt);/*is inside vs the center of the circle*/

	float distFromRoundedCorner(const s_2pt& pt);
	/*helpers to distFromRoundedCorner*/
	inline float distFromLine(const s_2pt& pt, const s_2pt& Uline_perp) { return vecMath::distLineGivenPerp(m_line_intersect, Uline_perp, pt); }
	float distFromClosestLine(const s_2pt& pt);
	inline float distFromCircle(const s_2pt& pt) { return -vecMath::distCircleArc(m_circle_radius, m_circle_center, pt); }

	/*image functions*/
	float stampIntensity(const s_2pt& pt);
	bool  isInRoundedCornerNoRot(const s_2pt& pt);
	float RoundedCornerIntensityNoRot(const s_2pt& pt);

	/*render functions*/
	unsigned char renderStampToImg(Img* img);
	bool stampCoordToImgCoord(Img* img, const s_2pt& pt, s_2pt_i& img_pt);
};
#endif

