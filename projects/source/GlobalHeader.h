/**
 *  @file   GlobalHeader.h
 *  @brief  共通して使用する変数など.
 */

#ifndef _GLOBALHEADER_H
#define _GLOBALHEADER_H

#include "sxsdk.cxx"

/**
 * LSCMを行うUUID.
 */
#define UV_UNWRAP_LSCM_ID sx::uuid_class("8243F088-25C4-4450-A6C6-1733AA3930D2")

/**
 * 選択エッジをSeamに登録するUUID.
 */
#define SEAM_ADD_EDGE_PLUGIN_ID sx::uuid_class("A6628A46-D8BB-4D80-BAF7-2F7705E041BF")

/**
 * 選択エッジをSeamから削除するUUID.
 */
#define SEAM_REMOVE_EDGE_PLUGIN_ID sx::uuid_class("CBD9B293-6BE9-4ADE-9694-E456DA91D28D")

/**
 * Seamをすべて削除するUUID.
 */
#define SEAM_REMOVE_ALL_EDGE_PLUGIN_ID sx::uuid_class("690AA69B-53A1-4890-9552-A9CC09DE23BA")

/**
 * ポリゴンメッシュのエッジのSeamを保持するAttribute.
 */
#define UV_SEAM_ATTRIBUTE_ID sx::uuid_class("7AB297AA-AC51-4419-997A-016EA64C1D94")

/**
 * Seam情報を保存するStreamのバージョン.
 */
#define UV_SEAM_STREAM_VERSION	0x100

#endif
