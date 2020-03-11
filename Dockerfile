FROM cbica/captk_centos7:devtoolset-4_superbuild

LABEL authors="CBICA_UPenn <software@cbica.upenn.edu>"

RUN yum update -y

RUN yum install git

RUN git clone https://github.com/CBICA/HistoReg.git; \
    cd HistoReg && mkdir bin; \
    git submodule init && git submodule update

RUN cd HistoReg/bin; \
    cmake -DITK_DIR=../../CaPTk/bin/ITK-build -DCMAKE_INSTALL_PREFIX="./install/" -DBUILD_TESTING=OFF ..; \
    make && make install/strip; 
    #cd .. && ./scripts/captk-pkg

# set up the docker for GUI
ENV QT_X11_NO_MITSHM=1
ENV QT_GRAPHICSSYSTEM="native"

# define entry point
ENTRYPOINT ["/HistoReg/bin/install/bin/HistoReg"]