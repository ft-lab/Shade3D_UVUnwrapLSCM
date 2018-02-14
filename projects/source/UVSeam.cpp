/**
 * ポリゴンメッシュのエッジで、UVの切れ目を入れる.
 */

#include "UVSeam.h"

CUVSeam::CUVSeam (sxsdk::shade_interface& shade) : shade(shade)
{
}

/**
 * 選択形状の属性として、Seam情報を保存.
 */
void CUVSeam::SaveSeamData (sxsdk::shape_class& shape, std::vector<int>& seamEdgeIndices)
{
	try {
		compointer<sxsdk::stream_interface> stream(shape.create_attribute_stream_interface_with_uuid(UV_SEAM_ATTRIBUTE_ID, UV_SEAM_ATTRIBUTE_ID));
		if (!stream) return;
		stream->set_pointer(0);
		stream->set_size(0);

		int iVersion = UV_SEAM_STREAM_VERSION;
		stream->write_int(iVersion);

		const int eCou = (int)seamEdgeIndices.size();
		stream->write_int(eCou);
		for (int i = 0; i < eCou; ++i) {
			stream->write_int(seamEdgeIndices[i]);
		}

	} catch (...) {}
}

/**
 * 選択形状の属性として、Seam情報を読み込み.
 */
void CUVSeam::LoadSeamData (sxsdk::shape_class& shape, std::vector<int>& seamEdgeIndices)
{
	seamEdgeIndices.clear();
	try {
		compointer<sxsdk::stream_interface> stream(shape.get_attribute_stream_interface_with_uuid(UV_SEAM_ATTRIBUTE_ID));
		if (!stream) return;
		stream->set_pointer(0);

		int iVersion;
		stream->read_int(iVersion);

		int eCou = 0;
		stream->read_int(eCou);
		if (eCou > 0) seamEdgeIndices.resize(eCou);

		for (int i = 0; i < eCou; ++i) {
			stream->read_int(seamEdgeIndices[i]);
		}
	}
	catch (...) {}
}

/*
* 指定のポリゴンメッシュの選択エッジをSeamとして追加.
*/
void CUVSeam::AddSeamActiveEdges (sxsdk::shape_class& shape)
{
	if (shape.get_type() != sxsdk::enums::polygon_mesh) return;

	// 保持されているSeam情報を取得.
	std::vector<int> seamEdgeIndices;
	CUVSeam::LoadSeamData(shape, seamEdgeIndices);

	try {
		sxsdk::polygon_mesh_class& pMesh = shape.get_polygon_mesh();
		const int versCou  = pMesh.get_total_number_of_control_points();
		const int facesCou = pMesh.get_number_of_faces();
		const int edgesCou = pMesh.get_number_of_edges();
		if (versCou <= 0 || facesCou <= 0 || edgesCou <= 0) return;

		std::vector<int> edgeIndices;
		edgeIndices.resize(edgesCou, -1);
		const int eCou = (int)seamEdgeIndices.size();
		for (int i = 0; i < eCou; ++i) {
			const int edgeIndex = seamEdgeIndices[i];
			if (edgeIndex >= 0 && edgeIndex < edgesCou) {
				edgeIndices[edgeIndex] = 1;
			}
		}

		for (int i = 0; i < edgesCou; ++i) {
			sxsdk::edge_class& e = pMesh.edge(i);
			if (e.get_active()) edgeIndices[i] = 1;
		}

		seamEdgeIndices.clear();
		for (int i = 0; i < edgesCou; ++i) {
			if (edgeIndices[i] > 0) seamEdgeIndices.push_back(i);
		}

		// 加算したエッジのアクティブ状態をstreamに保存.
		CUVSeam::SaveSeamData(shape, seamEdgeIndices);

	} catch (...) { }
}

/*
* 指定のポリゴンメッシュの選択エッジからSeamをクリア.
*/
void CUVSeam::RemoveSeamActiveEdges (sxsdk::shape_class& shape)
{
	if (shape.get_type() != sxsdk::enums::polygon_mesh) return;

	// 保持されているSeam情報を取得.
	std::vector<int> seamEdgeIndices;
	CUVSeam::LoadSeamData(shape, seamEdgeIndices);

	try {
		sxsdk::polygon_mesh_class& pMesh = shape.get_polygon_mesh();
		const int versCou = pMesh.get_total_number_of_control_points();
		const int facesCou = pMesh.get_number_of_faces();
		const int edgesCou = pMesh.get_number_of_edges();
		if (versCou <= 0 || facesCou <= 0 || edgesCou <= 0) return;

		std::vector<int> edgeIndices;
		edgeIndices.resize(edgesCou, -1);
		const int eCou = (int)seamEdgeIndices.size();
		for (int i = 0; i < eCou; ++i) {
			const int edgeIndex = seamEdgeIndices[i];
			if (edgeIndex >= 0 && edgeIndex < edgesCou) {
				edgeIndices[edgeIndex] = 1;
			}
		}

		for (int i = 0; i < edgesCou; ++i) {
			sxsdk::edge_class& e = pMesh.edge(i);
			if (e.get_active()) edgeIndices[i] = -1;
		}

		seamEdgeIndices.clear();
		for (int i = 0; i < edgesCou; ++i) {
			if (edgeIndices[i] > 0) seamEdgeIndices.push_back(i);
		}

		// エッジのアクティブ状態をstreamに保存.
		CUVSeam::SaveSeamData(shape, seamEdgeIndices);

	}
	catch (...) {}
}

/**
* 指定のポリゴンメッシュのすべてのSeamをクリア.
*/
void CUVSeam::RemoveAllSeam (sxsdk::shape_class& shape)
{
	if (shape.get_type() != sxsdk::enums::polygon_mesh) return;

	// エッジの選択なしとして保存.
	std::vector<int> seamEdgeIndices;
	CUVSeam::SaveSeamData(shape, seamEdgeIndices);
}
