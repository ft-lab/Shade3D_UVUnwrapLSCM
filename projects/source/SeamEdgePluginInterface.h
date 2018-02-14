/**
 * 選択エッジより、Seamの登録/削除する.
 */
#ifndef _SEAMEDGEPLUGININTERFACE_H
#define _SEAMEDGEPLUGININTERFACE_H

#include "GlobalHeader.h"

/**
 * Seamを追加する.
 */
class CSeamAddEdgePluginInterface : public sxsdk::plugin_interface
{
private:
	sxsdk::shade_interface& shade;

	virtual int get_shade_version () const { return SHADE_BUILD_NUMBER; }
	virtual sx::uuid_class get_uuid (void * = 0) { return SEAM_ADD_EDGE_PLUGIN_ID; }

	// プラグインメニューからの呼び出し.
	virtual void do_it (sxsdk::shade_interface *shade, sxsdk::scene_interface *scene, void * = 0);

public:
	CSeamAddEdgePluginInterface (sxsdk::shade_interface& shade);

	static const char *name(sxsdk::shade_interface *shade) { return shade->gettext("seam_add_edge_title"); }
};

/**
* Seamを削除する.
*/
class CSeamRemoveEdgePluginInterface : public sxsdk::plugin_interface
{
private:
	sxsdk::shade_interface& shade;

	virtual int get_shade_version () const { return SHADE_BUILD_NUMBER; }
	virtual sx::uuid_class get_uuid (void * = 0) { return SEAM_ADD_EDGE_PLUGIN_ID; }

	// プラグインメニューからの呼び出し.
	virtual void do_it (sxsdk::shade_interface *shade, sxsdk::scene_interface *scene, void * = 0);

public:
	CSeamRemoveEdgePluginInterface (sxsdk::shade_interface& shade);

	static const char *name(sxsdk::shade_interface *shade) { return shade->gettext("seam_remove_edge_title"); }
};

/**
* すべてのSeamを削除する.
*/
class CSeamRemoveAllEdgePluginInterface : public sxsdk::plugin_interface
{
private:
	sxsdk::shade_interface& shade;

	virtual int get_shade_version () const { return SHADE_BUILD_NUMBER; }
	virtual sx::uuid_class get_uuid (void * = 0) { return SEAM_REMOVE_ALL_EDGE_PLUGIN_ID; }

	// プラグインメニューからの呼び出し.
	virtual void do_it (sxsdk::shade_interface *shade, sxsdk::scene_interface *scene, void * = 0);

public:
	CSeamRemoveAllEdgePluginInterface (sxsdk::shade_interface& shade);

	static const char *name(sxsdk::shade_interface *shade) { return shade->gettext("seam_remove_all_edge_title"); }
};

#endif
