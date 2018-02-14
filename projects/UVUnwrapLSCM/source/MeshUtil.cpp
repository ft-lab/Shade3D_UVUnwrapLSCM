/**
 * メッシュ計算関数など.
 */
#include "MeshUtil.h"

namespace
{
	/**
	 * 多角形の三角形分割クラス.
	 */
	std::vector<int> m_triangleIndex; 

	class CDivideTrianglesOutput : public sxsdk::shade_interface::output_function_class {
	private:
	public:
		virtual void output (int i0 , int i1 , int i2 , int i3) {
			const int offset = m_triangleIndex.size();
			m_triangleIndex.resize(offset + 3);
			m_triangleIndex[offset + 0] = i0;
			m_triangleIndex[offset + 1] = i1;
			m_triangleIndex[offset + 2] = i2;
		}
	};
}

/**
 * 三角形の法線を計算.
 */
sxsdk::vec3 MeshUtil::CalcTriangleNormal (const sxsdk::vec3& v1, const sxsdk::vec3& v2, const sxsdk::vec3& v3)
{
	const sxsdk::vec3 ev1 = v2 - v1;
	const sxsdk::vec3 ev2 = v3 - v2;
	return normalize(sx::product(ev1, ev2));
}

/**
 * 多角形を三角形分割.
 * @param[in]   pointsList   三角形の頂点座標.
 * @param[out]  triIndices   三角形の頂点インデックスが返る (3つで1三角形).
 * @return 三角形数.
 */
int MeshUtil::DivideFaceToTriangles (sxsdk::shade_interface& shade, const std::vector<sxsdk::vec3>& pointsList, std::vector<int>& triIndices)
{
	const int pCou = (int)pointsList.size();
	if (pCou <= 2) return 0;

	::m_triangleIndex.clear();
	::CDivideTrianglesOutput divC;
	shade.divide_polygon(divC, pCou, &(pointsList[0]), true);
	const int triCou = ::m_triangleIndex.size() / 3;

	triIndices.resize(triCou * 3);
	for (int i = 0, iPos = 0; i < triCou; i++, iPos += 3) {
		triIndices[iPos + 0] = ::m_triangleIndex[iPos + 0];
		triIndices[iPos + 1] = ::m_triangleIndex[iPos + 1];
		triIndices[iPos + 2] = ::m_triangleIndex[iPos + 2];
	}
	return triCou;
}

