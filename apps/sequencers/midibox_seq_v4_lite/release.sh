# $Id: release.sh 2427 2016-11-03 01:47:58Z tk $

if [[ "$1" == "" ]]; then
  echo "SYNTAX: release.sh <release-directory>"
  exit 1
fi

RELEASE_DIR=$1

if [[ -e $RELEASE_DIR ]]; then
  echo "ERROR: the release directory '$RELEASE_DIR' already exists!"
  exit 1
fi

###############################################################################
echo "Creating $RELEASE_DIR"

mkdir $RELEASE_DIR
cp README.txt $RELEASE_DIR
cp CHANGELOG.txt $RELEASE_DIR

mkdir -p $RELEASE_DIR/hwcfg/standard_v4l
cp hwcfg/standard_v4l/MBSEQ_HW.V4L $RELEASE_DIR/hwcfg/standard_v4l

###############################################################################
configs=( stm32f1 lpc17 stm32f4 )
for i in "${configs[@]}"; do
  echo "Building for $i"
  source ../../../../source_me_${i}
  make cleanall
  mkdir -p $RELEASE_DIR/$MIOS32_BOARD
  make > $RELEASE_DIR/$MIOS32_BOARD/log.txt || exit 1
  cp project.hex $RELEASE_DIR/$MIOS32_BOARD
done

###############################################################################
make cleanall
echo "Done!"

