<?php

/**
 * 64bit整数が連続したファイルの中で，index番目の64bit整数を取得
 */
function get_value($index,  $filename) {
    $fh = fopen($filename, "rb");
    fseek($fh, 8*$index);
    $data = fread($fh, 8);
    fclose($fh);
    $array = unpack("C*", $data);
    $x = 0;
    for ($i=1; $i<=8; $i++){
        $x += $array[$i] << (8*($i-1));
    }
    return $x;
}

/**
 * 64bit整数が連続したファイルの中で，abinが何番目に現れるか取得
 */
function get_index($min, $max, $abin, $filename) {
    if ($min > $max) return -1;
    $mid = (int)(($min+$max)/2);

    $x = get_value($mid, $filename);

    if ($x == $abin) {
        return $mid;
    } else if ($abin > $x) {
        return get_index($mid+1, $max, $abin, $filename);
    } else {
        return get_index($min, $mid-1, $abin, $filename);
    }
}

/**
 * 次の局面を取得
 */
function get_next_abin($abin) {
    $index = get_index(0, 246803166, $abin, './all-state_sorted.dat');
    if ($index == -1) {
        return -1;
    } else {
        return get_value($index, './next_state.dat');
    }
}

/**
 * 次の局面を返す．
 */

if (isset($_GET['board'])) {
    $p = (int)($_GET['board']);
} else {
    $p = 0;
}
$next = (int)get_next_abin($p);

if ($next == -1) {
    header("HTTP/1.0 404 Not Found");
}
echo $next."\n";
