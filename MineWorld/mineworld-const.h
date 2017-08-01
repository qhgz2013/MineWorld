#pragma once
// MineWorld Generator Constants



//save data

//保存数据的子文件夹
const char*	DEFAULT_SAVEDATA_PATH        = "savedata/";
//保存的文件名
const char* DEFAULT_SAVEDATA_FMT         = "chunk-%d-%d.dat";



//chunk cache

//每个区块所保存的方块数 (2^n)
const int   DEFAULT_CHUNK_SIZE           = 10;
//默认最大缓存区块数
const int   DEFAULT_MAX_CHUNK_CACHE      = 100;


//world generation

//默认生成雷的概率
const int   DEFAULT_GEN_MINE_POSSIBILITY = 30;


//game rules
