/**
* UV展開を行うInterface.
*/
#include "UVUnwrapInterface.h"
#include "UnwrapLSCM.h"

// ダイアログボックスでのパラメータID.
enum {
	dlg_uv_layer_id = 101,			// UV Layer.
	dlg_all_faces_id = 102,			// All facesのチェック（すべての面が展開対象）.
};

CUVUnwrapParam::CUVUnwrapParam ()
{
	Clear();
}

void CUVUnwrapParam::Clear ()
{
	uvLayer  = 0;
	allFaces = true;
}

CUVUnwrapInterface::CUVUnwrapInterface (sxsdk::shade_interface& shade) : shade(shade)
{
}

CUVUnwrapInterface::~CUVUnwrapInterface ()
{
}

// プラグインメニューからの呼び出し.
void CUVUnwrapInterface::do_it (sxsdk::shade_interface *shade, sxsdk::scene_interface *scene, void *)
{
	compointer<sxsdk::dialog_interface> dlg(shade->create_dialog_interface_with_uuid(UV_UNWRAP_LSCM_ID));
	dlg->set_resource_name("uv_lscm_dlg");
	dlg->set_responder(this);
	this->AddRef();			// set_responder()に合わせて、参照カウンタを増やす。 .

	if (dlg->ask()) {
		scene->reset_undo_obsolete();		// UNDOを行うための前処理 .

		// LSCM展開を行う.
		CUnwrapLSCM unwrap(*shade);
		const int activeShapesCou = scene->get_number_of_active_shapes();
		for (int i = 0; i < activeShapesCou; ++i) {
			sxsdk::shape_class& shape = scene->active_shape(i);

			// UNDO処理のため.
			compointer<sxsdk::shape_saver_interface> shapeSaver(shape.create_shape_saver_interface());

			unwrap.DoUnwrap(&shape, m_data.uvLayer, m_data.allFaces);

			shapeSaver->set_undo_action();		// undoアクションを登録.
		}
	}
}

//--------------------------------------------------//
//	ダイアログのイベント処理用						//
//--------------------------------------------------//
// ダイアログの初期化.
void CUVUnwrapInterface::initialize_dialog (sxsdk::dialog_interface &d, void *)
{
}

// ダイアログのイベントを受け取る.
bool CUVUnwrapInterface::respond(sxsdk::dialog_interface &d, sxsdk::dialog_item_class &item, int action, void *)
{
	const int id = item.get_id();		// アクションがあったダイアログアイテムのID.

	if (id == dlg_uv_layer_id) {
		m_data.uvLayer = item.get_selection();
		return true;
	}

	if (id == dlg_all_faces_id) {
		m_data.allFaces = item.get_bool();
		return true;
	}

	return false;
}

// ダイアログのデータを設定する.
void CUVUnwrapInterface::load_dialog_data (sxsdk::dialog_interface &d, void *)
{
	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_uv_layer_id));
		item->set_selection(m_data.uvLayer);
	}

	{
		sxsdk::dialog_item_class* item;
		item = &(d.get_dialog_item(dlg_all_faces_id));
		item->set_bool(m_data.allFaces);
	}
}

