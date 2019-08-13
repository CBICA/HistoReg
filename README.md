# HistoReg
This repository describes a framework for automated registration of variably-stained digitized histology slices from various anatomical sites, based on a greedy diffeomorphic registration tool.

The dataset provided for the quantitative evaluation of the proposed approach was provided by the Automatic Non-rigid Histological Image Registration (ANHIR) challenge [2-6]. This challenge was a part of the IEEE International Symposium on Biomedical Imaging (ISBI) 2019 conference.
# Build Requirements 

- git
- CMake version
- C++ compiler (ex: make on linux)
- ITK

c3d package (itksnap.org/pmwiki/pmwiki.php?n=Downloads.C3D)


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
ccmake ..
make -j 8
```

# Run
./HistoReg --help : Print help

./HistoReg -m /path/to/moving/image -f /path/to/fix/image -o /path/to/output/dir/ -c if c3d not yet included : Compute registration metrics between moving and target images with the default parameters used for the ANHIR challenge, add -S or -F option to apply transformations respectively to resampled or full size images. 

# References
[1] J. Borovec, A. Munoz-Barrutia, J. Kybic, "Benchmarking of Image Registration Methods for Differently Stained Histological Slides," 25th IEEE International Conference on Image Processing (ICIP), 2018. DOI: 10.1109/icip.2018.8451040

[2] R. Fernandez-Gonzalez, A. Jones, E. Garcia-Rodriguez, P.Y. Chen, A. Idica, S.J. Lockett, et al., "System for combined three-dimensional morphological and molecular analysis of thick tissue specimens," Microsc Res Tech. 59:522–530, 2002.

[3] L. Gupta, B.M. Klinkhammer, P. Boor, D. Merhof, M. Gadermayr, "Stain independent segmentation of whole slide images: A case study in renal histology," IEEE 15th International Symposium on Biomedical Imaging (ISBI), 2018. DOI: 10.1109/isbi.2018.8363824

[4] I. Mikhailov, N. Danilova, P. Malkov, "The immune microenvironment of various histological types of ebv-associated gastric cancer," Virchows Archiv. 473(s1), 2018. DOI: 10.1007/s00428-018-2422-1

[5] G. Bueno, O. Deniz, "AIDPATH: Academia and Industry Collaboration for Digital Pathology," http://aidpath.eu/?page_id=279
