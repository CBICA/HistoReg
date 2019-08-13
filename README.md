# HistoReg
This repository describes a framework for automated registration of variably-stained digitized histology slices from various anatomical sites, based on a [greedy diffeomorphic registration tool](https://sites.google.com/view/greedyreg/about).

The dataset provided for the quantitative evaluation of the proposed approach was provided by the Automatic Non-rigid Histological Image Registration (ANHIR) challenge [2-6]. This challenge was a part of the IEEE International Symposium on Biomedical Imaging (ISBI) 2019 conference.
# Build Requirements 

- CMake: 2.8.12+
- C++ compiler: tested on GCC 4.9.2 and 7.4.0; MSVC 2015 on Windows
- ITK

# Runtime Requirements

- c2d from the c3d package: download [here](http://www.itksnap.org/pmwiki/pmwiki.php?n=Downloads.C3D)

# Download and Compile

Showing an example set of commands to download and compile HistoReg on a Linux machine (analogous steps apply for Windows machines):

```bash
git clone https://github.com/CBICA/HistoReg.git HistoReg
# git checkout cpp_conversion -  (Move cpp version to main ? What to do with bash version ?)
cd HistoReg
git submodule init 
git submodule update
mkdir bin
cd bin
ccmake -DCMAKE_INSTALL_PREFIX=${path_to_where_you_want_to_install} ..
make -j8
make install/strip # optional
```

# Run

```bash
./HistoReg --help : Print help message
./HistoReg -m /path/to/moving/image -f /path/to/fix/image -o /path/to/output/dir/ -c /path/to/c2d/executable [-OPTIONAL]
```

## Example

From the HistoReg directory, run :
```bash
bin/HistoReg -m Data/Images/CD68.jpg -f Data/Images/CD4.jpg -c /path/to/c2d/executable  -o Output/ -l Data/Landmarks/CD68.csv -S
```
The code will perform affine and defformable registration from the moving (or source) image CD68.jpg (-m option) to the fixed ( or target) image CD4.jpg (-f option) and will store the result in the Output/ directory (-o option). 

The -c option is to specify where the c2d executable is.

The -l and -S option are optionnal, the first one is to apply the transformation on landmarks defined in the source image, and the second one is to apply the transformation on the resampled images to have an idea if the registration succeed or failed.

After the registration completed, you should find a folder called Output/CD68_registered_to_CD4 where all the result were saved : 
- The output of the registration will be in the folder Output/CD68_registered_to_CD4/metrics where you will find the registration metrics computed at the resampled scale (small_resolution folder) and adapted to the original size of the images (full_resolution folder)
- The output of the -l command will be the file Output/CD68_registered_to_CD4/warped_landmarks.csv.
- The output of the -S command will be in the folder Output/CD68_registered_to_CD4/Saved_NIFTIs/small_resolution/, the source will be called new_small_source_padded.nii.gz, the target new_small_target_padded.nii.gz and the source registered to the target small_registeredImage.nii.gz, these images will be gray-scale images and padded after the preprocessing steps detailled in our paper (you can use the -F option to apply the transformation to the original RGB images at their original scale but the running time will be way longer) 

To check if the registration worked, you can overlay these two images : new_small_target_padded.nii.gz and small_registeredImage.nii.gz, the two images should be perfectly aligned. If you don't have a software to visualize these images, I strongly recommand to use itksnap which was used for this project (download [here](http://www.itksnap.org/pmwiki/pmwiki.php?n=Downloads.SNAP3)).

You can then compare the warped landmarks (Output/CD68_registered_to_CD4/warped_landmarks.csv) with the landmarks defined in the target space (Data/Landmarks/CD4.csv), these landmarks should be very close (from a different of a few pixels to a few tenth of pixels, which is very small compared to the original images sizes).

# References
[1] J. Borovec, A. Munoz-Barrutia, J. Kybic, "Benchmarking of Image Registration Methods for Differently Stained Histological Slides," 25th IEEE International Conference on Image Processing (ICIP), 2018. DOI: 10.1109/icip.2018.8451040

[2] R. Fernandez-Gonzalez, A. Jones, E. Garcia-Rodriguez, P.Y. Chen, A. Idica, S.J. Lockett, et al., "System for combined three-dimensional morphological and molecular analysis of thick tissue specimens," Microsc Res Tech. 59:522–530, 2002.

[3] L. Gupta, B.M. Klinkhammer, P. Boor, D. Merhof, M. Gadermayr, "Stain independent segmentation of whole slide images: A case study in renal histology," IEEE 15th International Symposium on Biomedical Imaging (ISBI), 2018. DOI: 10.1109/isbi.2018.8363824

[4] I. Mikhailov, N. Danilova, P. Malkov, "The immune microenvironment of various histological types of ebv-associated gastric cancer," Virchows Archiv. 473(s1), 2018. DOI: 10.1007/s00428-018-2422-1

[5] G. Bueno, O. Deniz, "AIDPATH: Academia and Industry Collaboration for Digital Pathology," http://aidpath.eu/?page_id=279
