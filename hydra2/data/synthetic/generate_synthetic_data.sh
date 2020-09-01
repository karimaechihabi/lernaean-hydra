GENERATOR=$PROJECTS_ROOT/tools/Cgenerator/generator/generator

export GSL_RNG_SEED=1184
#$GENERATOR --size 1000000 --length 256  --z-normalize --filename "$DATASETS/data_size1GB_seed1184_len256_znorm.bin"

#DATASETS 
#export GSL_RNG_SEED=1184
#$GENERATOR --size 250000000 --length 256  --z-normalize --filename "$DATASETS/data_size250GB_seed1184_len256_znorm.bin"
#$GENERATOR --size 390625 --length 16384  --z-normalize --filename "$DATASETS/data_size25GB_seed1184_len16384_znorm.bin"

#QUERIES
export GSL_RNG_SEED=14784
$GENERATOR --size 100  --length 256  --z-normalize --filename "$DATASETS/queries_size100_seed14784_len256_znorm.bin"
$GENERATOR --size 100 --length 16384  --z-normalize --filename "$DATASETS/queries_size100_seed14784_len16384_znorm.bin"






