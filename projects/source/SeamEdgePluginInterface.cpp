/**
 * 選択エッジより、Seamの登録/削除する.
 */

#include "SeamEdgePluginInterface.h"
#include "UVSeam.h"

//---------------------------------------------------.
CSeamAddEdgePluginInterface::CSeamAddEdgePluginInterface (sxsdk::shade_interface& shade) : shade(shade)
{
}

void CSeamAddEdgePluginInterface::do_it (sxsdk::shade_interface *shade, sxsdk::scene_interface *scene, void *)
{
	// アクティブなエッジをSeamとして登録.
	CUVSeam uvSeam(*shade);
	const int activeShapesCou = scene->get_number_of_active_shapes();
	for (int i = 0; i < activeShapesCou; ++i) {
		uvSeam.AddSeamActiveEdges(scene->active_shape(i));
	}
}

//---------------------------------------------------.
CSeamRemoveEdgePluginInterface::CSeamRemoveEdgePluginInterface (sxsdk::shade_interface& shade) : shade(shade)
{
}

void CSeamRemoveEdgePluginInterface::do_it (sxsdk::shade_interface *shade, sxsdk::scene_interface *scene, void *)
{
	// アクティブなエッジをSeamとして登録.
	CUVSeam uvSeam(*shade);
	const int activeShapesCou = scene->get_number_of_active_shapes();
	for (int i = 0; i < activeShapesCou; ++i) {
		uvSeam.RemoveSeamActiveEdges(scene->active_shape(i));
	}
}

//---------------------------------------------------.
CSeamRemoveAllEdgePluginInterface::CSeamRemoveAllEdgePluginInterface (sxsdk::shade_interface& shade) : shade(shade)
{
}

void CSeamRemoveAllEdgePluginInterface::do_it (sxsdk::shade_interface *shade, sxsdk::scene_interface *scene, void *)
{
	// アクティブなエッジをSeamとして登録.
	CUVSeam uvSeam(*shade);
	const int activeShapesCou = scene->get_number_of_active_shapes();
	for (int i = 0; i < activeShapesCou; ++i) {
		uvSeam.RemoveAllSeam(scene->active_shape(i));
	}
}
