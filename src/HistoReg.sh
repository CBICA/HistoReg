#!/usr/bin/env bash
####
################################## START OF EMBEDDED SGE COMMANDS ##########################

######## Common options ########
#$ -S /bin/bash  #### Default Shell to be Used
#$ -cwd  #### Run in current directory
#$ -N HistoReg  #### Job Name to be listed in qstat

######## Logging Options ########
#$ -o /cbica/comp_space/$USER/sge_job_output/$JOB_NAME_$JOB_ID.stdout  #### stdout default path
#$ -e /cbica/comp_space/$USER/sge_job_output/$JOB_NAME_$JOB_ID.stderr  #### stderr default path

######## Email Options ########
####$ -M Ludovic.Venet@uphs.upenn.edu  #### email address to nofity with following options/scenarios
####$ -m a ####abort, end notifications - see below lines for more options
####$ -m a  #### send mail in case the job is aborted
####$ -m b #### send mail when job begins
####$ -m e #### send mail when job ends
####$ -m n #### no mail is sent, overrides the other mail related flags
####$ -m s #### send mail when job is suspended

######## Job Related Options ########
#$ -j y #### merge stdout and stderr to a single file at -o path
####$ -b y #### handle command as binary, turn it on if you are calling an executable as qsub argument rather than 
####$ -hold_jid prevjobid #### for waiting as hwq until job with prevjobid finishes
####$ -terse #### print only job id - use this as pipe output of qsub to a variable in bash to use this job id as prerequisite for another job following this job

######## Job Resources Options ########
####$ -p priority #### -1023 - 0; higher number means faster/higher priority. End User can only submit up to 0, system may have priority higher than 0. Default is -100.
#$ -pe threaded 8 #### how many threads to utilize, defaults to one
#$ -l short=TRUE #### Send the job to short queue, which will get killed if not completed <15min CPU time
#$ -l h_vmem=32G #### How much memory total the job can use. Defaults to 4GB ( qconf -sc | grep h_vmem ) - My experience has been: Minimum 500M Recommended 1G at the lowest
#$ -l tmpfree=8G #### How much scratch space the job can use. Defaults to 0GB if not specified by user
####$ -l h_rt=hh:mm:ss #### the job will run at most for hh:mm:ss time

######## CPU Model Specific Request Options ########
####$ -l AMD_Opteron_4184=TRUE  #### Not Tested
####$ -l Intel_Xeon_E5-2630L_v2=TRUE  #### Not Tested
####$ -l Intel_Xeon_E5-2660_v2=TRUE  #### Older compute nodes and newer interactive nodes as of 2019-02-02
####$ -l Intel_Xeon_E5645=TRUE  #### Older interactive nodes as of 2019-02-02
####$ -l Intel_Xeon_Gold_6128=TRUE  #### Not Tested
#$ -l Intel_Xeon_Gold_6130=TRUE
####$ -l Intel_Xeon_X5550=TRUE  #### Not Tested
####$ -l Intel_Xeon_X5667=TRUE  #### Not Tested

######## CPU Instruction Set Availability Options ########
####$ -l AVX=TRUE  #### Has AVX CPU Instruciton available
####$ -l AVX2=TRUE  #### Has AVX2 CPU Instruciton available
####$ -l cubic=TRUE  #### Redirect to newer interactive/compute node

######## Hostname Specific Request Options ########
####$ -l hostname=c1-1  #### Use (qhost | grep lx-amd64) to get all available hostnames

######## Operating System Specific Request Options ########
####$ -l centos7=TRUE  ####
####$ -l centos6=TRUE  #### Resource no longer available as of 2019-02-02
####$ -l centos5=TRUE  #### Resource no longer available as of 2019-02-02

######## Other Options ########
####$ -l brats=TRUE  #### Only go to a node that has brats flag on - check with system administrator for details
############################## END OF DEFAULT EMBEDDED SGE COMMANDS #######################

version="06.11.2006";

function img_dim()
{
	image=${1?};
	which_dim=${2?};

	dim=`$c2d $image -info-full \
	| grep Dim \
	| sed -e "s/\[//" -e "s/\]//" -e "s/,//" -e "s/.*: //" \
	| cut -d ' ' -f${which_dim}`;

	echo $dim;
};

function Help {
  #echo_help_header;
  echo -e "${REGULAR_RED}";

  echo -e "\n";
  echo -e "  Description:";

  echo -e "\n";
  echo -e "    Description Verbose.";

  echo -e "\n";
  echo -e "  Usage:";

  echo -e "\n";
  echo -e "    ${script_name} -o /outdir/path -m /path/to/moving/image -f -path/to/fixed/image [-OPTION argument]";

  echo -e "\n";
  echo -e "  Compulsory Arguments:";

  echo -e "\n";
  echo -e "    -m	 [path]/full/path/to/${UNDERLINE_YELLOW}m${REGULAR_RED}oving/image";
  echo -e "    (variable name: ${UNDERLINE_YELLOW}input${REGULAR_RED})";
  echo -e "    (must be jpg)";

  echo -e "\n";
  echo -e "    -f	 [path]/full/path/to/${UNDERLINE_YELLOW}f${REGULAR_RED}ixed/image";
  echo -e "    (variable name: ${UNDERLINE_YELLOW}atlas${REGULAR_RED})";
  echo -e "    (must be nii.gz)";

  echo -e "\n";
  echo -e "    -o	 [path]/full/path/to/${UNDERLINE_YELLOW}o${REGULAR_RED}utput/directory";
  echo -e "    (variable name: ${UNDERLINE_YELLOW}outdir${REGULAR_RED})";
  echo -e "    (must be directory)";
    
  echo -e "\n";
  echo -e "  Optional Arguments:";

  echo -e "\n";
  echo -e "    -l	 [path]/full/path/to/${UNDERLINE_YELLOW}l${REGULAR_RED}andmarks";
  echo -e "    (variable name: ${UNDERLINE_YELLOW}mask${REGULAR_RED})";
  echo -e "    (must be csv with values XY starting 2nd ROW, 2nd LINE)";
  echo -e " ex: ,X,Y
		1,x,y
                2,x'y'
		....
	   with x,y;x',y' the landmarks coordinates"

  echo -e "\n";
  echo -e "    -s	 [path]/full/path/to/${UNDERLINE_YELLOW}s${REGULAR_RED}egm/file/for/segmentation";
  echo -e "    (variable name: ${UNDERLINE_YELLOW}segm${REGULAR_RED})";
  echo -e "    (must be nii.gz)";

  echo -e "\n";
  echo -e "    -e	 [path]/full/path/to/atlas/S${UNDERLINE_YELLOW}e${REGULAR_RED}egm/file/for/segmentation/of/the/atlas/file";
  echo -e "    (variable name: ${UNDERLINE_YELLOW}atlas_segm${REGULAR_RED})";
  echo -e "    (must be nii.gz)";

  echo -e "\n";
  echo -e "    -g  [path]/path/to/${UNDERLINE_YELLOW}g${REGULAR_RED}reedy/executable";
  echo -e "    (default=${greedy_executable_default})";
  echo -e "    (will try to grab with which command if not supplied)";
  echo -e "    (Currently no cluster installation of greedy exists, so calling on home directory cmake build with gcc/4.9.2 on centos7)";
  
  echo -e "\n";
  echo -e "    -V  [switch]${UNDERLINE_YELLOW}V${REGULAR_RED}ERBOSE mode on";
  echo -e "    (default=${VERBOSE})";

  echo -e "${RESET_ALL}";
};

set -e;
TEMP_Output=${SBIA_TMPDIR}"/"$JOB_NAME_$JOB_ID;

#Initial parameters
s1=6;
s2=5;
resample=4;
smoothing=12;
Kernel_Divider=40;

#Flag for optional parameters
VERBOSE=0;
SAVE=0;
c2d_executable_provided=0;
greedy_executable_provided=0;
resample_provided=0;
landmarks_provided=0;
apply_full_res=0;

#Executables
greedy_executable="/cbica/home/venetl/comp_space/Greedy/bin/greedy";
c2d_executable="/cbica/home/venetl/comp_space/itksnap-experimental-master-Linux-gcc64/itksnap-3.8.0-beta-20181028-Linux-gcc64/bin/c2d";




#get user inputs
while getopts m:f:o:l:g:c:k:s:r:hFVS option
do
  case "${option}"
  in
    s) multi+=("$OPTARG");; #smoothing_parameters+=$OPTARD;;
    h)	Help;   #show help documentation
        #echo_system_information;  #show system information
        #echo_sge_default;         #show SGE default flag information
        exit 0;
        ;;
    m)  moving=${OPTARG};;   #jpg
    f)  fixed=${OPTARG};;    #jpg
    o)	PATH_Output=${OPTARG};;  #nii.gz
    l)  PATH_to_landmarks=${OPTARG};
	landmarks_provided=1;;
    g)  greedy_executable=${OPTARG};
	greedy_executable_provided=1;;
    c)  c2d_executable=${OPTARG};
	c2d_executable_provided=1;;
    r)  resample=${OPTARG};
	resample_provided=1;;
    k) Kernel_Divider=${OPTARG};;
    V)	VERBOSE=1;
        #echoV "VERBOSE Mode on";
        ;;
    S) SAVE=1;
	echo "SAVE MODE ON";;
    F) apply_full_res=1;
	echo "Reslice full resolution images ON";;
    
    ?)	echo "Unrecognized Options. Exiting. " 1>&2;
        Help;
        echo_system_information;
        echo_sge_default;
        exit 1;
        ;;
  esac
done;

if [[ $multi != "" ]];then
	s1=$multi;
	if [[ ${multi[1]} != "" ]];then
		s2=${multi[1]};
	else
		echo -e "Error: Only one smoothing parameters provided out of 2. Exiting.";
		exit 1;
	fi
fi

if [ $resample_provided -eq 1 ];then
	temp=`bc <<< 2*$resample`;
	smoothing=`bc <<< 100/$temp`;
fi

name_fixed=`basename $fixed | cut -d . -f1`;
name_moving=`basename $moving | cut -d . -f1`;

mkdir -p $PATH_Output/$name_moving"_registered_to_"$name_fixed;

PATH_Output=$PATH_Output/$name_moving"_registered_to_"$name_fixed;

mkdir -p $TEMP_Output;

Size_H_source_big=`identify $moving | cut -d ' ' -f3 | cut -d 'x' -f2`
Size_W_source_big=`identify $moving | cut -d ' ' -f3 | cut -d 'x' -f1`

Size_H_big=`identify $fixed | cut -d ' ' -f3 | cut -d 'x' -f2`
Size_W_big=`identify $fixed | cut -d ' ' -f3 | cut -d 'x' -f1`

echo "--------------------------------------------------------------------------------------------------------------------------"
echo "Resampling images"
$c2d_executable $fixed -smooth-fast $smoothing"x"$smoothing"vox" -resample $resample"%" -spacing 1x1mm -orient LP -origin 0x0mm -o $TEMP_Output/new_small_target.nii.gz;

$c2d_executable $moving -smooth-fast $smoothing"x"$smoothing"vox" -resample $resample"%" -spacing 1x1mm -orient LP -origin 0x0mm -o $TEMP_Output/new_small_source.nii.gz;

Size_W_init=`/cbica/comp_space/venetl/HistoRegGreedy/src/img_dim.sh $TEMP_Output/new_small_target.nii.gz 1`;
Size_H_init=`/cbica/comp_space/venetl/HistoRegGreedy/src/img_dim.sh $TEMP_Output/new_small_target.nii.gz 2`;

###echo "test"
###a=`img_dim /cbica/comp_space/venetl/test/new_small_target.nii.gz 1`
###Size_W=`img_dim $TEMP_Output/new_small_target.nii.gz 1`;
###Size_H=`img_dim $TEMP_Output/new_small_target.nii.gz 2`;

kernel_W=`bc <<< $Size_W_init/$Kernel_Divider`
kernel_H=`bc <<< $Size_H_init/$Kernel_Divider`

kernel=$kernel_H

if [[ $kernel_H -lt $kernel_W ]];then
	kernel=$kernel_W
fi

echo "Size kernel : "$kernel"x"$kernel

# extract mean and std of the background for padding
stat_target=`$c2d_executable $TEMP_Output/new_small_target.nii.gz -dup -cmv -popas Y -popas X -push X -thresh 0 $kernel 1 0 -push Y -thresh 0 $kernel 1 0 -times -popas c00 -push X -thresh $(($Size_W_init-$kernel)) $Size_W_init 1 0 -push Y -thresh 0 $kernel 1 0 -times -popas c01 -push X -thresh $(($Size_W_init-$kernel)) $Size_W_init 1 0 -push Y -thresh $(($Size_H_init-$kernel)) $Size_H_init 1 0 -times -popas c11 -push X -thresh 0 $kernel 1 0 -push Y -thresh $(($Size_H_init-$kernel)) $Size_H_init 1 0 -times -popas c10 -push c00 -push c01 -push c11 -push c10 -add -add -add -lstat | grep " 1 "`

mean_target=`echo $stat_target | cut -d ' ' -f2`
std_target=`echo $stat_target | cut -d ' ' -f3`

Size_W_source_init=`/cbica/comp_space/venetl/HistoRegGreedy/src/img_dim.sh $TEMP_Output/new_small_source.nii.gz 1`;
Size_H_source_init=`/cbica/comp_space/venetl/HistoRegGreedy/src/img_dim.sh $TEMP_Output/new_small_source.nii.gz 2`;

kernel_H_source=`bc <<< $Size_H_source_init/$Kernel_Divider`
kernel_W_source=`bc <<< $Size_W_source_init/$Kernel_Divider`

kernel_source=$kernel_H_source

if [[ $kernel_H_source -lt $kernel_W_source ]];then
	kernel_source=$kernel_W_source
fi

stat_source=`$c2d_executable $TEMP_Output/new_small_source.nii.gz -dup -cmv -popas Y -popas X -push X -thresh 0 $kernel_source 1 0 -push Y -thresh 0 $kernel_source 1 0 -times -popas c00 -push X -thresh $(($Size_W_source_init-$kernel_source)) $Size_W_source_init 1 0 -push Y -thresh 0 $kernel_source 1 0 -times -popas c01 -push X -thresh $(($Size_W_source_init-$kernel_source)) $Size_W_source_init 1 0 -push Y -thresh $(($Size_H_source_init-$kernel_source)) $Size_H_source_init 1 0 -times -popas c11 -push X -thresh 0 $kernel_source 1 0 -push Y -thresh $(($Size_H_source_init-$kernel_source)) $Size_H_source_init 1 0 -times -popas c10 -push c00 -push c01 -push c11 -push c10 -add -add -add -lstat | grep " 1 "`

mean_source=`echo $stat_source | cut -d ' ' -f2`
std_source=`echo $stat_source | cut -d ' ' -f3`


four_kernel_H=`bc <<< $kernel*4`
four_kernel_W=`bc <<< $kernel*4`

big_four_kernel_H=`bc <<< $four_kernel_H*25`
big_four_kernel_W=`bc <<< $four_kernel_W*25`

echo "Size small target : "$Size_W_init"x"$Size_H_init
echo "Size small source : "$Size_W_source_init"x"$Size_H_source_init


# Match size
New_size_H=$Size_H_init
New_size_W=$Size_W_init
if [[ $Size_H_source_init"x"$Size_W_source_init != $Size_H_init"x"$Size_W_init ]];then

	echo "Images sizes are different"
	echo "Modifying sizes"

	if [[ $Size_H_init -lt $Size_H_source_init ]];then
		New_size_H=$Size_H_source_init
	fi	
	if [[ $Size_H_source_init -lt $Size_H_init ]];then
		New_size_H=$Size_H_init
	fi
	if [[ $Size_W_init -lt $Size_W_source_init ]];then
		New_size_W=$Size_W_source_init
	fi
	if [[ $Size_W_source_init -lt $Size_W_init ]];then
		New_size_W=$Size_W_init
	fi

	$c2d_executable $TEMP_Output/new_small_target.nii.gz -pad-to $New_size_W"x"$New_size_H 0 -o $TEMP_Output/new_small_target_padded.nii.gz	

	$c2d_executable $TEMP_Output/new_small_source.nii.gz -pad-to $New_size_W"x"$New_size_H 0 -o $TEMP_Output/new_small_source_padded.nii.gz
else
	cp $TEMP_Output/new_small_source.nii.gz $TEMP_Output/new_small_source_padded.nii.gz
	cp $TEMP_Output/new_small_target.nii.gz $TEMP_Output/new_small_target_padded.nii.gz
fi 


# Target 
echo "Create mask"
$c2d_executable $TEMP_Output/new_small_target_padded.nii.gz -thresh 1 inf 1 0 -pad $four_kernel_W"x"$four_kernel_H $four_kernel_W"x"$four_kernel_H 0 -o $TEMP_Output/mask_target.nii.gz

echo "Create mask with noise"
$c2d_executable $TEMP_Output/mask_target.nii.gz -replace 0 1 1 0 -popas invmask $TEMP_Output/mask_target.nii.gz -replace 0 1 1 1 -scale $mean_target -noise-gaussian $std_target -push invmask -times -o $TEMP_Output/test.nii.gz

echo "Padding image"
$c2d_executable $TEMP_Output/new_small_target_padded.nii.gz -pad $four_kernel_W"x"$four_kernel_H $four_kernel_W"x"$four_kernel_H 0 -o $TEMP_Output/new_small_target_padded.nii.gz

echo "Add mask with noise and padded image"
$c2d_executable $TEMP_Output/test.nii.gz $TEMP_Output/new_small_target_padded.nii.gz -add -o $TEMP_Output/new_small_target_padded.nii.gz


# Source
echo "Create mask"
$c2d_executable $TEMP_Output/new_small_source_padded.nii.gz -thresh 1 inf 1 0 -pad $four_kernel_W"x"$four_kernel_H $four_kernel_W"x"$four_kernel_H 0 -o $TEMP_Output/mask_source.nii.gz

echo "Create mask with noise"
$c2d_executable $TEMP_Output/mask_source.nii.gz -replace 0 1 1 0 -popas invmask $TEMP_Output/mask_source.nii.gz -replace 0 1 1 1 -scale $mean_source -noise-gaussian $std_source -push invmask -times -o $TEMP_Output/test.nii.gz

echo "Padding image"
$c2d_executable $TEMP_Output/new_small_source_padded.nii.gz -pad $four_kernel_W"x"$four_kernel_H $four_kernel_W"x"$four_kernel_H 0 -o $TEMP_Output/new_small_source_padded.nii.gz

echo "Add mask with noise and padded image"
$c2d_executable $TEMP_Output/test.nii.gz $TEMP_Output/new_small_source_padded.nii.gz -add -o $TEMP_Output/new_small_source_padded.nii.gz


Size_W=`fslinfo $TEMP_Output/new_small_target_padded.nii.gz | grep '^dim1' | cut -d ' ' -f12`
Size_H=`fslinfo $TEMP_Output/new_small_target_padded.nii.gz | grep '^dim2' | cut -d ' ' -f12`

Size_W_source=`fslinfo $TEMP_Output/new_small_source_padded.nii.gz | grep '^dim1' | cut -d ' ' -f12`
Size_H_source=`fslinfo $TEMP_Output/new_small_source_padded.nii.gz | grep '^dim2' | cut -d ' ' -f12`

echo "Size small target : "$Size_W"x"$Size_H
echo "Size small source : "$Size_W_source"x"$Size_H_source

echo "--------------------------------------------------------------------------------------------------------------------------"
echo "Registration parameters : "
echo "NCC Affine : "$kernel"x"$kernel
echo "NCC Deformable : "$kernel"x"$kernel
echo "********************"
# Compute Affine registration
echo "Computing affine registration"

offset=`bc <<< $Size_W/10`
start=`date +%s`

$greedy_executable -d 2 -a -search 5000 180 $offset -m NCC $kernel"x"$kernel -i $TEMP_Output/new_small_target_padded.nii.gz $TEMP_Output/new_small_source_padded.nii.gz -o $TEMP_Output/small_Affine.mat -gm-trim $kernel"x"$kernel -n 100x50x10 -ia-image-centers


echo "--------------------------------------------------------------------------------------------------------------------------"
# Compute non-rigid registration
echo "Computing non-rigid registration"

$greedy_executable -d 2 -m NCC $kernel"x"$kernel -i $TEMP_Output/new_small_target_padded.nii.gz $TEMP_Output/new_small_source_padded.nii.gz -it $TEMP_Output/small_Affine.mat -o $TEMP_Output/small_warp.nii.gz -oinv $TEMP_Output/small_inv_warp.nii.gz -n 100x50x10 -s $s1"vox" $s2"vox"

end=`date +%s`
runtime=$(($end-$start))
echo "Computation of the registrations metrics took :" $runtime" secondes"

if [ $landmarks_provided -eq 1 ];then
	echo "Applying trasnformations on the landmarks"
	LM_MOVING_FULL=$PATH_to_landmarks"/"$name_moving".csv"
	LM_MOVING_SMALL=$TEMP_Output/lm_small_source.csv
	LM_WARPED_SMALL=$TEMP_Output/lm_small_source_warped.csv
	LM_WARPED_FULL=$PATH_Output/warped_landmarks.csv

	Size_W_target_nopad=`fslinfo $TEMP_Output/new_small_target.nii.gz | grep '^dim1' | cut -d ' ' -f12`
	Size_H_target_nopad=`fslinfo $TEMP_Output/new_small_target.nii.gz | grep '^dim2' | cut -d ' ' -f12`
	Size_W_source_nopad=`fslinfo $TEMP_Output/new_small_source.nii.gz | grep '^dim1' | cut -d ' ' -f12`
	Size_H_source_nopad=`fslinfo $TEMP_Output/new_small_source.nii.gz | grep '^dim2' | cut -d ' ' -f12`

	# Step 1: map source landmarks into small image space
	cat $LM_MOVING_FULL \
	  | awk -F, -v wbig=$Size_W_source_big -v hbig=$Size_H_source_big \
	    -v wsmall=$Size_W_source_nopad -v hsmall=$Size_H_source_nopad \
	    'NR > 1 {printf("%f,%f\n",($2*1.0*wsmall)/wbig-0.5,($3*1.0*hsmall)/hbig-0.5)}' \
	    > $LM_MOVING_SMALL

	# Step 2: apply inverse warp and inverse affine to the moving landmarks
	$greedy_executable -d 2 \
	  -rf $TEMP_Output/new_small_source.nii.gz -rs $LM_MOVING_SMALL $LM_WARPED_SMALL \
	  -r $TEMP_Output/small_Affine.mat,-1 $TEMP_Output/small_inv_warp.nii.gz

	# Step 3: map warped landmarks to full size
	cat $LM_WARPED_SMALL \
	  | awk -F, -v wbig=$Size_W_big -v hbig=$Size_H_big \
	    -v wsmall=$Size_W_target_nopad -v hsmall=$Size_H_target_nopad \
	    'BEGIN {printf(",X,Y\n")} {printf("%d,%f,%f\n",NR-1,(($1+0.5)*wbig)/wsmall,(($2+0.5)*hbig)/hsmall)}' > $LM_WARPED_FULL
fi

# Create sshot
echo "Applying on small image registration"

$greedy_executable -d 2 -rf $TEMP_Output/new_small_target_padded.nii.gz -rm $TEMP_Output/new_small_source_padded.nii.gz $TEMP_Output/small_registeredImage.nii.gz -r $TEMP_Output/small_warp.nii.gz $TEMP_Output/small_Affine.mat

mkdir -p $TEMP_Output/sshot/
$c2d_executable $TEMP_Output/new_small_target_padded.nii.gz -stretch 0 99% 0 255 -type uchar -o $TEMP_Output/sshot/new_small_target.png
$c2d_executable $TEMP_Output/new_small_source_padded.nii.gz -stretch 0 99% 0 255 -type uchar -o $TEMP_Output/sshot/new_small_source.png
$c2d_executable $TEMP_Output/small_registeredImage.nii.gz -stretch 0 99% 0 255 -type uchar -o $TEMP_Output/sshot/small_registeredImage.png

mkdir -p $PATH_Output/sshot/

montage -geometry +0+0 -tile 3x $TEMP_Output/sshot/*.png $PATH_Output/sshot/$name_moving"_to_"$name_fixed.png


if [[ $apply_full_res -eq 1 ]];then

	echo "Reslicing full resolutions images"
	start=`date +%s`

	tr ' ' ',' <$TEMP_Output/small_Affine.mat> $TEMP_Output/temp.mat

	lines=`cat $TEMP_Output/temp.mat`
	rm -f $TEMP_Output/Affine.mat

	i=0

	for line in $lines
	do
		if [[ $i -lt 2 ]];then
			line_to_keep=`echo $line | cut -d , -f1-2`
			translation_to_change=`echo $line | cut -d , -f3`
			#new_translation=`echo $(( $translation_to_change * 25 ))`
			factor=`bc <<< 100/$resample`
			new_translation=`bc <<< $translation_to_change*$factor`
			new_line=$line_to_keep","$new_translation
		fi
		if [[ $i -eq 2 ]];then
			new_line=$line
		fi
		echo $new_line >> $TEMP_Output/temp_Affine.mat
		
		i=`bc <<< $i+1`
	done

	tr ',' ' ' <$TEMP_Output/temp_Affine.mat> $TEMP_Output/Affine.mat


	Diff_W_Target=`bc <<< $Size_W-$Size_W_init`
	Diff_H_Target=`bc <<< $Size_H-$Size_H_init`
	Pad_full_res_W=`bc <<< $Diff_W_Target*100/$resample`
	Pad_full_res_H=`bc <<< $Diff_H_Target*100/$resample`

	new_dim_W=`bc <<< $Size_W_big+$Pad_full_res_W`
	new_dim_H=`bc <<< $Size_H_big+$Pad_full_res_H`

	echo "$c2d_executable -mcs $TEMP_Output/small_warp.nii.gz -foreach -resample $new_dim_W"x"$new_dim_H -spacing 1x1mm -origin 0x0mm -endfor -omc $TEMP_Output/big_warp.nii.gz"
	$c2d_executable -mcs $TEMP_Output/small_warp.nii.gz -foreach -resample $new_dim_W"x"$new_dim_H -scale $factor -spacing 1x1mm -origin 0x0mm -endfor -omc $TEMP_Output/big_warp.nii.gz


	echo "$c2d_executable -background 1 -create $Size_W_big"x"$Size_H_big 1x1mm -origin 0x0mm -o $TEMP_Output/mask_full_res.nii.gz"
	$c2d_executable -background 1 -create $Size_W_big"x"$Size_H_big 1x1mm -orient LP -o $TEMP_Output/mask_full_res.nii.gz

	echo "$c2d_executable $TEMP_Output/mask_full_res.nii.gz -pad-to $new_dim_W"x"$new_dim_H 0 -o $TEMP_Output/mask_full_res_padded.nii.gz"
	$c2d_executable $TEMP_Output/mask_full_res.nii.gz -pad-to $new_dim_W"x"$new_dim_H 0 -o $TEMP_Output/mask_full_res_padded.nii.gz

	echo "$c2d_executable $TEMP_Output/mask_full_res_padded.nii.gz -spacing 1x1mm -origin 0x0mm -o $TEMP_Output/mask_full_res_padded.nii.gz"
	$c2d_executable $TEMP_Output/mask_full_res_padded.nii.gz -origin 0x0mm -o $TEMP_Output/mask_full_res_padded.nii.gz

	echo "$c2d_executable $TEMP_Output/mask_full_res_padded.nii.gz -popas mask -mcs $TEMP_Output/big_warp.nii.gz -foreach -push mask -times -endfor -omc $TEMP_Output/big_warp_no_pad.nii.gz"
	$c2d_executable $TEMP_Output/mask_full_res_padded.nii.gz -popas mask -mcs $TEMP_Output/big_warp.nii.gz -foreach -push mask -times -endfor -omc $TEMP_Output/big_warp_no_pad.nii.gz

	echo "$c2d_executable -mcs $TEMP_Output/big_warp_no_pad.nii.gz -foreach -trim 0vox -endfor -omc $TEMP_Output/big_warp_no_pad_trim.nii.gz"
	$c2d_executable -mcs $TEMP_Output/big_warp_no_pad.nii.gz -foreach -trim 0vox -endfor -omc $TEMP_Output/big_warp_no_pad_trim.nii.gz


		
	echo "$c2d_executable -mcs $fixed -foreach -orient LP -spacing 1x1mm -origin 0x0mm -endfor -omc $TEMP_Output/new_target.nii.gz"
	$c2d_executable -mcs $fixed -foreach -orient LP -spacing 1x1mm -origin 0x0mm -endfor -omc $TEMP_Output/new_target.nii.gz

	echo "$c2d_executable -mcs $moving -foreach -orient LP -spacing 1x1mm -origin 0x0mm -endfor -omc $TEMP_Output/new_source.nii.gz"
	$c2d_executable -mcs $moving -foreach -orient LP -spacing 1x1mm -origin 0x0mm -endfor -omc $TEMP_Output/new_source.nii.gz


	$c2d_executable -mcs $TEMP_Output/big_warp_no_pad_trim.nii.gz -foreach -origin 0x0mm -endfor -omc $TEMP_Output/big_warp_no_pad_trim.nii.gz

	$greedy_executable -d 2 -rf $TEMP_Output/new_target.nii.gz -rm $TEMP_Output/new_source.nii.gz $TEMP_Output/registeredImage.nii.gz -r $TEMP_Output/big_warp_no_pad_trim.nii.gz $TEMP_Output/Affine.mat

	end=`date +%s`
	runtime=$(($end-$start))
	echo "Reslicing the full resolution images took :" $runtime" secondes"


	mkdir -p $TEMP_Output/sshot/full_res
	$c2d_executable -mcs $TEMP_Output/new_target.nii.gz -foreach -stretch 0 99% 0 255 -type uchar -endfor -omc $TEMP_Output/sshot/full_res/new_target.png
	$c2d_executable -mcs $TEMP_Output/new_source.nii.gz -foreach -stretch 0 99% 0 255 -type uchar -endfor -omc $TEMP_Output/sshot/full_res/new_source.png
	$c2d_executable -mcs $TEMP_Output/registeredImage.nii.gz -foreach -stretch 0 99% 0 255 -type uchar -endfor -omc $TEMP_Output/sshot/full_res/registeredImage.png


	cp $TEMP_Output/new_target.nii.gz $PATH_Output/new_target.nii.gz 
	cp $TEMP_Output/new_source.nii.gz $PATH_Output/new_source.nii.gz
	cp $TEMP_Output/registeredImage.nii.gz $PATH_Output/registeredImage.nii.gz
	
	cp $TEMP_Output/sshot/full_res/new_target.png $PATH_Output/new_target.png
	cp $TEMP_Output/sshot/full_res/new_source.png $PATH_Output/new_source.png
	cp $TEMP_Output/sshot/full_res/registeredImage.png $PATH_Output/registeredImage.png

	mkdir -p $PATH_Output/sshot/full_res

	montage -geometry +0+0 -tile 3x $TEMP_Output/sshot/full_res/*.png $PATH_Output/sshot/full_res/$name_moving"_to_"$name_fixed"_full_res.png"



	#TEST
#	Diff_W_Source=`bc <<< $Size_W_source-$Size_W_source_init`
#	Diff_H_Source=`bc <<< $Size_H_source-$Size_H_source_init`
#	Pad_full_res_W=`bc <<< $Diff_W_Source*100/$resample`
#	Pad_full_res_H=`bc <<< $Diff_H_Source*100/$resample`
#
#	new_dim_W=`bc <<< $Size_W_source_big+$Pad_full_res_W`
#	new_dim_H=`bc <<< $Size_H_source_big+$Pad_full_res_H`
#
#	echo "$c2d_executable -mcs $TEMP_Output/small_inv_warp.nii.gz -foreach -resample $new_dim_W"x"$new_dim_H -scale $factor -spacing 1x1mm -origin 0x0mm -endfor -omc $TEMP_Output/big_inv_warp.nii.gz"
#	$c2d_executable -mcs $TEMP_Output/small_inv_warp.nii.gz -foreach -resample $new_dim_W"x"$new_dim_H -scale $factor -spacing 1x1mm -origin 0x0mm -endfor -omc $TEMP_Output/big_inv_warp.nii.gz
#
#	echo "$c2d_executable -background 1 -create $Size_W_big"x"$Size_H_big 1x1mm -orient LP -o $TEMP_Output/mask_full_res.nii.gz"
#	$c2d_executable -background 1 -create $Size_W_big"x"$Size_H_big 1x1mm -orient LP -o $TEMP_Output/mask_full_res.nii.gz
#
#	echo "$c2d_executable $TEMP_Output/mask_full_res.nii.gz -pad-to $new_dim_W"x"$new_dim_H 0 -o $TEMP_Output/mask_full_res_padded.nii.gz"
#	$c2d_executable $TEMP_Output/mask_full_res.nii.gz -pad-to $new_dim_W"x"$new_dim_H 0 -o $TEMP_Output/mask_full_res_padded.nii.gz
#
#	echo "$c2d_executable $TEMP_Output/mask_full_res_padded.nii.gz -origin 0x0mm -o $TEMP_Output/mask_full_res_padded.nii.gz"
#	$c2d_executable $TEMP_Output/mask_full_res_padded.nii.gz -origin 0x0mm -o $TEMP_Output/mask_full_res_padded.nii.gz
#
#	echo "$c2d_executable $TEMP_Output/mask_full_res_padded.nii.gz -popas mask -mcs $TEMP_Output/big_inv_warp.nii.gz -foreach -push mask -times -endfor -omc $TEMP_Output/big_inv_warp_no_pad.nii.gz"
#	$c2d_executable $TEMP_Output/mask_full_res_padded.nii.gz -popas mask -mcs $TEMP_Output/big_inv_warp.nii.gz -foreach -push mask -times -endfor -omc $TEMP_Output/big_inv_warp_no_pad.nii.gz
#
#	
#	echo "$c2d_executable -mcs $TEMP_Output/big_inv_warp_no_pad.nii.gz -foreach -trim 0vox -endfor -omc $TEMP_Output/big_inv_warp_no_pad_trim.nii.gz"
#	$c2d_executable -mcs $TEMP_Output/big_inv_warp_no_pad.nii.gz -foreach -trim 0vox -endfor -omc $TEMP_Output/big_inv_warp_no_pad_trim.nii.gz
#	
#
#	LM_WARPED_FULL_test=$PATH_Output/warped_landmarks_test.csv
#
#	echo "$greedy_executable -d 2 \
#  -rf $TEMP_Output/new_source.nii.gz -rs $LM_MOVING_FULL $LM_WARPED_FULL_test \
#  -r $TEMP_Output/Affine.mat,-1 $TEMP_Output/big_inv_warp_no_pad_trim.nii.gz"
#	$greedy_executable -d 2 \
#  -rf $TEMP_Output/new_source.nii.gz -rs $PATH_to_landmarks"/"$name_moving".csv" $LM_WARPED_FULL_test \
#  -r $TEMP_Output/Affine.mat,-1 $TEMP_Output/big_inv_warp_no_pad_trim.nii.gz

fi



if [[ $SAVE -eq 1 ]];then
	mv $TEMP_Output/new_small_target_padded.nii.gz $PATH_Output/new_small_target_padded.nii.gz
	mv $TEMP_Output/new_small_source_padded.nii.gz $PATH_Output/new_small_source_padded.nii.gz
	mv $TEMP_Output/small_Affine.mat $PATH_Output/small_Affine.mat
	mv $TEMP_Output/small_warp.nii.gz $PATH_Output/small_warp.nii.gz
	mv $TEMP_Output/small_registeredImage.nii.gz $PATH_Output/small_registeredImage.nii.gz
	
	if [[ $apply_full_res -eq 1 ]];then
		mv $TEMP_Output/new_target.nii.gz $PATH_Output/new_target.nii.gz
		mv $TEMP_Output/new_source.nii.gz $PATH_Output/new_source.nii.gz
		mv $TEMP_Output/registeredImage.nii.gz $PATH_Output/registeredImage.nii.gz
		mv $TEMP_Output/Affine.mat $PATH_Output/Affine.mat
		mv $TEMP_Output/big_warp_no_pad_trim.nii.gz $PATH_Output/warp.nii.gz
	fi
fi








