/**
 * ポリゴンメッシュのSeam情報を取得し、ワイヤーフレーム色を変更.
 */

#ifndef _UVSAEMATTRIBUTEINTERFACE_H
#define _UVSAEMATTRIBUTEINTERFACE_H

#include "GlobalHeader.h"

class CUVSeamAttributeInterface : public sxsdk::attribute_interface
{
private:
	sxsdk::shade_interface &shade;

	virtual int get_shade_version () const { return SHADE_BUILD_NUMBER; }
	virtual sx::uuid_class get_uuid (void * = 0) { return UV_SEAM_ATTRIBUTE_ID; }

	virtual void accepts_shape (bool &accept, void *) { accept = false; }

	/**
	 * ワイヤーフレーム描画.
	 */
	virtual void make_wireframe (sxsdk::shape_class &shape, const sxsdk::mat4 &mat, int view, int flags, void *aux = 0);

public:
	CUVSeamAttributeInterface (sxsdk::shade_interface &shade);

};

#endif

