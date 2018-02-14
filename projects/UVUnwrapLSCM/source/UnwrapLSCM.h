/**
 * LSCMによるUV展開を行う.
 */

#ifndef _UNWRAPLSCM_H
#define _UNWRAPLSCM_H

#include "GlobalHeader.h"
#include "OpenNL_psm.h"

#include <vector>
#include <map>

/**
 * LSCMを実行する.
 */
class CMeshData;
class CUnwrapLSCM
{
private:
	sxsdk::shade_interface& shade;

	/**
	 * メッシュをLSCMに渡す際の前処理.
	 */
	void m_Project (CMeshData& meshData);

	/**
	 * Copies u,v coordinates from the mesh to OpenNL solver.
	 */
	void m_MeshToSolver (CMeshData& meshData);

	void m_SetupLSCM (CMeshData& meshData);

	void m_SolverToMesh (CMeshData& meshData);

	void m_NormalizeUV (CMeshData& meshData);

	void m_SetupConformalMapRelations (CMeshData& meshData, const int v0, const int v1, const int v2);

	void m_ProjectTriangle (const sxsdk::vec3& p0, const sxsdk::vec3& p1, const sxsdk::vec3& p2, sxsdk::vec2& z0, sxsdk::vec2& z1, sxsdk::vec2& z2);

	/**
	 * グループごとにUVをずらして再配置.
	 */
	void m_RealignmentUVs (CMeshData& meshData);

	/**
	 * UVをShade3Dのジオメトリに反映.
	 * @param[in] meshData      メッシュ情報クラス.
	 * @param[in] shape         対象形状.
	 * @param[in] uvLayerIndex  反映するUV層番号.
	 */
	void m_UpdateUVs (CMeshData& meshData, sxsdk::shape_class* shape, const int uvLayerIndex);

public:
	CUnwrapLSCM (sxsdk::shade_interface& shade);

	/**
	 * 指定の形状のLSCM展開を行う.
	 * @param[in] shape         対象形状.
	 * @param[in] uvLayerIndex  UV層番号.
	 * @param[in] allFaces      全ての面を展開する場合はtrue.
	 */
	bool DoUnwrap (sxsdk::shape_class* shape, const int uvLayerIndex, const bool allFaces = true);
};

#endif
