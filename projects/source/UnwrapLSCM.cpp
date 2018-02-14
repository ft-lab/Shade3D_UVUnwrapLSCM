/**
 * LSCMによるUV展開を行う.
 */
#include "UnwrapLSCM.h"
#include "UVSeam.h"
#include "MeshData.h"

#include <algorithm>
#include <vector>
#include <cmath>

CUnwrapLSCM::CUnwrapLSCM (sxsdk::shade_interface& shade) : shade(shade)
{
}

/**
 * 指定の形状のLSCM展開を行う.
 * @param[in] shape         対象形状.
 * @param[in] uvLayerIndex  UV層番号.
 * @param[in] allFaces      全ての面を展開する場合はtrue.
 */
bool CUnwrapLSCM::DoUnwrap (sxsdk::shape_class* shape,  const int uvLayerIndex, const bool allFaces)
{
	if ((shape->get_type()) != sxsdk::enums::polygon_mesh) return false;

	sxsdk::polygon_mesh_class& pMesh = shape->get_polygon_mesh();
	const int versCou = pMesh.get_total_number_of_control_points();

	// Seam情報を取得.
	std::vector<int> seamEdgeIndices;
	CUVSeam::LoadSeamData(*shape, seamEdgeIndices);

	// メッシュ情報を取得.
	CMeshData meshData(shade);
	if (!meshData.StoreMesh(*shape, allFaces)) return false;

	// Seam情報により、共有するエッジの頂点を分離.
	meshData.UpdateSeamEdges(*shape, seamEdgeIndices);

	nlInitialize(0, NULL);

	const int nb_eigens = 10;
	nlNewContext();

	NLuint nb_vertices = NLuint(meshData.vertices.size());
	m_Project(meshData);

	nlSolverParameteri(NL_NB_VARIABLES, NLint(2*nb_vertices));
	nlSolverParameteri(NL_LEAST_SQUARES, NL_TRUE);
	nlSolverParameteri(NL_MAX_ITERATIONS, NLint(5*nb_vertices));

	nlSolverParameterd(NL_THRESHOLD, 1e-6);

	nlBegin(NL_SYSTEM);
	m_MeshToSolver(meshData);
	nlBegin(NL_MATRIX);
	m_SetupLSCM(meshData);
	nlEnd(NL_MATRIX);
	nlEnd(NL_SYSTEM);

	nlSolve();					// OpenNLでのLSCM計算を実行.

	m_SolverToMesh(meshData);		// OpenNLの計算結果をmeshDataに格納.
	m_RealignmentUVs(meshData);		// グループごとにUVを再配置.
	m_NormalizeUV(meshData);		// UVを0.0-1.0にリサイズ.

#if 0
	{
		double time;
		NLint iterations;	    
		nlGetDoublev(NL_ELAPSED_TIME, &time);
		nlGetIntegerv(NL_USED_ITERATIONS, &iterations);
	}
#endif

	nlDeleteContext(nlGetCurrent());

	m_UpdateUVs(meshData, shape, uvLayerIndex);	// UVをShade3Dのshapeに反映.

	return true;
}

/**
 * メッシュをLSCMに渡す際の前処理.
 * グループごとに端の頂点が移動しないようにロック.
 */
void CUnwrapLSCM::m_Project (CMeshData& meshData)
{
	const int versCou = (int)meshData.vertices.size();
	const int triCou  = (int)meshData.triangles.size();

	// ロックについては、面ごとのグループで行う.
	const int groupCount = meshData.GetGroupCount();		// グループの数.

	for (int groupID = 0; groupID < groupCount; ++groupID) {
		// バウンディングボックスを計算.
		sxsdk::vec3 bbMin, bbMax;
		bool firstF = true;
		for (int i = 0; i < triCou; ++i) {
			const CMeshTriangleData& triD = meshData.triangles[i];
			if (triD.groupID != groupID) continue;
			if (firstF) {
				firstF = false;
				bbMin = bbMax = meshData.vertices[triD.tri[0].vIndex].pos;
			}
			for (int j = 0; j < 3; ++j) {
				const sxsdk::vec3& v = meshData.vertices[triD.tri[j].vIndex].pos;
				bbMin.x = std::min(bbMin.x, v.x);
				bbMin.y = std::min(bbMin.y, v.y);
				bbMin.z = std::min(bbMin.z, v.z);
				bbMax.x = std::max(bbMax.x, v.x);
				bbMax.y = std::max(bbMax.y, v.y);
				bbMax.z = std::max(bbMax.z, v.z);
			}
		}
		if (firstF) continue;

		float dx = bbMax.x - bbMin.x;
		float dy = bbMax.y - bbMin.y;
		float dz = bbMax.z - bbMin.z;

		// Find shortest bbox axis.
		sxsdk::vec3 v1, v2;
		if (dx < dy && dx < dz) {
			if (dy > dz) {
				v1 = sxsdk::vec3(0,1,0);
				v2 = sxsdk::vec3(0,0,1);
			} else {
				v2 = sxsdk::vec3(0,1,0);
				v1 = sxsdk::vec3(0,0,1);
			}
		} else if (dy < dx && dy < dz) {
			if (dx > dz) {
				v1 = sxsdk::vec3(1,0,0);
				v2 = sxsdk::vec3(0,0,1);
			} else {
				v2 = sxsdk::vec3(1,0,0);
				v1 = sxsdk::vec3(0,0,1);
			}
		} else {
			if(dx > dy) {
				v1 = sxsdk::vec3(1,0,0);
				v2 = sxsdk::vec3(0,1,0);
			} else {
				v2 = sxsdk::vec3(1,0,0);
				v1 = sxsdk::vec3(0,1,0);
			}
		}

		// Project onto shortest bbox axis,
		// and lock extrema vertices
		CMeshVertexData* vxMin = NULL;
		CMeshVertexData* vxMax = NULL;
		float uMin = (float)(1e+10);
		float uMax = (float)-(1e+10);

		for (int i = 0; i < triCou; ++i) {
			const CMeshTriangleData& triD = meshData.triangles[i];
			if (triD.groupID != groupID) continue;

			for (int j = 0; j < 3; ++j) {
				CMeshVertexData& V = meshData.vertices[triD.tri[j].vIndex];
				float u = sx::inner_product(V.pos, v1);
				float v = sx::inner_product(V.pos, v2);
				V.uv = sxsdk::vec2(u, v);
				if (u < uMin) {
					vxMin = &V;
					uMin  = u;
				}
				if (u > uMax) {
					vxMax = &V;
					uMax  = u;
				}
			}
		}
		if (vxMin) vxMin->locked = true;
		if (vxMax) vxMax->locked = true;
	}
}

/**
 * Copies u,v coordinates from the mesh to OpenNL solver.
 */
void CUnwrapLSCM::m_MeshToSolver (CMeshData& meshData)
{
	const int versCou = (int)meshData.vertices.size();

	for (int i = 0, iPos = 0; i < versCou; ++i, iPos += 2) {
		CMeshVertexData& it = meshData.vertices[i];
		float u = it.uv.x;
		float v = it.uv.y;
		nlSetVariable(iPos    , u);
		nlSetVariable(iPos + 1, v);
		if (it.locked) {
			nlLockVariable(iPos    );
			nlLockVariable(iPos + 1);
		}
	}
}

void CUnwrapLSCM::m_SetupLSCM (CMeshData& meshData)
{
	const int facesCou = (int)meshData.triangles.size();
	for (int f = 0; f < facesCou; ++f) {
		const CMeshTriangleData& triD = meshData.triangles[f];
		m_SetupConformalMapRelations(meshData, triD.tri[0].vIndex, triD.tri[1].vIndex, triD.tri[2].vIndex);
	}
}
/*
 * Creates the LSCM equation in OpenNL, related with
 *   a given triangle, specified by vertex indices.
 */
void CUnwrapLSCM::m_SetupConformalMapRelations (CMeshData& meshData, const int v0, const int v1, const int v2)
{
	const sxsdk::vec3& p0 = meshData.vertices[v0].pos;
	const sxsdk::vec3& p1 = meshData.vertices[v1].pos;
	const sxsdk::vec3& p2 = meshData.vertices[v2].pos;

	sxsdk::vec2 z0, z1, z2;
	m_ProjectTriangle(p0, p1, p2, z0, z1, z2);

	sxsdk::vec2 z01 = z1 - z0;
	sxsdk::vec2 z02 = z2 - z0;
	float a = z01.x;
	float b = z01.y;
	float c = z02.x;
	float d = z02.y;
	assert(b == 0.0f);

	// Note  : 2*id + 0 --> u
	//         2*id + 1 --> v
	NLuint u0_id = 2*v0    ;
	NLuint v0_id = 2*v0 + 1;
	NLuint u1_id = 2*v1    ;
	NLuint v1_id = 2*v1 + 1;
	NLuint u2_id = 2*v2    ;
	NLuint v2_id = 2*v2 + 1;

	// Note : b = 0

	// Real part
	nlBegin(NL_ROW);
	nlCoefficient(u0_id, -a+c);
	nlCoefficient(v0_id,  b-d);
	nlCoefficient(u1_id,   -c);
	nlCoefficient(v1_id,    d);
	nlCoefficient(u2_id,    a);
	nlEnd(NL_ROW);

	// Imaginary part
	nlBegin(NL_ROW);
	nlCoefficient(u0_id, -b+d);
	nlCoefficient(v0_id, -a+c);
	nlCoefficient(u1_id,   -d);
	nlCoefficient(v1_id,   -c);
	nlCoefficient(v2_id,    a);
	nlEnd(NL_ROW);
}

void CUnwrapLSCM::m_ProjectTriangle (const sxsdk::vec3& p0, const sxsdk::vec3& p1, const sxsdk::vec3& p2, sxsdk::vec2& z0, sxsdk::vec2& z1, sxsdk::vec2& z2)
{
	sxsdk::vec3 X = p1 - p0;
	X = normalize(X);

	sxsdk::vec3 Z = sx::product(X, (p2 - p0));
	Z = normalize(Z);

	sxsdk::vec3 Y = sx::product(Z, X);
	const sxsdk::vec3& O = p0;
        
	float x0 = 0;
	float y0 = 0;
	float x1 = sxsdk::absolute(p1 - O);
	float y1 = 0;
	float x2 = sx::inner_product((p2 - O), X);
	float y2 = sx::inner_product((p2 - O), Y);
        
	z0 = sxsdk::vec2(x0, y0);
	z1 = sxsdk::vec2(x1, y1);
	z2 = sxsdk::vec2(x2, y2);        
}

void CUnwrapLSCM::m_SolverToMesh (CMeshData& meshData)
{
	const int versCou = (int)meshData.vertices.size();

	for (int i = 0, iPos = 0; i < versCou; ++i, iPos += 2) {
		CMeshVertexData& it = meshData.vertices[i];
		const float u = nlGetVariable(iPos);
		const float v = nlGetVariable(iPos + 1);
		it.uv = sxsdk::vec2(u, v);
		if (sx::isnan(it.uv)) {
			it.uv = sxsdk::vec2(0, 0);
		}
	}
}

/**
 * UVが0.0-1.0に収まるようにリサイズ.
 */
void CUnwrapLSCM::m_NormalizeUV (CMeshData& meshData)
{
	const int versCou = (int)meshData.vertices.size();

	float u_min, v_min, u_max, v_max;
	u_min = u_max = meshData.vertices[0].uv.x;
	v_min = v_max = meshData.vertices[0].uv.y;
	for (int i = 1; i < versCou; ++i) {
		const sxsdk::vec2& uv = meshData.vertices[i].uv;
		u_min = std::min(u_min, uv.x);
		v_min = std::min(v_min, uv.y);
		u_max = std::max(u_max, uv.x);
		v_max = std::max(v_max, uv.y);
	}
	const float l = std::max(u_max - u_min, v_max - v_min);

	if (!sx::zero(l)) {
		for (int i = 0; i < versCou; ++i) {
			const sxsdk::vec2 uv = meshData.vertices[i].uv;
			meshData.vertices[i].uv.x = (uv.x - u_min) / l;
			meshData.vertices[i].uv.y = (uv.y - v_min) / l;
		}
	}
}

/**
 * UVをShade3Dのジオメトリに反映.
 * @param[in] meshData      メッシュ情報クラス.
 * @param[in] shape         対象形状.
 * @param[in] uvLayerIndex  反映するUV層番号.
 */
void CUnwrapLSCM::m_UpdateUVs (CMeshData& meshData, sxsdk::shape_class* shape, const int uvLayerIndex)
{
	sxsdk::polygon_mesh_class& pMesh = shape->get_polygon_mesh();

	// UV層がない場合は追加.
	while(pMesh.get_number_of_uv_layers() <= uvLayerIndex) {
		pMesh.append_uv_layer();
	}

	const int triCou = (int)meshData.triangles.size();

	for (int i = 0; i < triCou; ++i) {
		const CMeshTriangleData& triD =  meshData.triangles[i];
		const int faceIndex = triD.orgFaceIndex;

		const sxsdk::vec2& uv0 = meshData.vertices[triD.tri[0].vIndex].uv;
		const sxsdk::vec2& uv1 = meshData.vertices[triD.tri[1].vIndex].uv;
		const sxsdk::vec2& uv2 = meshData.vertices[triD.tri[2].vIndex].uv;

		sxsdk::face_class& f = pMesh.face(faceIndex);
		f.set_face_uv(uvLayerIndex, triD.tri[0].orgFaceVIndex, uv0);
		f.set_face_uv(uvLayerIndex, triD.tri[1].orgFaceVIndex, uv1);
		f.set_face_uv(uvLayerIndex, triD.tri[2].orgFaceVIndex, uv2);
	}

	pMesh.update();
}

/**
 * グループごとにUVをずらして再配置.
 */
void CUnwrapLSCM::m_RealignmentUVs (CMeshData& meshData)
{
	// グループ数を取得.
	const int groupCount = meshData.GetGroupCount();
	if (groupCount <= 1) return;
	const int triCou  = (int)meshData.triangles.size();
	const int versCou = (int)meshData.vertices.size();

	// 各グループごとのUVのバウンディングボックスを保持.
	std::vector<sxsdk::vec2> groupBBMinList, groupBBMaxList, groupBBSizeList, orgBBMinList;
	groupBBMinList.resize(groupCount, sxsdk::vec2(0, 0));
	groupBBMaxList.resize(groupCount, sxsdk::vec2(0, 0));
	groupBBSizeList.resize(groupCount, sxsdk::vec2(0, 0));
	orgBBMinList.resize(groupCount, sxsdk::vec2(0, 0));

	for (int groupID = 0; groupID < groupCount; ++groupID) {
		bool firstF = true;
		sxsdk::vec2 bbMin, bbMax;
		for (int j = 0; j < triCou; ++j) {
			const CMeshTriangleData& triD = meshData.triangles[j];
			if (triD.groupID != groupID) continue;
			if (firstF) {
				firstF = false;
				bbMin = bbMax = meshData.vertices[triD.tri[0].vIndex].uv;
			}
			for (int k = 0; k < 3; ++k) {
				const sxsdk::vec2& v = meshData.vertices[triD.tri[k].vIndex].uv;
				bbMin.x = std::min(bbMin.x, v.x);
				bbMin.y = std::min(bbMin.y, v.y);
				bbMax.x = std::max(bbMax.x, v.x);
				bbMax.y = std::max(bbMax.y, v.y);
			}
		}
		if (!firstF) {
			groupBBMinList[groupID]  = bbMin;
			groupBBMaxList[groupID]  = bbMax;
			groupBBSizeList[groupID] = bbMax - bbMin;
			orgBBMinList[groupID]    = bbMin;
		}
	}

	// バウンディングボックスの面積が大きい順に順番を与える.
	std::vector<int> groupIndexList;
	groupIndexList.resize(groupCount);
	for (int i = 0; i < groupCount; ++i) groupIndexList[i] = i;

	std::vector<float> groupSizeList;
	groupSizeList.resize(groupCount, 0.0f);
	double allArea = 0.0;
	float maxGroupWidth = 0.0f;
	for (int i = 0; i < groupCount; ++i) {
		const sxsdk::vec2& bbSize = groupBBSizeList[i];
		groupSizeList[i] = bbSize.x * bbSize.y;
		maxGroupWidth = std::max(maxGroupWidth, bbSize.x);
		allArea += groupSizeList[i];
	}
	if (sx::zero(allArea)) return;

	for (int i = 0; i < groupCount; ++i) {
		for (int j = i + 1; j < groupCount; ++j) {
			const int i1 = groupIndexList[i];
			const int i2 = groupIndexList[j];
			const float size1 = groupSizeList[i1];
			const float size2 = groupSizeList[i2];
			if (size1 < size2) {
				std::swap(groupIndexList[i], groupIndexList[j]);
			}
		}
	}

	// 推定の横幅.
	const float tWidth   = std::max((float)std::sqrt(allArea) * 1.5f, maxGroupWidth * 1.2f);
	const float fMargin  = tWidth * 0.01f;	// 配置時に開ける隙間.
	const float fMargin2 = tWidth * 0.05f;

	// fMarginのマージンでバウンディングボックスを更新.
	for (int i = 0; i < groupCount; ++i) {
		sxsdk::vec2& bbSize = groupBBSizeList[i];
		if (sx::zero(bbSize)) continue;
		bbSize.x += fMargin;
		bbSize.y += fMargin;
		sxsdk::vec2& bbMax = groupBBMaxList[i];
		bbMax.x += fMargin;
		bbMax.y += fMargin;
	}

	// BL(Bottom Left)法で詰めていく.
	std::vector<sxsdk::vec2> blStablePoint;
	float maxY = 0.0f;
	blStablePoint.push_back(sxsdk::vec2(0, 0));

	bool firstF = true;
	for (int gLoop = 0; gLoop < groupCount; ++gLoop) {
		const int groupID = groupIndexList[gLoop];
		const sxsdk::vec2& groupSize = groupBBSizeList[groupID];
		if (sx::zero(groupSize)) continue;
		if (firstF) {
			firstF = false;
			groupBBMinList[groupID] = sxsdk::vec2(0, 0);
			groupBBMaxList[groupID] = groupBBMinList[groupID] + groupSize;

			// BL安定点の更新。長方形のバウンディングボックスの(UV図面で見た)右上と左下.
			blStablePoint.erase(blStablePoint.begin() + 0);
			blStablePoint.push_back(sxsdk::vec2(groupBBMaxList[groupID].x, groupBBMinList[groupID].y));
			blStablePoint.push_back(sxsdk::vec2(groupBBMinList[groupID].x, groupBBMaxList[groupID].y));
			maxY = groupBBMaxList[groupID].y;
			continue;
		}

		std::vector<sxsdk::vec2> storePosList;
		std::vector<int> storeIList;
		for (size_t i = 0; i < blStablePoint.size(); ++i) {
			const sxsdk::vec2& p = blStablePoint[i];

			if (p.x + groupSize.x <= tWidth) {
				// すでに格納済みのUVグループ枠とぶつからないか判定.
				const sxsdk::vec2 tmpBBMin = p;
				const sxsdk::vec2 tmpBBMax = tmpBBMin + groupSize;
				{
					bool collisionF = false;
					for (int j = 0; j < gLoop; j++) {
						const int groupID2 = groupIndexList[j];
						const sxsdk::vec2& bbMin = groupBBMinList[groupID2];
						const sxsdk::vec2& bbMax = groupBBMaxList[groupID2];
						if (tmpBBMax.x <= bbMin.x || tmpBBMax.y <= bbMin.y || tmpBBMin.x >= bbMax.x || tmpBBMin.y >= bbMax.y) continue;
						collisionF = true;
						break;
					}
					if (collisionF) continue;
				}
				storePosList.push_back(tmpBBMin);
				storeIList.push_back(i);
			}
		}
		if (storePosList.size() == 0) {
			const int pIndex = (int)blStablePoint.size();
			sxsdk::vec2 p0(0.0f, maxY);
			blStablePoint.push_back(p0);
			storePosList.push_back(p0);
			storeIList.push_back(pIndex);
		}

		// 格納候補のうち、よりYとXが小さい位置を採用.
		// 少しの誤差がある場合はfMargin2で丸め込んでいる.
		int minI = 0;
		float minXPos = storePosList[0].x;
		float minYPos = storePosList[0].y;
		for (int i = 1; i < storePosList.size(); i++) {
			if (storePosList[i].y + fMargin2 < minYPos || (std::abs(storePosList[i].y - minYPos) <= fMargin2 && storePosList[i].x < minXPos)) {
				minXPos = storePosList[i].x;
				minYPos = storePosList[i].y;
				minI    = i;
			}
		}

		{
			groupBBMinList[groupID] = sxsdk::vec2(storePosList[minI].x, storePosList[minI].y);
			groupBBMaxList[groupID] = groupBBMinList[groupID] + groupSize;

			blStablePoint.erase(blStablePoint.begin() + storeIList[minI]);

			// BL安定点の更新。長方形のバウンディングボックスの(UV図面で見た)右上と左下.
			blStablePoint.push_back(sxsdk::vec2(groupBBMaxList[groupID].x, groupBBMinList[groupID].y));
			blStablePoint.push_back(sxsdk::vec2(groupBBMinList[groupID].x, groupBBMaxList[groupID].y));
			maxY = std::max(maxY, groupBBMaxList[groupID].y);
		}
	}

	// UVを置き換える.
	for (int gLoop = 0; gLoop < groupCount; ++gLoop) {
		const int groupID = groupIndexList[gLoop];
		const sxsdk::vec2& groupSize = groupBBSizeList[groupID];
		const sxsdk::vec2& bbMin     = groupBBMinList[groupID];
		const sxsdk::vec2& orgBBMin  = orgBBMinList[groupID];
		const sxsdk::vec2 dV = -orgBBMin + bbMin;

		// 採用する頂点を取得.
		std::vector<bool> uvUseVerticesI;
		uvUseVerticesI.resize(versCou, false);
		for (int i = 0; i < triCou; ++i) {
			const CMeshTriangleData& triD = meshData.triangles[i];
			if (triD.groupID != groupID) continue;
			for (int j = 0; j < 3; ++j) {
				uvUseVerticesI[triD.tri[j].vIndex] = true;
			}
		}

		// UV値をシフト.
		for (int i = 0; i < versCou; ++i) {
			if (uvUseVerticesI[i]) {
				meshData.vertices[i].uv += dV;
			}
		}
	}
}
