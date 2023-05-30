#include "HexEyeImg.h"

unsigned char HexEyeImg::init(Img* baseImg, s_2pt& eye_center, HexEye* genHexEye,
	float sigmaVsR,
	float IMaskRVsR) {
	if (baseImg == NULL)
		return ECODE_FAIL;
	if (baseImg->getWidth() <= 1)
		return ECODE_FAIL;
	if (baseImg->getHeight() <= 1)
		return ECODE_FAIL;
	m_refHexEye = new s_HexEye;
	if(Err(genHexEye->spawn(m_refHexEye)))
		return ECODE_FAIL;
	long num_nodes_set = 0;
	for (int i_level = 0; i_level < m_refHexEye->N; i_level++) {
		s_HexPlate* lev_plate = m_refHexEye->get(i_level);
		for (long i_node = 0; i_node < lev_plate->N; i_node++) {
			s_Hex* hex_node = lev_plate->get(i_node);
			unsigned char err = resetNodeToImgCoord(baseImg, eye_center, hex_node);
			if (IsErrFail(err))
				return err;
			if (RetOk(err))
				num_nodes_set++;
		}
	}
	if (num_nodes_set < 1)
		return ECODE_ABORT;/*the eye is off the image*/
	m_Convol = new ConvolHex;
	if (Err(m_Convol->init(baseImg, m_hexEye->getBottomR())))
		return ECODE_FAIL;
	return ECODE_OK;
}

unsigned char HexEyeImg::resetNodeToImgCoord(Img* baseImg, s_2pt& eye_center, s_Hex* hex_nd) {
	if (hex_nd == NULL)
		return ECODE_FAIL;
	/*get the offsets vs the center of the eye which is what genHexEye originally puts in the node*/
	float offset_x = hex_nd->x;
	float offset_y = hex_nd->y;
	float x=offset_x+eye_center.x0;
	float y = offset_y + eye_center.x1;
	/*reset the nodes xy to the image coordinate instead of the offset coordinates*/
	hex_nd->x = x;
	hex_nd->y = y;
	long _i = (long)lroundf(x);
	long _j = (long)lroundf(y);
	if (_i < 0 || _i>=baseImg->getWidth())
		_i = -1;
	if (_j < 0 || _j>=baseImg->getHeight())
		_j = -1;
	unsigned char retcode = ECODE_OK;
	if (_i < 0 || _j < 0) {/*check if node is off the image*/
		_i = -1; _j = -1;
		retcode = ECODE_ABORT;
	}
	hex_nd->i = _i;
	hex_nd->j = _j;
	return retcode;
}
unsigned char HexEyeImg::runSingleThread(Img* baseImg, s_HexEye& heye) {
	s_HexPlate* eyeBase = heye.getBottom();
	s_convKernVars kernVars;
	n_HexEyeImg::fillStruct(*baseImg, *m_Convol, *eyeBase, kernVars);
	for (long i = 0; i < kernVars.num_Hex; i++) {
		kernVars.hex_index = i;
		threadedConvol::convCellKernel(kernVars);/*if the indexes of the current hex (ij) are either less than 0 this just returns*/
	}
	return ECODE_OK;
}

void n_HexEyeImg::fillStruct(Img& inimg, ConvolHex& genConvol, s_HexPlate& s_botPlate, s_convKernVars& IOVars) {
	IOVars.img_pix = inimg.getImg();
	IOVars.Img_height = inimg.getHeight();
	IOVars.Img_width = inimg.getWidth();
	IOVars.Img_bpp = inimg.getColorMode();
	IOVars.Img_maxIndex = inimg.getMaxIndex();

	IOVars.mask_pix = genConvol.getMaskF();
	IOVars.MaskBL_offsetX = genConvol.getIMaskBL_offset().x0;
	IOVars.MaskBL_offsetY = genConvol.getIMaskBL_offset().x1;
	IOVars.Mask_height = genConvol.getMaskHeight();
	IOVars.Mask_width = genConvol.getMaskWidth();

	IOVars.hex_index = 0;

	IOVars.num_Hex = s_botPlate.N;
	IOVars.outHex = s_botPlate.getNodes();
}