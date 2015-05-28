## dbtshougi-analysis

### create_all_board.sh

あり得る全ての盤面を
取得する．１つの盤面は64ビットで表現され，２GB
強程度のファイル（all-state.dat）を出力する．

### remove_end_board.sh

all-state.datから末端局面を除いたall-state_removeend.datを出力する．

### create_win_lose_board.sh

勝ち確定局面と負け確定局面を出力する．
all-state.datをソートしたall-state_sorted.dat を入力として，judge
.dat と judje_count.dat を出力する．

### create_next_board.sh

all-state_sorted.dat，judge.dat，judge_count.datを読み込み，
ある局面とその局面から遷移すべき次の局面データ next_state.dat を出力する．
