/**
 * メッシュ計算関数など.
 */
#ifndef _MESHUTIL_H
#define _MESHUTIL_H

#include "GlobalHeader.h"

namespace MeshUtil
{
	/**
	 * 三角形の法線を計算.
	 */
	sxsdk::vec3 CalcTriangleNormal (const sxsdk::vec3& v1, const sxsdk::vec3& v2, const sxsdk::vec3& v3);

	/**
	 * 多角形を三角形分割.
	 * @param[in]   pointsList   三角形の頂点座標.
	 * @param[out]  triIndices   三角形の頂点インデックスが返る (3つで1三角形).
	 * @return 三角形数.
	 */
	int DivideFaceToTriangles (sxsdk::shade_interface& shade, const std::vector<sxsdk::vec3>& pointsList, std::vector<int>& triIndices);
}

#endif
