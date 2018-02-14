/**
 * メッシュデータ.
 */
#ifndef _MESHDATA_H
#define _MESHDATA_H

#include "GlobalHeader.h"
#include <vector>
#include <map>

// 2つの整数インデックス.
typedef struct SIndex2 {
public:
	int v1, v2;

	SIndex2 () {
		v1 = v2 = 0;
	}

	// std::mapで使用する場合の比較用.
	bool operator<(const SIndex2 &p) const {return ((this->v1) < (p.v1) || ((this->v1) == (p.v1) && (this->v2) < (p.v2)));}

	bool operator == (SIndex2 p) { return (this->v1 == p.v1 && this->v2 == p.v2); }

	SIndex2 (int v1, int v2) {
		(this->v1) = v1;
		(this->v2) = v2;
	};

	const int& operator [ ] (const int i) const {
		if (i == 0) return (this->v1);
		if (i == 1) return (this->v2);
		return (this->v1);
	}

	int& operator [ ] (const int i) {
		if (i == 0) return (this->v1);
		if (i == 1) return (this->v2);
		return (this->v1);
	}
} SIndex2;

/**
 * 頂点情報.
 */
class CMeshVertexData
{
public:
	sxsdk::vec3 pos;		// 座標値.
	sxsdk::vec2 uv;			// UV値.
	bool locked;

public:
	CMeshVertexData () {
		locked        = false;
		uv            = sxsdk::vec2(0, 0);
	}
};

/**
 * 三角形を構成する際の頂点情報.
 */
class CMeshTriangleVertexData
{
public:
	int vIndex;				// 頂点番号.
	int orgFaceVIndex;		// オリジナルの面上の頂点番号.
	int orgVIndex;			// オリジナルの頂点番号.

public:
	CMeshTriangleVertexData () {
		vIndex        = -1;
		orgFaceVIndex = -1;
		orgVIndex     = -1;
	}
};

/**
 * 三角形の情報.
 */
class CMeshTriangleData
{
public:
	CMeshTriangleVertexData tri[3];		// 三角形の頂点ごとの情報.
	int orgFaceIndex;					// オリジナルの面番号.
	int groupID;						// Seamで区切られるグループ番号.

public:
	CMeshTriangleData () {
		groupID       = -1;
		orgFaceIndex  = -1;
	}
};

/**
 * メッシュ情報を保持するクラス.
 */
class CMeshData
{
private:
	sxsdk::shade_interface& shade;
	int m_groupCount;										// グループの数.

	std::vector< std::vector<int> > m_versTriIndexList;		// 頂点ごとが共有する面番号を一時的に保持.
	std::map<SIndex2, int> m_seamEdgeIndexMap;				// Seamの2頂点の組み合わせを保持。値はSeam番号.

private:
	/**
	 * 頂点ごとが共有する面番号を一時的に保持。m_versTriIndexListに蓄える.
	 */
	void m_BeginVerticesTriIndexList (const bool useOrgVertex = false);

	/**
	 * 頂点ごとが共有する面番号を一時的に保持する処理を終了.
	 */
	void m_EndVerticesTriIndexList ();

	/**
	 * Seamのエッジリストより、頂点番号の組み合わせをマップ。m_seamEdgeIndexMapに格納される.
	 */
	void m_MapSeamEdgeIndex (sxsdk::shape_class& shape, const std::vector<int>& seamEdgeIndices);

	/**
	 * Seamを区切りにして面ごとにグループ番号を割り当てる.
	 */
	void m_SetGroupID (sxsdk::shape_class& shape, const std::vector<int>& seamEdgeIndices);

	/**
	 * 同一グループ内でSeamでの分割がある場合、頂点を分離.
	 */
	void m_DividePointsInSameGroup (sxsdk::shape_class& shape, const std::vector<int>& seamEdgeIndices);

	/**
	 * 2つの三角形の頂点インデックスより、エッジの共有がある場合のエッジの2頂点を取得.
	 * @param[in]  triIndex1  三角形の頂点インデックス1.
	 * @param[in]  triIndex2  三角形の頂点インデックス2.
	 * @param[out] edgeV      共有エッジを持つ場合はそのエッジの2頂点のインデックスを返す.
	 * @return 共有のエッジがある場合はtrue.
	 */
	bool m_ChkTrianglesShareEdge (int* triIndex1, int* triIndex2, sx::vec<int,2>& edgeV);

	/**
	 * エッジの2頂点を持つ面を取得。m_versTriIndexListを参照する.
	 * @param[in]  edgeP0, edgeP1  エッジの2頂点のインデックス.
	 * @param[out] triList         三角形番号が返る.
	 * @param[in]  forwardOnly     edgeP0 - edgeP1の順方向のみチェックする場合はtrue.
	 */
	bool m_FindTriangleFromEdgeIndex (const int edgeP0, const int edgeP1, std::vector<int>& triList, const bool forwardOnly = false);

	/**
	 * 指定の頂点を共有する三角形を取得。m_versTriIndexListを参照する.
	 * @param[in]  pIndex    頂点インデックス.
	 * @param[in]  groupID   グループID.
	 * @param[out] triList   三角形番号が返る.
	 */
	int m_FindTriangleFromPoint (const int pIndex, const int groupID, std::vector<int>& triList);

	/**
	 * 指定の頂点を共有するエッジを取得。m_versTriIndexListを参照する.
	 * @param[in]  pMesh     Shade3Dでのメッシュクラス.
	 * @param[in]  pIndex    頂点インデックス.
	 * @param[in]  groupID   グループID.
	 * @param[out] edgesList エッジ情報が入る.
	 */
	int m_FindEdgesFromPoint (sxsdk::polygon_mesh_class& pMesh, const int pIndex, const int groupID, std::vector<SIndex2>& edgesList);

	/**
	 * バラバラのSeamリストをつないで分離.
	 * @param[in]  seamList       対象のSeamリスト.
	 * @param[out] seamGroupList  つながっているSeamごとに並び替えたもの.
	 */
	int m_ConnectSeam (const std::vector<SIndex2>& seamList, std::vector< std::vector<int> >& seamGroupList);

public:

	std::vector<CMeshVertexData> vertices;					// 頂点を格納.
	std::vector<CMeshTriangleData> triangles;				// 三角形情報を格納.

public:
	CMeshData (sxsdk::shade_interface& shade);

	void Clear ();

	/**
	 * 指定の形状を格納.
	 * @param[in] shape       対象のポリゴンメッシュ形状.
	 * @param[in] allFaces    全ての面を展開する場合はtrue.
	 */
	bool StoreMesh (sxsdk::shape_class& shape, const bool allFaces = true);

	/**
	 * Seam情報のある頂点で、頂点を共有しないように変換.
	 * @param[in] seamEdgeIndices  Seamとなるエッジ番号のリスト.
	 */
	void UpdateSeamEdges (sxsdk::shape_class& shape, const std::vector<int>& seamEdgeIndices);

	/**
	 * グループの数を取得.
	 */
	int GetGroupCount () { return m_groupCount; }
};

#endif
