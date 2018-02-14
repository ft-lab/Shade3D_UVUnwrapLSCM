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
Macの場合は、ビルドされた UVUnwrapLSCM64.shdpluginをShade3Dのpluginsディレクトリに格納してShade 3Dを起動。  
メインメニューの「ツール」-「プラグイン」-「選択エッジをシーンとして追加」が表示されるのを確認します。  

### Shade3D ver.16/17の標準搭載のLSCMとの違い

* LSCMによるUV展開後、グループごとに重ならないように再配置します。
* UV展開でグループに分けた際に、個々のグループごとのUVサイズが大きく変わらないようにしています。  
UV展開でのサイズが、完全にジオメトリと比例するわけではありません。
* LSCM展開後に1つのグループとなる場合、同一グループで切れ目(Seam)を入れた展開ができるようにしています。  
円柱の側面に縦線1本分の切れ目(Seam)を入れて、ぐるっと展開する場合など。  

## ビルド方法 (開発向け)

Windows環境では「Visual Studio 2017」(無償のCommunity版でビルドできます)、  
Mac環境では「XCode 6.4」でビルドしました。  

### OpenNLのファイルをソースに追加  
LSCM展開に「OpenNL」( http://alice.loria.fr/index.php/software/4-library/23-opennl.html )を使用しています。  
「OpenNL_psm_1.5.2.zip」をダウンロードし、解凍後にOpenNL_psm.c/OpenNL_psm.hの2つのファイルを  
projects/sourceディレクトリにコピーします。  

### Shade3D Plugin SDKをダウンロード

https://shade3d.jp/community/sdn/sdk.html より、  
Shade 3D ver.15.1 Plugin SDKをダウンロードします。  
もしくは、GitHub上の  
https://github.com/shadedev/pluginsdk  
をダウンロードします。 

### プロジェクトの配置

SDKの「plugin_projects」ディレクトリに「UVUnwrapLSCM」ディレクトリを作成し、  
GitHubでダウンロードしたファイルの「projects」ディレクトリ内を複製します。  
```
  [plugin_projects]  
    [UVUnwrapLSCM]  
      [mac]  
      [source]  
      [win_vs2017]  
```

Win環境の場合は「plugin_projects/UVUnwrapLSCM/win_vs2017/UVUnwrapLSCM.sln」を  
Visual Studio 2017で開いてビルドします。   

## ライセンス  

This software is released under the MIT License, see [LICENSE.txt](./LICENSE.txt).  
LSCM展開に「OpenNL」( http://alice.loria.fr/index.php/software/4-library/23-opennl.html )を使用しています。  
