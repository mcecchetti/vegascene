
Build steps - Linux:

cd [vegascene root folder]
mkdir build
cd build
qmake -r -spec linux-g++ ../vegascene/vegascene.pro
make

Build steps - Mac:

brew install qt5
cd [vegascene root folder]
mkdir build
cd build
/usr/local/opt/qt5/bin/qmake -r ../vegascene/vegascene.pro
make

then you can run ./vegascene spec_file [out_file]

If you want a debug build add the CONFIG+=debug option to the qmake command.
Remember that vegascene executable looks for JavaScript modules in "../jsmodules", where the path is relative to the current working directory.


