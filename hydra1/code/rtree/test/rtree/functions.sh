#!/bin/bash
echo "*** Running leaf size experiments ***"

###########################
# Experiment run patterns #
###########################
### EXPERIMENT SETTINGS ###
#FLUSH_LIMIT=20000000
FLUSH_LIMIT=23000000
SAX=8
PAA=4

function index_data() {
    DATASET_FILE=$1
    INITIAL_LEAF_SIZE=$2;
    DATASET_SIZE=$3;

    TARGET_DIRECTORY=$4;
    OUTPUT_FILE=$TARGET_DIRECTORY".tsv"

	echo "*** Indexing ADS experiment for leaf size: $1"
	echo ">>> Starting ADS."
    ./serial-search/ads_isax --dataset $DATASET_FILE \
			--dataset-size $DATASET_SIZE \
                        --paa-segments $PAA \
			--flush-limit $FLUSH_LIMIT \
			--sax-cardinality $SAX \
			--timeseries-size 256 \
            --initial-lbl-size $INITIAL_LEAF_SIZE \
            --leaf-size $INITIAL_LEAF_SIZE \
            --min-leaf-size $INITIAL_LEAF_SIZE \
            --index-path $TARGET_DIRECTORY"/" \
			--complete-type 1 | tee $OUTPUT_FILE;

	echo ">>> Experiment finished, sending results via email.";
}

function serial_query_index() {
    INITIAL_LEAF_SIZE=$1;
    DATASET_SIZE=$2;
    QUERIES_SIZE=$3;
    MIN_LEAVES=$4;
    QUERIES_FILE=$5;
    TARGET_DIRECTORY=$6;

    echo ">>> Cloning index.";
    cp -r $TARGET_DIRECTORY $TARGET_DIRECTORY".copy";

    echo ">>> Issuing $QUERIES_SIZE queries with serial ($MIN_LEAVES).";
    ./serial-search/ads_isax --queries $QUERIES_FILE \
        --use-index \
        --queries-size $QUERIES_SIZE \
        --index-path $TARGET_DIRECTORY"/" \
        --serial \
        --tight-bound \
        --min-leaf-size $INITIAL_LEAF_SIZE \
        --min-checked-leaves $MIN_LEAVES;

    rm -rf $TARGET_DIRECTORY
    mv $TARGET_DIRECTORY".copy" $TARGET_DIRECTORY
}

function tight_query_index() {
    INITIAL_LEAF_SIZE=$1;
    DATASET_SIZE=$2;
    QUERIES_SIZE=$3;
    MIN_LEAVES=$4;
    QUERIES_FILE=$5;
    TARGET_DIRECTORY=$6;
    

    echo ">>> Cloning index.";
    cp -r $TARGET_DIRECTORY $TARGET_DIRECTORY".copy/";

    echo ">>> Issuing $QUERIES_SIZE queries with tight bounds.";
    ./serial-search/ads_isax --queries $QUERIES_FILE \
        --use-index \
        --queries-size $QUERIES_SIZE \
        --index-path $TARGET_DIRECTORY"/" \
        --min-checked-leaves $MIN_LEAVES \
        --min-leaf-size $INITIAL_LEAF_SIZE \
        --tight-bound

    rm -rf $TARGET_DIRECTORY
    mv $TARGET_DIRECTORY".copy" $TARGET_DIRECTORY
}


