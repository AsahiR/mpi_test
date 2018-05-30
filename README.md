# 内容
MPIをつかってdiffusion問題を並列化した.分散メモリ難しや.
# 実行環境
tsubame3のfノード(28core)
# ディレクトリ構成  
.
├── Makefile  
├── diffusion.c  
├── job.sh  
├── log  
├── plot.py  
├── sort_by_data.sh  
├── sort_by_proc.sh  
* sort_by_data.sh  
データサイズに関するplotをするのにplot.pyに渡すcsvをlog中のcsvからつくるスクリプト  
* sort_by_proc.sh  
プロセス数に関するplotをするのにpolot.pyに渡すcsvをlog中のcsvからつくるスクリプト
* sort  
マージソートのソース+Makefile+ジョブスクリプトjob.sh
* log  
出力結果.csvファイルが格納される.
* plot.py 
logファイルを読み込んでplot図をeps形式で出力する.  
# ビルド&実行方法
Makefileと同じ階層でmakeすると実行形式をget
* diffusion  
  * job.sh  
`qsub job.sh -t time_size -c core_size -i data_size` 
    * -n 1ノードにあたりのプロセス数.デフォルトはプロセスサイズと同じ.   
    * -p プロセスサイズ  デフォルトは28  
    * -i 行列の1辺のサイズを2の対数で指定.デフォルトは13  
  * 実行形式  
`diffusion_y -l -x column_size -y row_size -t time_step` 
    * -l 結果をoutput/以下に出力.答え合わせにつかえる.  
    * -y 行列の列サイズ.デフォルトは8192  
    * -t 時間数.デフォルトは40   
  
# 実行結果  
* diffusion問題  
<figure>
<legend>データサイズ or スレッド数を変更した場合</legend>
<img src="http://art1.photozou.jp/pub/135/3222135/photo/256023390.v1527684966.png">
</figure>  
