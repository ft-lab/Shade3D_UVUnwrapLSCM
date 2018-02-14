# UVUnwrap LSCM for Shade3D

Shade3DでUV展開(LSCM)を行うプラグインです。  
Shade3D ver.16/17は、Standard/Professional版で標準機能としてLSCMが使えますが  
その機能強化版になります。  

## 動作環境
* Windows 7/8/10以降のOS
* Mac 10.9以降のOS
* Shade3D ver.16/17以降で、Standard/Professional版（Basic版では動作しません）

## 使い方

### プラグインダウンロード
以下から最新版をダウンロードしてください。  
https://github.com/ft-lab/Shade3D_UVUnwrapLSCM/releases  

### プラグインを配置し、Shade3Dを起動
Winの場合は、ビルドされた UVUnwrapLSCM64.dll をShade3Dのpluginsディレクトリに格納してShade3Dを起動。  
Macの場合は、ビルドされた UVUnwrapLSCM.shdpluginをShade3Dのpluginsディレクトリに格納してShade3Dを起動。  
メインメニューの「ツール」-「プラグイン」-「選択エッジをシームとして追加」が表示されるのを確認します。  

### 使い方

ポリゴンメッシュの稜線に対して、「シーム(Seam)」として切れ目を入れていきます。  
この操作は、形状編集モード＋稜線選択モードに移行し、稜線を選択。  
メインメニューの「ツール」-「プラグイン」より、「選択エッジをシームとして追加」でシームを追加。   
「選択エッジをシームから削除」で、選択された稜線のシームを削除。  
「シームをすべて削除」で、すべてのシームを削除。  
図形ウィンドウ上では、オレンジのラインでシームが表示されます。  
<img src="https://github.com/ft-lab/Shade3D_UVUnwrapLSCM/blob/master/wiki_images/UVUnwrap_lscm_02.png"/>  
シームは、ポリゴンメッシュに切れ目を入れて複数に分離されるように与えます。  
ぐるっと回り込むような指定の場合は、シームを複数与えるのが有効です。  

UV図面でUVメニューを開き、コンテキストメニューで「UV展開 (LSCM)」を選択します。  
<img src="https://github.com/ft-lab/Shade3D_UVUnwrapLSCM/blob/master/wiki_images/UVUnwrap_lscm_03.png"/>  
<img src="https://github.com/ft-lab/Shade3D_UVUnwrapLSCM/blob/master/wiki_images/UVUnwrap_lscm_04.png"/>  
「UVのLSCM展開」ウィンドウで、UV層番号を指定、  
「すべての面」チェックボックスをオンにすると、選択状態にかかわらずすべての面がLSCM展開されます。  
オフにすると、選択面のみがLSCM展開されます。  
以下のように重ならないようにシームに沿って展開されました。  
<img src="https://github.com/ft-lab/Shade3D_UVUnwrapLSCM/blob/master/wiki_images/UVUnwrap_lscm_05.png"/>  


### Shade3D ver.16/17の標準搭載のLSCMとの違い

* LSCMによるUV展開後、グループごとに重ならないように再配置します。
* UV展開でグループに分けた際に、個々のグループごとのUVサイズが大きく変わらないようにしています。  
UV展開でのサイズが、完全にジオメトリと比例するわけではありません。
* LSCM展開後に1つのグループとなる場合、同一グループで切れ目(Seam)を入れた展開ができるようにしています。  
円柱の側面に縦線1本分の切れ目(Seam)を入れて、ぐるっと展開する場合など。  
<img src="https://github.com/ft-lab/Shade3D_UVUnwrapLSCM/blob/master/wiki_images/UVUnwrap_lscm_01.png"/>  

### 制限事項

* 頂点を記憶する「ピン止め」機能は実装していません。
* LSCMによるUV展開はUNDO/REDO対応していますが、ポリゴンメッシュの稜線にシームを与える操作はUNDO/REDO対応していません。

## ビルド方法 (開発向け)

Windows環境では「Visual Studio 2017」(無償のCommunity版でビルドできます)、  
Mac環境では「Xcode 6.4」でビルドしました。  

### OpenNLのファイルをソースに追加  
LSCM展開に「OpenNL」( http://alice.loria.fr/index.php/software/4-library/23-opennl.html )を使用しています。  
「OpenNL_psm_1.5.2.zip」をダウンロードし、解凍後にOpenNL_psm.c/OpenNL_psm.hの2つのファイルを  
projects/UVUnwrapLSCM/sourceディレクトリにコピーします。  

### OpenNL_psm.cの変更 (Macのみ)  
Macの場合は、「OpenNL_psm.c」をコンパイルする際に  
```
typedef enum {NO, YES}     yes_no_t;
```
の箇所の「NO」「YES」でエラーになります。そのため、この行の前に以下を追加するようにします。  
```
#undef NO
#undef YES
```

### Shade3D Plugin SDKをダウンロード

https://shade3d.jp/community/sdn/sdk.html より、  
Shade 3D ver.15.1 Plugin SDKをダウンロードします。  
もしくは、GitHub上の  
https://github.com/shadedev/pluginsdk  
をダウンロードします。 

### プロジェクトの配置

SDKの「plugin_projects」ディレクトリに   
GitHubでダウンロードしたファイルの「projects」ディレクトリ内の「UVUnwrapLSCM」を複製します。  
以下のようなディレクトリ構成にします。  
```
  [plugin_projects]  
    [UVUnwrapLSCM]  
      [mac]  
      [source]  
      [win_vs2017]  
```

Win環境の場合は「plugin_projects/UVUnwrapLSCM/win_vs2017/UVUnwrapLSCM.sln」を  
Visual Studio 2017で開いてビルドします。   
Mac環境の場合は「plugin_projects/UVUnwrapLSCM/mac/plugins/UVUnwrapLSCM.xcodeproj」をXcodeで開いてビルドします。  

## ライセンス  

This software is released under the MIT License, see [LICENSE.txt](./LICENSE).  
LSCM展開に「OpenNL」( http://alice.loria.fr/index.php/software/4-library/23-opennl.html )を使用しています。  

## 更新履歴

[2018/02/14]  ver.1.0.0.0  
* 初回バージョン
