# In case you are wondering, this file should be read with 'source' or '.' bash builtin
# TODO: Shouldn't this be put in a proper Makefile?

TARGET_DIR=$(ls -1 results/ | tail -n1)
cd results/$TARGET_DIR/

