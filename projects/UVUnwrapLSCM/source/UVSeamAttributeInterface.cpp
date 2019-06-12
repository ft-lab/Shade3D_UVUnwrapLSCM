/**
 * ポリゴンメッシュのSeam情報を取得し、ワイヤーフレーム色を変更.
 */

#include "UVSeamAttributeInterface.h"
#include "UVSeam.h"

CUVSeamAttributeInterface::CUVSeamAttributeInterface (sxsdk::shade_interface &shade) : shade(shade)
{
}

/**
* ワイヤーフレーム描画.
*/
void CUVSeamAttributeInterface::make_wireframe (sxsdk::shape_class &shape, const sxsdk::mat4 &mat, int view, int flags, void *)
{
	if (flags != 0) return;		// ピック処理時はスキップ.
	if (shape.get_type() != sxsdk::enums::polygon_mesh) return;

	// Seamとして採用するエッジ番号を取得.
	std::vector<int> seamEdgeIndices;
	CUVSeam::LoadSeamData(shape, seamEdgeIndices);
	const int eCou = (int)seamEdgeIndices.size();
	if (eCou == 0) return;

	sxsdk::mat4 m = mat;

	try {
		sxsdk::polygon_mesh_class& pMesh = shape.get_polygon_mesh();
		const int versCou  = pMesh.get_total_number_of_control_points();
		const int facesCou = pMesh.get_number_of_faces();
		const int edgesCou = pMesh.get_number_of_edges();
		if (versCou <= 0 || facesCou <= 0 || edgesCou <= 0) return;

		sxsdk::polygon_mesh_saver_class* pMeshSaver = pMesh.get_polygon_mesh_saver();

		sxsdk::vec3 vList[2];

		shade.set_wireframe_color_obsolete(sxsdk::rgb_class(1.0f, 0.5f, 0.0f));
		for (int i = 0; i < eCou; ++i) {
			const int edgeIndex = seamEdgeIndices[i];
			if (edgeIndex < 0 || edgeIndex >= edgesCou) continue;
			sxsdk::edge_class& e = pMesh.edge(edgeIndex);

			const int v0 = e.get_v0();
			const int v1 = e.get_v1();
			vList[0] = pMeshSaver->get_point(v0);
			vList[1] = pMeshSaver->get_point(v1);
			shape.make_line_wireframe(m, 2, vList);
		}

	} catch (...) {}
}

