/**
 * UV展開を行うInterface.
 */
#ifndef _UVUNWRAPINTERFACE_H
#define _UVUNWRAPINTERFACE_H

#include "GlobalHeader.h"

/**
 * ダイアログボックスのパラメータ.
 */
class CUVUnwrapParam
{
public:
	int uvLayer;		// UV層番号.
	bool allFaces;		// 全ての面を展開する場合はtrue.

public:
	CUVUnwrapParam ();

	void Clear ();
};

/**
 * UV展開を行う、UVメニューから呼ばれるPluginInterface.
 */
class CUVUnwrapInterface : public sxsdk::plugin_interface
{
private:
	sxsdk::shade_interface& shade;
	CUVUnwrapParam m_data;		// ダイアログボックスのパラメータ.

	virtual int get_shade_version () const { return SHADE_BUILD_NUMBER; }
	virtual sx::uuid_class get_uuid (void * = 0) { return UV_UNWRAP_LSCM_ID; }

	// UNDO処理をサポート.
	virtual bool undoable (void * = 0) const { return true; } 

	// UVメニューに表示.
	virtual bool use_uv_menu (void * = 0) { return true; }

	// プラグインメニューからの呼び出し.
	virtual void do_it (sxsdk::shade_interface *shade, sxsdk::scene_interface *scene, void * = 0);

	//--------------------------------------------------//
	//	ダイアログのイベント処理用						//
	//--------------------------------------------------//
	// ダイアログの初期化.
	virtual void initialize_dialog (sxsdk::dialog_interface &d, void * = 0);

	// ダイアログのイベントを受け取る.
	virtual bool respond(sxsdk::dialog_interface &d, sxsdk::dialog_item_class &item, int action, void * = 0);

	// ダイアログのデータを設定する.
	virtual void load_dialog_data (sxsdk::dialog_interface &d, void * = 0);

public:
	CUVUnwrapInterface (sxsdk::shade_interface& shade);
	virtual ~CUVUnwrapInterface ();

	static const char *name(sxsdk::shade_interface *shade) { return shade->gettext("uvunwrap_title"); }
};

#endif
