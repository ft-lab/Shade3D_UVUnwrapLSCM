/**
 * ポリゴンメッシュのエッジで、UVの切れ目を入れる.
 */

#ifndef _UVSEAM_H
#define _UVSEAM_H

#include "GlobalHeader.h"
#include <vector>

/**
 * Seamの操作処理を行う.
 */
class CUVSeam
{
private:
	sxsdk::shade_interface& shade;

public:
	CUVSeam (sxsdk::shade_interface& shade);

	/**
	* 選択形状の属性として、Seam情報を保存.
	*/
	static void SaveSeamData (sxsdk::shape_class& shape, std::vector<int>& seamEdgeIndices);

	/**
	* 選択形状の属性として、Seam情報を読み込み.
	*/
	static void LoadSeamData (sxsdk::shape_class& shape, std::vector<int>& seamEdgeIndices);

	/*
	 * 指定のポリゴンメッシュの選択エッジをSeamとして追加. 
	 */
	void AddSeamActiveEdges (sxsdk::shape_class& shape);

	/*
	 * 指定のポリゴンメッシュの選択エッジからSeamをクリア.
	 */
	void RemoveSeamActiveEdges (sxsdk::shape_class& shape);

	/**
	 * 指定のポリゴンメッシュのすべてのSeamをクリア. 
	 */
	void RemoveAllSeam (sxsdk::shape_class& shape);
};

#endif
