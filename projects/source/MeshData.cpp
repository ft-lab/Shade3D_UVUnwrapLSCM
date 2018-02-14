/**
 * メッシュデータ.
 */

#include "MeshData.h"
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

CMeshData::CMeshData (sxsdk::shade_interface& shade) : shade(shade)
{
	Clear();
}

void CMeshData::Clear ()
{
	vertices.clear();
	triangles.clear();
	m_groupCount = 0;
}

/**
 * 指定の形状を格納.
 * @param[in] shape       対象のポリゴンメッシュ形状.
 * @param[in] allFaces    全ての面を展開する場合はtrue.
 */
bool CMeshData::StoreMesh (sxsdk::shape_class& shape, const bool allFaces)
{
	Clear();

	if (shape.get_type() != sxsdk::enums::polygon_mesh) return false;
	try {
		sxsdk::polygon_mesh_class& pMesh = shape.get_polygon_mesh();
		const int versCou  = pMesh.get_total_number_of_control_points();
		const int facesCou = pMesh.get_number_of_faces();
		if (versCou <= 0 || facesCou <= 0) return false;

		// 頂点を保持.
		sxsdk::polygon_mesh_saver_class* pMeshSaver = pMesh.get_polygon_mesh_saver();
		vertices.resize(versCou);
		for (int i = 0; i < versCou; ++i) {
			vertices[i].pos    = pMeshSaver->get_point(i);
			vertices[i].locked = false;
			vertices[i].uv     = sxsdk::vec2(0, 0);
		}

		// 面を三角形分割して保持.
		std::vector<sxsdk::vec3> versList;
		std::vector<int> indices;
		std::vector<int> triIndices;
		CMeshTriangleData triData;
		for (int i = 0; i < facesCou; ++i) {
			sxsdk::face_class& f = pMesh.face(i);
			if (!allFaces) {
				if (!f.get_active()) continue;
			}
			const int vCou = f.get_number_of_vertices();
			if (vCou <= 2) continue;

			indices.resize(vCou);
			f.get_vertex_indices(&(indices[0]));
			versList.resize(vCou);
			for (int j = 0; j < vCou; ++j) {
				versList[j] = vertices[ indices[j] ].pos;
			}

			// 三角形分割.
			const int triCou = MeshUtil::DivideFaceToTriangles(shade, versList, triIndices);
			if (triCou == 0) continue;

			// 三角形情報を格納.
			for (int j = 0, iPos = 0; j < triCou; ++j, iPos += 3) {
				for (int k = 0; k < 3; ++k) {
					triData.tri[k].orgFaceVIndex = triIndices[iPos + k];
					triData.tri[k].vIndex        = indices[triData.tri[k].orgFaceVIndex];
					triData.tri[k].orgVIndex     = triData.tri[k].vIndex;
				}
				triData.orgFaceIndex = i;
				triData.groupID      = -1;
				triangles.push_back(triData);
			}
		}
		return true;

	} catch (...) { }

	return false;
}

/**
 * 2つの三角形の頂点インデックスより、エッジの共有がある場合のエッジの2頂点を取得.
 * @param[in]  triIndex1  三角形の頂点インデックス1.
 * @param[in]  triIndex2  三角形の頂点インデックス2.
 * @param[out] edgeV      共有エッジを持つ場合はそのエッジの2頂点のインデックスを返す.
 * @return 共有のエッジがある場合はtrue.
 */
bool CMeshData::m_ChkTrianglesShareEdge (int* triIndex1, int* triIndex2, sx::vec<int,2>& edgeV)
{
	bool ret = false;
	for (int i = 0; i < 3; ++i) {
		int v0 = triIndex1[i];
		int v1 = triIndex1[(i + 1) % 3];
		if (v0 > v1) std::swap(v0, v1);

		for (int j = 0; j < 3; ++j) {
			int v0_2 = triIndex2[j];
			int v1_2 = triIndex2[(j + 1) % 3];
			if (v0_2 > v1_2) std::swap(v0_2, v1_2);
			if (v0 == v0_2 && v1 == v1_2) {
				ret = true;
				edgeV[0] = v0;
				edgeV[1] = v1;
				break;
			}
		}
		if (ret) break;
	}
	return ret;
}

/**
 * 頂点ごとが共有する面番号を一時的に保持。m_versTriIndexListに蓄える.
 */
void CMeshData::m_BeginVerticesTriIndexList (const bool useOrgVertex)
{
	m_versTriIndexList.clear();

	const int versCou = (int)vertices.size();
	const int triCou  = (int)triangles.size();
	m_versTriIndexList.resize(versCou);
	for (int i = 0; i < versCou; ++i) m_versTriIndexList[i].clear();

	for (int i = 0; i < triCou; ++i) {
		const CMeshTriangleData& triD = triangles[i];
		for (int j = 0; j < 3; ++j) {
			if (useOrgVertex) {
				m_versTriIndexList[ triD.tri[j].orgVIndex ].push_back(i);
			} else {
				m_versTriIndexList[ triD.tri[j].vIndex ].push_back(i);
			}
		}
	}
}

/**
 * 頂点ごとが共有する面番号を一時的に保持する処理を終了.
 */
void CMeshData::m_EndVerticesTriIndexList ()
{
	m_versTriIndexList.clear();
}

/**
 * Seamのエッジリストより、頂点番号の組み合わせをマップ。m_seamEdgeIndexMapに格納される.
 */
void CMeshData::m_MapSeamEdgeIndex (sxsdk::shape_class& shape, const std::vector<int>& seamEdgeIndices)
{
	const int eCou    = (int)seamEdgeIndices.size();
	sxsdk::polygon_mesh_class& pMesh = shape.get_polygon_mesh();

	// エッジの頂点インデックスに対するSeam番号をマップ.
	m_seamEdgeIndexMap.clear();
	SIndex2 edgeV;
	for (int i = 0; i < eCou; ++i) {
		const int edgeIndex = seamEdgeIndices[i];
		const sxsdk::edge_class& e = pMesh.edge(edgeIndex);
		edgeV[0] = e.get_v0();
		edgeV[1] = e.get_v1();
		if (edgeV[0] > edgeV[1]) std::swap(edgeV[0], edgeV[1]);
		m_seamEdgeIndexMap[edgeV] = i;
	}
}

/**
 * Seamを区切りにして面ごとにグループ番号を割り当てる.
 */
void CMeshData::m_SetGroupID (sxsdk::shape_class& shape, const std::vector<int>& seamEdgeIndices)
{
	m_groupCount = 0;
	const int eCou    = (int)seamEdgeIndices.size();
	const int versCou = (int)vertices.size();
	const int triCou  = (int)triangles.size();
	for (int i = 0; i < triCou; ++i) triangles[i].groupID = -1;

	// 頂点に対する三角形番号の共有を格納.
	m_BeginVerticesTriIndexList();

	sxsdk::polygon_mesh_class& pMesh = shape.get_polygon_mesh();

	// 面ごとに隣接をたどり同一グループ番号を割り当て.
	std::vector<int> triIList;
	int triIndexA[3], triIndexB[3];
	int groupID = 0;
	sx::vec<int,2> edgeIV;
	for (int i = 0, iPos = 0; i < triCou; ++i, iPos += 3) {
		if (triangles[i].groupID >= 0) continue;
		triIList.push_back(i);

		while (!triIList.empty()) {
			const int triIndex  = triIList.back();
			triIList.pop_back();

			CMeshTriangleData& triD = triangles[triIndex];
			CMeshTriangleVertexData& triV0 = triD.tri[0];
			CMeshTriangleVertexData& triV1 = triD.tri[1];
			CMeshTriangleVertexData& triV2 = triD.tri[2];
			triD.groupID = groupID;
			triIndexA[0] = triV0.vIndex;
			triIndexA[1] = triV1.vIndex;
			triIndexA[2] = triV2.vIndex;

			// 三角形の頂点に隣接する三角形より、エッジを共有するものを隣接面としてたどる.
			for (int j = 0; j < 3; ++j) {
				const CMeshTriangleVertexData& triV = triD.tri[j];
				const std::vector<int>& faceI = m_versTriIndexList[triV.vIndex];
				const int fCou = (int)faceI.size();
				for (int k = 0; k < fCou; ++k) {
					const int triIndex_2 = faceI[k];
					CMeshTriangleData& triD2 = triangles[triIndex_2];
					if (triIndex_2 == i || triD2.groupID >= 0) continue;
					triIndexB[0] = triD2.tri[0].vIndex;
					triIndexB[1] = triD2.tri[1].vIndex;
					triIndexB[2] = triD2.tri[2].vIndex;

					// triIndexA[], triIndexB[]の三角形で共有エッジを持つかどうか.
					if (!m_ChkTrianglesShareEdge(triIndexA, triIndexB, edgeIV)) continue;

					// Seamのエッジでさえぎられる場合はスキップ.
					if (m_seamEdgeIndexMap.count(SIndex2(edgeIV[0], edgeIV[1])) != 0) continue;

					triIList.push_back(triIndex_2);
				}
			}
		}
		groupID++;
	}
	m_groupCount = groupID;

	m_EndVerticesTriIndexList();
}

/**
 * エッジの2頂点を持つ面を取得。m_versTriIndexListを参照する.
 * @param[in]  edgeP0, edgeP1  エッジの2頂点のインデックス.
 * @param[out] triList         三角形番号が返る.
 * @param[in]  forwardOnly     edgeP0 - edgeP1の順方向のみチェックする場合はtrue.
 */
bool CMeshData::m_FindTriangleFromEdgeIndex (const int edgeP0, const int edgeP1, std::vector<int>& triList, const bool forwardOnly)
{
	const std::vector<int>& vTriPList = m_versTriIndexList[edgeP0];

	triList.clear();
	for (size_t i = 0; i < vTriPList.size(); ++i) {
		const int triIndex = vTriPList[i];
		const CMeshTriangleData& triD = triangles[triIndex];
		bool chkF = false;
		for (int j = 0; j < 3; ++j) {
			const int v0 = triD.tri[j].orgVIndex;
			const int v1 = triD.tri[(j + 1) % 3].orgVIndex;

			if (forwardOnly) {
				if (v0 == edgeP0 && v1 == edgeP1) {
					chkF = true;
					break;
				}
			} else {
				if ((v0 == edgeP0 && v1 == edgeP1) || (v0 == edgeP1 && v1 == edgeP0)) {
					chkF = true;
					break;
				}
			}
		}
		if (chkF) {
			triList.push_back(triIndex);
		}
	}

	return !triList.empty();
}

/**
 * 指定の頂点を共有する三角形を取得。m_versTriIndexListを参照する.
 * @param[in]  pMesh     Shade3Dでのメッシュクラス.
 * @param[in]  pIndex    頂点インデックス.
 * @param[in]  groupID   グループID.
 * @param[out] triList   三角形番号が返る.
 */
int CMeshData::m_FindTriangleFromPoint (const int pIndex, const int groupID, std::vector<int>& triList)
{
	const std::vector<int>& vTriPList = m_versTriIndexList[pIndex];

	triList.clear();
	for (size_t i = 0; i < vTriPList.size(); ++i) {
		const int triIndex = vTriPList[i];
		const CMeshTriangleData& triD = triangles[triIndex];
		if (triD.groupID != groupID) continue;
		triList.push_back(triIndex);
	}
	return (int)triList.size();
}

/**
 * 指定の頂点を共有するエッジを取得。m_versTriIndexListを参照する.
 * @param[in]  pIndex    頂点インデックス.
 * @param[in]  groupID   グループID.
 * @param[out] edgesList エッジ情報が入る.
 */
int CMeshData::m_FindEdgesFromPoint (sxsdk::polygon_mesh_class& pMesh, const int pIndex, const int groupID, std::vector<SIndex2>& edgesList)
{
	edgesList.clear();

	// 頂点pIndexに接続される三角形を取得.
	std::vector<int> triList;
	const int triCou = m_FindTriangleFromPoint(pIndex, groupID, triList);
	if (triCou == 0) return 0;

	// 三角形情報から、オリジナルの面リストを作成.
	std::vector<int> faceIndexList;
	for (int i = 0; i < triCou; ++i) {
		const CMeshTriangleData& triD = triangles[ triList[i] ];
		if (std::find(faceIndexList.begin(), faceIndexList.end(), triD.orgFaceIndex) == faceIndexList.end()) {
			faceIndexList.push_back(triD.orgFaceIndex);
		}
	}

	// 頂点番号がpIndexの場所を探し、それを共有するエッジを格納.
	std::vector<int> indices;
	std::vector<int > triEList;
	SIndex2 edgeI;
	for (size_t i = 0; i < faceIndexList.size(); ++i) {
		sxsdk::face_class& f = pMesh.face(faceIndexList[i]);
		const int vCou = f.get_number_of_vertices();
		indices.resize(vCou);
		f.get_vertex_indices(&(indices[0]));	// i番目の面の頂点インデックスを取得.

		auto iter = std::find(indices.begin(), indices.end(), pIndex);
		const int iPos = (int)std::distance(indices.begin(), iter);
		if (iPos == vCou) continue;
		edgeI[0] = pIndex;
		edgeI[1] = indices[(iPos + 1) % vCou];
		if (edgeI[0] > edgeI[1]) std::swap(edgeI[0], edgeI[1]);
		if (std::find(edgesList.begin(), edgesList.end(), edgeI) == edgesList.end()) {
			// edgeI[0]-edgeI[1]のエッジを挟む三角形を取得.
			if (m_FindTriangleFromEdgeIndex(edgeI[0], edgeI[1], triEList) && triEList.size() == 2) {
				// 三角形が2つともgroupIDに属するか.
				if (triangles[ triEList[0] ].groupID == groupID && triangles[ triEList[1] ].groupID == groupID) {
					edgesList.push_back(edgeI);
				}
			}
		}

		if (vCou >= 3) {
			edgeI[0] = pIndex;
			edgeI[1] = indices[(iPos - 1 + vCou) % vCou];
			if (edgeI[0] > edgeI[1]) std::swap(edgeI[0], edgeI[1]);
			if (std::find(edgesList.begin(), edgesList.end(), edgeI) == edgesList.end()) {
				// edgeI[0]-edgeI[1]のエッジを挟む三角形を取得.
				if (m_FindTriangleFromEdgeIndex(edgeI[0], edgeI[1], triEList) && triEList.size() == 2) {
					// 三角形が2つともgroupIDに属するか.
					if (triangles[ triEList[0] ].groupID == groupID && triangles[ triEList[1] ].groupID == groupID) {
						edgesList.push_back(edgeI);
					}
				}
			}
		}
	}
	return (int)edgesList.size();
}

/**
 * 同一グループ内でSeamでの分割がある場合、頂点を分離.
 */
void CMeshData::m_DividePointsInSameGroup (sxsdk::shape_class& shape, const std::vector<int>& seamEdgeIndices)
{
	const float fMin = (float)(1e-4);
	const int versCou = (int)vertices.size();
	const int triCou  = (int)triangles.size();
	if (versCou == 0 || triCou == 0 || m_groupCount == 0) return;

	const int eCou = (int)seamEdgeIndices.size();
	if (eCou == 0) return;

	// 頂点に対する三角形番号の共有を格納。オリジナルの頂点番号で検索できるようにする.
	m_BeginVerticesTriIndexList(true);

	sxsdk::polygon_mesh_class& pMesh = shape.get_polygon_mesh();

	// グループごとに、Seamで三角形が分断されるか調べる.
	std::vector< std::vector<int> > seamGroupList;
	SIndex2 edgeI;
	std::vector<int> triList;
	std::vector<int> cTriList;
	CMeshVertexData vData;
	for (int groupID = 0; groupID < m_groupCount; ++groupID) {
		std::vector<SIndex2> gSeamList;
		for (int i = 0; i < triCou; ++i) {
			const CMeshTriangleData& triD = triangles[i];
			if (triD.groupID != groupID) continue;

			bool chkTriF = false;
			for (int j = 0; j < 3; ++j) {
				const int i0 = triD.tri[j].orgVIndex;
				const int i1 = triD.tri[(j + 1) % 3].orgVIndex;
				edgeI[0] = i0;
				edgeI[1] = i1;
				if (edgeI[0] > edgeI[1]) std::swap(edgeI[0], edgeI[1]);
				if (m_seamEdgeIndexMap.count(edgeI) == 0) continue;

				// i0-i1のエッジを持つ三角形をリストアップ.
				if (!m_FindTriangleFromEdgeIndex(i0, i1, triList)) continue;
				if (triList.size() != 2) continue;
				if (triList[0] != i && triList[1] != i) continue;

				// i0-i1のエッジをはさむ、もう片方の三角形番号.
				const int aTriIndex = (triList[0] == i) ? triList[1] : triList[0];

				// aTriIndexの三角形がgroupIDとは異なる場合は、別のグループなのでスキップ.
				if (triangles[aTriIndex].groupID != groupID) continue;

				if (std::find(gSeamList.begin(), gSeamList.end(), edgeI) == gSeamList.end()) {
					gSeamList.push_back(edgeI);
				}
				chkTriF = true;
			}
		}
		if (gSeamList.empty()) continue;

		// gSeamList[]に、groupIDのグループで切れ目として使用するエッジ情報が入る.
		// このgSeamListが順番につながるように並び替え.
		m_ConnectSeam(gSeamList, seamGroupList);

		const int seamGroupCou = (int)seamGroupList.size();
		for (int i = 0; i < seamGroupCou; ++i) {
			const std::vector<int>& seamPointsList = seamGroupList[i];
			const int pCou = (int)seamPointsList.size();
			if (pCou < 2) continue;

			std::vector<bool> seamPointsLock;			// 頂点を置き換えるかのフラグ.
			seamPointsLock.resize(pCou);
			for (int j = 0; j < pCou; ++j) seamPointsLock[j] = false;

			int vI, vI0, vI1, vI2;
			sxsdk::vec3 v0, v1, v2;
			std::vector<SIndex2> edgesList;

			// seamPointsList[]のSeam用の頂点の一番先頭と末尾について、.
			// 頂点を分離させるかどうかの判定.
			// grpupIDのグループ内で、seamPointsList[0]またはseamPointsList.back() に接続するオリジナルのエッジを列挙.
			// 接続されるオリジナルのエッジがすべてSeamの場合は、その頂点は分離対象になる.
			// SeamのエッジとSeamでないエッジが混じっている場合は、その頂点は分離しない。seamPointsLock[]にロックフラグを入れる.
			for (int j = 0; j < 2; ++j) {
				vI = (j == 0) ? seamPointsList[0] : seamPointsList.back();

				// groupIDのグループに属する、vIの頂点を共有するエッジを列挙.
				const int edgesCou = m_FindEdgesFromPoint(pMesh, vI, groupID, edgesList);

				// edgesList[]のエッジのうち、seamである数を取得.
				int seamsCou = 0;
				for (int k = 0; k < edgesCou; ++k) {
					if (m_seamEdgeIndexMap.count(edgesList[k]) > 0) seamsCou++;
				}
				if (seamsCou != edgesCou) {
					if (j == 0) seamPointsLock[0] = true;
					else seamPointsLock.back() = true;
				}
			}

			// ＜＜ 置き換えが必要な三角形の頂点インデックス ＞＞.
			// 
			// 頂点seamPointsList[]を共有する三角形で、新しい頂点に置き換える必要があるものを判定.
			//
			//  1. seamPointsList[j] (Aとする) - seamPointsList[j + 1] (Bとする) の順番のエッジを持つ三角形を検出.
			//  2. 同一グループ内で[1]の三角形の各エッジの隣接三角形をたどる。.
			//     このとき、seamPointsList[]の頂点を共有するものをたどる。それ以外の三角形はたどらない.
			//  3. seamPointsList[]の最後の頂点までたどれば終了.
			//     個々の三角形をたどる際に、seamPointsList[]の頂点を共有する場合だけたどるようにすると、最後までたどりつける.
			//     このときの検出された三角形のSeamでの頂点が、頂点を置き換える対象となる.

			cTriList.clear();

			vI0 = seamPointsList[0];
			vI1 = seamPointsList[1];

			// vI0-vI1のエッジを持つ三角形を取得。これよりseamPointsList[]に沿った三角形をたどる.
			// 第三引数で順方向だけを取り出すため、三角形は1つだけ返されるはず.
			if (!m_FindTriangleFromEdgeIndex(vI0, vI1, triList, true)) continue;
			if (triList.size() != 1) continue;
			const int triIndex0 = triList[0];

			std::vector<int> tmpTriList;
			std::vector<int> tmpSeamPointsTriCountList;	// 各Seamポイントで共有する三角形数.

			tmpSeamPointsTriCountList.resize(pCou, 0);
			cTriList.push_back(triIndex0);
			tmpTriList.push_back(triIndex0);

			// seamPointsList[]に沿った三角形を探してcTriList[]に格納.
			// ここで検出された三角形上の頂点インデックスで、seamPointsList[]にあるSeam上の頂点が分離対象になる.
			int triSeamPointIndexA[3];
			while (!tmpTriList.empty()) {
				// triDの三角形の3つのエッジに隣接する三角形をたどる.
				const int curTriIndex = tmpTriList.back();
				const CMeshTriangleData& triD = triangles[curTriIndex];
				tmpTriList.pop_back();

				for (int j = 0; j < 3; ++j) {
					vI0 = triD.tri[j].orgVIndex;
					vI1 = triD.tri[(j + 1) % 3].orgVIndex;
					if (vI0 > vI1) std::swap(vI0, vI1);
					if (m_seamEdgeIndexMap.count(SIndex2(vI0, vI1)) > 0) continue;		// Seamのエッジの場合はスキップ.
					if (!m_FindTriangleFromEdgeIndex(vI0, vI1, triList) || triList.size() != 2) continue;
					
					const int triIndex = (triList[0] == curTriIndex) ? triList[1] : triList[0];
					if (triangles[triIndex].groupID != groupID) continue;

					if (std::find(cTriList.begin(), cTriList.end(), triIndex) != cTriList.end()) continue;

					// triIndexの三角形で、seamPointsList[]の頂点を持つかチェック.
					const CMeshTriangleData& triD2 = triangles[triIndex];
					int cou = 0;
					bool skipF = false;
					for (int k = 0; k < 3; ++k) {
						auto iter = std::find(seamPointsList.begin(), seamPointsList.end(), triD2.tri[k].orgVIndex);
						if (iter != seamPointsList.end()) {
							const int sIndex = std::distance(seamPointsList.begin(), iter);
							tmpSeamPointsTriCountList[sIndex]++;

							// seamPointsList[]の先頭か末尾の場合は、ロック状態によってスキップフラグを立てる.
							if (sIndex == 0 || sIndex == pCou - 1) {
								const int tmpTriCou = tmpSeamPointsTriCountList[sIndex];
								if (tmpTriCou >= 2) {
									if (seamPointsLock[sIndex]) {
										skipF = true;
									} else {
										if (tmpTriCou > 3) {
											skipF = true;
										}
									}
								}
							}
							cou++;
						}
					}
					if (cou == 0) continue;
					cTriList.push_back(triIndex);
					if (cou == 1 && skipF) continue;
					tmpTriList.push_back(triIndex);
				}
			}

			// seamPointsList[]の頂点を新しく複製し、cTriList[]の三角形での対象頂点でのインデックスを入れ替え.
			if (!cTriList.empty()) {
				// 新しい頂点番号の対応を作成.
				std::vector<int> seamPointNewIndex;
				seamPointNewIndex.resize(pCou, -1);
				for (int j = 0; j < pCou; ++j) {
					if (seamPointsLock[j]) continue;
					vI = seamPointsList[j];
					seamPointNewIndex[j] = (int)vertices.size();
					vData = vertices[vI];
					vertices.push_back(vData);
				}

				// 三角形の頂点インデックスでseamPointsList[]に含まれる頂点がある場合、新しい頂点インデックスに置き換え.
				for (size_t j = 0; j < cTriList.size(); ++j) {
					const int triIndex = cTriList[j];
					CMeshTriangleData& triD = triangles[triIndex];
					for (int k = 0; k < 3; ++k) {
						const int vOrgIndex = triD.tri[k].orgVIndex;
						auto iter = std::find(seamPointsList.begin(), seamPointsList.end(), vOrgIndex);
						const size_t index = std::distance(seamPointsList.begin(), iter);
						if (index < 0 || (int)index >= pCou) continue;
						if ((int)index == pCou || seamPointsLock[index]) continue;
						triD.tri[k].vIndex = seamPointNewIndex[index];
					}
				}
			}
		}
	}

	m_EndVerticesTriIndexList();
}

/**
 * バラバラのSeamリストをつないで分離.
 * 最終的なSeamのグループは、一周にはつながっていないものができるはず(つながっているとすれば、別グループになるので).
 * @param[in]  seamList       対象のSeamリスト.
 * @param[out] seamGroupList  つながっているSeamごとに並び替えたもの.
 */
int CMeshData::m_ConnectSeam (const std::vector<SIndex2>& seamList, std::vector< std::vector<int> >& seamGroupList)
{
	const int sCou = (int)seamList.size();

	seamGroupList.clear();

	SIndex2 edgeI;
	std::vector<bool> usedList;
	usedList.resize(sCou, false);
	for (int i = 0; i < sCou; ++i) {
		if (usedList[i]) continue;

		std::vector<int> vIndexList;
		edgeI = seamList[i];

		vIndexList.push_back(edgeI[0]);
		vIndexList.push_back(edgeI[1]);
		usedList[i] = true;

		for (int j = 0; j < sCou; ++j) {
			if (usedList[j]) continue;
			edgeI = seamList[j];

			int prevI = -1;
			int nextI = -1;
			if (vIndexList[0] == edgeI[0]) {
				prevI = edgeI[1];
			} else if (vIndexList[0] == edgeI[1]) {
				prevI = edgeI[0];
			} else if (vIndexList.back() == edgeI[0]) {
				nextI = edgeI[1];
			} else if (vIndexList.back() == edgeI[1]) {
				nextI = edgeI[0];
			}
			if (prevI < 0 && nextI < 0) continue;
			if (prevI >= 0) {
				vIndexList.insert(vIndexList.begin(), prevI);
			}
			if (nextI >= 0) {
				vIndexList.push_back(nextI);
			}
			usedList[j] = true;
			j = 0;
		}

		if (!vIndexList.empty()) {
			seamGroupList.push_back(vIndexList);
		}
	}
	return (int)seamGroupList.size();
}

/**
 * Seam情報のある頂点で、頂点を共有しないように変換.
 * @param[in] seamEdgeIndices  Seamとなるエッジ番号のリスト.
 */
void CMeshData::UpdateSeamEdges (sxsdk::shape_class& shape, const std::vector<int>& seamEdgeIndices)
{
	// seamのエッジでの、頂点番号の組み合わせをマップ。m_seamEdgeIndexMapに情報が保持される.
	m_MapSeamEdgeIndex(shape, seamEdgeIndices);

	// 面ごとにグループ化.
	m_SetGroupID(shape, seamEdgeIndices);

	const int eCou = (int)seamEdgeIndices.size();
	if (eCou == 0) return;

	//  同一グループ内でSeamでのエッジの分割がある場合、頂点を分離.
	m_DividePointsInSameGroup(shape, seamEdgeIndices);

	try {
		sxsdk::polygon_mesh_class& pMesh = shape.get_polygon_mesh();

		const int versCou = (int)vertices.size();
		const int triCou  = (int)triangles.size();

		// 頂点ごとのグループID保持バッファ.
		std::vector< sx::vec<int,2> > verticesGroupList;		// グループIDと、グループが異なる次の同一頂点のインデックス.
		verticesGroupList.resize(versCou, sx::vec<int,2>(-1, -1));

		// 面ごとの頂点を調べ、グループごとに頂点を分離する.
		CMeshVertexData vData;
		int vIndex2;
		for (int i = 0; i < triCou; ++i) {
			int curGroupID = triangles[i].groupID;

			for (int j = 0; j < 3; ++j) {
				const int vIndex = triangles[i].tri[j].vIndex;
				if (verticesGroupList[vIndex][0] < 0) {
					verticesGroupList[vIndex] = sx::vec<int,2>(curGroupID, -1);
					continue;
				}
				if (verticesGroupList[vIndex][0] == curGroupID) continue;

				// 同一頂点のインデックスをたどる.
				bool findF = false;
				vIndex2 = vIndex;
				if (verticesGroupList[vIndex2][1] >= 0) {
					vIndex2 = verticesGroupList[vIndex2][1];
					while (vIndex2 >= 0) {
						if (verticesGroupList[vIndex2][0] == curGroupID) {
							triangles[i].tri[j].vIndex = vIndex2;
							findF = true;
							break;
						}
						if (verticesGroupList[vIndex2][1] < 0) break;
						vIndex2 = verticesGroupList[vIndex2][1];
					}
				}

				if (!findF) {
					const int vIndex1 = (int)vertices.size();
					vData = vertices[ triangles[i].tri[j].vIndex ];
					vertices.push_back(vData);
					verticesGroupList.push_back(sx::vec<int,2>(curGroupID, -1));
					verticesGroupList[vIndex2][1] = vIndex1;
					triangles[i].tri[j].vIndex = vIndex1;
				}
			}
		}
	} catch (...) { }
}
